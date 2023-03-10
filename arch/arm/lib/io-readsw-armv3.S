/*
 *  linux/arch/arm/lib/io-readsb.S
 *
 *  Copyright (C) 1995-2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/hardware.h>

.insw_bad_alignment:
		adr	r0, .insw_bad_align_msg
		mov	r2, lr
		b	SYMBOL_NAME(panic)
.insw_bad_align_msg:
		.asciz	"insw: bad buffer alignment (0x%p, lr=0x%08lX)\n"
		.align

.insw_align:	tst	r1, #1
		bne	.insw_bad_alignment

		ldr	r3, [r0]
		strb	r3, [r1], #1
		mov	r3, r3, lsr #8
		strb	r3, [r1], #1

		subs	r2, r2, #1
		RETINSTR(moveq, pc, lr)

ENTRY(__arch_readsw)
		tst	r1, #3
		bne	.insw_align

.insw_aligned:	mov	ip, #0xff
		orr	ip, ip, ip, lsl #8
		stmfd	sp!, {r4, r5, r6, lr}

		subs	r2, r2, #8
		bmi	.no_insw_8

.insw_8_lp:	ldr	r3, [r0]
		and	r3, r3, ip
		ldr	r4, [r0]
		orr	r3, r3, r4, lsl #16

		ldr	r4, [r0]
		and	r4, r4, ip
		ldr	r5, [r0]
		orr	r4, r4, r5, lsl #16

		ldr	r5, [r0]
		and	r5, r5, ip
		ldr	r6, [r0]
		orr	r5, r5, r6, lsl #16

		ldr	r6, [r0]
		and	r6, r6, ip
		ldr	lr, [r0]
		orr	r6, r6, lr, lsl #16

		stmia	r1!, {r3 - r6}
		subs	r2, r2, #8
		bpl	.insw_8_lp
		tst	r2, #7
		LOADREGS(eqfd, sp!, {r4, r5, r6, pc})

.no_insw_8:	tst	r2, #4
		beq	.no_insw_4

		ldr	r3, [r0]
		and	r3, r3, ip
		ldr	r4, [r0]
		orr	r3, r3, r4, lsl #16

		ldr	r4, [r0]
		and	r4, r4, ip
		ldr	r5, [r0]
		orr	r4, r4, r5, lsl #16

		stmia	r1!, {r3, r4}

.no_insw_4:	tst	r2, #2
		beq	.no_insw_2

		ldr	r3, [r0]
		and	r3, r3, ip
		ldr	r4, [r0]
		orr	r3, r3, r4, lsl #16

		str	r3, [r1], #4

.no_insw_2:	tst	r2, #1
		ldrne	r3, [r0]
		strneb	r3, [r1], #1
		movne	r3, r3, lsr #8
		strneb	r3, [r1]
		LOADREGS(fd, sp!, {r4, r5, r6, pc})


