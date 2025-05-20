/*
This is the linker script for the bootloader on STM32F103CBT6.
*/

/* include the common memory spaces definitions subscript */
INCLUDE "bootloader/memory_sections.ld"

/* include the bootloader-specific sections management subscript */
INCLUDE "bootloader/sections_flash_bootloader.ld"