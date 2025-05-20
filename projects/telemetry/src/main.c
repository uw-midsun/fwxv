#include "can.h"
#include "can_board_ids.h"
#include "datagram.h"
#include "gpio.h"
#include "log.h"
#include "master_task.h"
#include "tasks.h"
#include "telemetry.h"
#include "uart.h"

TelemetryStorage telemetry_storage;

TelemetryConfig telemetry_config = { .message_transmit_frequency_hz = 1000U,
                                     .uart_port = UART_PORT_2,
                                     .uart_settings = { .tx = { .port = GPIO_PORT_A, .pin = 2 },
                                                        .rx = { .port = GPIO_PORT_A, .pin = 3 },
                                                        .baudrate = 115200 } };

int main() {
  gpio_init();
  tasks_init();

  telemetry_init(&telemetry_storage, &telemetry_config);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
