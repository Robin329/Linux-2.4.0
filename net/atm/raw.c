/* net/atm/raw.c - Raw AAL0 and AAL5 transports */

/* Written 1995-2000 by Werner Almesberger, EPFL LRC/ICA */


#include <linux/module.h>
#include <linux/sched.h>
#include <linux/atmdev.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/mm.h>

#include "common.h"
#include "protocols.h"


#if 0
#define DPRINTK(format,args...) printk(KERN_DEBUG format,##args)
#else
#define DPRINTK(format,args...)
#endif


/*
 * SKB == NULL indicates that the link is being closed
 */

void atm_push_raw(struct atm_vcc *vcc,struct sk_buff *skb)
{
	if (skb) {
		skb_queue_tail(&vcc->recvq,skb);
		wake_up(&vcc->sleep);
	}
}


static void atm_pop_raw(struct atm_vcc *vcc,struct sk_buff *skb)
{
	DPRINTK("APopR (%d) %d -= %d\n",vcc->vci,vcc->tx_inuse,skb->truesize);
	atomic_sub(skb->truesize+ATM_PDU_OVHD,&vcc->tx_inuse);
	dev_kfree_skb_any(skb);
	wake_up(&vcc->sleep);
}


static int atm_send_aal0(struct atm_vcc *vcc,struct sk_buff *skb)
{
	/*
	 * Note that if vpi/vci are _ANY or _UNSPEC the below will
	 * still work
	 */
	if (!capable(CAP_NET_ADMIN) &&
            (((u32 *) skb->data)[0] & (ATM_HDR_VPI_MASK | ATM_HDR_VCI_MASK)) !=
            ((vcc->vpi << ATM_HDR_VPI_SHIFT) | (vcc->vci << ATM_HDR_VCI_SHIFT)))
	    {
		kfree_skb(skb);
		return -EADDRNOTAVAIL;
        }
	return vcc->dev->ops->send(vcc,skb);
}


int atm_init_aal0(struct atm_vcc *vcc)
{
	vcc->push = atm_push_raw;
	vcc->pop = atm_pop_raw;
	vcc->push_oam = NULL;
	vcc->send = atm_send_aal0;
	return 0;
}


int atm_init_aal34(struct atm_vcc *vcc)
{
	vcc->push = atm_push_raw;
	vcc->pop = atm_pop_raw;
	vcc->push_oam = NULL;
	vcc->send = vcc->dev->ops->send;
	return 0;
}


int atm_init_aal5(struct atm_vcc *vcc)
{
	vcc->push = atm_push_raw;
	vcc->pop = atm_pop_raw;
	vcc->push_oam = NULL;
	vcc->send = vcc->dev->ops->send;
	return 0;
}


EXPORT_SYMBOL(atm_init_aal5);
