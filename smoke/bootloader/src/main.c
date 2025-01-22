#include <stdio.h>
#include <string.h>

#include "boot_crc32.h"
#include "boot_flash.h"
#include "can_board_ids.h"
#include "delay.h"
#include "log.h"
#include "tasks.h"

const Boot_CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_BOOTLOADER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .loopback = false,
};

Boot_CanMessage msg = { 0 };

TASK(read_write, TASK_STACK_512) {
  boot_crc_init();
  uint8_t flash_buffer[1024];
  const uint8_t test_data[] = "Test data for CRC32";
  size_t test_data_len = sizeof(test_data);
  boot_align_to_32bit_words(test_data, &test_data_len);
  uint32_t crc32_value =
      boot_crc_calculate((const uint32_t *)test_data, BYTES_TO_WORD(test_data_len));

  extern uint32_t _bootloader_size;
  extern uint32_t _flash_start;
  while (true) {
    LOG_DEBUG("CRC32: %lu\n", crc32_value);
    LOG_DEBUG("FLASH START %lX\n", (uint32_t)(&_flash_start));
    LOG_DEBUG("BOOTLOADER SIZE %lX\n", (uint32_t)(&_bootloader_size));
    boot_flash_erase(BOOTLOADER_ADDR_TO_PAGE(0x8009000));
    delay_ms(10);

    for (uint8_t i = 0; i < 240; i++) {
      flash_buffer[i] = i;
    }

    // LOG_DEBUG("ADDRESS: %X\n", BOOTLOADER_PAGE_TO_ADDR(12));
    boot_flash_write(0x8009000, flash_buffer, 1024);
    delay_ms(10);
    boot_flash_read(0x8009000, flash_buffer, 1024);
    for (uint8_t i = 0; i < 240; i++) {
      // LOG_DEBUG("FLASH BUFFER %d\n", flash_buffer[i
      if (flash_buffer[i] != i) {
        // Hang
        while (true) {
        }
      }
      delay_ms(5);
    }

    memset(flash_buffer, 0, sizeof(flash_buffer));
    delay_ms(1000);
  }
}

int main() {
  tasks_init();
  log_init();
  LOG_DEBUG("Welcome to TEST!");

  tasks_init_task(read_write, 2, NULL);
  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
