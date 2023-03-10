/* 
 * HPMC (High Priority Machine Check) handler.
 *
 * Copyright (C) 1999 Philipp Rumpf <prumpf@tux.org>
 * Copyright (C) 1999 Hewlett-Packard (Frank Rowand)
 * Copyright (C) 2000 Hewlett-Packard (John Marvin)
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/*
 * This HPMC handler retrieves the HPMC pim data, resets IO and
 * returns to the default trap handler with code set to 1 (HPMC).
 * The default trap handler calls handle interruption, which
 * does a stack and register dump. This at least allows kernel
 * developers to get back to C code in virtual mode, where they
 * have the option to examine and print values from memory that
 * would help in debugging an HPMC caused by a software bug.
 *
 * There is more to do here:
 *
 *      1) On MP systems we need to synchronize processors
 *         before calling pdc/iodc.
 *      2) We should be checking the system state and not
 *         returning to the fault handler if things are really
 *         bad.
 *
 */

	.level		1.1
	.data

#define __ASSEMBLY__
#include <asm/assembly.h>
#include <asm/pdc.h>

	/*
	 * stack for os_hpmc, the HPMC handler.
	 * buffer for IODC procedures (for the HPMC handler).
	 *
	 * IODC requires 7K byte stack.  That leaves 1K byte for os_hpmc.
	 */

	.align 4096
hpmc_stack:
	.block 16384

#define HPMC_IODC_BUF_SIZE 0x8000

	.align 4096
hpmc_iodc_buf:
	.block HPMC_IODC_BUF_SIZE

	.align 8
hpmc_raddr:
	.block 128

#define HPMC_PIM_DATA_SIZE 896 /* Enough to hold all architected 2.0 state */

	.export hpmc_pim_data, data
	.align 8
hpmc_pim_data:
	.block HPMC_PIM_DATA_SIZE

	.text

	.export os_hpmc, code
	.import intr_save, code

os_hpmc:

	/*
	 * registers modified:
	 *
	 *   Using callee saves registers without saving them.  The
	 *   original values are in the pim dump if we need them.
	 *
	 *   r2   (rp)  return pointer
	 *   r3   address of PDCE_PROC
	 *   r4   scratch
	 *   r5   scratch
	 *   r23  (arg3) procedure arg
	 *   r24  (arg2) procedure arg
	 *   r25  (arg1) procedure arg
	 *   r26  (arg0) procedure arg
	 *   r30  (sp)   stack pointer
	 *
	 * registers read:
	 *
	 *   r26  contains address of PDCE_PROC on entry
	 *   r28  (ret0) return value from procedure
	 */

	copy    arg0, %r3       /* save address of PDCE_PROC */

	/*
	 *  disable nested HPMCs
	 *
	 * Increment os_hpmc checksum to invalidate it.
	 * Do this before turning the PSW M bit off.
	 */

	mfctl   %cr14, %r4
	ldw     52(%r4),%r5
	addi    1,%r5,%r5
	stw     %r5,52(%r4)

	/* MP_FIXME: synchronize all processors. */

	/* Setup stack pointer. */

	ldil    L%PA(hpmc_stack),sp
	ldo     R%PA(hpmc_stack)(sp),sp
	
	ldo     128(sp),sp /* leave room for arguments */

	/*
	 * Most PDC routines require that the M bit be off.
	 * So turn on the Q bit and turn off the M bit.
	 */

	ldo     8(%r0),%r4                       /* PSW Q on, PSW M off */
	mtctl   %r4,ipsw
	mtctl   %r0,pcsq
	mtctl   %r0,pcsq
	ldil    L%PA(os_hpmc_1),%r4
	ldo     R%PA(os_hpmc_1)(%r4),%r4
	mtctl   %r4,pcoq
	ldo     4(%r4),%r4
	mtctl   %r4,pcoq
	rfi
	nop

os_hpmc_1:

	/* Call PDC_PIM to get HPMC pim info */

	/*
	 * Note that on some newer boxes, PDC_PIM must be called
	 * before PDC_IO if you want IO to be reset. PDC_PIM sets
	 * a flag that PDC_IO examines.
	 */

	ldo     PDC_PIM(%r0), arg0
	ldo     PDC_PIM_HPMC(%r0),arg1          /* Transfer HPMC data */
	ldil    L%PA(hpmc_raddr),arg2
	ldo     R%PA(hpmc_raddr)(arg2),arg2
	ldil    L%PA(hpmc_pim_data),arg3
	ldo     R%PA(hpmc_pim_data)(arg3),arg3
	ldil    L%HPMC_PIM_DATA_SIZE,%r4
	ldo     R%HPMC_PIM_DATA_SIZE(%r4),%r4
	stw     %r4,-52(sp)

	ldil    L%PA(os_hpmc_2), rp
	bv      (r3)                            /* call pdce_proc */
	ldo     R%PA(os_hpmc_2)(rp), rp

os_hpmc_2:
	comib,<>  0,ret0, os_hpmc_fail

	/* Reset IO by calling the hversion dependent PDC_IO routine */

	ldo     PDC_IO(%r0),arg0
	ldo     0(%r0),arg1                     /* log IO errors */
	ldo     0(%r0),arg2                     /* reserved */
	ldo     0(%r0),arg3                     /* reserved */
	stw     %r0,-52(sp)                     /* reserved */

	ldil    L%PA(os_hpmc_3),rp
	bv      (%r3)                           /* call pdce_proc */
	ldo     R%PA(os_hpmc_3)(rp),rp

os_hpmc_3:

	/* FIXME? Check for errors from PDC_IO (-1 might be OK) */

	/*
	 * Initialize the IODC console device (HPA,SPA, path etc.
	 * are stored on page 0.
	 */

	/*
	 * Load IODC into hpmc_iodc_buf by calling PDC_IODC.
	 * Note that PDC_IODC handles flushing the appropriate
	 * data and instruction cache lines.
	 */

	ldo     PDC_IODC(%r0),arg0
	ldo     PDC_IODC_READ(%r0),arg1
	ldil    L%PA(hpmc_raddr),arg2
	ldo     R%PA(hpmc_raddr)(arg2),arg2
	ldw     BOOT_CONSOLE_HPA_OFFSET(%r0),arg3 /* console hpa */
	ldo     PDC_IODC_RI_INIT(%r0),%r4
	stw     %r4,-52(sp)
	ldil    L%PA(hpmc_iodc_buf),%r4
	ldo     R%PA(hpmc_iodc_buf)(%r4),%r4
	stw     %r4,-56(sp)
	ldil    L%HPMC_IODC_BUF_SIZE,%r4
	ldo     R%HPMC_IODC_BUF_SIZE(%r4),%r4
	stw     %r4,-60(sp)

	ldil    L%PA(os_hpmc_4),rp
	bv      (%r3)                            /* call pdce_proc */
	ldo     R%PA(os_hpmc_4)(rp),rp

os_hpmc_4:
	comib,<>  0,ret0,os_hpmc_fail

	/* Call the entry init (just loaded by PDC_IODC) */

	ldw     BOOT_CONSOLE_HPA_OFFSET(%r0),arg0  /* console hpa */
	ldo     ENTRY_INIT_MOD_DEV(%r0), arg1
	ldw     BOOT_CONSOLE_SPA_OFFSET(%r0),arg2  /* console spa */
	depi    0,31,11,arg2                       /* clear bits 21-31    */
	ldo     BOOT_CONSOLE_PATH_OFFSET(%r0),arg3 /* console path */
	ldil    L%PA(hpmc_raddr),%r4
	ldo     R%PA(hpmc_raddr)(%r4),%r4
	stw     %r4, -52(sp)
	stw     %r0, -56(sp)                    /* HV                  */
	stw     %r0, -60(sp)                    /* HV                  */
	stw     %r0, -64(sp)                    /* HV                  */
	stw     %r0, -68(sp)                    /* lang, must be zero  */

	ldil    L%PA(hpmc_iodc_buf),%r5
	ldo     R%PA(hpmc_iodc_buf)(%r5),%r5
	ldil    L%PA(os_hpmc_5),rp
	bv      (%r5)
	ldo     R%PA(os_hpmc_5)(rp),rp

os_hpmc_5:
	comib,<>  0,ret0,os_hpmc_fail

	/* Prepare to call intr_save */

	/*
	 * Load kernel page directory (load into user also, since
	 * we don't intend to ever return to user land anyway)
	 */

	ldil		L%PA(swapper_pg_dir),%r4
	ldo		R%PA(swapper_pg_dir)(%r4),%r4
	mtctl		%r4,%cr24	/* Initialize kernel root pointer */
	mtctl		%r4,%cr25	/* Initialize user root pointer */

	/* Clear sr4-sr7 */

	mtsp	%r0, %sr4
	mtsp	%r0, %sr5
	mtsp	%r0, %sr6
	mtsp	%r0, %sr7

	tovirt  %r30        /* make sp virtual */

	rsm 8,%r0           /* Clear Q bit */
	ldi     1,%r1
	mtctl   %r1,%cr29   /* Set trap code to "1" for HPMC */
	mtctl   %r0,%cr30   /* Force interruptions to use hpmc stack */
	ldil    L%PA(intr_save), %r1
	ldo     R%PA(intr_save)(%r1), %r1
	be      0(%sr7,%r1)
	nop

os_hpmc_fail:

	/*
	 * Reset the system
	 *
	 * Some systems may lockup from a broadcast reset, so try the
	 * hversion PDC_BROADCAST_RESET() first.
	 * MP_FIXME: reset all processors if more than one central bus.
	 */

	/* PDC_BROADCAST_RESET() */

	ldo     PDC_BROADCAST_RESET(%r0),arg0
	ldo     0(%r0),arg1                     /* do reset */

	ldil    L%PA(os_hpmc_6),rp
	bv      (%r3)                           /* call pdce_proc */
	ldo     R%PA(os_hpmc_6)(rp),rp

os_hpmc_6:

	/*
	 * possible return values:
	 *  -1  non-existent procedure
	 *  -2  non-existent option
	 *  -16 unaligned stack
	 *
	 * If call returned, do a broadcast reset.
	 */

	ldil    L%0xfffc0000,%r4        /* IO_BROADCAST */
	ldo     5(%r0),%r5
	stw     %r5,48(%r4)             /*  CMD_RESET to IO_COMMAND offset */

	b .
	nop

	/* this label used to compute os_hpmc checksum */

	.export os_hpmc_end, code

os_hpmc_end:

	nop
