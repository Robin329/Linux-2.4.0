/*
 * Miscellaneous Mac68K-specific stuff 
 */

#include <stdarg.h>
#include <linux/config.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/malloc.h>
#include <linux/time.h>
#include <linux/kd.h>
#include <linux/mm.h>

#include <linux/adb.h>
#include <linux/cuda.h>
#include <linux/pmu.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/segment.h>
#include <asm/setup.h>
#include <asm/macintosh.h>
#include <asm/mac_via.h>
#include <asm/mac_oss.h>

#define BOOTINFO_COMPAT_1_0
#include <asm/bootinfo.h>
#include <asm/machdep.h>

/* Offset between Unix time (1970-based) and Mac time (1904-based) */

#define RTC_OFFSET 2082844800

extern struct mac_booter_data mac_bi_data;
static void (*rom_reset)(void);

/*
 * Return the current time as the number of seconds since January 1, 1904.
 */

static long adb_read_time(void)
{
	volatile struct adb_request req;
	long time;

	adb_request((struct adb_request *) &req, NULL,
			ADBREQ_RAW|ADBREQ_SYNC,
			2, CUDA_PACKET, CUDA_GET_TIME);

	time = (req.reply[3] << 24) | (req.reply[4] << 16)
		| (req.reply[5] << 8) | req.reply[6];
	return time - RTC_OFFSET;
}

/*
 * Set the current system time
 */

static void adb_write_time(long data)
{
	volatile struct adb_request req;

	data += RTC_OFFSET;
	
	adb_request((struct adb_request *) &req, NULL,
			ADBREQ_RAW|ADBREQ_SYNC,
			6, CUDA_PACKET, CUDA_SET_TIME,
			(data >> 24) & 0xFF, (data >> 16) & 0xFF,
			(data >> 8) & 0xFF, data & 0xFF);
}

/*
 * Get a byte from the NVRAM
 */

static __u8 adb_read_pram(int offset)
{
	volatile struct adb_request req;

	adb_request((struct adb_request *) &req, NULL,
			ADBREQ_RAW|ADBREQ_SYNC,
			4, CUDA_PACKET, CUDA_GET_PRAM,
			(offset >> 8) & 0xFF, offset & 0xFF);
	return req.reply[3];
}

/*
 * Write a byte to the NVRAM
 */

static void adb_write_pram(int offset, __u8 data)
{
	volatile struct adb_request req;

	adb_request((struct adb_request *) &req, NULL,
			ADBREQ_RAW|ADBREQ_SYNC,
			5, CUDA_PACKET, CUDA_SET_PRAM,
			(offset >> 8) & 0xFF, offset & 0xFF,
			data);
}

/*
 * VIA PRAM/RTC access routines
 *
 * Must be called with interrupts disabled and
 * the RTC should be enabled.
 */

static __u8 via_pram_readbyte(void)
{
	int	i,reg;
	__u8	data;

	reg = via1[vBufB] & ~VIA1B_vRTCClk;

	/* Set the RTC data line to be an input. */

	via1[vDirB] &= ~VIA1B_vRTCData;

	/* The bits of the byte come out in MSB order */

	data = 0;
	for (i = 0 ; i < 8 ; i++) {
		via1[vBufB] = reg;
		via1[vBufB] = reg | VIA1B_vRTCClk;
		data = (data << 1) | (via1[vBufB] & VIA1B_vRTCData);
	}

	/* Return RTC data line to output state */

	via1[vDirB] |= VIA1B_vRTCData;

	return data;
}

static void via_pram_writebyte(__u8 data)
{
	int	i,reg,bit;

	reg = via1[vBufB] & ~(VIA1B_vRTCClk | VIA1B_vRTCData);

	/* The bits of the byte go in in MSB order */

	for (i = 0 ; i < 8 ; i++) {
		bit = data & 0x80? 1 : 0;
		data <<= 1;
		via1[vBufB] = reg | bit;
		via1[vBufB] = reg | bit | VIA1B_vRTCClk;
	}
}

/*
 * Execute a VIA PRAM/RTC command. For read commands
 * data should point to a one-byte buffer for the
 * resulting data. For write commands it should point
 * to the data byte to for the command.
 *
 * This function disables all interrupts while running.
 */

static void via_pram_command(int command, __u8 *data)
{
	unsigned long cpu_flags;
	int	is_read;

	save_flags(cpu_flags);
	cli();

	/* Enable the RTC and make sure the strobe line is high */

	via1[vBufB] = (via1[vBufB] | VIA1B_vRTCClk) & ~VIA1B_vRTCEnb;

	if (command & 0xFF00) {		/* extended (two-byte) command */
		via_pram_writebyte((command & 0xFF00) >> 8);
		via_pram_writebyte(command & 0xFF);
		is_read = command & 0x8000;
	} else {			/* one-byte command */
		via_pram_writebyte(command);
		is_read = command & 0x80;
	}
	if (is_read) {
		*data = via_pram_readbyte();
	} else {
		via_pram_writebyte(*data);
	}

	/* All done, disable the RTC */

	via1[vBufB] |= VIA1B_vRTCEnb;

	restore_flags(cpu_flags);
}

static __u8 via_read_pram(int offset)
{
	return 0;
}

static void via_write_pram(int offset, __u8 data)
{
}

/*
 * Return the current time in seconds since January 1, 1904.
 *
 * This only works on machines with the VIA-based PRAM/RTC, which
 * is basically any machine with Mac II-style ADB.
 */

static long via_read_time(void)
{
	union {
		__u8  cdata[4];
		long  idata;
	} result, last_result;
	int	ct;

	/*
	 * The NetBSD guys say to loop until you get the same reading
	 * twice in a row.
	 */

	ct = 0;
	do {
		if (++ct > 10) {
			printk("via_read_time: couldn't get valid time, "
			       "last read = 0x%08X and 0x%08X\n", last_result.idata,
			       result.idata);
			break;
		}

		last_result.idata = result.idata;
		result.idata = 0;

		via_pram_command(0x81, &result.cdata[3]);
		via_pram_command(0x85, &result.cdata[2]);
		via_pram_command(0x89, &result.cdata[1]);
		via_pram_command(0x8D, &result.cdata[0]);
	} while (result.idata != last_result.idata);

	return result.idata - RTC_OFFSET;
}

/*
 * Set the current time to a number of seconds since January 1, 1904.
 *
 * This only works on machines with the VIA-based PRAM/RTC, which
 * is basically any machine with Mac II-style ADB.
 */

static void via_write_time(long time)
{
	union {
		__u8  cdata[4];
		long  idata;
	} data;
	__u8	temp;

	/* Clear the write protect bit */

	temp = 0x55;
	via_pram_command(0x35, &temp);

	data.idata = time + RTC_OFFSET;
	via_pram_command(0x01, &data.cdata[3]);
	via_pram_command(0x05, &data.cdata[2]);
	via_pram_command(0x09, &data.cdata[1]);
	via_pram_command(0x0D, &data.cdata[0]);

	/* Set the write protect bit */

	temp = 0xD5;
	via_pram_command(0x35, &temp);
}

static void via_shutdown(void)
{
	if (rbv_present) {
		via2[rBufB] &= ~0x04;
	} else {
		/* Direction of vDirB is output */
		via2[vDirB] |= 0x04;
		/* Send a value of 0 on that line */
		via2[vBufB] &= ~0x04;
		mdelay(1000);
	}
}

/*
 * FIXME: not sure how this is supposed to work exactly...
 */

static void oss_shutdown(void)
{
	oss->rom_ctrl = OSS_POWEROFF;
}

#ifdef CONFIG_ADB_CUDA

static void cuda_restart(void)
{
	adb_request(NULL, NULL, ADBREQ_RAW|ADBREQ_SYNC,
			2, CUDA_PACKET, CUDA_RESET_SYSTEM);
}

static void cuda_shutdown(void)
{
	adb_request(NULL, NULL, ADBREQ_RAW|ADBREQ_SYNC,
			2, CUDA_PACKET, CUDA_POWERDOWN);
}

#endif /* CONFIG_ADB_CUDA */

#ifdef CONFIG_ADB_PMU

void pmu_restart(void)
{
	adb_request(NULL, NULL, ADBREQ_RAW|ADBREQ_SYNC,
			3, PMU_PACKET, PMU_SET_INTR_MASK,
			PMU_INT_ADB|PMU_INT_TICK);
	
	adb_request(NULL, NULL, ADBREQ_RAW|ADBREQ_SYNC,
			2, PMU_PACKET, PMU_RESET);
}

void pmu_shutdown(void)
{
	adb_request(NULL, NULL, ADBREQ_RAW|ADBREQ_SYNC,
			3, PMU_PACKET, PMU_SET_INTR_MASK,
			PMU_INT_ADB|PMU_INT_TICK);

	adb_request(NULL, NULL, ADBREQ_RAW|ADBREQ_SYNC,
			6, PMU_PACKET, PMU_SHUTDOWN,
			'M', 'A', 'T', 'T');
}

#endif /* CONFIG_ADB_PMU */

/*
 *-------------------------------------------------------------------
 * Below this point are the generic routines; they'll dispatch to the
 * correct routine for the hardware on which we're running.
 *-------------------------------------------------------------------
 */

void mac_pram_read(int offset, __u8 *buffer, int len)
{
	__u8 (*func)(int) = NULL;
	int i;

	if (macintosh_config->adb_type == MAC_ADB_IISI ||
	    macintosh_config->adb_type == MAC_ADB_PB1 ||
	    macintosh_config->adb_type == MAC_ADB_PB2 ||
	    macintosh_config->adb_type == MAC_ADB_CUDA) {
		func = adb_read_pram;
	} else {
		func = via_read_pram;
	}
	for (i = 0 ; i < len ; i++) {
		buffer[i] = (*func)(offset++);
	}
}

void mac_pram_write(int offset, __u8 *buffer, int len)
{
	void (*func)(int, __u8) = NULL;
	int i;

	if (macintosh_config->adb_type == MAC_ADB_IISI ||
	    macintosh_config->adb_type == MAC_ADB_PB1 ||
	    macintosh_config->adb_type == MAC_ADB_PB2 ||
	    macintosh_config->adb_type == MAC_ADB_CUDA) {
		func = adb_write_pram;
	} else {
		func = via_write_pram;
	}
	for (i = 0 ; i < len ; i++) {
		(*func)(offset++, buffer[i]);
	}
}

void mac_poweroff(void)
{
	/*
	 * MAC_ADB_IISI may need to be moved up here if it doesn't actually
	 * work using the ADB packet method.  --David Kilzer
	 */

	if (oss_present) {
		oss_shutdown();
	} else if (macintosh_config->adb_type == MAC_ADB_II) {
		via_shutdown();
#ifdef CONFIG_ADB_CUDA
	} else if (macintosh_config->adb_type == MAC_ADB_CUDA) {
		cuda_shutdown();
#endif
#ifdef CONFIG_ADB_PMU
	} else if (macintosh_config->adb_type == MAC_ADB_PB1
		|| macintosh_config->adb_type == MAC_ADB_PB2) {
		pmu_shutdown();
#endif
	}
	sti();
	printk("It is now safe to turn off your Macintosh.\n");
	while(1);
}

void mac_reset(void)
{
	if (macintosh_config->adb_type == MAC_ADB_II) {
		unsigned long cpu_flags;

		/* need ROMBASE in booter */
		/* indeed, plus need to MAP THE ROM !! */

		if (mac_bi_data.rombase == 0)
			mac_bi_data.rombase = 0x40800000;

		/* works on some */
		rom_reset = (void *) (mac_bi_data.rombase + 0xa);

		if (macintosh_config->ident == MAC_MODEL_SE30) {
			/*
			 * MSch: Machines known to crash on ROM reset ...
			 */
		} else {
			save_flags(cpu_flags);
			cli();

			rom_reset();

			restore_flags(cpu_flags);
		}
#ifdef CONFIG_ADB_CUDA
	} else if (macintosh_config->adb_type == MAC_ADB_CUDA) {
		cuda_restart();
#endif
#ifdef CONFIG_ADB_PMU
	} else if (macintosh_config->adb_type == MAC_ADB_PB1
		|| macintosh_config->adb_type == MAC_ADB_PB2) {
		pmu_restart();
#endif
	} else if (CPU_IS_030) {

		/* 030-specific reset routine.  The idea is general, but the
		 * specific registers to reset are '030-specific.  Until I
		 * have a non-030 machine, I can't test anything else.
		 *  -- C. Scott Ananian <cananian@alumni.princeton.edu>
		 */

		unsigned long rombase = 0x40000000;

		/* make a 1-to-1 mapping, using the transparent tran. reg. */
		unsigned long virt = (unsigned long) mac_reset;
		unsigned long phys = virt_to_phys(mac_reset);
		unsigned long offset = phys-virt;
		cli(); /* lets not screw this up, ok? */
		__asm__ __volatile__(".chip 68030\n\t"
				     "pmove %0,%/tt0\n\t"
				     ".chip 68k"
				     : : "m" ((phys&0xFF000000)|0x8777));
		/* Now jump to physical address so we can disable MMU */
		__asm__ __volatile__(
                    ".chip 68030\n\t"
		    "lea %/pc@(1f),%/a0\n\t"
		    "addl %0,%/a0\n\t"/* fixup target address and stack ptr */
		    "addl %0,%/sp\n\t" 
		    "pflusha\n\t"
		    "jmp %/a0@\n\t" /* jump into physical memory */
		    "0:.long 0\n\t" /* a constant zero. */
		    /* OK.  Now reset everything and jump to reset vector. */
		    "1:\n\t"
		    "lea %/pc@(0b),%/a0\n\t"
		    "pmove %/a0@, %/tc\n\t" /* disable mmu */
		    "pmove %/a0@, %/tt0\n\t" /* disable tt0 */
		    "pmove %/a0@, %/tt1\n\t" /* disable tt1 */
		    "movel #0, %/a0\n\t"
		    "movec %/a0, %/vbr\n\t" /* clear vector base register */
		    "movec %/a0, %/cacr\n\t" /* disable caches */
		    "movel #0x0808,%/a0\n\t"
		    "movec %/a0, %/cacr\n\t" /* flush i&d caches */
		    "movew #0x2700,%/sr\n\t" /* set up status register */
		    "movel %1@(0x0),%/a0\n\t"/* load interrupt stack pointer */
		    "movec %/a0, %/isp\n\t" 
		    "movel %1@(0x4),%/a0\n\t" /* load reset vector */
		    "reset\n\t" /* reset external devices */
		    "jmp %/a0@\n\t" /* jump to the reset vector */
		    ".chip 68k"
		    : : "r" (offset), "a" (rombase) : "a0");
	}

	/* should never get here */
	sti();
	printk ("Restart failed.  Please restart manually.\n");
	while(1);
}

/*
 * This function translates seconds since 1970 into a proper date.
 *
 * Algorithm cribbed from glibc2.1, __offtime().
 */
#define SECS_PER_MINUTE (60)
#define SECS_PER_HOUR  (SECS_PER_MINUTE * 60)
#define SECS_PER_DAY   (SECS_PER_HOUR * 24)

static void unmktime(unsigned long time, long offset,
		     int *yearp, int *monp, int *dayp,
		     int *hourp, int *minp, int *secp)
{
        /* How many days come before each month (0-12).  */
	static const unsigned short int __mon_yday[2][13] =
	{
		/* Normal years.  */
		{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
		/* Leap years.  */
		{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
	};
	long int days, rem, y, wday, yday;
	const unsigned short int *ip;

	days = time / SECS_PER_DAY;
	rem = time % SECS_PER_DAY;
	rem += offset;
	while (rem < 0) {
		rem += SECS_PER_DAY;
		--days;
	}
	while (rem >= SECS_PER_DAY) {
		rem -= SECS_PER_DAY;
		++days;
	}
	*hourp = rem / SECS_PER_HOUR;
	rem %= SECS_PER_HOUR;
	*minp = rem / SECS_PER_MINUTE;
	*secp = rem % SECS_PER_MINUTE;
	/* January 1, 1970 was a Thursday. */
	wday = (4 + days) % 7; /* Day in the week. Not currently used */
	if (wday < 0) wday += 7;
	y = 1970;

#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))
#define __isleap(year)	\
  ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

	while (days < 0 || days >= (__isleap (y) ? 366 : 365))
	{
		/* Guess a corrected year, assuming 365 days per year.  */
		long int yg = y + days / 365 - (days % 365 < 0);

		/* Adjust DAYS and Y to match the guessed year.  */
		days -= ((yg - y) * 365
			 + LEAPS_THRU_END_OF (yg - 1)
			 - LEAPS_THRU_END_OF (y - 1));
		y = yg;
	}
	*yearp = y - 1900;
	yday = days; /* day in the year.  Not currently used. */
	ip = __mon_yday[__isleap(y)];
	for (y = 11; days < (long int) ip[y]; --y)
		continue;
	days -= ip[y];
	*monp = y;
	*dayp = days + 1; /* day in the month */
	return;
}

/*
 * Return the boot time for use in initializing the kernel clock.
 *
 * I'd like to read the hardware clock here but many machines read
 * the PRAM through ADB, and interrupts aren't initialized when this
 * is called so ADB obviously won't work.
 */

void mac_gettod(int *yearp, int *monp, int *dayp,
	       int *hourp, int *minp, int *secp)
{
	/* Yes the GMT bias is backwards.  It looks like Penguin is
           screwing up the boottime it gives us... This works for me
           in Canada/Eastern but it might be wrong everywhere else. */
	unmktime(mac_bi_data.boottime, -mac_bi_data.gmtbias * 60,
		yearp, monp, dayp, hourp, minp, secp);
	/* For some reason this is off by one */
	*monp = *monp + 1;
}

/* 
 * Read/write the hardware clock.
 */

int mac_hwclk(int op, struct hwclk_time *t)
{
	unsigned long now;

	if (!op) { /* read */
		if (macintosh_config->adb_type == MAC_ADB_II) {
			now = via_read_time();
		} else if ((macintosh_config->adb_type == MAC_ADB_IISI) ||
			   (macintosh_config->adb_type == MAC_ADB_PB1) ||
			   (macintosh_config->adb_type == MAC_ADB_PB2) ||
			   (macintosh_config->adb_type == MAC_ADB_CUDA)) {
			now = adb_read_time();
		} else if (macintosh_config->adb_type == MAC_ADB_IOP) {
			now = via_read_time();
		} else {
			now = 0;
		}

		t->wday = 0;
		unmktime(now, 0,
			 &t->year, &t->mon, &t->day,
			 &t->hour, &t->min, &t->sec);
		printk("mac_hwclk: read %04d-%02d-%-2d %02d:%02d:%02d\n",
			t->year + 1900, t->mon + 1, t->day, t->hour, t->min, t->sec);
	} else { /* write */
		printk("mac_hwclk: tried to write %04d-%02d-%-2d %02d:%02d:%02d\n",
			t->year + 1900, t->mon + 1, t->day, t->hour, t->min, t->sec);

#if 0	/* it trashes my rtc */
		now = mktime(t->year + 1900, t->mon + 1, t->day,
			     t->hour, t->min, t->sec);

		if (macintosh_config->adb_type == MAC_ADB_II) {
			via_write_time(now);
		} else if ((macintosh_config->adb_type == MAC_ADB_IISI) ||
			   (macintosh_config->adb_type == MAC_ADB_PB1) ||
			   (macintosh_config->adb_type == MAC_ADB_PB2) ||
			   (macintosh_config->adb_type == MAC_ADB_CUDA)) {
			adb_write_time(now);
		} else if (macintosh_config->adb_type == MAC_ADB_IOP) {
			via_write_time(now);
		}
#endif
	}
	return 0;
}

/*
 * Set minutes/seconds in the hardware clock
 */

int mac_set_clock_mmss (unsigned long nowtime)
{
	struct hwclk_time now;

	mac_hwclk(0, &now);
	now.sec = nowtime % 60;
	now.min = (nowtime / 60) % 60;
	mac_hwclk(1, &now);

	return 0;
}
