OUTPUT_FORMAT("elf64-alpha")
ENTRY(__start)
SECTIONS
{
  . = 0x20000000;
  .text : { *(.text) }
  _etext = .;
  PROVIDE (etext = .);
  .rodata : { *(.rodata) }
  .data : { *(.data) CONSTRUCTORS }
  .got : { *(.got) }
  .sdata : { *(.sdata) }
  _edata = .;
  PROVIDE (edata = .);
  .sbss : { *(.sbss) *(.scommon) }
  .bss : { *(.bss) *(COMMON) }
  _end = . ;
  PROVIDE (end = .);

  .mdebug 0 : { *(.mdebug) }
  .note 0 : { *(.note) }
  .comment 0 : { *(.comment) }
}
