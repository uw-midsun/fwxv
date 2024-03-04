#include <stdio.h>

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "ltc_afe.h"
#include "ltc_afe_impl.h"
#include "status.h"
#include "tasks.h"

LtcAfeStorage s_ltc_store;
StatusCode status = STATUS_CODE_OK;

LtcAfeSettings s_afe_settings = {
  // Settings pending hardware validation
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },

  .spi_port = SPI_PORT_2,
  .spi_baudrate = 750000,

  .adc_mode = LTC_AFE_ADC_MODE_7KHZ,

  .cell_bitset = { 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF },
  .aux_bitset = { 0 },

  .num_devices = 2,
  .num_cells = 12,
  .num_thermistors = 12,
};

TASK(smoke_ltc, TASK_STACK_1024) {
  ltc_afe_init(&s_ltc_store, &s_afe_settings);
  delay_ms(10);
  while (1) {
    // Trigger conversions
    status = ltc_afe_impl_trigger_cell_conv(&s_ltc_store);
    LOG_DEBUG("Status after cell_conv: %d\n", status);
    // Wait for conversion to complete
    delay_ms(10);

    // Read converted cell values
    status = ltc_afe_impl_read_cells(&s_ltc_store);
    LOG_DEBUG("Status after read_cells: %d\n", status);
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
    // ltc_afe_impl_write_config(&s_ltc_store);

    for (size_t i = 0; i < 10; i++) {
      // Thermistor select cell 0
      ltc_afe_impl_trigger_aux_conv(&s_ltc_store, i);

      // Wait
      delay_ms(10);

      // Thermistor read cell 0
      ltc_afe_impl_read_aux(&s_ltc_store, i);

      // Log thermistor result
      LOG_DEBUG("Thermistor reading: %d\n",
                s_ltc_store.aux_voltages[s_ltc_store.aux_result_lookup[i]]);
    }

    // Delay until next cycle
    // Discharging should happen here
    LOG_DEBUG("\n\n\r");
    delay_ms(10000);
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
