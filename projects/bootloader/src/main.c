#include <stdio.h>

#include "boot_can.h"
#include "can_board_ids.h"
#include "bootloader.h"
#include "can_datagram.h"

const Boot_CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_BOOTLOADER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .loopback = false,
};

Boot_CanMessage msg = { 0 };

int main() {
  // boot_can_init(&can_settings);

  // bootloader_init();
  bool extended = false;

  __asm volatile (
        // Load the address of the stack pointer (0x08002000) into r0
        "ldr r0, =0x08002000\n"
        
        // Load the value from the address (which is the initial stack pointer) into r1
        "ldr r1, [r0]\n"
        "msr msp, r1\n"
        
        // Load the address of the Reset_Handler (which is at 0x08002000 + 4) into r2
        "ldr r2, [r0, #4]\n"
        "bx r2\n"
        :
        :
        : "r0", "r1", "r2"
    );
  
  while (true) {
    // if (boot_can_receive(&msg) == BOOTLOADER_ERROR_NONE) {
    //   bootloader_run(&msg);
    // }
    // Listen for messages with bootloader+start ID (add to can.c)
    // Check which node it is for
    // Capture data
    // Erase memory and flash
  }

  return 0;
}
