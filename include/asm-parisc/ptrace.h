#ifndef _PARISC_PTRACE_H
#define _PARISC_PTRACE_H

/* written by Philipp Rumpf, Copyright (C) 1999 SuSE GmbH Nuernberg
** Copyright (C) 2000 Grant Grundler, Hewlett-Packard
*/

#include <linux/types.h>

/* This struct defines the way the registers are stored on the 
   stack during a system call. */

struct pt_regs {
	unsigned long gr[32];	/* PSW is in gr[0] */
	__u64 fr[32];
	unsigned long sr[ 8];
	unsigned long iasq[2];
	unsigned long iaoq[2];
	unsigned long cr24;
	unsigned long cr25;
	unsigned long cr26;
	unsigned long cr27;
	unsigned long cr30;
	unsigned long orig_r28;
	unsigned long ksp;
	unsigned long kpc;
	unsigned long sar;	/* CR11 */
	unsigned long iir;	/* CR19 */
	unsigned long isr;	/* CR20 */
	unsigned long ior;	/* CR21 */
	unsigned long ipsw;	/* CR22 */
	unsigned long cr_pid[4]; /* CR8,9,12,13 */
};

#define task_regs(task) ((struct pt_regs *) ((char *)(task) + TASK_REGS))
/*
 * The numbers chosen here are somewhat arbitrary but absolutely MUST
 * not overlap with any of the number assigned in <linux/ptrace.h>.
 *
 * These ones are taken from IA-64 on the assumption that theirs are
 * the most correct (and we also want to support PTRACE_SINGLEBLOCK
 * since we have taken branch traps too)
 */
#define PTRACE_SINGLEBLOCK	12	/* resume execution until next branch */
#define PTRACE_GETSIGINFO	13	/* get child's siginfo structure */
#define PTRACE_SETSIGINFO	14	/* set child's siginfo structure */

#ifdef __KERNEL__

/* XXX should we use iaoq[1] or iaoq[0] ? */
#define user_mode(regs)			(((regs)->iaoq[0] &  3) ? 1 : 0)
#define instruction_pointer(regs)	((regs)->iaoq[0] & ~3)
extern void show_regs(struct pt_regs *);
#endif

#endif
