#include <linux/linkage.h>

#include <asm/page.h>
#include <asm/contregs.h>
#include <asm/sun3-head.h>

PSL_HIGHIPL     = 0x2700
NBSG            = 0x20000
ICACHE_ONLY	= 0x00000009
CACHES_OFF	= 0x00000008	| actually a clear and disable --m
#define MAS_STACK INT_STACK 
ROOT_TABLE_SIZE = 128
PAGESIZE	= 8192
SUN3_INVALID_PMEG = 255
.globl SYMBOL_NAME(bootup_user_stack)
.globl SYMBOL_NAME(bootup_kernel_stack)
.globl SYMBOL_NAME(pg0)
.globl SYMBOL_NAME(empty_bad_page)
.globl SYMBOL_NAME(empty_bad_page_table)
.globl SYMBOL_NAME(empty_zero_page)
.globl SYMBOL_NAME(swapper_pg_dir)
.globl SYMBOL_NAME(kernel_pmd_table)
.globl SYMBOL_NAME(availmem)
.global SYMBOL_NAME(m68k_pgtable_cachemode)
.global SYMBOL_NAME(kpt)
| todo: all these should be in bss!
SYMBOL_NAME(swapper_pg_dir):                .skip 0x2000
SYMBOL_NAME(pg0):                           .skip 0x2000
SYMBOL_NAME(empty_bad_page):                .skip 0x2000
SYMBOL_NAME(empty_bad_page_table):          .skip 0x2000
SYMBOL_NAME(kernel_pmd_table):              .skip 0x2000
SYMBOL_NAME(empty_zero_page):               .skip 0x2000

.globl SYMBOL_NAME(kernel_pg_dir)
.equ    SYMBOL_NAME(kernel_pg_dir),SYMBOL_NAME(kernel_pmd_table)

	.section .head
ENTRY(_stext)
ENTRY(_start)

/* Firstly, disable interrupts and set up function codes. */
	movew	#PSL_HIGHIPL, %sr
	moveq	#FC_CONTROL, %d0	
	movec	%d0, %sfc	
	movec	%d0, %dfc	

/* Make sure we're in context zero. */
	moveq	#0, %d0
	movsb	%d0, AC_CONTEXT

/* map everything the bootloader left us into high memory, clean up the
   excess later */
	lea	(AC_SEGMAP+0),%a0 
	lea	(AC_SEGMAP+KERNBASE),%a1	
1:	
	movsb	%a0@, %d1
	movsb	%d1, %a1@
	cmpib	#SUN3_INVALID_PMEG, %d1
	beq	2f
	addl	#NBSG,%a0
	addl	#NBSG,%a1
	jmp	1b
	
2:				
	
/* Disable caches and jump to high code. */
	moveq	#ICACHE_ONLY,%d0	| Cache disabled until we're ready to enable it
	movc	%d0, %cacr	|   is this the right value? (yes --m)
	jmp	1f:l		

/* Following code executes at high addresses (0xE000xxx). */
1:	lea	SYMBOL_NAME(init_task_union),%a2	| get initial thread...
	lea	%a2@(KTHREAD_SIZE),%sp			| ...and its stack.

/* copy bootinfo records from the loader to _end */
	lea	SYMBOL_NAME(_end), %a1
	lea	BI_START, %a0
	/* number of longs to copy */
	movel	%a0@, %d0
1:	addl	#4, %a0
	movel   %a0@, %a1@
	addl	#4, %a1
	dbf	%d0, 1b
	
/* Point MSP at an invalid page to trap if it's used. --m */
	movl	#(PAGESIZE),%d0
	movc	%d0,%msp
	moveq	#-1,%d0
	movsb	%d0,(AC_SEGMAP+0x0)

	jbsr	SYMBOL_NAME(sun3_init)

	jbsr	SYMBOL_NAME(base_trap_init)
			
        jbsr    SYMBOL_NAME(start_kernel)
	trap	#15			

        .data
        .even
SYMBOL_NAME_LABEL(kpt)
        .long 0
SYMBOL_NAME_LABEL(availmem)
        .long 0
| todo: remove next two. --m
SYMBOL_NAME_LABEL(is_medusa)
        .long 0
SYMBOL_NAME_LABEL(m68k_pgtable_cachemode)
        .long 0

