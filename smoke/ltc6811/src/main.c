#include <stdio.h>

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "ltc_afe.h"
#include "ltc_afe_impl.h"
#include "tasks.h"

LtcAfeStorage s_ltc_store;

#define TEMP_RESISTANCE 10000
#define VREF2 30000
#define TABLE_SIZE 125

typedef enum ThermistorMap {
  THERMISTOR_2 = 0,
  THERMISTOR_1,
  THERMISTOR_0,
  THERMISTOR_3,
  THERMISTOR_4,
  THERMISTOR_7,
  THERMISTOR_5,
  THERMISTOR_6,
  NUM_THERMISTORS
} ThermistorMap;

static uint8_t s_thermistor_map[NUM_THERMISTORS] = {
  [0] = THERMISTOR_0, [1] = THERMISTOR_1, [2] = THERMISTOR_2, [3] = THERMISTOR_3,
  [4] = THERMISTOR_4, [5] = THERMISTOR_5, [6] = THERMISTOR_6, [7] = THERMISTOR_7
};

static const uint16_t s_resistance_lookup[TABLE_SIZE] = {
  27219, 26076, 24988, 23951, 22963, 22021, 21123, 20267, 19450, 18670, 17925, 17214, 16534, 15886,
  15266, 14674, 14173, 13718, 13256, 12805, 12394, 12081, 11628, 11195, 10780, 10000, 9634,  9283,
  8947,  8624,  8314,  8018,  7733,  7460,  7199,  6947,  6706,  6475,  6252,  6039,  5834,  5636,
  5445,  5262,  5093,  4927,  4763,  4601,  4446,  4300,  4161,  4026,  3896,  3771,  3651,  3535,
  3423,  3315,  3211,  3111,  3014,  2922,  2833,  2748,  2665,  2586,  2509,  2435,  2364,  2294,
  2227,  2162,  2101,  2040,  1981,  1925,  1868,  1817,  1765,  1716,  1668,  1622,  1577,  1533,
  1490,  1449,  1410,  1371,  1334,  1298,  1263,  1229,  1197,  1164,  1134,  1107,  1078,  1052,
  1025,  999,   973,   949,   925,   902,   880,   858,   837,   816,   796,   777,   758,   739,
  721,   704,   687,   671,   655,   640,   625,   610,   596,   582,   569,   556,   543
};

int calculate_temperature(uint16_t thermistor) {
  // INCOMPLETE
  uint16_t thermistor_resistance = (thermistor * TEMP_RESISTANCE) / (VREF2 - thermistor);
  uint16_t min_diff = abs(thermistor_resistance - s_resistance_lookup[0]);

  for (int i = 1; i < TABLE_SIZE; ++i) {
    if (abs(thermistor_resistance - s_resistance_lookup[i]) < min_diff) {
      min_diff = abs(thermistor_resistance - s_resistance_lookup[i]);
      thermistor = i;
    }
  }
  return thermistor;
}

LtcAfeSettings s_afe_settings = {
  // Settings pending hardware validation
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },

  .spi_port = SPI_PORT_2,
  .spi_baudrate = 750000,

  .adc_mode = LTC_AFE_ADC_MODE_7KHZ,

  .cell_bitset = { 0xFFF, 0xFFF, 0xFFF },
  .aux_bitset = { 0xFF, 0xFF, 0xFF },

  .num_devices = 1,
  .num_cells = 12,
  .num_thermistors = 6,
};

TASK(smoke_ltc, TASK_STACK_1024) {
  ltc_afe_init(&s_ltc_store, &s_afe_settings);
  delay_ms(10);
  while (1) {
    // Trigger conversions
    ltc_afe_impl_trigger_cell_conv(&s_ltc_store);

    // Wait for conversion to complete
    delay_ms(10);

    // Read converted cell values
    ltc_afe_impl_read_cells(&s_ltc_store);

    // Log cell values
    for (size_t cell = 0; cell < (s_afe_settings.num_devices * s_afe_settings.num_cells); cell++) {
      LOG_DEBUG("CELL %d: %d\n\r", cell,
                s_ltc_store.cell_voltages[s_ltc_store.cell_result_lookup[cell]]);
      delay_ms(3);
    }

    // Get min cell reading
    uint16_t min_cell = UINT16_MAX;
    for (size_t cell = 0; cell < (s_afe_settings.num_devices * s_afe_settings.num_cells); cell++) {
      if (s_ltc_store.cell_voltages[s_ltc_store.cell_result_lookup[cell]] < min_cell) {
        min_cell = s_ltc_store.cell_voltages[s_ltc_store.cell_result_lookup[cell]];
      }
    }
    LOG_DEBUG("Min cell voltage: %d\n", min_cell);
    delay_ms(1);

    // Every cell that is 10mV or greater above (unbalanced) the minimum voltage cell should be
    // discharged
    min_cell += 100;  // Add 100 to set the min discharge value (units of 100uV)
    for (size_t cell = 0; cell < (s_afe_settings.num_devices * s_afe_settings.num_cells); cell++) {
      if (s_ltc_store.cell_voltages[s_ltc_store.cell_result_lookup[cell]] > min_cell) {
        ltc_afe_impl_toggle_cell_discharge(&s_ltc_store, cell, true);
        LOG_DEBUG("Cell %d unbalanced \n", cell);
        delay_ms(1);
      } else {
        ltc_afe_impl_toggle_cell_discharge(&s_ltc_store, cell, false);
      }
    }

    // Re-write the config with the new cells to discharge
    // ltc_afe_impl_toggle_cell_discharge(&s_ltc_store, 3, true);
    LOG_DEBUG("Config discharge bitset %d\n", s_ltc_store.discharge_bitset[0]);
    delay_ms(1);
    ltc_afe_impl_write_config(&s_ltc_store);

    for (size_t i = 0; i < s_afe_settings.num_thermistors * s_afe_settings.num_devices; i += 2) {
      // Thermistor select cell 0
      ltc_afe_impl_trigger_aux_conv(&s_ltc_store, s_thermistor_map[i]);
      // DELAY NEEDED for adc conv to happen
      delay_ms(1);
      // Thermistor read cell 0
      ltc_afe_impl_read_aux(&s_ltc_store, i);

      // Log thermistor result
      s_ltc_store.aux_result_lookup[i] =
          calculate_temperature(s_ltc_store.aux_voltages[s_ltc_store.aux_result_lookup[i]]);
      LOG_DEBUG("Thermistor reading: %d\n", s_ltc_store.aux_result_lookup[i]);
    }

    Delay until next cycle
    Discharging should happen here
    LOG_DEBUG("\n\n\r");
    delay_ms(500);
  }
}

int main() {
  gpio_init();
  interrupt_init();
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  tasks_init_task(smoke_ltc, 1, NULL);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
