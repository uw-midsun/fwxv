/*
Default linker script for STM32F103CBT6 128k flash 20k ram
Used when building non-bootloader projects normally.
*/

/* include the memory spaces definitions sub-script */
INCLUDE "default/stm32f1discovery_def.ld"

/* include the sections management sub-script for FLASH mode */
INCLUDE "default/sections_flash.ld"
