/* $Id$
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * SGI IP27 specific setup.
 *
 * Copyright (C) 1995 - 1997, 1999 Silcon Graphics, Inc.
 * Copyright (C) 1999 Ralf Baechle (ralf@gnu.org)
 */
#ifndef _ASM_SN_SN0_ARCH_H
#define _ASM_SN_SN0_ARCH_H

#include <linux/config.h>

#ifndef SABLE

#ifndef SN0XXL  /* 128 cpu SMP max */
/*
 * This is the maximum number of nodes that can be part of a kernel.
 * Effectively, it's the maximum number of compact node ids (cnodeid_t).
 */
#define MAX_COMPACT_NODES       64

/*
 * MAXCPUS refers to the maximum number of CPUs in a single kernel.
 * This is not necessarily the same as MAXNODES * CPUS_PER_NODE
 */
#define MAXCPUS                 128

#else /* SN0XXL system */

#define MAX_COMPACT_NODES       128
#define MAXCPUS                 256

#endif /* SN0XXL */

/*
 * This is the maximum number of NASIDS that can be present in a system.
 * (Highest NASID plus one.)
 */
#define MAX_NASIDS		256

/*
 * MAX_REGIONS refers to the maximum number of hardware partitioned regions.
 */
#define	MAX_REGIONS		64
#define MAX_NONPREMIUM_REGIONS  16
#define MAX_PREMIUM_REGIONS     MAX_REGIONS

/*
 * MAX_PARITIONS refers to the maximum number of logically defined 
 * partitions the system can support.
 */
#define MAX_PARTITIONS		MAX_REGIONS


#else

#define MAX_COMPACT_NODES	4
#define MAX_NASIDS		4
#define MAXCPUS			8

#endif

#define NASID_MASK_BYTES	((MAX_NASIDS + 7) / 8)

/*
 * Slot constants for SN0
 */
#ifdef CONFIG_SGI_SN0_N_MODE
#define MAX_MEM_SLOTS   16                      /* max slots per node */
#else /* !CONFIG_SGI_SN0_N_MODE, assume M_MODE */
#define MAX_MEM_SLOTS   32                      /* max slots per node */
#endif /* defined(N_MODE) */

#if SABLE_RTL
#define SLOT_SHIFT      	(28)
#define SLOT_MIN_MEM_SIZE	(16*1024*1024)
#else
#define SLOT_SHIFT      	(27)
#define SLOT_MIN_MEM_SIZE	(32*1024*1024)
#endif

#endif /* _ASM_SN_SN0_ARCH_H */
