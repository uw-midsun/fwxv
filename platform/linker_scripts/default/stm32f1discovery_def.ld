/*
Linker subscript for STM32F103C definitions with 64K Flash and 20K RAM
*/

_flash_start = 0x08000000;
_flash_size = 64K;
_flash_page_size = 1K;

_ram_start = 0x20000000;
_ram_size = 20K;

_application_start = _flash_start;

_bootloader_size = 0;
_application_size = _flash_size;

_vector_table_size = 0xEC; /* used for relocating the vector table to RAM for the application */

/* Memory Spaces Definitions */

ENTRY(Reset_Handler)

/* TODO: explicitly set aside the last page for calib */

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 64K
  RAM  (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* highest address of the user mode stack */
_estack = 0x20004000;
_flash_start = ORIGIN(FLASH);
_flash_size = LENGTH(FLASH);
_flash_page_size = 0x800;
