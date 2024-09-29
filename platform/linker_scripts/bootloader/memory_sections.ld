ENTRY(Reset_Handler)

_bootloader_size = 11K;

_flash_start = 0x08000000;
_flash_size = 64K;
_flash_page_size = 1K;

_ram_start = 0x20000000;
_ram_size = 20K;

_bootloader_start = _flash_start;
_application_start = _bootloader_start + _bootloader_size;

_application_size = _flash_size - (_application_start - _flash_start);

_vector_table_size = 0xEC; /* used for relocating the vector table to RAM for the application */

/* TODO: explicitly set aside the last page for calib */

MEMORY
{
  BOOTLOADER  (rx) : ORIGIN = _flash_start, LENGTH = _bootloader_size
  APPLICATION (rx) : ORIGIN = _application_start, LENGTH = _application_size
  RAM        (rwx) : ORIGIN = _ram_start, LENGTH = _ram_size
}

/* highest address of the user mode stack */
_estack = _ram_start + _ram_size;