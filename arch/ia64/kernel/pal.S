/*
 * PAL Firmware support
 * IA-64 Processor Programmers Reference Vol 2
 *
 * Copyright (C) 1999 Don Dugger <don.dugger@intel.com>
 * Copyright (C) 1999 Walt Drummond <drummond@valinux.com>
 * Copyright (C) 1999-2000 David Mosberger <davidm@hpl.hp.com>
 * Copyright (C) 2000 Stephane Eranian <eranian@hpl.hp.com>
 *
 * 05/22/2000 eranian Added support for stacked register calls
 * 05/24/2000 eranian Added support for physical mode static calls
 */

#include <asm/asmmacro.h>
#include <asm/processor.h>

	.text
	.psr abi64
	.psr lsb
	.lsb

	.data
pal_entry_point:
	data8 ia64_pal_default_handler
	.text

/*
 * Set the PAL entry point address.  This could be written in C code, but we do it here
 * to keep it all in one module (besides, it's so trivial that it's
 * not a big deal).
 *
 * in0		Address of the PAL entry point (text address, NOT a function descriptor).
 */
GLOBAL_ENTRY(ia64_pal_handler_init)
	alloc r3=ar.pfs,1,0,0,0
	movl r2=pal_entry_point
	;;
	st8 [r2]=in0
	br.ret.sptk.few rp
END(ia64_pal_handler_init)

/*
 * Default PAL call handler.  This needs to be coded in assembly because it uses
 * the static calling convention, i.e., the RSE may not be used and calls are
 * done via "br.cond" (not "br.call").
 */
GLOBAL_ENTRY(ia64_pal_default_handler)
	mov r8=-1
	br.cond.sptk.few rp
END(ia64_pal_default_handler)

/*
 * Make a PAL call using the static calling convention.
 *
 * in0         Index of PAL service
 * in1 - in3   Remaining PAL arguments
 * in4	       1 ==> clear psr.ic,  0 ==> don't clear psr.ic
 *
 */
GLOBAL_ENTRY(ia64_pal_call_static)
	UNW(.prologue ASM_UNW_PRLG_RP|ASM_UNW_PRLG_PFS, ASM_UNW_PRLG_GRSAVE(6))
	alloc loc1 = ar.pfs,6,90,0,0
	movl loc2 = pal_entry_point
1:	{
	  mov r28 = in0
	  mov r29 = in1
	  mov r8 = ip
	}
	;;
	ld8 loc2 = [loc2]		// loc2 <- entry point
	tbit.nz p6,p7 = in4, 0
	adds r8 = 1f-1b,r8
	;;
	mov loc3 = psr
	mov loc0 = rp
	UNW(.body)
	mov r30 = in2

(p6)	rsm psr.i | psr.ic
	mov r31 = in3
	mov b7 = loc2

(p7)	rsm psr.i
	;;
(p6)	srlz.i
	mov rp = r8
	br.cond.sptk.few b7
1:	mov psr.l = loc3
	mov ar.pfs = loc1
	mov rp = loc0
	;;
	srlz.d				// seralize restoration of psr.l
	br.ret.sptk.few	b0
END(ia64_pal_call_static)

/*
 * Make a PAL call using the stacked registers calling convention.
 *
 * Inputs:
 * 	in0         Index of PAL service
 * 	in2 - in3   Remaning PAL arguments
 */
GLOBAL_ENTRY(ia64_pal_call_stacked)
	UNW(.prologue ASM_UNW_PRLG_RP|ASM_UNW_PRLG_PFS, ASM_UNW_PRLG_GRSAVE(5))
	alloc loc1 = ar.pfs,5,4,87,0
	movl loc2 = pal_entry_point

	mov r28  = in0			// Index MUST be copied to r28
	mov out0 = in0			// AND in0 of PAL function
	mov loc0 = rp
	UNW(.body)
	;;
	ld8 loc2 = [loc2]		// loc2 <- entry point
	mov out1 = in1
	mov out2 = in2
	mov out3 = in3
	mov loc3 = psr
	;;
	rsm psr.i
	mov b7 = loc2
	;; 
	br.call.sptk.many rp=b7		// now make the call
.ret0:	mov psr.l  = loc3
	mov ar.pfs = loc1
	mov rp = loc0
	;;
	srlz.d				// serialize restoration of psr.l
	br.ret.sptk.few	b0
END(ia64_pal_call_stacked)

/*
 * Make a physical mode PAL call using the static registers calling convention.
 *
 * Inputs:
 * 	in0         Index of PAL service
 * 	in2 - in3   Remaning PAL arguments
 *
 * PSR_DB, PSR_LP, PSR_TB, PSR_ID, PSR_DA are never set by the kernel.
 * So we don't need to clear them.
 */
#define PAL_PSR_BITS_TO_CLEAR						\
	(IA64_PSR_I | IA64_PSR_IT | IA64_PSR_DT | IA64_PSR_RT |		\
	 IA64_PSR_DD | IA64_PSR_SS | IA64_PSR_RI | IA64_PSR_ED |	\
	 IA64_PSR_DFL | IA64_PSR_DFH)

#define PAL_PSR_BITS_TO_SET						\
	(IA64_PSR_BN)


GLOBAL_ENTRY(ia64_pal_call_phys_static)
	UNW(.prologue ASM_UNW_PRLG_RP|ASM_UNW_PRLG_PFS, ASM_UNW_PRLG_GRSAVE(6))
	alloc loc1 = ar.pfs,6,90,0,0
	movl loc2 = pal_entry_point
1:	{
	  mov r28  = in0		// copy procedure index
	  mov r8   = ip			// save ip to compute branch
	  mov loc0 = rp			// save rp
	}
	UNW(.body)
	;;
	ld8 loc2 = [loc2]		// loc2 <- entry point
	mov r29  = in1			// first argument
	mov r30  = in2			// copy arg2
	mov r31  = in3			// copy arg3
	;;
	mov loc3 = psr			// save psr
	adds r8  = 1f-1b,r8		// calculate return address for call
	;; 
	mov loc4=ar.rsc			// save RSE configuration
	dep.z loc2=loc2,0,61		// convert pal entry point to physical
	dep.z r8=r8,0,61		// convert rp to physical
	;;
	mov b7 = loc2			// install target to branch reg
	mov ar.rsc=r0			// put RSE in enforced lazy, LE mode
	movl r16=PAL_PSR_BITS_TO_CLEAR
	movl r17=PAL_PSR_BITS_TO_SET
	;;
	or loc3=loc3,r17		// add in psr the bits to set
	;;
	andcm r16=loc3,r16		// removes bits to clear from psr
	br.call.sptk.few rp=ia64_switch_mode
.ret1:	mov rp = r8			// install return address (physical)
	br.cond.sptk.few b7
1:
	mov ar.rsc=r0			// put RSE in enforced lazy, LE mode
	mov r16=loc3			// r16= original psr
	br.call.sptk.few rp=ia64_switch_mode // return to virtual mode
.ret2:
	mov psr.l = loc3		// restore init PSR

	mov ar.pfs = loc1
	mov rp = loc0
	;;
	mov ar.rsc=loc4			// restore RSE configuration
	srlz.d				// seralize restoration of psr.l
	br.ret.sptk.few	b0
END(ia64_pal_call_phys_static)

/*
 * Make a PAL call using the stacked registers in physical mode.
 *
 * Inputs:
 * 	in0         Index of PAL service
 * 	in2 - in3   Remaning PAL arguments
 */
GLOBAL_ENTRY(ia64_pal_call_phys_stacked)
	UNW(.prologue ASM_UNW_PRLG_RP|ASM_UNW_PRLG_PFS, ASM_UNW_PRLG_GRSAVE(5))
	alloc	loc1 = ar.pfs,5,5,86,0
	movl	loc2 = pal_entry_point
1:	{
	  mov r28  = in0		// copy procedure index
	  mov loc0 = rp		// save rp
	}
	.body
	;;
	ld8 loc2 = [loc2]		// loc2 <- entry point
	mov out0 = in0		// first argument
	mov out1 = in1		// copy arg2
	mov out2 = in2		// copy arg3
	mov out3 = in3		// copy arg3
	;;
	mov loc3 = psr		// save psr
	;; 
	mov loc4=ar.rsc			// save RSE configuration
	dep.z loc2=loc2,0,61		// convert pal entry point to physical
	;;
	mov ar.rsc=r0			// put RSE in enforced lazy, LE mode
	movl r16=PAL_PSR_BITS_TO_CLEAR
	movl r17=PAL_PSR_BITS_TO_SET
	;;
	or loc3=loc3,r17		// add in psr the bits to set
	mov b7 = loc2			// install target to branch reg
	;;
	andcm r16=loc3,r16		// removes bits to clear from psr
	br.call.sptk.few rp=ia64_switch_mode
.ret6:
	br.call.sptk.many rp=b7		// now make the call
.ret7:
	mov ar.rsc=r0			// put RSE in enforced lazy, LE mode
	mov r16=loc3			// r16= original psr
	br.call.sptk.few rp=ia64_switch_mode	// return to virtual mode

.ret8:	mov psr.l  = loc3		// restore init PSR
	mov ar.pfs = loc1
	mov rp = loc0
	;;
	mov ar.rsc=loc4			// restore RSE configuration
	srlz.d				// seralize restoration of psr.l
	br.ret.sptk.few	b0
END(ia64_pal_call_phys_stacked)

