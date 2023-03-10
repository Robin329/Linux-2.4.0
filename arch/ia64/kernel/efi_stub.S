/*
 * EFI call stub.
 *
 * Copyright (C) 1999-2000 Hewlett-Packard Co
 * Copyright (C) 1999-2000 David Mosberger <davidm@hpl.hp.com>
 *
 * This stub allows us to make EFI calls in physical mode with interrupts
 * turned off.  We need this because we can't call SetVirtualMap() until
 * the kernel has booted far enough to allow allocation of struct vma_struct
 * entries (which we would need to map stuff with memory attributes other
 * than uncached or writeback...).  Since the GetTime() service gets called
 * earlier than that, we need to be able to make physical mode EFI calls from
 * the kernel.
 */

/*
 * PSR settings as per SAL spec (Chapter 8 in the "IA-64 System
 * Abstraction Layer Specification", revision 2.6e).  Note that
 * psr.dfl and psr.dfh MUST be cleared, despite what this manual says.
 * Otherwise, SAL dies whenever it's trying to do an IA-32 BIOS call
 * (the br.ia instruction fails unless psr.dfl and psr.dfh are
 * cleared).  Fortunately, SAL promises not to touch the floating
 * point regs, so at least we don't have to save f2-f127.
 */
#define PSR_BITS_TO_CLEAR						\
	(IA64_PSR_I | IA64_PSR_IT | IA64_PSR_DT | IA64_PSR_RT |		\
	 IA64_PSR_DD | IA64_PSR_SS | IA64_PSR_RI | IA64_PSR_ED |	\
	 IA64_PSR_DFL | IA64_PSR_DFH)

#define PSR_BITS_TO_SET							\
	(IA64_PSR_BN)

#include <asm/processor.h>
#include <asm/asmmacro.h>

	.text
	.psr abi64
	.psr lsb
	.lsb

	.text

/*
 * Inputs:
 *	in0 = address of function descriptor of EFI routine to call
 *	in1..in7 = arguments to routine
 *
 * Outputs:
 *	r8 = EFI_STATUS returned by called function
 */

GLOBAL_ENTRY(efi_call_phys)
	UNW(.prologue ASM_UNW_PRLG_RP|ASM_UNW_PRLG_PFS, ASM_UNW_PRLG_GRSAVE(8))
	alloc loc1=ar.pfs,8,5,7,0
	ld8 r2=[in0],8			// load EFI function's entry point
	mov loc0=rp
	UNW(.body)
	;;
	mov loc2=gp			// save global pointer
	mov loc4=ar.rsc			// save RSE configuration
	mov ar.rsc=r0			// put RSE in enforced lazy, LE mode
	;;

	ld8 gp=[in0]			// load EFI function's global pointer
	mov out0=in1
	mov out1=in2
	movl r16=PSR_BITS_TO_CLEAR

	mov loc3=psr			// save processor status word
	movl r17=PSR_BITS_TO_SET
	;;
	mov out2=in3
	or loc3=loc3,r17
	mov b6=r2
	;;
	andcm r16=loc3,r16		// get psr with IT, DT, and RT bits cleared
	mov out3=in4
	br.call.sptk.few rp=ia64_switch_mode
.ret0:	mov out4=in5
	mov out5=in6
	mov out6=in7
	br.call.sptk.few rp=b6		// call the EFI function
.ret1:	mov ar.rsc=r0			// put RSE in enforced lazy, LE mode
	mov r16=loc3
	br.call.sptk.few rp=ia64_switch_mode // return to virtual mode
.ret2:	mov ar.rsc=loc4			// restore RSE configuration
	mov ar.pfs=loc1
	mov rp=loc0
	mov gp=loc2
	br.ret.sptk.few rp
END(efi_call_phys)
