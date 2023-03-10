#ifndef _NET_PSNAP_H
#define _NET_PSNAP_H

extern struct datalink_proto *register_snap_client(unsigned char *desc, int (*rcvfunc)(struct sk_buff *, struct net_device *, struct packet_type *));
extern void unregister_snap_client(unsigned char *desc);

#endif
