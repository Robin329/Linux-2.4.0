/*
 * arch/m68k/mvme147/147ints.c
 *
 * Copyright (C) 1997 Richard Hirst [richard@sleepie.demon.co.uk]
 *
 * based on amiints.c -- Amiga Linux interrupt handling code
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file README.legal in the main directory of this archive
 * for more details.
 *
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/errno.h>

#include <asm/ptrace.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/traps.h>

static void mvme147_defhand (int irq, void *dev_id, struct pt_regs *fp);

/*
 * This should ideally be 4 elements only, for speed.
 */

static struct {
	void		(*handler)(int, void *, struct pt_regs *);
	unsigned long	flags;
	void		*dev_id;
	const char	*devname;
	unsigned	count;
} irq_tab[256];

/*
 * void mvme147_init_IRQ (void)
 *
 * Parameters:	None
 *
 * Returns:	Nothing
 *
 * This function is called during kernel startup to initialize
 * the mvme147 IRQ handling routines.
 */

void mvme147_init_IRQ (void)
{
	int i;

	for (i = 0; i < 256; i++) {
		irq_tab[i].handler = mvme147_defhand;
		irq_tab[i].flags = IRQ_FLG_STD;
		irq_tab[i].dev_id = NULL;
		irq_tab[i].devname = NULL;
		irq_tab[i].count = 0;
	}
}

int mvme147_request_irq(unsigned int irq,
		void (*handler)(int, void *, struct pt_regs *),
                unsigned long flags, const char *devname, void *dev_id)
{
	if (irq > 255) {
		printk("%s: Incorrect IRQ %d from %s\n", __FUNCTION__, irq, devname);
		return -ENXIO;
	}
	if (!(irq_tab[irq].flags & IRQ_FLG_STD)) {
		if (irq_tab[irq].flags & IRQ_FLG_LOCK) {
			printk("%s: IRQ %d from %s is not replaceable\n",
			       __FUNCTION__, irq, irq_tab[irq].devname);
			return -EBUSY;
		}
		if (flags & IRQ_FLG_REPLACE) {
			printk("%s: %s can't replace IRQ %d from %s\n",
			       __FUNCTION__, devname, irq, irq_tab[irq].devname);
			return -EBUSY;
		}
	}
	irq_tab[irq].handler = handler;
	irq_tab[irq].flags   = flags;
	irq_tab[irq].dev_id  = dev_id;
	irq_tab[irq].devname = devname;
	return 0;
}

void mvme147_free_irq(unsigned int irq, void *dev_id)
{
	if (irq > 255) {
		printk("%s: Incorrect IRQ %d\n", __FUNCTION__, irq);
		return;
	}
	if (irq_tab[irq].dev_id != dev_id)
		printk("%s: Removing probably wrong IRQ %d from %s\n",
		       __FUNCTION__, irq, irq_tab[irq].devname);

	irq_tab[irq].handler = mvme147_defhand;
	irq_tab[irq].flags   = IRQ_FLG_STD;
	irq_tab[irq].dev_id  = NULL;
	irq_tab[irq].devname = NULL;
}

void mvme147_process_int (unsigned long vec, struct pt_regs *fp)
{
	if (vec > 255)
		printk ("mvme147_process_int: Illegal vector %ld\n", vec);
	else
	{
		irq_tab[vec].count++;
		irq_tab[vec].handler(vec, irq_tab[vec].dev_id, fp);
	}
}

int mvme147_get_irq_list (char *buf)
{
	int i, len = 0;

	for (i = 0; i < 256; i++) {
		if (irq_tab[i].count)
			len += sprintf (buf+len, "Vec 0x%02x: %8d  %s\n",
			    i, irq_tab[i].count,
			    irq_tab[i].devname ? irq_tab[i].devname : "free");
	}
	return len;
}


static void mvme147_defhand (int irq, void *dev_id, struct pt_regs *fp)
{
	printk ("Unknown interrupt 0x%02x\n", irq);
}

void mvme147_enable_irq (unsigned int irq)
{
}


void mvme147_disable_irq (unsigned int irq)
{
}

