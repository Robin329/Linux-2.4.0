/*
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2000 Hewlett Packard (Paul Bame bame@puffin.external.hp.com)
 *
 */
#define __ASSEMBLY__
#include <asm/assembly.h>
#include <asm/psw.h>

	.section	.bss
	.export real_stack
	.align	64
real_stack:
	.block	8192

#ifdef __LP64__
#  define REG_SZ 8
#else
#  define REG_SZ 4
#endif

#define N_SAVED_REGS 9

save_cr_space:
	.block	REG_SZ * N_SAVED_REGS


/************************ 32-bit real-mode calls ***********************/
/* This can be called in both narrow and wide kernels */
	.text
	.export real32_call_asm
	/* unsigned long real32_call_asm(unsigned int *sp,
	 *		unsigned int *arg0p,
	 *		unsigned int iodc_fn)
	 *	sp is value of stack pointer to adopt before calling PDC (virt)
	 *	arg0p points to where saved arg values may be found
	 *	iodc_fn is the IODC function to call
	 */
real32_call_asm:
	STREG	%rp, -RP_OFFSET(%sp)	/* save RP */
#ifdef __LP64__
	callee_save
	ldo	2*REG_SZ(%sp), %sp	/* room for a couple more saves */
	STREG	%r27, -1*REG_SZ(%sp)
	STREG	%r29, -2*REG_SZ(%sp)
#endif
	STREG	%sp, -REG_SZ(%arg0)	/* save SP on real-mode stack */
	copy	%arg0, %sp		/* adopt the real-mode SP */

	/* save iodc_fn */
	copy	%arg2, %r31

	/* load up the arg registers from the saved arg area */
	/* 32-bit calling convention passes first 4 args in registers */
	ldw	0(%arg1), %arg0		/* note overwriting arg0 */
	ldw	-8(%arg1), %arg2
	ldw	-12(%arg1), %arg3
	ldw	-4(%arg1), %arg1	/* obviously must do this one last! */

	tophys	%sp

	b,l	rfi_virt2real,%r2
	nop

	b,l	save_control_regs,%r2		/* modifies r1, r2, r28 */
	nop

#ifdef __LP64__
	rsm	PSW_SM_W, %r0		/* go narrow */
#endif

	ldil	L%PA(ric_ret), %r2
	ldo	R%PA(ric_ret)(%r2), %r2
	bv	0(%r31)
	nop
ric_ret:
#ifdef __LP64__
	ssm	PSW_SM_W, %r0		/* go wide */
#endif
	/* restore CRs before going virtual in case we page fault */
	b,l	restore_control_regs, %r2	/* modifies r1, r2, r26 */
	nop

	b,l	rfi_real2virt,%r2
	nop

	tovirt	%sp
	LDREG	-REG_SZ(%sp), %sp	/* restore SP */
#ifdef __LP64__
	LDREG	-1*REG_SZ(%sp), %r27
	LDREG	-2*REG_SZ(%sp), %r29
	ldo	-2*REG_SZ(%sp), %sp
	callee_rest
#endif
	LDREG	-RP_OFFSET(%sp), %rp	/* restore RP */
	bv	0(%rp)
	nop


#  define PUSH_CR(r, where) mfctl r, %r1 ! STREG,ma %r1, REG_SZ(where)
#  define POP_CR(r, where) LDREG,mb -REG_SZ(where), %r1 ! mtctl %r1, r

	.text
save_control_regs:
	load32	PA(save_cr_space), %r28
	PUSH_CR(%cr24, %r28)
	PUSH_CR(%cr25, %r28)
	PUSH_CR(%cr26, %r28)
	PUSH_CR(%cr27, %r28)
	PUSH_CR(%cr28, %r28)
	PUSH_CR(%cr29, %r28)
	PUSH_CR(%cr30, %r28)
	PUSH_CR(%cr31, %r28)
	PUSH_CR(%cr15, %r28)
	bv 0(%r2)
	nop

restore_control_regs:
	load32	PA(save_cr_space + (N_SAVED_REGS * REG_SZ)), %r26
	POP_CR(%cr15, %r26)
	POP_CR(%cr31, %r26)
	POP_CR(%cr30, %r26)
	POP_CR(%cr29, %r26)
	POP_CR(%cr28, %r26)
	POP_CR(%cr27, %r26)
	POP_CR(%cr26, %r26)
	POP_CR(%cr25, %r26)
	POP_CR(%cr24, %r26)
	bv 0(%r2)
	nop

/* rfi_virt2real() and rfi_real2virt() could perhaps be adapted for
 * more general-purpose use by the several places which need RFIs
 */
	.align 128
	.text
rfi_virt2real:
	/* switch to real mode... */
	ssm		0,0		/* See "relied upon translation" */
	nop				/* comment in interruption.S */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	
	mtsm		0		/* disable interruptions */
	mtctl		0, %cr17	/* space 0 */
	mtctl		0, %cr17
	load32		PA(rfi_v2r_1), %r1
	mtctl		%r1, %cr18
	ldo		4(%r1), %r1
	mtctl		%r1, %cr18
	load32		PDC_PSW, %r1
	mtctl		%r1, %cr22
	rfi
	
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
rfi_v2r_1:
	tophys	%r2
	bv	0(%r2)
	nop

	.text
	.align 128
rfi_real2virt:
	ssm		0,0		/* See "relied upon translation" */
	nop				/* comment in interruption.S */
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	
	mtsm		0		/* disable interruptions */
	mtctl		0, %cr17	/* space 0 */
	mtctl		0, %cr17
	load32		(rfi_r2v_1), %r1
	mtctl		%r1, %cr18
	ldo		4(%r1), %r1
	mtctl		%r1, %cr18
	load32		KERNEL_PSW, %r1
	mtctl		%r1, %cr22
	rfi
	
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
rfi_r2v_1:
	tovirt	%r2
	bv	0(%r2)
	nop

#ifdef __LP64__

/************************ 64-bit real-mode calls ***********************/
/* This is only usable in wide kernels right now and will probably stay so */
	.text
	.export real64_call_asm
	/* unsigned long real64_call_asm(unsigned long *sp,
	 *		unsigned long *arg0p,
	 *		unsigned long fn)
	 *	sp is value of stack pointer to adopt before calling PDC (virt)
	 *	arg0p points to where saved arg values may be found
	 *	iodc_fn is the IODC function to call
	 */
real64_call_asm:
	std	%rp, -0x10(%sp)		/* save RP */
	std	%sp, -8(%arg0)		/* save SP on real-mode stack */
	copy	%arg0, %sp		/* adopt the real-mode SP */

	/* save fn */
	copy	%arg2, %r31

	/* set up the new ap */
	ldo	64(%arg1), %r29

	/* load up the arg registers from the saved arg area */
	/* 32-bit calling convention passes first 4 args in registers */
	ldd	0*REG_SZ(%arg1), %arg0		/* note overwriting arg0 */
	ldd	2*REG_SZ(%arg1), %arg2
	ldd	3*REG_SZ(%arg1), %arg3
	ldd	4*REG_SZ(%arg1), %r22
	ldd	5*REG_SZ(%arg1), %r21
	ldd	6*REG_SZ(%arg1), %r20
	ldd	7*REG_SZ(%arg1), %r19
	ldd	1*REG_SZ(%arg1), %arg1		/* do this one last! */

	tophys	%sp

	b,l	rfi_virt2real,%r2
	nop

	b,l	save_control_regs,%r2		/* modifies r1, r2, r28 */
	nop

	load32	PA(r64_ret), %r2
	bv	0(%r31)
	nop
r64_ret:
	/* restore CRs before going virtual in case we page fault */
	b,l	restore_control_regs, %r2	/* modifies r1, r2, r26 */
	nop

	b,l	rfi_real2virt,%r2
	nop

	tovirt	%sp
	ldd	-8(%sp), %sp		/* restore SP */
	ldd	-0x10(%sp), %rp		/* restore RP */
	bv	0(%rp)
	nop

#endif
