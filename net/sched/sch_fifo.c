/*
 * net/sched/sch_fifo.c	The simplest FIFO queue.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 */

#include <linux/config.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/bitops.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/in.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/if_ether.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/notifier.h>
#include <net/ip.h>
#include <net/route.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/pkt_sched.h>

/* 1 band FIFO pseudo-"scheduler" */

struct fifo_sched_data
{
	unsigned limit;
};

static int
bfifo_enqueue(struct sk_buff *skb, struct Qdisc* sch)
{
	struct fifo_sched_data *q = (struct fifo_sched_data *)sch->data;

	if (sch->stats.backlog <= q->limit) {
		__skb_queue_tail(&sch->q, skb);
		sch->stats.backlog += skb->len;
		sch->stats.bytes += skb->len;
		sch->stats.packets++;
		return 0;
	}
	sch->stats.drops++;
#ifdef CONFIG_NET_CLS_POLICE
	if (sch->reshape_fail==NULL || sch->reshape_fail(skb, sch))
#endif
		kfree_skb(skb);
	return NET_XMIT_DROP;
}

static int
bfifo_requeue(struct sk_buff *skb, struct Qdisc* sch)
{
	__skb_queue_head(&sch->q, skb);
	sch->stats.backlog += skb->len;
	return 0;
}

static struct sk_buff *
bfifo_dequeue(struct Qdisc* sch)
{
	struct sk_buff *skb;

	skb = __skb_dequeue(&sch->q);
	if (skb)
		sch->stats.backlog -= skb->len;
	return skb;
}

static int
fifo_drop(struct Qdisc* sch)
{
	struct sk_buff *skb;

	skb = __skb_dequeue_tail(&sch->q);
	if (skb) {
		sch->stats.backlog -= skb->len;
		kfree_skb(skb);
		return 1;
	}
	return 0;
}

static void
fifo_reset(struct Qdisc* sch)
{
	skb_queue_purge(&sch->q);
	sch->stats.backlog = 0;
}

static int
pfifo_enqueue(struct sk_buff *skb, struct Qdisc* sch)
{
	struct fifo_sched_data *q = (struct fifo_sched_data *)sch->data;

	if (sch->q.qlen <= q->limit) {
		__skb_queue_tail(&sch->q, skb);
		sch->stats.bytes += skb->len;
		sch->stats.packets++;
		return 0;
	}
	sch->stats.drops++;
#ifdef CONFIG_NET_CLS_POLICE
	if (sch->reshape_fail==NULL || sch->reshape_fail(skb, sch))
#endif
		kfree_skb(skb);
	return NET_XMIT_DROP;
}

static int
pfifo_requeue(struct sk_buff *skb, struct Qdisc* sch)
{
	__skb_queue_head(&sch->q, skb);
	return 0;
}


static struct sk_buff *
pfifo_dequeue(struct Qdisc* sch)
{
	return __skb_dequeue(&sch->q);
}

static int fifo_init(struct Qdisc *sch, struct rtattr *opt)
{
	struct fifo_sched_data *q = (void*)sch->data;

	if (opt == NULL) {
		if (sch->ops == &bfifo_qdisc_ops)
			q->limit = sch->dev->tx_queue_len*sch->dev->mtu;
		else	
			q->limit = sch->dev->tx_queue_len;
	} else {
		struct tc_fifo_qopt *ctl = RTA_DATA(opt);
		if (opt->rta_len < RTA_LENGTH(sizeof(*ctl)))
			return -EINVAL;
		q->limit = ctl->limit;
	}
	return 0;
}

#ifdef CONFIG_RTNETLINK
static int fifo_dump(struct Qdisc *sch, struct sk_buff *skb)
{
	struct fifo_sched_data *q = (void*)sch->data;
	unsigned char	 *b = skb->tail;
	struct tc_fifo_qopt opt;

	opt.limit = q->limit;
	RTA_PUT(skb, TCA_OPTIONS, sizeof(opt), &opt);

	return skb->len;

rtattr_failure:
	skb_trim(skb, b - skb->data);
	return -1;
}
#endif

struct Qdisc_ops pfifo_qdisc_ops =
{
	NULL,
	NULL,
	"pfifo",
	sizeof(struct fifo_sched_data),

	pfifo_enqueue,
	pfifo_dequeue,
	pfifo_requeue,
	fifo_drop,

	fifo_init,
	fifo_reset,
	NULL,
	fifo_init,

#ifdef CONFIG_RTNETLINK
	fifo_dump,
#endif
};

struct Qdisc_ops bfifo_qdisc_ops =
{
	NULL,
	NULL,
	"bfifo",
	sizeof(struct fifo_sched_data),

	bfifo_enqueue,
	bfifo_dequeue,
	bfifo_requeue,
	fifo_drop,

	fifo_init,
	fifo_reset,
	NULL,
	fifo_init,
#ifdef CONFIG_RTNETLINK
	fifo_dump,
#endif
};
