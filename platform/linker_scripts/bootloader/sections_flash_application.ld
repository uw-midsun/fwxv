/*
Linker subscript for section definitions for the bootloaded application.
See https://uwmidsun.atlassian.net/l/c/mHxmqjvn for more information.
*/

SECTIONS
{
  /* The vector table stored at .isr_vector must end up at the beginning of the application flash.
  This isn't at address 0, but the bootloader will set the vector table here when jumping to the
  application's reset handler. */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))            /* Startup code */
    . = ALIGN(4);
  } >APPLICATION

  /* application program code is stored in the .text section, which goes to application flash */
  .text :
  {
    . = ALIGN(4);
    *(.hardfault)              /* Hardfault handlers */
    *(.text)                   /* normal code */
    *(.text.*)                 /* -ffunction-sections code */
    *(.rodata)                 /* read-only data (constants) */
    *(.rodata*)                /* -fdata-sections read only data */
    *(.glue_7)                 /* TBD - needed ? */
    *(.glue_7t)                /* TBD - needed ? */

    /* Necessary KEEP sections (see http://sourceware.org/ml/newlib/2005/msg00255.html) */
    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;
    /* This is used by the startup in order to initialize the .data section */
    _sidata = _etext;
  } >APPLICATION

  /* Dummy section to provide an offset for the vector table at the start of RAM.
  The bootloader relocates the vector table to the start of RAM, so the .data section needs to start
  after the vector table. This phony section increments the RAM counter to accomplish this. */
  .data_phony : AT ( _sidata )
  {
    . += _vector_table_size;
  } >RAM

  /* This is the initialized data section
  The program executes knowing that the data is in the RAM
  but the loader puts the initial values in the APPLICATION flash.
  It is one task of the startup to copy the initial values from flash to RAM. */
  .data : AT ( _sidata )
  {
    . = ALIGN(4);
    /* This is used by the startup in order to initialize the .data secion */
    _sdata = .;
    _data = .;

    *(.data)
    *(.data.*)
    *(.RAMtext)

    . = ALIGN(4);
    /* This is used by the startup in order to initialize the .data secion */
    _edata = .;
  } >RAM

  /* This is the uninitialized data section */
  .bss :
  {
    . = ALIGN(4);
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;
    __bss_start__ = .;
    _bss = .;
    *(.bss)
    *(.bss.*) /* patched by elias - allows the use of -fdata-sections */
    *(COMMON)

    . = ALIGN(4);
    /* This is used by the startup in order to initialize the .bss secion */
    _ebss = . ;
    __bss_end__ = .;
  } >RAM

  .eh_frame :
  {
    . = ALIGN(4);
    KEEP (*(.eh_frame))
  } >RAM

  PROVIDE ( end = _ebss );
  PROVIDE ( _end = _ebss );

  __exidx_start = .;
  __exidx_end = .;

  /* after that it's only debugging information. */

  /* remove the debugging information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  /* DWARF debug sections.
    Symbols in the DWARF debugging sections are relative to the beginning
    of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
}