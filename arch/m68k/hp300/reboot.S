/*
 *  linux/arch/m68k/hp300/reboot.S
 *
 *  Copyright (C) 1998 Philip Blundell <philb@gnu.org>
 *
 *  Do the dirty work of rebooting the machine.  Basically we need to undo all the
 *  good stuff that head.S did when we started up.  The caches and MMU must be
 *  disabled and then we jump back to the PROM.  This is a bit gruesome but we put
 *  a brave face on it.
 */

/* XXX Doesn't work yet.  Not sure why and can't be bothered to fix it at the moment. */

	.globl	hp300_reset
hp300_reset:
	.chip	68030
	oriw    #0x0700,%sr			/* cli() */
	movel	hp300_phys_ram_base, %d1
	movel	#0, %d0
	movec	%d0, %vbr			/* reset vector table */
	lea	zero, %a0
	lea	1f, %a1
	add	%d1, %a0
	add	%d1, %a1
	pmove	%tc, %a0@
	bclr	#7, %a0@
	pmove	%a0@, %tc			/* goodbye MMU */
	jmp	%a1@
1:	movel	#0x808, %d0
	movec	%d0, %cacr			/* cache off */
	moveb	#0, 0x1ffff
	movel	#0x1a4, %a0
	jmp	%a0@

zero:	.quad	0
