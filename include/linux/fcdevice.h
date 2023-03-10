/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  NET  is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Definitions for the Fibre Channel handlers.
 *
 * Version:	@(#)fcdevice.h	1.0.0	09/26/98
 *
 * Authors:	Vineet Abraham <vma@iol.unh.edu>
 *
 *		Relocated to include/linux where it belongs by Alan Cox 
 *							<gw4pts@gw4pts.ampr.org>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 *	WARNING: This move may well be temporary. This file will get merged with others RSN.
 *
 */
#ifndef _LINUX_FCDEVICE_H
#define _LINUX_FCDEVICE_H


#include <linux/if_fc.h>

#ifdef __KERNEL__
extern int		fc_header(struct sk_buff *skb, struct net_device *dev,
				   unsigned short type, void *daddr,
				   void *saddr, unsigned len);
extern int		fc_rebuild_header(struct sk_buff *skb);
//extern unsigned short	fc_type_trans(struct sk_buff *skb, struct net_device *dev); 

extern struct net_device    * init_fcdev(struct net_device *, int);

#endif

#endif	/* _LINUX_FCDEVICE_H */
