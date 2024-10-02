#include <stdio.h>

#include "boot_can.h"
#include "bootloader.h"
#include "can_board_ids.h"
#include "can_datagram.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_tim.h"

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
  SysTick_Config(SystemCoreClock / 1000);

  boot_can_init(&can_settings);
  bootloader_init();

  while (true) {
    if (boot_can_receive(&msg) == BOOTLOADER_ERROR_NONE) {
      bootloader_timer = 0;
      bootloader_run(&msg);
    }
  }

  if (bootloader_timer > BOOTLOADER_TIMEOUT_MS) {
      bootloader_timer = 0;
      if (boot_verify_flash_memory()) bootloader_jump_app();
  }
  return 0;
}
