#include <stdint.h>
#include <stdio.h>

#include "bms.h"
#include "can.h"
#include "can_board_ids.h"
#include "current_sense.h"
#include "fault_bps.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "ltc_afe.h"
#include "ltc_afe_impl.h"
#include "master_task.h"
#include "max17261_fuel_gauge.h"
#include "relays_fsm.h"
#include "tasks.h"

#define FUEL_GAUGE_CYCLE_TIME_MS 100

static CurrentStorage s_currentsense_storage;
static LtcAfeStorage s_ltc_store;
static CanStorage s_can_storage = { 0 };

static const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_BMS_CARRIER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

static const LtcAfeSettings s_afe_settings = {
  // Settings pending hardware validation
  .mosi = { .port = GPIO_PORT_B, .pin = 15 },
  .miso = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },

  .spi_port = SPI_PORT_2,
  .spi_baudrate = 750000,

  .adc_mode = LTC_AFE_ADC_MODE_7KHZ,

  .cell_bitset = { 0xFF },
  .aux_bitset = { 0 },

  .num_devices = 1,
  .num_cells = 12,
  .num_thermistors = 12,
};

static const I2CSettings i2c_settings = {
  .speed = I2C_SPEED_STANDARD,
  .sda = BMS_PERIPH_I2C_SDA_PIN,
  .scl = BMS_PERIPH_I2C_SCL_PIN,
};

void pre_loop_init() {
  ltc_afe_init(&s_ltc_store, &s_afe_settings);
  current_sense_init(&s_currentsense_storage, &i2c_settings, FUEL_GAUGE_CYCLE_TIME_MS);
  LOG_DEBUG("Welcome to BMS \n");
}

void run_fast_cycle() {
  run_current_sense_cycle();
  wait_tasks(1);

  ltc_afe_impl_trigger_cell_conv(&s_ltc_store);
  delay_ms(10);
  ltc_afe_impl_read_cells(&s_ltc_store);
  for (int cell = 0; cell < 12; cell++) {
    LOG_DEBUG("CELL %d: %d\n\r", cell,
              s_ltc_store.cell_voltages[s_ltc_store.cell_result_lookup[cell]]);
    // delay_ms(1);
  }

  fsm_run_cycle(relays);
  wait_tasks(1);
}

void run_medium_cycle() {
  run_can_rx_cycle();
  wait_tasks(1);

  run_can_tx_cycle();
  wait_tasks(1);
}

void run_slow_cycle() {}

int main() {
  tasks_init();
  log_init();
  gpio_init();
  can_init(&s_can_storage, &can_settings);
  i2c_init(BMS_PERIPH_I2C_PORT, &i2c_settings);

  init_relays();

  LOG_DEBUG("Welcome to BMS!\n");
  init_master_task();

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
