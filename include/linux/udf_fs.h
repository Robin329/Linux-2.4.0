/*
 * udf_fs.h
 *
 * PURPOSE
 *  Included by fs/filesystems.c
 *
 * DESCRIPTION
 *  OSTA-UDF(tm) = Optical Storage Technology Association
 *  Universal Disk Format.
 *
 *  This code is based on version 2.00 of the UDF specification,
 *  and revision 3 of the ECMA 167 standard [equivalent to ISO 13346].
 *    http://www.osta.org/ *    http://www.ecma.ch/
 *    http://www.iso.org/
 *
 * CONTACTS
 *	E-mail regarding any portion of the Linux UDF file system should be
 *	directed to the development team mailing list (run by majordomo):
 *		linux_udf@hootie.lvld.hp.com
 *
 * COPYRIGHT
 *	This file is distributed under the terms of the GNU General Public
 *	License (GPL). Copies of the GPL can be obtained from:
 *		ftp://prep.ai.mit.edu/pub/gnu/GPL
 *	Each contributing author retains all rights to their own work.
 *
 *  (C) 1999-2000 Ben Fennema
 *  (C) 1999-2000 Stelias Computing Inc
 *
 * HISTORY
 *
 */

#if !defined(_LINUX_UDF_FS_H)
#define _LINUX_UDF_FS_H

#define UDF_PREALLOCATE
#define UDF_DEFAULT_PREALLOC_BLOCKS		8

#define UDFFS_DATE		"2000/02/29"
#define UDFFS_VERSION	"0.9.1"

#define UDFFS_DEBUG

#ifdef UDFFS_DEBUG
#define udf_debug(f, a...) \
	{ \
		printk (KERN_DEBUG "UDF-fs DEBUG %s:%d:%s: ", \
			__FILE__, __LINE__, __FUNCTION__); \
		printk (f, ##a); \
	}
#else
#define udf_debug(f, a...) /**/
#endif

#define udf_info(f, a...) \
		printk (KERN_INFO "UDF-fs INFO " f, ##a);

#endif /* !defined(_LINUX_UDF_FS_H) */
