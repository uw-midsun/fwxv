#include <stdio.h>

#include "boot_can.h"
#include "can_board_ids.h"
#include "bootloader.h"
#include "can_datagram.h"
#include "stm32f10x_flash.h"

const Boot_CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_BOOTLOADER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .loopback = false,
};

Boot_CanMessage msg = { 0 };

int main() {
  volatile uint32_t can_byte_counter = 0;
  boot_can_init(&can_settings);
  bootloader_init();

  while (true) {
    // bootloader_get_state_data();
    if (boot_can_receive(&msg) == BOOTLOADER_ERROR_NONE) {
      can_byte_counter += 8;
      bootloader_run(&msg);
    }

    // Listen for messages with bootloader+start ID (add to can.c)
    // Check which naode it is for
    // Capture data
    // Erase memory and flash
  }

  return 0;
}
