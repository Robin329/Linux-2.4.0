/* $Id: watch.h,v 1.1 1999/09/27 16:01:41 ralf Exp $
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1996, 1997, 1998, 1999 by Ralf Baechle
 */
#ifndef _ASM_WATCH_H
#define _ASM_WATCH_H

#include <linux/linkage.h>

/*
 * Types of reference for watch_set()
 */
enum wref_type {
	wr_save = 1,
	wr_load = 2
};

extern char watch_available;

extern asmlinkage void __watch_set(unsigned long addr, enum wref_type ref);
extern asmlinkage void __watch_clear(void);
extern asmlinkage void __watch_reenable(void);

#define watch_set(addr, ref)					\
	if (watch_available)					\
		__watch_set(addr, ref)
#define watch_clear()						\
	if (watch_available)					\
		__watch_clear()
#define watch_reenable()					\
	if (watch_available)					\
		__watch_reenable()

#endif _ASM_WATCH_H
