/* net/atm/protocols.h - ATM protocol handler entry points */

/* Written 1995-1997 by Werner Almesberger, EPFL LRC */


#ifndef NET_ATM_PROTOCOLS_H
#define NET_ATM_PROTOCOLS_H

void atm_push_raw(struct atm_vcc *vcc,struct sk_buff *skb);

int atm_init_aal0(struct atm_vcc *vcc);	/* "raw" AAL0 */
int atm_init_aal34(struct atm_vcc *vcc);/* "raw" AAL3/4 transport */
int atm_init_aal5(struct atm_vcc *vcc);	/* "raw" AAL5 transport */
int atm_init_atmarp(struct atm_vcc *vcc);/* ATM ARP */

#endif
