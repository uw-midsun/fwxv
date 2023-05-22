#include "can.h"
#include "can_debug.h"
#include "log.h"
#include "tasks.h"

#define FIRST_BYTE_HEX_MASK 0xFF
#define LAST_BYTE_HEX_MASK 0xFF00000000000000
#define LAST_BYTE_BIT_SHIFT 56

static CanStorage s_can_storage = { 0 };
const CanSettings can_settings = {
  .device_id = 0x1,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
  .mode = CAN_ONE_SHOT_MODE,
};

uint8_t extract_first_byte(uint64_t data) {
  return data & FIRST_BYTE_HEX_MASK;
}

uint8_t extract_last_byte(uint64_t data) {
  return (data & LAST_BYTE_HEX_MASK) >> LAST_BYTE_BIT_SHIFT;
}

void hello_world(uint64_t data) {
  LOG_DEBUG("Hello World!\n");
}

void add(uint64_t data) {
  uint16_t sum = extract_first_byte(data) + extract_last_byte(data);
  LOG_DEBUG("The sum of the first and last byte is %X\n", sum);
}

void subtract(uint64_t data) {
  uint16_t diff = extract_first_byte(data) - extract_last_byte(data);
  LOG_DEBUG("The difference between the first and last byte is %X\n", diff);
}

void multiply(uint64_t data) {
  uint16_t product = extract_first_byte(data) * extract_last_byte(data);
  LOG_DEBUG("The product of the first and last byte is %X\n", product);
}

void divide(uint64_t data) {
  uint16_t quotient = extract_first_byte(data) / extract_last_byte(data);
  LOG_DEBUG("The quotient of the first and last byte is %X\n", quotient);
}

TASK(master_task, TASK_MIN_STACK_SIZE) {
  while (true) {
    check_can_messages();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main() {
  tasks_init();
  log_init();

  LOG_DEBUG("Welcome to CAN Debug!\n");
  can_init(&s_can_storage, &can_settings);

  CanMessageId hello_world_id = 0x00E;
  CanMessageId add_id = 0x01E;
  CanMessageId subtract_id = 0x02E;
  CanMessageId multiply_id = 0x03E;
  CanMessageId divide_id = 0x04E;

  can_debug_register(hello_world_id, hello_world);
  can_debug_register(add_id, add);
  can_debug_register(subtract_id, subtract);
  can_debug_register(multiply_id, multiply);
  can_debug_register(divide_id, divide);

  tasks_init_task(master_task, TASK_PRIORITY(2), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?\n");
  return 0;
}
