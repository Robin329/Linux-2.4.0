
OUTPUT_FORMAT("elf64-ia64-little")
OUTPUT_ARCH(ia64)
ENTRY(_start)
SECTIONS
{
  v = 0x0000000000000000 ;	/* this symbol is here to make debugging with kdb easier... */

  . = (0x000000000000000  + 0x100000) ;

  _text = .;
  .text : AT(ADDR(.text) - 0x0000000000000000 )
    {
	*(__ivt_section)
	/* these are not really text pages, but the zero page needs to be in a fixed location: */
	*(__special_page_section)
	__start_gate_section = .;
	*(__gate_section)
	__stop_gate_section = .;
	*(.text)
    }

  /* Global data */
  _data = .;

  .rodata : AT(ADDR(.rodata) - 0x0000000000000000 )
	{ *(.rodata) }
  .opd : AT(ADDR(.opd) - 0x0000000000000000 )
	{ *(.opd) }
  .data : AT(ADDR(.data) - 0x0000000000000000 )
	{ *(.data) *(.gnu.linkonce.d*) CONSTRUCTORS }

  __gp = ALIGN (8) + 0x200000;

  .got : AT(ADDR(.got) - 0x0000000000000000 )
	{ *(.got.plt) *(.got) }
  /* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */
  .sdata : AT(ADDR(.sdata) - 0x0000000000000000 )
	{ *(.sdata) }
  _edata  =  .;
  _bss = .;
  .sbss : AT(ADDR(.sbss) - 0x0000000000000000 )
	{ *(.sbss) *(.scommon) }
  .bss : AT(ADDR(.bss) - 0x0000000000000000 )
	{ *(.bss) *(COMMON) }
  . = ALIGN(64 / 8);
  _end = .;

  /* Sections to be discarded */
  /DISCARD/ : {
	*(.text.exit)
	*(.data.exit)
	}

  /* Stabs debugging sections.  */
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .stab.excl 0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
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
  .debug_info     0 : { *(.debug_info) }
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
  /* These must appear regardless of  .  */
  /* Discard them for now since Intel SoftSDV cannot handle them.
  .comment 0 : { *(.comment) }
  .note 0 : { *(.note) }
  */
  /DISCARD/ : { *(.comment) }
  /DISCARD/ : { *(.note) }
}
