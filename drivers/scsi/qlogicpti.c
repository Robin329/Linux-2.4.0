/* qlogicpti.c: Performance Technologies QlogicISP sbus card driver.
 *
 * Copyright (C) 1996 David S. Miller (davem@caipfs.rutgers.edu)
 *
 * A lot of this driver was directly stolen from Erik H. Moe's PCI
 * Qlogic ISP driver.  Mucho kudos to him for this code.
 *
 * An even bigger kudos to John Grana at Performance Technologies
 * for providing me with the hardware to write this driver, you rule
 * John you really do.
 *
 * May, 2, 1997: Added support for QLGC,isp --jj
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/malloc.h>
#include <linux/blk.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/spinlock.h>

#include <asm/byteorder.h>

#include "scsi.h"
#include "hosts.h"
#include "qlogicpti.h"

#include <asm/sbus.h>
#include <asm/dma.h>
#include <asm/system.h>
#include <asm/ptrace.h>
#include <asm/pgtable.h>
#include <asm/oplib.h>
#include <asm/vaddrs.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <linux/module.h>

#define MAX_TARGETS	16
#define MAX_LUNS	8	/* 32 for 1.31 F/W */

#define DEFAULT_LOOP_COUNT	10000

#include "qlogicpti_asm.c"

static struct qlogicpti *qptichain = NULL;
static spinlock_t qptichain_lock = SPIN_LOCK_UNLOCKED;
static int qptis_running = 0;

#define PACKB(a, b)			(((a)<<4)|(b))

const u_char mbox_param[] = {
	PACKB(1, 1),	/* MBOX_NO_OP */
	PACKB(5, 5),	/* MBOX_LOAD_RAM */
	PACKB(2, 0),	/* MBOX_EXEC_FIRMWARE */
	PACKB(5, 5),	/* MBOX_DUMP_RAM */
	PACKB(3, 3),	/* MBOX_WRITE_RAM_WORD */
	PACKB(2, 3),	/* MBOX_READ_RAM_WORD */
	PACKB(6, 6),	/* MBOX_MAILBOX_REG_TEST */
	PACKB(2, 3),	/* MBOX_VERIFY_CHECKSUM	*/
	PACKB(1, 3),	/* MBOX_ABOUT_FIRMWARE */
	PACKB(0, 0),	/* 0x0009 */
	PACKB(0, 0),	/* 0x000a */
	PACKB(0, 0),	/* 0x000b */
	PACKB(0, 0),	/* 0x000c */
	PACKB(0, 0),	/* 0x000d */
	PACKB(1, 2),	/* MBOX_CHECK_FIRMWARE */
	PACKB(0, 0),	/* 0x000f */
	PACKB(5, 5),	/* MBOX_INIT_REQ_QUEUE */
	PACKB(6, 6),	/* MBOX_INIT_RES_QUEUE */
	PACKB(4, 4),	/* MBOX_EXECUTE_IOCB */
	PACKB(2, 2),	/* MBOX_WAKE_UP	*/
	PACKB(1, 6),	/* MBOX_STOP_FIRMWARE */
	PACKB(4, 4),	/* MBOX_ABORT */
	PACKB(2, 2),	/* MBOX_ABORT_DEVICE */
	PACKB(3, 3),	/* MBOX_ABORT_TARGET */
	PACKB(2, 2),	/* MBOX_BUS_RESET */
	PACKB(2, 3),	/* MBOX_STOP_QUEUE */
	PACKB(2, 3),	/* MBOX_START_QUEUE */
	PACKB(2, 3),	/* MBOX_SINGLE_STEP_QUEUE */
	PACKB(2, 3),	/* MBOX_ABORT_QUEUE */
	PACKB(2, 4),	/* MBOX_GET_DEV_QUEUE_STATUS */
	PACKB(0, 0),	/* 0x001e */
	PACKB(1, 3),	/* MBOX_GET_FIRMWARE_STATUS */
	PACKB(1, 2),	/* MBOX_GET_INIT_SCSI_ID */
	PACKB(1, 2),	/* MBOX_GET_SELECT_TIMEOUT */
	PACKB(1, 3),	/* MBOX_GET_RETRY_COUNT	*/
	PACKB(1, 2),	/* MBOX_GET_TAG_AGE_LIMIT */
	PACKB(1, 2),	/* MBOX_GET_CLOCK_RATE */
	PACKB(1, 2),	/* MBOX_GET_ACT_NEG_STATE */
	PACKB(1, 2),	/* MBOX_GET_ASYNC_DATA_SETUP_TIME */
	PACKB(1, 3),	/* MBOX_GET_SBUS_PARAMS */
	PACKB(2, 4),	/* MBOX_GET_TARGET_PARAMS */
	PACKB(2, 4),	/* MBOX_GET_DEV_QUEUE_PARAMS */
	PACKB(0, 0),	/* 0x002a */
	PACKB(0, 0),	/* 0x002b */
	PACKB(0, 0),	/* 0x002c */
	PACKB(0, 0),	/* 0x002d */
	PACKB(0, 0),	/* 0x002e */
	PACKB(0, 0),	/* 0x002f */
	PACKB(2, 2),	/* MBOX_SET_INIT_SCSI_ID */
	PACKB(2, 2),	/* MBOX_SET_SELECT_TIMEOUT */
	PACKB(3, 3),	/* MBOX_SET_RETRY_COUNT	*/
	PACKB(2, 2),	/* MBOX_SET_TAG_AGE_LIMIT */
	PACKB(2, 2),	/* MBOX_SET_CLOCK_RATE */
	PACKB(2, 2),	/* MBOX_SET_ACTIVE_NEG_STATE */
	PACKB(2, 2),	/* MBOX_SET_ASYNC_DATA_SETUP_TIME */
	PACKB(3, 3),	/* MBOX_SET_SBUS_CONTROL_PARAMS */
	PACKB(4, 4),	/* MBOX_SET_TARGET_PARAMS */
	PACKB(4, 4),	/* MBOX_SET_DEV_QUEUE_PARAMS */
	PACKB(0, 0),	/* 0x003a */
	PACKB(0, 0),	/* 0x003b */
	PACKB(0, 0),	/* 0x003c */
	PACKB(0, 0),	/* 0x003d */
	PACKB(0, 0),	/* 0x003e */
	PACKB(0, 0),	/* 0x003f */
	PACKB(0, 0),	/* 0x0040 */
	PACKB(0, 0),	/* 0x0041 */
	PACKB(0, 0)	/* 0x0042 */
};

#define MAX_MBOX_COMMAND	(sizeof(mbox_param)/sizeof(u_short))

/* queue length's _must_ be power of two: */
#define QUEUE_DEPTH(in, out, ql)	((in - out) & (ql))
#define REQ_QUEUE_DEPTH(in, out)	QUEUE_DEPTH(in, out, 		     \
						    QLOGICPTI_REQ_QUEUE_LEN)
#define RES_QUEUE_DEPTH(in, out)	QUEUE_DEPTH(in, out, RES_QUEUE_LEN)

static inline void qlogicpti_enable_irqs(struct qlogicpti *qpti)
{
	sbus_writew(SBUS_CTRL_ERIRQ | SBUS_CTRL_GENAB,
		    qpti->qregs + SBUS_CTRL);
}

static inline void qlogicpti_disable_irqs(struct qlogicpti *qpti)
{
	sbus_writew(0, qpti->qregs + SBUS_CTRL);
}

static inline void set_sbus_cfg1(struct qlogicpti *qpti)
{
	u16 val;
	u8 bursts = qpti->bursts;

#if 0	/* It appears that at least PTI cards do not support
	 * 64-byte bursts and that setting the B64 bit actually
	 * is a nop and the chip ends up using the smallest burst
	 * size. -DaveM
	 */
	if (sbus_can_burst64(qpti->sdev) && (bursts & DMA_BURST64)) {
		val = (SBUS_CFG1_BENAB | SBUS_CFG1_B64);
	} else
#endif
	if (bursts & DMA_BURST32) {
		val = (SBUS_CFG1_BENAB | SBUS_CFG1_B32);
	} else if (bursts & DMA_BURST16) {
		val = (SBUS_CFG1_BENAB | SBUS_CFG1_B16);
	} else if (bursts & DMA_BURST8) {
		val = (SBUS_CFG1_BENAB | SBUS_CFG1_B8);
	} else {
		val = 0; /* No sbus bursts for you... */
	}
	sbus_writew(val, qpti->qregs + SBUS_CFG1);
}

static int qlogicpti_mbox_command(struct qlogicpti *qpti, u_short param[], int force)
{
	int loop_count;
	u16 tmp;

	if (mbox_param[param[0]] == 0)
		return 1;

	/* Set SBUS semaphore. */
	tmp = sbus_readw(qpti->qregs + SBUS_SEMAPHORE);
	tmp |= SBUS_SEMAPHORE_LCK;
	sbus_writew(tmp, qpti->qregs + SBUS_SEMAPHORE);

	/* Wait for host IRQ bit to clear. */
	loop_count = DEFAULT_LOOP_COUNT;
	while (--loop_count && (sbus_readw(qpti->qregs + HCCTRL) & HCCTRL_HIRQ))
		barrier();
	if (!loop_count)
		printk(KERN_EMERG "qlogicpti: mbox_command loop timeout #1\n");

	/* Write mailbox command registers. */
	switch (mbox_param[param[0]] >> 4) {
	case 6: sbus_writew(param[5], qpti->qregs + MBOX5);
	case 5: sbus_writew(param[4], qpti->qregs + MBOX4);
	case 4: sbus_writew(param[3], qpti->qregs + MBOX3);
	case 3: sbus_writew(param[2], qpti->qregs + MBOX2);
	case 2: sbus_writew(param[1], qpti->qregs + MBOX1);
	case 1: sbus_writew(param[0], qpti->qregs + MBOX0);
	}

	/* Clear RISC interrupt. */
	tmp = sbus_readw(qpti->qregs + HCCTRL);
	tmp |= HCCTRL_CRIRQ;
	sbus_writew(tmp, qpti->qregs + HCCTRL);

	/* Clear SBUS semaphore. */
	sbus_writew(0, qpti->qregs + SBUS_SEMAPHORE);

	/* Set HOST interrupt. */
	tmp = sbus_readw(qpti->qregs + HCCTRL);
	tmp |= HCCTRL_SHIRQ;
	sbus_writew(tmp, qpti->qregs + HCCTRL);

	/* Wait for HOST interrupt clears. */
	loop_count = DEFAULT_LOOP_COUNT;
	while (--loop_count &&
	       (sbus_readw(qpti->qregs + HCCTRL) & HCCTRL_CRIRQ))
		udelay(20);
	if (!loop_count)
		printk(KERN_EMERG "qlogicpti: mbox_command[%04x] loop timeout #2\n",
		       param[0]);

	/* Wait for SBUS semaphore to get set. */
	loop_count = DEFAULT_LOOP_COUNT;
	while (--loop_count &&
	       !(sbus_readw(qpti->qregs + SBUS_SEMAPHORE) & SBUS_SEMAPHORE_LCK)) {
		udelay(20);

		/* Workaround for some buggy chips. */
		if (sbus_readw(qpti->qregs + MBOX0) & 0x4000)
			break;
	}
	if (!loop_count)
		printk(KERN_EMERG "qlogicpti: mbox_command[%04x] loop timeout #3\n",
		       param[0]);

	/* Wait for MBOX busy condition to go away. */
	loop_count = DEFAULT_LOOP_COUNT;
	while (--loop_count && (sbus_readw(qpti->qregs + MBOX0) == 0x04))
		udelay(20);
	if (!loop_count)
		printk(KERN_EMERG "qlogicpti: mbox_command[%04x] loop timeout #4\n",
		       param[0]);

	/* Read back output parameters. */
	switch (mbox_param[param[0]] & 0xf) {
	case 6: param[5] = sbus_readw(qpti->qregs + MBOX5);
	case 5: param[4] = sbus_readw(qpti->qregs + MBOX4);
	case 4: param[3] = sbus_readw(qpti->qregs + MBOX3);
	case 3: param[2] = sbus_readw(qpti->qregs + MBOX2);
	case 2: param[1] = sbus_readw(qpti->qregs + MBOX1);
	case 1: param[0] = sbus_readw(qpti->qregs + MBOX0);
	}

	/* Clear RISC interrupt. */
	tmp = sbus_readw(qpti->qregs + HCCTRL);
	tmp |= HCCTRL_CRIRQ;
	sbus_writew(tmp, qpti->qregs + HCCTRL);

	/* Release SBUS semaphore. */
	tmp = sbus_readw(qpti->qregs + SBUS_SEMAPHORE);
	tmp &= ~(SBUS_SEMAPHORE_LCK);
	sbus_writew(tmp, qpti->qregs + SBUS_SEMAPHORE);

	/* We're done. */
	return 0;
}

static inline void qlogicpti_set_hostdev_defaults(struct qlogicpti *qpti)
{
	int i;

	qpti->host_param.initiator_scsi_id = qpti->scsi_id;
	qpti->host_param.bus_reset_delay = 3;
	qpti->host_param.retry_count = 0;
	qpti->host_param.retry_delay = 5;
	qpti->host_param.async_data_setup_time = 3;
	qpti->host_param.req_ack_active_negation = 1;
	qpti->host_param.data_line_active_negation = 1;
	qpti->host_param.data_dma_burst_enable = 1;
	qpti->host_param.command_dma_burst_enable = 1;
	qpti->host_param.tag_aging = 8;
	qpti->host_param.selection_timeout = 250;
	qpti->host_param.max_queue_depth = 256;

	for(i = 0; i < MAX_TARGETS; i++) {
		/*
		 * disconnect, parity, arq, reneg on reset, and, oddly enough
		 * tags...the midlayer's notion of tagged support has to match
		 * our device settings, and since we base whether we enable a
		 * tag on a  per-cmnd basis upon what the midlayer sez, we
		 * actually enable the capability here.
		 */
		qpti->dev_param[i].device_flags = 0xcd;
		qpti->dev_param[i].execution_throttle = 16;
		if (qpti->ultra) {
			qpti->dev_param[i].synchronous_period = 12;
			qpti->dev_param[i].synchronous_offset = 8;
		} else {
			qpti->dev_param[i].synchronous_period = 25;
			qpti->dev_param[i].synchronous_offset = 12;
		}
		qpti->dev_param[i].device_enable = 1;
	}
	/* this is very important to set! */
	qpti->sbits = 1 << qpti->scsi_id;
}

static int qlogicpti_reset_hardware(struct Scsi_Host *host)
{
	struct qlogicpti *qpti = (struct qlogicpti *) host->hostdata;
	u_short param[6];
	unsigned short risc_code_addr;
	int loop_count, i;
	unsigned long flags;

	risc_code_addr = 0x1000;	/* all load addresses are at 0x1000 */

	save_flags(flags); cli();

	sbus_writew(HCCTRL_PAUSE, qpti->qregs + HCCTRL);

	/* Only reset the scsi bus if it is not free. */
	if (sbus_readw(qpti->qregs + CPU_PCTRL) & CPU_PCTRL_BSY) {
		sbus_writew(CPU_ORIDE_RMOD, qpti->qregs + CPU_ORIDE);
		sbus_writew(CPU_CMD_BRESET, qpti->qregs + CPU_CMD);
		udelay(400);
	}

	sbus_writew(SBUS_CTRL_RESET, qpti->qregs + SBUS_CTRL);
	sbus_writew((DMA_CTRL_CCLEAR | DMA_CTRL_CIRQ), qpti->qregs + CMD_DMA_CTRL);
	sbus_writew((DMA_CTRL_CCLEAR | DMA_CTRL_CIRQ), qpti->qregs + DATA_DMA_CTRL);

	loop_count = DEFAULT_LOOP_COUNT;
	while (--loop_count && ((sbus_readw(qpti->qregs + MBOX0) & 0xff) == 0x04))
		udelay(20);
	if (!loop_count)
		printk(KERN_EMERG "qlogicpti: reset_hardware loop timeout\n");

	sbus_writew(HCCTRL_PAUSE, qpti->qregs + HCCTRL);
	set_sbus_cfg1(qpti);
	qlogicpti_enable_irqs(qpti);

	if (sbus_readw(qpti->qregs + RISC_PSR) & RISC_PSR_ULTRA) {
		qpti->ultra = 1;
		sbus_writew((RISC_MTREG_P0ULTRA | RISC_MTREG_P1ULTRA),
			    qpti->qregs + RISC_MTREG);
	} else {
		qpti->ultra = 0;
		sbus_writew((RISC_MTREG_P0DFLT | RISC_MTREG_P1DFLT),
			    qpti->qregs + RISC_MTREG);
	}

	/* reset adapter and per-device default values. */
	/* do it after finding out whether we're ultra mode capable */
	qlogicpti_set_hostdev_defaults(qpti);

	/* Release the RISC processor. */
	sbus_writew(HCCTRL_REL, qpti->qregs + HCCTRL);

	/* Get RISC to start executing the firmware code. */
	param[0] = MBOX_EXEC_FIRMWARE;
	param[1] = risc_code_addr;
	if (qlogicpti_mbox_command(qpti, param, 1)) {
		printk(KERN_EMERG "qlogicpti%d: Cannot execute ISP firmware.\n",
		       qpti->qpti_id);
		restore_flags(flags);
		return 1;
	}

	/* Set initiator scsi ID. */
	param[0] = MBOX_SET_INIT_SCSI_ID;
	param[1] = qpti->host_param.initiator_scsi_id;
	if (qlogicpti_mbox_command(qpti, param, 1) ||
	   (param[0] != MBOX_COMMAND_COMPLETE)) {
		printk(KERN_EMERG "qlogicpti%d: Cannot set initiator SCSI ID.\n",
		       qpti->qpti_id);
		restore_flags(flags);
		return 1;
	}

	/* Initialize state of the queues, both hw and sw. */
	qpti->req_in_ptr = qpti->res_out_ptr = 0;

	param[0] = MBOX_INIT_RES_QUEUE;
	param[1] = RES_QUEUE_LEN + 1;
	param[2] = (u_short) (qpti->res_dvma >> 16);
	param[3] = (u_short) (qpti->res_dvma & 0xffff);
	param[4] = param[5] = 0;
	if (qlogicpti_mbox_command(qpti, param, 1)) {
		printk(KERN_EMERG "qlogicpti%d: Cannot init response queue.\n",
		       qpti->qpti_id);
		restore_flags(flags);
		return 1;
	}

	param[0] = MBOX_INIT_REQ_QUEUE;
	param[1] = QLOGICPTI_REQ_QUEUE_LEN + 1;
	param[2] = (u_short) (qpti->req_dvma >> 16);
	param[3] = (u_short) (qpti->req_dvma & 0xffff);
	param[4] = param[5] = 0;
	if (qlogicpti_mbox_command(qpti, param, 1)) {
		printk(KERN_EMERG "qlogicpti%d: Cannot init request queue.\n",
		       qpti->qpti_id);
		restore_flags(flags);
		return 1;
	}

	param[0] = MBOX_SET_RETRY_COUNT;
	param[1] = qpti->host_param.retry_count;
	param[2] = qpti->host_param.retry_delay;
	qlogicpti_mbox_command(qpti, param, 0);

	param[0] = MBOX_SET_TAG_AGE_LIMIT;
	param[1] = qpti->host_param.tag_aging;
	qlogicpti_mbox_command(qpti, param, 0);

	for (i = 0; i < MAX_TARGETS; i++) {
		param[0] = MBOX_GET_DEV_QUEUE_PARAMS;
		param[1] = (i << 8);
		qlogicpti_mbox_command(qpti, param, 0);
	}

	param[0] = MBOX_GET_FIRMWARE_STATUS;
	qlogicpti_mbox_command(qpti, param, 0);

	param[0] = MBOX_SET_SELECT_TIMEOUT;
	param[1] = qpti->host_param.selection_timeout;
	qlogicpti_mbox_command(qpti, param, 0);

	for (i = 0; i < MAX_TARGETS; i++) {
		param[0] = MBOX_SET_TARGET_PARAMS;
		param[1] = (i << 8);
		param[2] = (qpti->dev_param[i].device_flags << 8);
		/*
		 * Since we're now loading 1.31 f/w, force narrow/async.
		 */
		param[2] |= 0xc0;
		param[3] = 0;	/* no offset, we do not have sync mode yet */
		qlogicpti_mbox_command(qpti, param, 0);
	}

	/*
	 * Always (sigh) do an initial bus reset (kicks f/w).
	 */
	param[0] = MBOX_BUS_RESET;
	param[1] = qpti->host_param.bus_reset_delay;
	qlogicpti_mbox_command(qpti, param, 0);
	qpti->send_marker = 1;

	restore_flags(flags);
	return 0;
}

#define PTI_RESET_LIMIT 400

static int __init qlogicpti_load_firmware(struct qlogicpti *qpti)
{
	unsigned short csum = 0;
	unsigned short param[6];
	unsigned short *risc_code, risc_code_addr, risc_code_length;
	unsigned long flags;
	int i, timeout;

	risc_code = &sbus_risc_code01[0];
	risc_code_addr = 0x1000;	/* all f/w modules load at 0x1000 */
	risc_code_length = sbus_risc_code_length01;

	save_flags(flags); cli();

	/* Verify the checksum twice, one before loading it, and once
	 * afterwards via the mailbox commands.
	 */
	for (i = 0; i < risc_code_length; i++)
		csum += risc_code[i];
	if (csum) {
		restore_flags(flags);
		printk(KERN_EMERG "qlogicpti%d: Aieee, firmware checksum failed!",
		       qpti->qpti_id);
		return 1;
	}		
	sbus_writew(SBUS_CTRL_RESET, qpti->qregs + SBUS_CTRL);
	sbus_writew((DMA_CTRL_CCLEAR | DMA_CTRL_CIRQ), qpti->qregs + CMD_DMA_CTRL);
	sbus_writew((DMA_CTRL_CCLEAR | DMA_CTRL_CIRQ), qpti->qregs + DATA_DMA_CTRL);
	timeout = PTI_RESET_LIMIT;
	while (--timeout && (sbus_readw(qpti->qregs + SBUS_CTRL) & SBUS_CTRL_RESET))
		udelay(20);
	if (!timeout) {
		restore_flags(flags);
		printk(KERN_EMERG "qlogicpti%d: Cannot reset the ISP.", qpti->qpti_id);
		return 1;
	}

	sbus_writew(HCCTRL_RESET, qpti->qregs + HCCTRL);
	mdelay(1);

	sbus_writew((SBUS_CTRL_GENAB | SBUS_CTRL_ERIRQ), qpti->qregs + SBUS_CTRL);
	set_sbus_cfg1(qpti);
	sbus_writew(0, qpti->qregs + SBUS_SEMAPHORE);

	if (sbus_readw(qpti->qregs + RISC_PSR) & RISC_PSR_ULTRA) {
		qpti->ultra = 1;
		sbus_writew((RISC_MTREG_P0ULTRA | RISC_MTREG_P1ULTRA),
			    qpti->qregs + RISC_MTREG);
	} else {
		qpti->ultra = 0;
		sbus_writew((RISC_MTREG_P0DFLT | RISC_MTREG_P1DFLT),
			    qpti->qregs + RISC_MTREG);
	}

	sbus_writew(HCCTRL_REL, qpti->qregs + HCCTRL);

	/* Pin lines are only stable while RISC is paused. */
	sbus_writew(HCCTRL_PAUSE, qpti->qregs + HCCTRL);
	if (sbus_readw(qpti->qregs + CPU_PDIFF) & CPU_PDIFF_MODE)
		qpti->differential = 1;
	else
		qpti->differential = 0;
	sbus_writew(HCCTRL_REL, qpti->qregs + HCCTRL);

	/* This shouldn't be necessary- we've reset things so we should be
	   running from the ROM now.. */

	param[0] = MBOX_STOP_FIRMWARE;
	param[1] = param[2] = param[3] = param[4] = param[5] = 0;
	if (qlogicpti_mbox_command(qpti, param, 1)) {
		printk(KERN_EMERG "qlogicpti%d: Cannot stop firmware for reload.\n",
		       qpti->qpti_id);
		restore_flags(flags);
		return 1;
	}		

	/* Load it up.. */
	for (i = 0; i < risc_code_length; i++) {
		param[0] = MBOX_WRITE_RAM_WORD;
		param[1] = risc_code_addr + i;
		param[2] = risc_code[i];
		if (qlogicpti_mbox_command(qpti, param, 1) ||
		    param[0] != MBOX_COMMAND_COMPLETE) {
			printk("qlogicpti%d: Firmware dload failed, I'm bolixed!\n",
			       qpti->qpti_id);
			restore_flags(flags);
			return 1;
		}
	}

	/* Reset the ISP again. */
	sbus_writew(HCCTRL_RESET, qpti->qregs + HCCTRL);
	mdelay(1);

	qlogicpti_enable_irqs(qpti);
	sbus_writew(0, qpti->qregs + SBUS_SEMAPHORE);
	sbus_writew(HCCTRL_REL, qpti->qregs + HCCTRL);

	/* Ask ISP to verify the checksum of the new code. */
	param[0] = MBOX_VERIFY_CHECKSUM;
	param[1] = risc_code_addr;
	if (qlogicpti_mbox_command(qpti, param, 1) ||
	    (param[0] != MBOX_COMMAND_COMPLETE)) {
		printk(KERN_EMERG "qlogicpti%d: New firmware csum failure!\n",
		       qpti->qpti_id);
		restore_flags(flags);
		return 1;
	}

	/* Start using newly downloaded firmware. */
	param[0] = MBOX_EXEC_FIRMWARE;
	param[1] = risc_code_addr;
	qlogicpti_mbox_command(qpti, param, 1);

	param[0] = MBOX_ABOUT_FIRMWARE;
	if (qlogicpti_mbox_command(qpti, param, 1) ||
	    (param[0] != MBOX_COMMAND_COMPLETE)) {
		printk(KERN_EMERG "qlogicpti%d: AboutFirmware cmd fails.\n",
		       qpti->qpti_id);
		restore_flags(flags);
		return 1;
	}

	/* Snag the major and minor revisions from the result. */
	qpti->fware_majrev = param[1];
	qpti->fware_minrev = param[2];
	qpti->fware_micrev = param[3];

	/* Set the clock rate */
	param[0] = MBOX_SET_CLOCK_RATE;
	param[1] = qpti->clock;
	if (qlogicpti_mbox_command(qpti, param, 1) ||
	    (param[0] != MBOX_COMMAND_COMPLETE)) {
		printk(KERN_EMERG "qlogicpti%d: could not set clock rate.\n",
		       qpti->qpti_id);
		restore_flags(flags);
		return 1;
	}

	if (qpti->is_pti != 0) {
		/* Load scsi initiator ID and interrupt level into sbus static ram. */
		param[0] = MBOX_WRITE_RAM_WORD;
		param[1] = 0xff80;
		param[2] = (unsigned short) qpti->scsi_id;
		qlogicpti_mbox_command(qpti, param, 1);

		param[0] = MBOX_WRITE_RAM_WORD;
		param[1] = 0xff00;
		param[2] = (unsigned short) 3;
		qlogicpti_mbox_command(qpti, param, 1);
	}

	restore_flags(flags);
	return 0;
}

static int qlogicpti_verify_tmon(struct qlogicpti *qpti)
{
	int curstat = sbus_readb(qpti->sreg);

	curstat &= 0xf0;
	if (!(curstat & SREG_FUSE) && (qpti->swsreg & SREG_FUSE))
		printk("qlogicpti%d: Fuse returned to normal state.\n", qpti->qpti_id);
	if (!(curstat & SREG_TPOWER) && (qpti->swsreg & SREG_TPOWER))
		printk("qlogicpti%d: termpwr back to normal state.\n", qpti->qpti_id);
	if (curstat != qpti->swsreg) {
		int error = 0;
		if (curstat & SREG_FUSE) {
			error++;
			printk("qlogicpti%d: Fuse is open!\n", qpti->qpti_id);
		}
		if (curstat & SREG_TPOWER) {
			error++;
			printk("qlogicpti%d: termpwr failure\n", qpti->qpti_id);
		}
		if (qpti->differential &&
		    (curstat & SREG_DSENSE) != SREG_DSENSE) {
			error++;
			printk("qlogicpti%d: You have a single ended device on a "
			       "differential bus!  Please fix!\n", qpti->qpti_id);
		}
		qpti->swsreg = curstat;
		return error;
	}
	return 0;
}

static void qpti_intr(int irq, void *dev_id, struct pt_regs *regs);

static void __init qpti_chain_add(struct qlogicpti *qpti)
{
	spin_lock_irq(&qptichain_lock);
	if (qptichain != NULL) {
		struct qlogicpti *qlink = qptichain;

		while(qlink->next)
			qlink = qlink->next;
		qlink->next = qpti;
	} else {
		qptichain = qpti;
	}
	qpti->next = NULL;
	spin_unlock_irq(&qptichain_lock);
}

static void __init qpti_chain_del(struct qlogicpti *qpti)
{
	spin_lock_irq(&qptichain_lock);
	if (qptichain == qpti) {
		qptichain = qpti->next;
	} else {
		struct qlogicpti *qlink = qptichain;
		while(qlink->next != qpti)
			qlink = qlink->next;
		qlink->next = qpti->next;
	}
	qpti->next = NULL;
	spin_unlock_irq(&qptichain_lock);
}

static int __init qpti_map_regs(struct qlogicpti *qpti)
{
	struct sbus_dev *sdev = qpti->sdev;

	qpti->qregs = sbus_ioremap(&sdev->resource[0], 0,
				   sdev->reg_addrs[0].reg_size,
				   "PTI Qlogic/ISP");
	if (!qpti->qregs) {
		printk("PTI: Qlogic/ISP registers are unmappable\n");
		return -1;
	}
	if (qpti->is_pti) {
		qpti->sreg = sbus_ioremap(&sdev->resource[0], (16 * 4096),
					  sizeof(unsigned char),
					  "PTI Qlogic/ISP statreg");
		if (!qpti->sreg) {
			printk("PTI: Qlogic/ISP status register is unmappable\n");
			return -1;
		}
	}
	return 0;
}

static int __init qpti_register_irq(struct qlogicpti *qpti)
{
	struct sbus_dev *sdev = qpti->sdev;

	qpti->qhost->irq = qpti->irq = sdev->irqs[0];

	/* We used to try various overly-clever things to
	 * reduce the interrupt processing overhead on
	 * sun4c/sun4m when multiple PTI's shared the
	 * same IRQ.  It was too complex and messy to
	 * sanely maintain.
	 */
	if (request_irq(qpti->irq, qpti_intr,
			SA_SHIRQ, "Qlogic/PTI", qpti))
		goto fail;

	printk("qpti%d: IRQ %s ", qpti->qpti_id, __irq_itoa(qpti->irq));

	return 0;

fail:
	printk("qpti%d: Cannot acquire irq line\n", qpti->qpti_id);
	return -1;
}

static void __init qpti_get_scsi_id(struct qlogicpti *qpti)
{
	qpti->scsi_id = prom_getintdefault(qpti->prom_node,
					   "initiator-id",
					   -1);
	if (qpti->scsi_id == -1)
		qpti->scsi_id = prom_getintdefault(qpti->prom_node,
						   "scsi-initiator-id",
						   -1);
	if (qpti->scsi_id == -1)
		qpti->scsi_id =
			prom_getintdefault(qpti->sdev->bus->prom_node,
					   "scsi-initiator-id", 7);
	qpti->qhost->this_id = qpti->scsi_id;

	printk("SCSI ID %d ", qpti->scsi_id);
}

static void qpti_get_bursts(struct qlogicpti *qpti)
{
	struct sbus_dev *sdev = qpti->sdev;
	u8 bursts, bmask;

	bursts = prom_getintdefault(qpti->prom_node, "burst-sizes", 0xff);
	bmask = prom_getintdefault(sdev->bus->prom_node,
				   "burst-sizes", 0xff);
	if (bmask != 0xff)
		bursts &= bmask;
	if (bursts == 0xff ||
	    (bursts & DMA_BURST16) == 0 ||
	    (bursts & DMA_BURST32) == 0)
		bursts = (DMA_BURST32 - 1);

	qpti->bursts = bursts;
}

static void qpti_get_clock(struct qlogicpti *qpti)
{
	unsigned int cfreq;

	/* Check for what the clock input to this card is.
	 * Default to 40Mhz.
	 */
	cfreq = prom_getintdefault(qpti->prom_node,"clock-frequency",40000000);
	qpti->clock = (cfreq + 500000)/1000000;
	if (qpti->clock == 0) /* bullshit */
		qpti->clock = 40;
}

/* The request and response queues must each be aligned
 * on a page boundry.
 */
static int __init qpti_map_queues(struct qlogicpti *qpti)
{
	struct sbus_dev *sdev = qpti->sdev;

#define QSIZE(entries)	(((entries) + 1) * QUEUE_ENTRY_LEN)
	qpti->res_cpu = sbus_alloc_consistent(sdev,
					      QSIZE(RES_QUEUE_LEN),
					      &qpti->res_dvma);
	if (qpti->res_cpu == NULL ||
	    qpti->res_dvma == 0) {
		printk("QPTI: Cannot map response queue.\n");
		return -1;
	}

	qpti->req_cpu = sbus_alloc_consistent(sdev,
					      QSIZE(QLOGICPTI_REQ_QUEUE_LEN),
					      &qpti->req_dvma);
	if (qpti->req_cpu == NULL ||
	    qpti->req_dvma == 0) {
		sbus_free_consistent(sdev, QSIZE(RES_QUEUE_LEN),
				     qpti->res_cpu, qpti->res_dvma);
		printk("QPTI: Cannot map request queue.\n");
		return -1;
	}
	memset(qpti->res_cpu, 0, QSIZE(RES_QUEUE_LEN));
	memset(qpti->req_cpu, 0, QSIZE(QLOGICPTI_REQ_QUEUE_LEN));
	return 0;
}

/* Detect all PTI Qlogic ISP's in the machine. */
int __init qlogicpti_detect(Scsi_Host_Template *tpnt)
{
	struct qlogicpti *qpti;
	struct Scsi_Host *qpti_host;
	struct sbus_bus *sbus;
	struct sbus_dev *sdev;
	int nqptis = 0, nqptis_in_use = 0;

	tpnt->proc_name = "qlogicpti";
	for_each_sbus(sbus) {
		for_each_sbusdev(sdev, sbus) {
			/* Is this a red snapper? */
			if (strcmp(sdev->prom_name, "ptisp") &&
			    strcmp(sdev->prom_name, "PTI,ptisp") &&
			    strcmp(sdev->prom_name, "QLGC,isp"))
				continue;

			/* Sometimes Antares cards come up not completely
			 * setup, and we get a report of a zero IRQ.
			 * Skip over them in such cases so we survive.
			 */
			if (sdev->irqs[0] == 0) {
				printk("qpti%d: Adapter reports no interrupt, "
				       "skipping over this card.", nqptis);
				continue;
			}

			/* Yep, register and allocate software state. */
			qpti_host = scsi_register(tpnt, sizeof(struct qlogicpti));
			if (!qpti_host) {
				printk("QPTI: Cannot register PTI Qlogic ISP SCSI host");
				continue;
			}
			qpti = (struct qlogicpti *) qpti_host->hostdata;

			spin_lock_init(&qpti->lock);

			/* We are wide capable, 16 targets. */
			qpti_host->max_id = MAX_TARGETS;

			/* Setup back pointers and misc. state. */
			qpti->qhost = qpti_host;
			qpti->sdev = sdev;
			qpti->qpti_id = nqptis++;
			qpti->prom_node = sdev->prom_node;
			prom_getstring(qpti->prom_node, "name",
				       qpti->prom_name,
				       sizeof(qpti->prom_name));

			/* This is not correct, actually. There's a switch
			 * on the PTI cards that put them into "emulation"
			 * mode- i.e., report themselves as QLGC,isp
			 * instead of PTI,ptisp. The only real substantive
			 * difference between non-pti and pti cards is
			 * the tmon register. Which is possibly even
			 * there for Qlogic cards, but non-functional.
			 */
			qpti->is_pti = (strcmp (qpti->prom_name, "QLGC,isp") != 0);

			qpti_chain_add(qpti);
			if (qpti_map_regs(qpti) < 0)
				goto fail_unlink;

			if (qpti_register_irq(qpti) < 0)
				goto fail_unmap_regs;

			qpti_get_scsi_id(qpti);
			qpti_get_bursts(qpti);
			qpti_get_clock(qpti);

			/* Clear out Scsi_Cmnd array. */
			memset(qpti->cmd_slots, 0, sizeof(qpti->cmd_slots));

			if (qpti_map_queues(qpti) < 0)
				goto fail_free_irq;

			/* Load the firmware. */
			if (qlogicpti_load_firmware(qpti))
				goto fail_unmap_queues;
			if (qpti->is_pti) {
				/* Check the PTI status reg. */
				if (qlogicpti_verify_tmon(qpti))
					goto fail_unmap_queues;
			}

			/* Reset the ISP and init res/req queues. */
			if (qlogicpti_reset_hardware(qpti_host))
				goto fail_unmap_queues;

			printk("(Firmware v%d.%d.%d)", qpti->fware_majrev,
			    qpti->fware_minrev, qpti->fware_micrev);
			{
				char buffer[60];
				
				prom_getstring (qpti->prom_node,
						"isp-fcode", buffer, 60);
				if (buffer[0])
					printk("(Firmware %s)", buffer);
				if (prom_getbool(qpti->prom_node, "differential"))
					qpti->differential = 1;
			}
			
			printk (" [%s Wide, using %s interface]\n",
			       (qpti->ultra ? "Ultra" : "Fast"),
			       (qpti->differential ? "differential" : "single ended"));

			nqptis_in_use++;
			continue;

		fail_unmap_queues:
#define QSIZE(entries)	(((entries) + 1) * QUEUE_ENTRY_LEN)
			sbus_free_consistent(qpti->sdev,
					     QSIZE(RES_QUEUE_LEN),
					     qpti->res_cpu, qpti->res_dvma);
			sbus_free_consistent(qpti->sdev,
					     QSIZE(QLOGICPTI_REQ_QUEUE_LEN),
					     qpti->req_cpu, qpti->req_dvma);
#undef QSIZE
		fail_free_irq:
			free_irq(qpti->irq, qpti);

		fail_unmap_regs:
			sbus_iounmap(qpti->qregs,
				     qpti->sdev->reg_addrs[0].reg_size);
			if (qpti->is_pti)
				sbus_iounmap(qpti->sreg, sizeof(unsigned char));
		fail_unlink:
			qpti_chain_del(qpti);
			scsi_unregister(qpti->qhost);
		}
	}
	if (nqptis)
		printk("QPTI: Total of %d PTI Qlogic/ISP hosts found, %d actually in use.\n",
		       nqptis, nqptis_in_use);
	qptis_running = nqptis_in_use;
	return nqptis;
}

int qlogicpti_release(struct Scsi_Host *host)
{
	struct qlogicpti *qpti = (struct qlogicpti *) host->hostdata;

	/* Remove visibility from IRQ handlers. */
	qpti_chain_del(qpti);

	/* Shut up the card. */
	sbus_writew(0, qpti->qregs + SBUS_CTRL);

	/* Free IRQ handler and unmap Qlogic,ISP and PTI status regs. */
	free_irq(qpti->irq, qpti);

#define QSIZE(entries)	(((entries) + 1) * QUEUE_ENTRY_LEN)
	sbus_free_consistent(qpti->sdev,
			     QSIZE(RES_QUEUE_LEN),
			     qpti->res_cpu, qpti->res_dvma);
	sbus_free_consistent(qpti->sdev,
			     QSIZE(QLOGICPTI_REQ_QUEUE_LEN),
			     qpti->req_cpu, qpti->req_dvma);
#undef QSIZE

	sbus_iounmap(qpti->qregs, qpti->sdev->reg_addrs[0].reg_size);
	if (qpti->is_pti)
		sbus_iounmap(qpti->sreg, sizeof(unsigned char));

	return 0;
}

const char *qlogicpti_info(struct Scsi_Host *host)
{
	static char buf[80];
	struct qlogicpti *qpti = (struct qlogicpti *) host->hostdata;

	sprintf(buf, "PTI Qlogic,ISP SBUS SCSI irq %s regs at %lx",
		__irq_itoa(qpti->qhost->irq), qpti->qregs);
	return buf;
}

/* I am a certified frobtronicist. */
static inline void marker_frob(struct Command_Entry *cmd)
{
	struct Marker_Entry *marker = (struct Marker_Entry *) cmd;

	memset(marker, 0, sizeof(struct Marker_Entry));
	marker->hdr.entry_cnt = 1;
	marker->hdr.entry_type = ENTRY_MARKER;
	marker->modifier = SYNC_ALL;
	marker->rsvd = 0;
}

static inline void cmd_frob(struct Command_Entry *cmd, Scsi_Cmnd *Cmnd,
			    struct qlogicpti *qpti)
{
	memset(cmd, 0, sizeof(struct Command_Entry));
	cmd->hdr.entry_cnt = 1;
	cmd->hdr.entry_type = ENTRY_COMMAND;
	cmd->target_id = Cmnd->target;
	cmd->target_lun = Cmnd->lun;
	cmd->cdb_length = Cmnd->cmd_len;
	cmd->control_flags = 0;
	if (Cmnd->device->tagged_supported) {
		if (qpti->cmd_count[Cmnd->target] == 0)
			qpti->tag_ages[Cmnd->target] = jiffies;
		if ((jiffies - qpti->tag_ages[Cmnd->target]) > (5*HZ)) {
			cmd->control_flags = CFLAG_ORDERED_TAG;
			qpti->tag_ages[Cmnd->target] = jiffies;
		} else
			cmd->control_flags = CFLAG_SIMPLE_TAG;
	}
	if ((Cmnd->cmnd[0] == WRITE_6) ||
	    (Cmnd->cmnd[0] == WRITE_10) ||
	    (Cmnd->cmnd[0] == WRITE_12))
		cmd->control_flags |= CFLAG_WRITE;
	else
		cmd->control_flags |= CFLAG_READ;
	cmd->time_out = 30;
	memcpy(cmd->cdb, Cmnd->cmnd, Cmnd->cmd_len);
}

/* Do it to it baby. */
static inline int load_cmd(Scsi_Cmnd *Cmnd, struct Command_Entry *cmd,
			   struct qlogicpti *qpti, u_int in_ptr, u_int out_ptr)
{
	struct dataseg *ds;
	struct scatterlist *sg;
	int i, n;

	if (Cmnd->use_sg) {
		int sg_count;

		sg = (struct scatterlist *) Cmnd->buffer;
		sg_count = sbus_map_sg(qpti->sdev, sg, Cmnd->use_sg, scsi_to_sbus_dma_dir(Cmnd->sc_data_direction));

		ds = cmd->dataseg;
		cmd->segment_cnt = sg_count;

		/* Fill in first four sg entries: */
		n = sg_count;
		if (n > 4)
			n = 4;
		for (i = 0; i < n; i++, sg++) {
			ds[i].d_base = sg_dma_address(sg);
			ds[i].d_count = sg_dma_len(sg);
		}
		sg_count -= 4;
		while (sg_count > 0) {
			struct Continuation_Entry *cont;

			++cmd->hdr.entry_cnt;
			cont = (struct Continuation_Entry *) &qpti->req_cpu[in_ptr];
			in_ptr = NEXT_REQ_PTR(in_ptr);
			if (in_ptr == out_ptr)
				return -1;

			cont->hdr.entry_type = ENTRY_CONTINUATION;
			cont->hdr.entry_cnt = 0;
			cont->hdr.sys_def_1 = 0;
			cont->hdr.flags = 0;
			cont->reserved = 0;
			ds = cont->dataseg;
			n = sg_count;
			if (n > 7)
				n = 7;
			for (i = 0; i < n; i++, sg++) {
				ds[i].d_base = sg_dma_address(sg);
				ds[i].d_count = sg_dma_len(sg);
			}
			sg_count -= n;
		}
	} else if (Cmnd->request_bufflen) {
		Cmnd->SCp.ptr = (char *)(unsigned long)
			sbus_map_single(qpti->sdev,
					Cmnd->request_buffer,
					Cmnd->request_bufflen,
					scsi_to_sbus_dma_dir(Cmnd->sc_data_direction));

		cmd->dataseg[0].d_base = (u32) ((unsigned long)Cmnd->SCp.ptr);
		cmd->dataseg[0].d_count = Cmnd->request_bufflen;
		cmd->segment_cnt = 1;
	} else {
		cmd->dataseg[0].d_base = 0;
		cmd->dataseg[0].d_count = 0;
		cmd->segment_cnt = 1; /* Shouldn't this be 0? */
	}

	/* Committed, record Scsi_Cmd so we can find it later. */
	cmd->handle = in_ptr;
	qpti->cmd_slots[in_ptr] = Cmnd;

	qpti->cmd_count[Cmnd->target]++;
	sbus_writew(in_ptr, qpti->qregs + MBOX4);
	qpti->req_in_ptr = in_ptr;

	return in_ptr;
}

static inline void update_can_queue(struct Scsi_Host *host, u_int in_ptr, u_int out_ptr)
{
	/* Temporary workaround until bug is found and fixed (one bug has been found
	   already, but fixing it makes things even worse) -jj */
	int num_free = QLOGICPTI_REQ_QUEUE_LEN - REQ_QUEUE_DEPTH(in_ptr, out_ptr) - 64;
	host->can_queue = host->host_busy + num_free;
	host->sg_tablesize = QLOGICPTI_MAX_SG(num_free);
}

/*
 * Until we scan the entire bus with inquiries, go throught this fella...
 */
static void ourdone(Scsi_Cmnd *Cmnd)
{
	struct qlogicpti *qpti = (struct qlogicpti *) Cmnd->host->hostdata;
	int tgt = Cmnd->target;
	void (*done) (Scsi_Cmnd *);

	/* This grot added by DaveM, blame him for ugliness.
	 * The issue is that in the 2.3.x driver we use the
	 * host_scribble portion of the scsi command as a
	 * completion linked list at interrupt service time,
	 * so we have to store the done function pointer elsewhere.
	 */
	done = (void (*)(Scsi_Cmnd *))
		(((unsigned long) Cmnd->SCp.Message)
#ifdef __sparc_v9__
		 | ((unsigned long) Cmnd->SCp.Status << 32UL)
#endif
		 );

	if ((qpti->sbits & (1 << tgt)) == 0) {
		int ok = host_byte(Cmnd->result) == DID_OK;
		if (Cmnd->cmnd[0] == 0x12 && ok) {
			unsigned char *iqd;
			if (Cmnd->use_sg == 0) {
				iqd = ((unsigned char *)Cmnd->buffer);
			} else {
				iqd = ((struct scatterlist *) Cmnd->request_buffer)->address;
			}
			/* tags handled in midlayer */
			/* enable sync mode? */
			if (iqd[7] & 0x10) {
				qpti->dev_param[tgt].device_flags |= 0x10;
			} else {
				qpti->dev_param[tgt].synchronous_offset = 0;
				qpti->dev_param[tgt].synchronous_period = 0;
			}
			/* are we wide capable? */
			if (iqd[7] & 0x20) {
				qpti->dev_param[tgt].device_flags |= 0x20;
			}
			qpti->sbits |= (1 << tgt);
		} else if (!ok) {
			qpti->sbits |= (1 << tgt);
		}
	}
	done(Cmnd);
}

int qlogicpti_queuecommand_slow(Scsi_Cmnd *Cmnd, void (*done)(Scsi_Cmnd *))
{
	unsigned long flags;
	struct qlogicpti *qpti = (struct qlogicpti *) Cmnd->host->hostdata;

	/*
	 * done checking this host adapter?
	 * If not, then rewrite the command
	 * to finish through ourdone so we
	 * can peek at Inquiry data results.
	 */
	if (qpti->sbits && qpti->sbits != 0xffff) {
		/* See above about in ourdone this ugliness... */
		Cmnd->SCp.Message = ((unsigned long)done) & 0xffffffff;
#ifdef __sparc_v9__
		Cmnd->SCp.Status = ((unsigned long)done >> 32UL) & 0xffffffff;
#endif
		return qlogicpti_queuecommand(Cmnd, ourdone);
	}
	save_flags(flags); cli();

	/*
	 * We've peeked at all targets for this bus- time
	 * to set parameters for devices for real now.
	 */
	if (qpti->sbits == 0xffff) {
		int i;
		for(i = 0; i < MAX_TARGETS; i++) {
			u_short param[6];
			param[0] = MBOX_SET_TARGET_PARAMS;
			param[1] = (i << 8);
			param[2] = (qpti->dev_param[i].device_flags << 8);
			if (qpti->dev_param[i].device_flags & 0x10) {
				param[3] = (qpti->dev_param[i].synchronous_offset << 8) |
					qpti->dev_param[i].synchronous_period;
			} else {
				param[3] = 0;
			}
			(void) qlogicpti_mbox_command(qpti, param, 0);
		}
		/*
		 * set to zero so any traverse through ourdone
		 * doesn't start the whole process again,
		 */
		qpti->sbits = 0;
	}

	/* check to see if we're done with all adapters... */
	for (qpti = qptichain; qpti != NULL; qpti = qpti->next) {
		if (qpti->sbits) {
			break;
		}
	}

	/*
	 * if we hit the end of the chain w/o finding adapters still
	 * capability-configuring, then we're done with all adapters
	 * and can rock on..
	 */
	if (qpti == NULL)
		Cmnd->host->hostt->queuecommand = qlogicpti_queuecommand;

	restore_flags(flags);
	return qlogicpti_queuecommand(Cmnd, done);
}

/*
 * The middle SCSI layer ensures that queuecommand never gets invoked
 * concurrently with itself or the interrupt handler (though the
 * interrupt handler may call this routine as part of
 * request-completion handling).
 *
 * "This code must fly." -davem
 */
int qlogicpti_queuecommand(Scsi_Cmnd *Cmnd, void (*done)(Scsi_Cmnd *))
{
	struct Scsi_Host *host = Cmnd->host;
	struct qlogicpti *qpti = (struct qlogicpti *) host->hostdata;
	struct Command_Entry *cmd;
	unsigned long flags;
	u_int out_ptr;
	int in_ptr;

	Cmnd->scsi_done = done;

	spin_lock_irqsave(&qpti->lock, flags);

	in_ptr = qpti->req_in_ptr;
	cmd = (struct Command_Entry *) &qpti->req_cpu[in_ptr];
	out_ptr = sbus_readw(qpti->qregs + MBOX4);
	in_ptr = NEXT_REQ_PTR(in_ptr);
	if (in_ptr == out_ptr)
		goto toss_command;

	if (qpti->send_marker) {
		marker_frob(cmd);
		qpti->send_marker = 0;
		if (NEXT_REQ_PTR(in_ptr) == out_ptr) {
			sbus_writew(in_ptr, qpti->qregs + MBOX4);
			qpti->req_in_ptr = in_ptr;
			goto toss_command;
		}
		cmd = (struct Command_Entry *) &qpti->req_cpu[in_ptr];
		in_ptr = NEXT_REQ_PTR(in_ptr);
	}
	cmd_frob(cmd, Cmnd, qpti);
	if ((in_ptr = load_cmd(Cmnd, cmd, qpti, in_ptr, out_ptr)) == -1)
		goto toss_command;

	update_can_queue(host, in_ptr, out_ptr);

	spin_unlock_irqrestore(&qpti->lock, flags);
	return 0;

toss_command:
	printk(KERN_EMERG "qlogicpti%d: request queue overflow\n",
	       qpti->qpti_id);

	spin_unlock_irqrestore(&qpti->lock, flags);

	/* Unfortunately, unless you use the new EH code, which
	 * we don't, the midlayer will ignore the return value,
	 * which is insane.  We pick up the pieces like this.
	 */
	Cmnd->result = DID_BUS_BUSY;
	done(Cmnd);
	return 1;
}

static int qlogicpti_return_status(struct Status_Entry *sts, int id)
{
	int host_status = DID_ERROR;

	switch (sts->completion_status) {
	      case CS_COMPLETE:
		host_status = DID_OK;
		break;
	      case CS_INCOMPLETE:
		if (!(sts->state_flags & SF_GOT_BUS))
			host_status = DID_NO_CONNECT;
		else if (!(sts->state_flags & SF_GOT_TARGET))
			host_status = DID_BAD_TARGET;
		else if (!(sts->state_flags & SF_SENT_CDB))
			host_status = DID_ERROR;
		else if (!(sts->state_flags & SF_TRANSFERRED_DATA))
			host_status = DID_ERROR;
		else if (!(sts->state_flags & SF_GOT_STATUS))
			host_status = DID_ERROR;
		else if (!(sts->state_flags & SF_GOT_SENSE))
			host_status = DID_ERROR;
		break;
	      case CS_DMA_ERROR:
	      case CS_TRANSPORT_ERROR:
		host_status = DID_ERROR;
		break;
	      case CS_RESET_OCCURRED:
	      case CS_BUS_RESET:
		host_status = DID_RESET;
		break;
	      case CS_ABORTED:
		host_status = DID_ABORT;
		break;
	      case CS_TIMEOUT:
		host_status = DID_TIME_OUT;
		break;
	      case CS_DATA_OVERRUN:
	      case CS_COMMAND_OVERRUN:
	      case CS_STATUS_OVERRUN:
	      case CS_BAD_MESSAGE:
	      case CS_NO_MESSAGE_OUT:
	      case CS_EXT_ID_FAILED:
	      case CS_IDE_MSG_FAILED:
	      case CS_ABORT_MSG_FAILED:
	      case CS_NOP_MSG_FAILED:
	      case CS_PARITY_ERROR_MSG_FAILED:
	      case CS_DEVICE_RESET_MSG_FAILED:
	      case CS_ID_MSG_FAILED:
	      case CS_UNEXP_BUS_FREE:
		host_status = DID_ERROR;
		break;
	      case CS_DATA_UNDERRUN:
		host_status = DID_OK;
		break;
	      default:
		printk(KERN_EMERG "qpti%d: unknown completion status 0x%04x\n",
		       id, sts->completion_status);
		host_status = DID_ERROR;
		break;
	}

	return (sts->scsi_status & STATUS_MASK) | (host_status << 16);
}

static Scsi_Cmnd *qlogicpti_intr_handler(struct qlogicpti *qpti)
{
	Scsi_Cmnd *Cmnd, *done_queue = NULL;
	struct Status_Entry *sts;
	u_int in_ptr, out_ptr;

	if (!(sbus_readw(qpti->qregs + SBUS_STAT) & SBUS_STAT_RINT))
		return NULL;
		
	in_ptr = sbus_readw(qpti->qregs + MBOX5);
	sbus_writew(HCCTRL_CRIRQ, qpti->qregs + HCCTRL);
	if (sbus_readw(qpti->qregs + SBUS_SEMAPHORE) & SBUS_SEMAPHORE_LCK) {
		switch (sbus_readw(qpti->qregs + MBOX0)) {
		case ASYNC_SCSI_BUS_RESET:
		case EXECUTION_TIMEOUT_RESET:
			qpti->send_marker = 1;
			break;
		case INVALID_COMMAND:
		case HOST_INTERFACE_ERROR:
		case COMMAND_ERROR:
		case COMMAND_PARAM_ERROR:
			break;
		};
		sbus_writew(0, qpti->qregs + SBUS_SEMAPHORE);
	}

	/* This looks like a network driver! */
	out_ptr = qpti->res_out_ptr;
	while (out_ptr != in_ptr) {
		u_int cmd_slot;

		sts = (struct Status_Entry *) &qpti->res_cpu[out_ptr];
		out_ptr = NEXT_RES_PTR(out_ptr);

		/* We store an index in the handle, not the pointer in
		 * some form.  This avoids problems due to the fact
		 * that the handle provided is only 32-bits. -DaveM
		 */
		cmd_slot = sts->handle;
		Cmnd = qpti->cmd_slots[cmd_slot];
		qpti->cmd_slots[cmd_slot] = NULL;

		if (sts->completion_status == CS_RESET_OCCURRED ||
		    sts->completion_status == CS_ABORTED ||
		    (sts->status_flags & STF_BUS_RESET))
			qpti->send_marker = 1;

		if (sts->state_flags & SF_GOT_SENSE)
			memcpy(Cmnd->sense_buffer, sts->req_sense_data,
			       sizeof(Cmnd->sense_buffer));

		if (sts->hdr.entry_type == ENTRY_STATUS)
			Cmnd->result =
			    qlogicpti_return_status(sts, qpti->qpti_id);
		else
			Cmnd->result = DID_ERROR << 16;

		if (Cmnd->use_sg) {
			sbus_unmap_sg(qpti->sdev,
				      (struct scatterlist *)Cmnd->buffer,
				      Cmnd->use_sg,
				      scsi_to_sbus_dma_dir(Cmnd->sc_data_direction));
		} else {
			sbus_unmap_single(qpti->sdev,
					  (__u32)((unsigned long)Cmnd->SCp.ptr),
					  Cmnd->request_bufflen,
					  scsi_to_sbus_dma_dir(Cmnd->sc_data_direction));
		}
		qpti->cmd_count[Cmnd->target]--;
		sbus_writew(out_ptr, qpti->qregs + MBOX5);
		Cmnd->host_scribble = (unsigned char *) done_queue;
		done_queue = Cmnd;
	}
	qpti->res_out_ptr = out_ptr;

	return done_queue;
}

static void qpti_intr(int irq, void *dev_id, struct pt_regs *regs)
{
	struct qlogicpti *qpti = dev_id;
	unsigned long flags;
	Scsi_Cmnd *dq;

	spin_lock_irqsave(&qpti->lock, flags);
	dq = qlogicpti_intr_handler(qpti);
	spin_unlock(&qpti->lock);

	if (dq != NULL) {
		spin_lock(&io_request_lock);
		do {
			Scsi_Cmnd *next;

			next = (Scsi_Cmnd *) dq->host_scribble;
			dq->scsi_done(dq);
			dq = next;
		} while (dq != NULL);
		spin_unlock(&io_request_lock);
	}
	__restore_flags(flags);
}

int qlogicpti_abort(Scsi_Cmnd *Cmnd)
{
	u_short param[6];
	struct Scsi_Host *host = Cmnd->host;
	struct qlogicpti *qpti = (struct qlogicpti *) host->hostdata;
	int return_status = SCSI_ABORT_SUCCESS;
	unsigned long flags;
	u32 cmd_cookie;
	int i;

	printk(KERN_WARNING "qlogicpti : Aborting cmd for tgt[%d] lun[%d]\n",
	       (int)Cmnd->target, (int)Cmnd->lun);

	spin_lock_irqsave(&qpti->lock, flags);

	qlogicpti_disable_irqs(qpti);

	/* Find the 32-bit cookie we gave to the firmware for
	 * this command.
	 */
	for (i = 0; i < QLOGICPTI_REQ_QUEUE_LEN + 1; i++)
		if (qpti->cmd_slots[i] == Cmnd)
			break;
	cmd_cookie = i;

	param[0] = MBOX_ABORT;
	param[1] = (((u_short) Cmnd->target) << 8) | Cmnd->lun;
	param[2] = cmd_cookie >> 16;
	param[3] = cmd_cookie & 0xffff;
	if (qlogicpti_mbox_command(qpti, param, 0) ||
	    (param[0] != MBOX_COMMAND_COMPLETE)) {
		printk(KERN_EMERG "qlogicpti : scsi abort failure: %x\n", param[0]);
		return_status = SCSI_ABORT_ERROR;
	}

	qlogicpti_enable_irqs(qpti);

	spin_unlock_irqrestore(&qpti->lock, flags);

	return return_status;
}

int qlogicpti_reset(Scsi_Cmnd *Cmnd, unsigned int reset_flags)
{
	u_short param[6];
	struct Scsi_Host *host = Cmnd->host;
	struct qlogicpti *qpti = (struct qlogicpti *) host->hostdata;
	int return_status = SCSI_RESET_SUCCESS;
	unsigned long flags;

	printk(KERN_WARNING "qlogicpti : Resetting SCSI bus!\n");

	spin_lock_irqsave(&qpti->lock, flags);

	qlogicpti_disable_irqs(qpti);

	param[0] = MBOX_BUS_RESET;
	param[1] = qpti->host_param.bus_reset_delay;
	if (qlogicpti_mbox_command(qpti, param, 0) ||
	   (param[0] != MBOX_COMMAND_COMPLETE)) {
		printk(KERN_EMERG "qlogicisp : scsi bus reset failure: %x\n", param[0]);
		return_status = SCSI_RESET_ERROR;
	}

	qlogicpti_enable_irqs(qpti);

	spin_unlock_irqrestore(&qpti->lock, flags);

	return return_status;
}

static Scsi_Host_Template driver_template = QLOGICPTI;

#include "scsi_module.c"

EXPORT_NO_SYMBOLS;
