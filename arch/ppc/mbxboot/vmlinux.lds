OUTPUT_ARCH(powerpc)
SEARCH_DIR(/lib); SEARCH_DIR(/usr/lib); SEARCH_DIR(/usr/local/lib); SEARCH_DIR(/usr/local/powerpc-any-elf/lib);
/* Do we need any of these for elf?
   __DYNAMIC = 0;    */
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  . = + SIZEOF_HEADERS;
  .interp : { *(.interp) }
  .hash          : { *(.hash)		}
  .dynsym        : { *(.dynsym)		}
  .dynstr        : { *(.dynstr)		}
  .rel.text      : { *(.rel.text)		}
  .rela.text     : { *(.rela.text) 	}
  .rel.data      : { *(.rel.data)		}
  .rela.data     : { *(.rela.data) 	}
  .rel.rodata    : { *(.rel.rodata) 	}
  .rela.rodata   : { *(.rela.rodata) 	}
  .rel.got       : { *(.rel.got)		}
  .rela.got      : { *(.rela.got)		}
  .rel.ctors     : { *(.rel.ctors)	}
  .rela.ctors    : { *(.rela.ctors)	}
  .rel.dtors     : { *(.rel.dtors)	}
  .rela.dtors    : { *(.rela.dtors)	}
  .rel.bss       : { *(.rel.bss)		}
  .rela.bss      : { *(.rela.bss)		}
  .rel.plt       : { *(.rel.plt)		}
  .rela.plt      : { *(.rela.plt)		}
/*  .init          : { *(.init)	} =0*/
  .plt : { *(.plt) }
  .text      :
  {
    *(.text)
    *(.fixup)
    *(.got1)
  }
  _etext = .;
  PROVIDE (etext = .);
  .rodata    :
  {
    *(.rodata)
    *(.rodata1)
  }
  .fini      : { *(.fini)    } =0
  .ctors     : { *(.ctors)   }
  .dtors     : { *(.dtors)   }
  /* Read-write section, merged into data segment: */
  . = (. + 0x0FFF) & 0xFFFFF000;
  .data    :
  {
    *(.data)
    *(.data1)
    *(.sdata)
    *(.sdata2)
    *(.got.plt) *(.got)
    *(.dynamic)
    CONSTRUCTORS
  }
  _edata  =  .;
  PROVIDE (edata = .);

  .fixup   : { *(.fixup) }
  __start___ex_table = .;
  __ex_table : { *(__ex_table) }
  __stop___ex_table = .;

  . = ALIGN(32);
  .data.cacheline_aligned : { *(.data.cacheline_aligned) }

  . = ALIGN(4096);
  __init_begin = .;
  .text.init : { *(.text.init) }
  .data.init : { 
    *(.data.init);
    __vtop_table_begin = .;
    *(.vtop_fixup);
    __vtop_table_end = .;
    __ptov_table_begin = .;
    *(.ptov_fixup);
    __ptov_table_end = .;
  }
  . = ALIGN(16);
  __setup_start = .;
  .setup.init : { *(.setup.init) }
  __setup_end = .;
  __initcall_start = .;
  .initcall.init : { *(.initcall.init) }
  __initcall_end = .;
  . = ALIGN(4096);
  __init_end = .;

  . = ALIGN(4096);
  __pmac_begin = .;
  .text.pmac : { *(.text.pmac) }
  .data.pmac : { *(.data.pmac) }
  . = ALIGN(4096);
  __pmac_end = .;

  . = ALIGN(4096);
  __prep_begin = .;
  .text.prep : { *(.text.prep) }
  .data.prep : { *(.data.prep) }
  . = ALIGN(4096);
  __prep_end = .;

  . = ALIGN(4096);
  __apus_begin = .;
  .text.apus : { *(.text.apus) }
  .data.apus : { *(.data.apus) }
  . = ALIGN(4096);
  __apus_end = .;

  . = ALIGN(4096);
  __apus_begin = .;
  .text.apus : { *(.text.apus) }
  .data.apus : { *(.data.apus) }
  . = ALIGN(4096);
  __apus_end = .;

  . = ALIGN(4096);
  __openfirmware_begin = .;
  .text.openfirmware : { *(.text.openfirmware) }
  .data.openfirmware : { *(.data.openfirmware) }
  . = ALIGN(4096);
  __openfirmware_end = .;

  __bss_start = .;
  .bss       :
  {
   *(.sbss) *(.scommon)
   *(.dynbss)
   *(.bss)
   *(COMMON)
  }
  _end = . ;
  PROVIDE (end = .);

  /*
   * For loader only: Put the zImage after everything else
   */
  _gzstart = . ;
  .gzimage : { *(.gzimage) }
  _gzend   = . ;
  
  /*
   * For loader only: Put the initrd after zImage
   */
  _rdstart = . ;
  .rdimage : { *(.rdimage) }
  _rdend   = . ;
  
}
