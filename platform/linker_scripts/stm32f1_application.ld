/*
This is the linker script for the bootloaded application code on STM32F103CBT6.
*/

/* include the common memory spaces definitions subscript */
INCLUDE "bootloader/memory_sections.ld"

/* include the application-specific sections management subscript */
INCLUDE "bootloader/sections_flash_application.ld"