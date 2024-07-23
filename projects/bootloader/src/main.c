#include <stdio.h>

#include "can.h"
#include "can_board_ids.h"
#include "bootloader.h"
#include "can_datagram.h"

static CanStorage s_can_storage = { 0 };

const CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_BOOTLOADER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

CanMessage msg = { 0 };

int main() {
  can_init(&s_can_storage, &can_settings);

  bootloader_init();
  bool extended = false;
  while (true) {
    if (can_hw_receive(&msg.id.raw, &extended, &msg.data, &msg.dlc)) {
      bootloader_run(&msg);
    }
    // Listen for messages with bootloader+start ID (add to can.c)
    // Check which node it is for
    // Capture data
    // Erase memory and flash
  }

  return 0;
}
