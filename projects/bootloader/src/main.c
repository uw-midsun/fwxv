#include <stdio.h>

#include "boot_can.h"
#include "bootloader.h"
#include "can_board_ids.h"
#include "can_datagram.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_it.h"

#define BOOTLOADER_TIMEOUT_MS        5000

volatile uint32_t bootloader_timer = 0;

const Boot_CanSettings can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_BOOTLOADER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .loopback = false,
};

void SysTick_Handler(void) {
  bootloader_timer++;
}

Boot_CanMessage msg = { 0 };

int main() {
  /* Set the vector table offset at flash base */
  SCB->VTOR = FLASH_BASE;
  boot_can_init(&can_settings);
  bootloader_init();
  if(SysTick_Config(SystemCoreClock / 1000)) {
    send_ack_datagram(0, BOOTLOADER_INTERNAL_ERR);
    while(true); // Hang
  }

  while (true) {
    if (boot_can_receive(&msg) == BOOTLOADER_ERROR_NONE) {
      bootloader_timer = 0;
      bootloader_run(&msg);
    }
    if (bootloader_timer > BOOTLOADER_TIMEOUT_MS) {
        bootloader_timer = 0;
        if (boot_verify_flash_memory() == BOOTLOADER_ERROR_NONE) {
          send_ack_datagram(NACK, BOOTLOADER_TIMEOUT);
          bootloader_jump_app();
        }
        send_ack_datagram(NACK, BOOTLOADER_FLASH_MEMORY_VERIFY_FAILED);
    }
  }

  return 0;
}
