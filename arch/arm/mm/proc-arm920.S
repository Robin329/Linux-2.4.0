/*
 *  linux/arch/arm/mm/arm920.S: MMU functions for ARM920
 *
 *  Copyright (C) 1999,2000 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * These are the low level assembler for performing cache and TLB
 * functions on the arm920.
 */
#include <linux/linkage.h>
#include <linux/config.h>
#include <asm/assembler.h>
#include <asm/procinfo.h>
#include <asm/hardware.h>
#include "../lib/constants.h"

/*
 * This is the maximum size of an area which will be invalidated
 * using the single invalidate entry instructions.  Anything larger
 * than this, and we go for the whole cache.
 *
 * This value should be chosen such that we choose the cheapest
 * alternative.
 */
#define MAX_AREA_SIZE	16384

/*
 * the cache line size of the I and D cache
 */
#define DCACHELINESIZE	32
#define ICACHELINESIZE	32

/*
 * and the page size
 */
#define PAGESIZE	4096

	.text

/*
 * cpu_arm920_data_abort()
 *
 * obtain information about current aborted instruction
 *
 * r0 = address of aborted instruction
 *
 * Returns:
 *  r0 = address of abort
 *  r1 != 0 if writing
 *  r3 = FSR
 */
	.align	5
ENTRY(cpu_arm920_data_abort)
	ldr	r1, [r0]			@ read aborted instruction
	mrc	p15, 0, r0, c6, c0, 0		@ get FAR
	mov	r1, r1, lsr #19			@ b1 = L
	mrc	p15, 0, r3, c5, c0, 0		@ get FSR
	and	r1, r1, #2
	and	r3, r3, #255
	mov	pc, lr

/*
 * cpu_arm920_check_bugs()
 */
ENTRY(cpu_arm920_check_bugs)
	mrs	ip, cpsr
	bic	ip, ip, #F_BIT
	msr	cpsr, ip
	mov	pc, lr

/*
 * cpu_arm920_proc_init()
 */
ENTRY(cpu_arm920_proc_init)
	mov	pc, lr

/*
 * cpu_arm920_proc_fin()
 */
ENTRY(cpu_arm920_proc_fin)
	stmfd	sp!, {lr}
	mov	ip, #F_BIT | I_BIT | SVC_MODE
	msr	cpsr_c, ip
	bl	cpu_arm920_cache_clean_invalidate_all
	mrc	p15, 0, r0, c1, c0, 0		@ ctrl register
	bic	r0, r0, #0x1000			@ ...i............
	bic	r0, r0, #0x000e			@ ............wca.
	mcr	p15, 0, r0, c1, c0, 0		@ disable caches
	ldmfd	sp!, {pc}

/*
 * cpu_arm920_reset(loc)
 *
 * Perform a soft reset of the system.  Put the CPU into the
 * same state as it would be if it had been reset, and branch
 * to what would be the reset vector.
 *
 * loc: location to jump to for soft reset
 */
	.align	5
ENTRY(cpu_arm920_reset)
	mov	ip, #0
	mcr	p15, 0, ip, c7, c7, 0		@ invalidate I,D caches
	mcr	p15, 0, ip, c7, c10, 4		@ drain WB
	mcr	p15, 0, ip, c8, c7, 0		@ invalidate I & D TLBs
	mrc	p15, 0, ip, c1, c0, 0		@ ctrl register
	bic	ip, ip, #0x000f			@ ............wcam
	bic	ip, ip, #0x1100			@ ...i...s........
	mcr	p15, 0, ip, c1, c0, 0		@ ctrl register
	mov	pc, r0

/*
 * cpu_arm920_do_idle()
 */
	.align	5
ENTRY(cpu_arm920_do_idle)
#if defined(CONFIG_CPU_ARM920_CPU_IDLE)
	mcr	p15, 0, r0, c7, c0, 4		@ Wait for interrupt
#endif
	mov	pc, lr

/* ================================= CACHE ================================ */


/*
 * cpu_arm920_cache_clean_invalidate_all()
 *
 * clean and invalidate all cache lines
 *
 * Note:
 *  1. we should preserve r0 at all times
 */
	.align	5
ENTRY(cpu_arm920_cache_clean_invalidate_all)
	mov	r2, #1
cpu_arm920_cache_clean_invalidate_all_r2:
	mov	ip, #0
#ifdef CONFIG_CPU_ARM920_FORCE_WRITE_THROUGH
	mcr	p15, 0, ip, c7, c6, 0		@ invalidate D cache
#else
/*
 * 'Clean & Invalidate whole DCache'
 * Re-written to use Index Ops.
 * Uses registers r1, r3 and ip
 */
	mov	r1, #7 << 5			@ 8 segments
1:	orr	r3, r1, #63 << 26		@ 64 entries
2:	mcr	p15, 0, r3, c7, c14, 2		@ clean & invalidate D index
	subs	r3, r3, #1 << 26
	bcs	2b				@ entries 63 to 0
	subs	r1, r1, #1 << 5
	bcs	1b				@ segments 7 to 0
#endif
	teq	r2, #0
	mcrne	p15, 0, ip, c7, c5, 0		@ invalidate I cache
	mcr	p15, 0, ip, c7, c10, 4		@ drain WB
	mov	pc, lr

/*
 * cpu_arm920_cache_clean_invalidate_range(start, end, flags)
 *
 * clean and invalidate all cache lines associated with this area of memory
 *
 * start: Area start address
 * end:   Area end address
 * flags: nonzero for I cache as well
 */
	.align	5
ENTRY(cpu_arm920_cache_clean_invalidate_range)
	bic	r0, r0, #DCACHELINESIZE - 1	@ && added by PGM
	sub	r3, r1, r0
	cmp	r3, #MAX_AREA_SIZE
	bgt	cpu_arm920_cache_clean_invalidate_all_r2
1:	mcr	p15, 0, r0, c7, c14, 1		@ clean and invalidate D entry
	add	r0, r0, #DCACHELINESIZE
	mcr	p15, 0, r0, c7, c14, 1		@ clean and invalidate D entry
	add	r0, r0, #DCACHELINESIZE
	cmp	r0, r1
	blt	1b
	teq	r2, #0
	movne	r0, #0
	mcrne	p15, 0, r0, c7, c5, 0		@ invalidate I cache
	mov	pc, lr

/*
 * cpu_arm920_flush_ram_page(page)
 *
 * clean and invalidate all cache lines associated with this area of memory
 *
 * page: page to clean and invalidate
 */
	.align	5
ENTRY(cpu_arm920_flush_ram_page)
	mov	r1, #PAGESIZE
1:	mcr	p15, 0, r0, c7, c14, 1		@ clean and invalidate D entry
	add	r0, r0, #DCACHELINESIZE
	mcr	p15, 0, r0, c7, c14, 1		@ clean and invalidate D entry
	add	r0, r0, #DCACHELINESIZE
	subs	r1, r1, #2 * DCACHELINESIZE
	bne	1b
	mcr	p15, 0, r1, c7, c10, 4		@ drain WB
	mov	pc, lr

/* ================================ D-CACHE =============================== */

/*
 * cpu_arm920_dcache_invalidate_range(start, end)
 *
 * throw away all D-cached data in specified region without an obligation
 * to write them back.  Note however that we must clean the D-cached entries
 * around the boundaries if the start and/or end address are not cache
 * aligned.
 *
 * start: virtual start address
 * end:   virtual end address
 */
	.align	5
ENTRY(cpu_arm920_dcache_invalidate_range)
	tst	r0, #DCACHELINESIZE - 1
	bic	r0, r0, #DCACHELINESIZE - 1
	mcrne	p15, 0, r0, c7, c10, 1		@ clean D entry
	tst	r1, #DCACHELINESIZE - 1
	mcrne	p15, 0, r1, c7, c10, 1		@ clean D entry
1:	mcr	p15, 0, r0, c7, c6, 1		@ invalidate D entry
	add	r0, r0, #DCACHELINESIZE
	cmp	r0, r1
	blt	1b
	mov	pc, lr

/*
 * cpu_arm920_dcache_clean_range(start, end)
 *
 * For the specified virtual address range, ensure that all caches contain
 * clean data, such that peripheral accesses to the physical RAM fetch
 * correct data.
 *
 * start: virtual start address
 * end:   virtual end address
 */
	.align	5
ENTRY(cpu_arm920_dcache_clean_range)
	bic	r0, r0, #DCACHELINESIZE - 1
	sub	r1, r1, r0
	cmp	r1, #MAX_AREA_SIZE
	mov	r2, #0
	bgt	cpu_arm920_cache_clean_invalidate_all_r2

1:	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
	add	r0, r0, #DCACHELINESIZE
	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
	add	r0, r0, #DCACHELINESIZE
	subs	r1, r1, #2 * DCACHELINESIZE
	bpl	1b
	mcr	p15, 0, r2, c7, c10, 4		@ drain WB
	mov	pc, lr

/*
 * cpu_arm920_dcache_clean_page(page)
 *
 * Cleans a single page of dcache so that if we have any future aliased
 * mappings, they will be consistent at the time that they are created.
 *
 * page: virtual address of page to clean from dcache
 *
 * Note:
 *  1. we don't need to flush the write buffer in this case.
 *  2. we don't invalidate the entries since when we write the page
 *     out to disk, the entries may get reloaded into the cache.
 */
	.align	5
ENTRY(cpu_arm920_dcache_clean_page)
#ifndef CONFIG_CPU_ARM920_FORCE_WRITE_THROUGH
	mov	r1, #PAGESIZE
1:	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
	add	r0, r0, #DCACHELINESIZE
	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
	add	r0, r0, #DCACHELINESIZE
	subs	r1, r1, #2 * DCACHELINESIZE
	bne	1b
#endif
	mov	pc, lr

/*
 * cpu_arm920_dcache_clean_entry(addr)
 *
 * Clean the specified entry of any caches such that the MMU
 * translation fetches will obtain correct data.
 *
 * addr: cache-unaligned virtual address
 */
	.align	5
ENTRY(cpu_arm920_dcache_clean_entry)
#ifndef CONFIG_CPU_ARM920_FORCE_WRITE_THROUGH
	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
#endif
	mcr	p15, 0, r0, c7, c10, 4		@ drain WB
	mov	pc, lr

/* ================================ I-CACHE =============================== */

/*
 * cpu_arm920_icache_invalidate_range(start, end)
 *
 * invalidate a range of virtual addresses from the Icache
 *
 * start: virtual start address
 * end:   virtual end address
 */
	.align	5
ENTRY(cpu_arm920_icache_invalidate_range)
1:	mcr	p15, 0, r0, c7, c10, 1		@ Clean D entry
	add	r0, r0, #DCACHELINESIZE
	cmp	r0, r1
	blo	1b
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ drain WB
ENTRY(cpu_arm920_icache_invalidate_page)
	/* why no invalidate I cache --rmk */
	mov	pc, lr


/* ================================== TLB ================================= */

/*
 * cpu_arm920_tlb_invalidate_all()
 *
 * Invalidate all TLB entries
 */
	.align	5
ENTRY(cpu_arm920_tlb_invalidate_all)
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ drain WB
	mcr	p15, 0, r0, c8, c7, 0		@ invalidate I & D TLBs
	mov	pc, lr

/*
 * cpu_arm920_tlb_invalidate_range(start, end)
 *
 * invalidate TLB entries covering the specified range
 *
 * start: range start address
 * end:   range end address
 */
	.align	5
ENTRY(cpu_arm920_tlb_invalidate_range)
	mov	r3, #0
	mcr	p15, 0, r3, c7, c10, 4		@ drain WB
1:	mcr	p15, 0, r0, c8, c6, 1		@ invalidate D TLB entry
	mcr	p15, 0, r0, c8, c5, 1		@ invalidate I TLB entry
	add	r0, r0, #PAGESIZE
	cmp	r0, r1
	blt	1b
	mov	pc, lr

/*
 * cpu_arm920_tlb_invalidate_page(page, flags)
 *
 * invalidate the TLB entries for the specified page.
 *
 * page:  page to invalidate
 * flags: non-zero if we include the I TLB
 */
	.align	5
ENTRY(cpu_arm920_tlb_invalidate_page)
	mov	r3, #0
	mcr	p15, 0, r3, c7, c10, 4		@ drain WB
	teq	r1, #0
	mcr	p15, 0, r0, c8, c6, 1		@ invalidate D TLB entry
	mcrne	p15, 0, r0, c8, c5, 1		@ invalidate I TLB entry
	mov	pc, lr

/* =============================== PageTable ============================== */

/*
 * cpu_arm920_set_pgd(pgd)
 *
 * Set the translation base pointer to be as described by pgd.
 *
 * pgd: new page tables
 */
	.align	5
ENTRY(cpu_arm920_set_pgd)
	mov	ip, #0
#ifdef CONFIG_CPU_ARM920_FORCE_WRITE_THROUGH
	/* Any reason why we don't use mcr p15, 0, r0, c7, c7, 0 here? --rmk */
	mcr	p15, 0, ip, c7, c6, 0		@ invalidate D cache
#else
@ && 'Clean & Invalidate whole DCache'
@ && Re-written to use Index Ops.
@ && Uses registers r1, r3 and ip

	mov	r1, #7 << 5			@ 8 segments
1:	orr	r3, r1, #63 << 26		@ 64 entries
2:	mcr	p15, 0, r3, c7, c14, 2		@ clean & invalidate D index
	subs	r3, r3, #1 << 26
	bcs	2b				@ entries 63 to 0
	subs	r1, r1, #1 << 5
	bcs	1b				@ segments 7 to 0
#endif
	mcr	p15, 0, ip, c7, c5, 0		@ invalidate I cache
	mcr	p15, 0, ip, c7, c10, 4		@ drain WB
	mcr	p15, 0, r0, c2, c0, 0		@ load page table pointer
	mcr	p15, 0, ip, c8, c7, 0		@ invalidate I & D TLBs
	mov	pc, lr

/*
 * cpu_arm920_set_pmd(pmdp, pmd)
 *
 * Set a level 1 translation table entry, and clean it out of
 * any caches such that the MMUs can load it correctly.
 *
 * pmdp: pointer to PMD entry
 * pmd:  PMD value to store
 */
	.align	5
ENTRY(cpu_arm920_set_pmd)
#ifdef CONFIG_CPU_ARM920_FORCE_WRITE_THROUGH
	eor	r2, r1, #0x0a			@ C & Section
	tst	r2, #0x0b
	biceq	r1, r1, #4			@ clear bufferable bit
#endif
	str	r1, [r0]
	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
	mcr	p15, 0, r0, c7, c10, 4		@ drain WB
	mov	pc, lr

/*
 * cpu_arm920_set_pte(ptep, pte)
 *
 * Set a PTE and flush it out
 */
	.align	5
ENTRY(cpu_arm920_set_pte)
	str	r1, [r0], #-1024		@ linux version

	eor	r1, r1, #LPTE_PRESENT | LPTE_YOUNG | LPTE_WRITE | LPTE_DIRTY

	bic	r2, r1, #0xff0
	bic	r2, r2, #3
	orr	r2, r2, #HPTE_TYPE_SMALL

	tst	r1, #LPTE_USER | LPTE_EXEC	@ User or Exec?
	orrne	r2, r2, #HPTE_AP_READ

	tst	r1, #LPTE_WRITE | LPTE_DIRTY	@ Write and Dirty?
	orreq	r2, r2, #HPTE_AP_WRITE

	tst	r1, #LPTE_PRESENT | LPTE_YOUNG	@ Present and Young?
	movne	r2, #0

#ifdef CONFIG_CPU_ARM920_FORCE_WRITE_THROUGH
	eor	r3, r1, #0x0a			@ C & small page?
	tst	r3, #0x0b
	biceq	r2, r2, #4
#endif
	str	r2, [r0]			@ hardware version
	mov	r0, r0
	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
	mcr	p15, 0, r0, c7, c10, 4		@ drain WB
	mov	pc, lr


cpu_manu_name:
	.asciz	"ARM/VLSI"
ENTRY(cpu_arm920_name)
	.ascii	"Arm920"
#if defined(CONFIG_CPU_ARM920_CPU_IDLE)
	.ascii	"s"
#endif
#if defined(CONFIG_CPU_ARM920_I_CACHE_ON)
	.ascii	"i"
#endif
#if defined(CONFIG_CPU_ARM920_D_CACHE_ON)
	.ascii	"d"
#if defined(CONFIG_CPU_ARM920_FORCE_WRITE_THROUGH)
	.ascii	"(wt)"
#else
	.ascii	"(wb)"
#endif
#endif
	.ascii	"\0"
	.align

	.section ".text.init", #alloc, #execinstr

__arm920_setup:
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7		@ invalidate I,D caches on v4
	mcr	p15, 0, r0, c7, c10, 4		@ drain write buffer on v4
	mcr	p15, 0, r0, c8, c7		@ invalidate I,D TLBs on v4
	mcr	p15, 0, r4, c2, c0		@ load page table pointer
	mov	r0, #0x1f			@ Domains 0, 1 = client
	mcr	p15, 0, r0, c3, c0		@ load domain access register
	mrc	p15, 0, r0, c1, c0		@ get control register v4
/*
 * Clear out 'unwanted' bits (then put them in if we need them)
 */
	bic	r0, r0, #0x0e00			@ ....??r.........
	bic	r0, r0, #0x0002			@ ..............a.
	bic	r0, r0, #0x000c			@ W,D
	bic	r0, r0, #0x1000			@ I
/*
 * Turn on what we want
 */
	orr	r0, r0, #0x0031			@ ..........DP...M
	orr	r0, r0, #0x0100			@ .......S........

#ifdef CONFIG_CPU_ARM920_D_CACHE_ON
	orr	r0, r0, #0x0004			@ Enable D cache
#endif
#ifdef CONFIG_CPU_ARM920_I_CACHE_ON
	orr	r0, r0, #0x1000                 @ I Cache on
#endif
	mov	pc, lr

	.text

/*
 * Purpose : Function pointers used to access above functions - all calls
 *	     come through these
 */
	.type	arm920_processor_functions, #object
arm920_processor_functions:
	.word	cpu_arm920_data_abort
	.word	cpu_arm920_check_bugs
	.word	cpu_arm920_proc_init
	.word	cpu_arm920_proc_fin
	.word	cpu_arm920_reset
	.word   cpu_arm920_do_idle

	/* cache */
	.word	cpu_arm920_cache_clean_invalidate_all
	.word	cpu_arm920_cache_clean_invalidate_range
	.word	cpu_arm920_flush_ram_page

	/* dcache */
	.word	cpu_arm920_dcache_invalidate_range
	.word	cpu_arm920_dcache_clean_range
	.word	cpu_arm920_dcache_clean_page
	.word	cpu_arm920_dcache_clean_entry

	/* icache */
	.word	cpu_arm920_icache_invalidate_range
	.word	cpu_arm920_icache_invalidate_page

	/* tlb */
	.word	cpu_arm920_tlb_invalidate_all
	.word	cpu_arm920_tlb_invalidate_range
	.word	cpu_arm920_tlb_invalidate_page

	/* pgtable */
	.word	cpu_arm920_set_pgd
	.word	cpu_arm920_set_pmd
	.word	cpu_arm920_set_pte
	.size	arm920_processor_functions, . - arm920_processor_functions

	.type	cpu_arm920_info, #object
cpu_arm920_info:
	.long	cpu_manu_name
	.long	cpu_arm920_name
	.size	cpu_arm920_info, . - cpu_arm920_info

	.type	cpu_arch_name, #object
cpu_arch_name:
	.asciz	"armv4"
	.size	cpu_arch_name, . - cpu_arch_name

	.type	cpu_elf_name, #object
cpu_elf_name:
	.asciz	"v4"
	.size	cpu_elf_name, . - cpu_elf_name
	.align

	.section ".proc.info", #alloc, #execinstr

	.type	__arm920_proc_info,#object
__arm920_proc_info:
	.long	0x41009200
	.long	0xff00fff0
	.long	0x00000c1e			@ mmuflags
	b	__arm920_setup
	.long	cpu_arch_name
	.long	cpu_elf_name
	.long	HWCAP_SWP | HWCAP_HALF | HWCAP_26BIT
	.long	cpu_arm920_info
	.long	arm920_processor_functions
	.size	__arm920_proc_info, . - __arm920_proc_info
