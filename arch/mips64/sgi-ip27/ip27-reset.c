/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Reset an IP27.
 *
 * Copyright (C) 1997, 1998, 1999, 2000 by Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 */
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/smp.h>
#include <linux/mmzone.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/sgialib.h>
#include <asm/sn/addrs.h>
#include <asm/sn/arch.h>
#include <asm/sn/gda.h>
#include <asm/sn/sn0/hub.h>

void machine_restart(char *command) __attribute__((noreturn));
void machine_halt(void) __attribute__((noreturn));
void machine_power_off(void) __attribute__((noreturn));

#define noreturn while(1);				/* Silence gcc.  */

/* XXX How to pass the reboot command to the firmware??? */
void machine_restart(char *command)
{
#if 0
	int i;
#endif

	printk("Reboot started from CPU %d\n", smp_processor_id());
#ifdef CONFIG_SMP
	smp_send_stop();
#endif
#if 0
	for (i = 0; i < numnodes; i++)
		REMOTE_HUB_S(COMPACT_TO_NASID_NODEID(i), PROMOP_REG, 
							PROMOP_REBOOT);
#else
	LOCAL_HUB_S(NI_PORT_RESET, NPR_PORTRESET | NPR_LOCALRESET);
#endif
	noreturn;
}

void machine_halt(void)
{
	int i;

#ifdef CONFIG_SMP
	smp_send_stop();
#endif
	for (i = 0; i < numnodes; i++)
		REMOTE_HUB_S(COMPACT_TO_NASID_NODEID(i), PROMOP_REG, 
							PROMOP_RESTART);
	LOCAL_HUB_S(NI_PORT_RESET, NPR_PORTRESET | NPR_LOCALRESET);
	noreturn;
}

void machine_power_off(void)
{
	/* To do ...  */
	noreturn;
}

void ip27_reboot_setup(void)
{
	/* Nothing to do on IP27.  */
}
