/*
    NetWinder Floating Point Emulator
    (c) Corel Computer Corporation, 1998
    (c) Philip Blundell 1998-1999

    Direct questions, comments to Scott Bambrough <scottb@netwinder.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* This is the kernel's entry point into the floating point emulator.
It is called from the kernel with code similar to this:

	adrsvc	al, r9, ret_from_exception	@ r9  = normal FP return
	adrsvc	al, lr, fpundefinstr		@ lr  = undefined instr return

	get_current_task r10
	mov	r8, #1
	strb	r8, [r10, #TSK_USED_MATH]	@ set current->used_math
	add	r10, r10, #TSS_FPESAVE		@ r10 = workspace
	ldr	r4, .LC2
	ldr	pc, [r4]			@ Call FP emulator entry point

The kernel expects the emulator to return via one of two possible
points of return it passes to the emulator.  The emulator, if
successful in its emulation, jumps to ret_from_exception (passed in
r9) and the kernel takes care of returning control from the trap to
the user code.  If the emulator is unable to emulate the instruction,
it returns via _fpundefinstr (passed via lr) and the kernel halts the
user program with a core dump.

On entry to the emulator r10 points to an area of private FP workspace
reserved in the thread structure for this process.  This is where the
emulator saves its registers across calls.  The first word of this area
is used as a flag to detect the first time a process uses floating point,
so that the emulator startup cost can be avoided for tasks that don't
want it.

This routine does three things:

1) It saves SP into a variable called userRegisters.  The kernel has
created a struct pt_regs on the stack and saved the user registers
into it.  See /usr/include/asm/proc/ptrace.h for details.  The
emulator code uses userRegisters as the base of an array of words from
which the contents of the registers can be extracted.

2) It calls EmulateAll to emulate a floating point instruction.
EmulateAll returns 1 if the emulation was successful, or 0 if not.

3) If an instruction has been emulated successfully, it looks ahead at
the next instruction.  If it is a floating point instruction, it
executes the instruction, without returning to user space.  In this
way it repeatedly looks ahead and executes floating point instructions
until it encounters a non floating point instruction, at which time it
returns via _fpreturn.

This is done to reduce the effect of the trap overhead on each
floating point instructions.  GCC attempts to group floating point
instructions to allow the emulator to spread the cost of the trap over
several floating point instructions.  */

	.globl	nwfpe_enter
nwfpe_enter:
	/* ?? Could put userRegisters and fpa11 into fixed regs during
	   emulation.  This would reduce load/store overhead at the expense
	   of stealing two regs from the register allocator.  Not sure if
	   it's worth it.  */
	ldr r4, =userRegisters
        str sp, [r4]			@ save pointer to user regs
	ldr r4, =fpa11
	str r10, [r4]			@ store pointer to our state
        mov r4, sp			@ use r4 for local pointer
        mov r10, lr			@ save the failure-return addresses

        ldr r5, [r4, #60]	 	@ get contents of PC;
	sub r8, r5, #4
.Lx2:	ldrt r0, [r8], #0		@ get actual instruction into r0
emulate:
	bl EmulateAll			@ emulate the instruction
   	cmp r0, #0			@ was emulation successful
        moveq pc, r10			@ no, return failure

next:
.Lx1:	ldrt r6, [r5], #4		@ get the next instruction and
					@ increment PC

	and   r2, r6, #0x0F000000	@ test for FP insns
        teq   r2, #0x0C000000
        teqne r2, #0x0D000000
        teqne r2, #0x0E000000
        movne pc, r9			@ return ok if not a fp insn

        str r5, [r4, #60]		@ update PC copy in regs

        mov r0, r6			@ save a copy
        ldr r1, [r4, #64]		@ fetch the condition codes
   	bl  checkCondition		@ check the condition
   	cmp r0, #0			@ r0 = 0 ==> condition failed

        @ if condition code failed to match, next insn
   	beq next			@ get the next instruction;
   	    
        mov r0, r6			@ prepare for EmulateAll()
   	b emulate			@ if r0 != 0, goto EmulateAll

	@ We need to be prepared for the instruction at .Lx1 or .Lx2
	@ to fault.
	.section .fixup,"ax"
	.align
.Lfix:	mov	pc, r9
	.previous

	.section __ex_table,"a"
	.align 3
	.long	.Lx2, .Lfix
	.long	.Lx1, .Lfix
	.previous
