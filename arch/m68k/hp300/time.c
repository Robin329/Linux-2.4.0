/*
 *  linux/arch/m68k/hp300/time.c
 *
 *  Copyright (C) 1998 Philip Blundell <philb@gnu.org>
 *
 *  This file contains the HP300-specific time handling code.
 */

#include <asm/ptrace.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel_stat.h>
#include <linux/interrupt.h>
#include <asm/machdep.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/traps.h>
#include "ints.h"

/* Clock hardware definitions */

#define CLOCKBASE	0xf05f8000

#define	CLKCR1		0x1
#define	CLKCR2		0x3
#define	CLKCR3		CLKCR1
#define	CLKSR		CLKCR2
#define	CLKMSB1		0x5
#define	CLKMSB2		0x9
#define	CLKMSB3		0xD

/* This is for machines which generate the exact clock. */
#define USECS_PER_JIFFY (1000000/HZ)

#define INTVAL ((10000 / 4) - 1)

static void hp300_tick(int irq, void *dev_id, struct pt_regs *regs)
{
  unsigned long tmp;
  void (*vector)(int, void *, struct pt_regs *) = dev_id;
  readb(CLOCKBASE + CLKSR);
  asm volatile ("movpw %1@(5),%0" : "=r" (tmp) : "a" (CLOCKBASE));
  vector(irq, NULL, regs);
}

unsigned long hp300_gettimeoffset(void)
{
  /* Read current timer 1 value */
  unsigned char lsb, msb1, msb2;
  unsigned short ticks;

  msb1 = readb(CLOCKBASE + 5);
  lsb = readb(CLOCKBASE + 7);
  msb2 = readb(CLOCKBASE + 5);
  if (msb1 != msb2)
    /* A carry happened while we were reading.  Read it again */
    lsb = readb(CLOCKBASE + 7);
  ticks = INTVAL - ((msb2 << 8) | lsb);
  return (USECS_PER_JIFFY * ticks) / INTVAL;
}

void __init hp300_sched_init(void (*vector)(int, void *, struct pt_regs *))
{
  writeb(0x1, CLOCKBASE + CLKCR2);		/* select CR1 */
  writeb(0x1, CLOCKBASE + CLKCR1);		/* reset */

  asm volatile(" movpw %0,%1@(5)" : : "d" (INTVAL), "a" (CLOCKBASE));

  sys_request_irq(6, hp300_tick, IRQ_FLG_STD, "timer tick", vector);

  writeb(0x1, CLOCKBASE + CLKCR2);		/* select CR1 */
  writeb(0x40, CLOCKBASE + CLKCR1);		/* enable irq */
}
