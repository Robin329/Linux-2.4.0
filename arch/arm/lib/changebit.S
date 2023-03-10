/*
 *  linux/arch/arm/lib/changebit.S
 *
 *  Copyright (C) 1995-1996 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/linkage.h>
#include <asm/assembler.h>
                .text

/* Purpose  : Function to change a bit
 * Prototype: int change_bit(int bit, void *addr)
 */
ENTRY(change_bit)
		and	r2, r0, #7
		mov	r3, #1
		mov	r3, r3, lsl r2
		save_and_disable_irqs ip, r2
		ldrb	r2, [r1, r0, lsr #3]
		eor	r2, r2, r3
		strb	r2, [r1, r0, lsr #3]
		restore_irqs ip
		RETINSTR(mov,pc,lr)
