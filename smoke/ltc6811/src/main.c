#include <stdio.h>

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "ltc_afe.h"
#include "ltc_afe_impl.h"
#include "tasks.h"

LtcAfeStorage s_ltc_store;

LtcAfeSettings s_afe_settings = {
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

TASK(smoke_ltc, TASK_STACK_1024) {
  ltc_afe_init(&s_ltc_store, &s_afe_settings);
  delay_ms(10);
  while (1) {
    ltc_afe_impl_trigger_cell_conv(&s_ltc_store);
    delay_ms(10);
    ltc_afe_impl_read_cells(&s_ltc_store);
    for (int cell = 0; cell < 12; cell++) {
      LOG_DEBUG("CELL %d: %d\n\r", cell,
                s_ltc_store.cell_voltages[s_ltc_store.cell_result_lookup[cell]]);
      delay_ms(1);
    }
    LOG_DEBUG("\n\n\r");
    delay_ms(5000);
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
