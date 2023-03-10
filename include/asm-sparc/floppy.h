/* asm-sparc/floppy.h: Sparc specific parts of the Floppy driver.
 *
 * Copyright (C) 1995 David S. Miller (davem@caip.rutgers.edu)
 */

#ifndef __ASM_SPARC_FLOPPY_H
#define __ASM_SPARC_FLOPPY_H

#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/system.h>
#include <asm/idprom.h>
#include <asm/machines.h>
#include <asm/oplib.h>
#include <asm/auxio.h>
#include <asm/irq.h>

/* We don't need no stinkin' I/O port allocation crap. */
#undef release_region
#undef check_region
#undef request_region
#define release_region(X, Y)	do { } while(0)
#define check_region(X, Y)	(0)
#define request_region(X, Y, Z)	(1)

/* References:
 * 1) Netbsd Sun floppy driver.
 * 2) NCR 82077 controller manual
 * 3) Intel 82077 controller manual
 */
struct sun_flpy_controller {
	volatile unsigned char status_82072;  /* Main Status reg. */
#define dcr_82072              status_82072   /* Digital Control reg. */
#define status1_82077          status_82072   /* Auxiliary Status reg. 1 */

	volatile unsigned char data_82072;    /* Data fifo. */
#define status2_82077          data_82072     /* Auxiliary Status reg. 2 */

	volatile unsigned char dor_82077;     /* Digital Output reg. */
	volatile unsigned char tapectl_82077; /* What the? Tape control reg? */

	volatile unsigned char status_82077;  /* Main Status Register. */
#define drs_82077              status_82077   /* Digital Rate Select reg. */

	volatile unsigned char data_82077;    /* Data fifo. */
	volatile unsigned char ___unused;
	volatile unsigned char dir_82077;     /* Digital Input reg. */
#define dcr_82077              dir_82077      /* Config Control reg. */
};

/* You'll only ever find one controller on a SparcStation anyways. */
static struct sun_flpy_controller *sun_fdc = NULL;
volatile unsigned char *fdc_status;

struct sun_floppy_ops {
	unsigned char (*fd_inb)(int port);
	void (*fd_outb)(unsigned char value, int port);
};

static struct sun_floppy_ops sun_fdops;

#define fd_inb(port)              sun_fdops.fd_inb(port)
#define fd_outb(value,port)       sun_fdops.fd_outb(value,port)
#define fd_enable_dma()           sun_fd_enable_dma()
#define fd_disable_dma()          sun_fd_disable_dma()
#define fd_request_dma()          (0) /* nothing... */
#define fd_free_dma()             /* nothing... */
#define fd_clear_dma_ff()         /* nothing... */
#define fd_set_dma_mode(mode)     sun_fd_set_dma_mode(mode)
#define fd_set_dma_addr(addr)     sun_fd_set_dma_addr(addr)
#define fd_set_dma_count(count)   sun_fd_set_dma_count(count)
#define fd_enable_irq()           /* nothing... */
#define fd_disable_irq()          /* nothing... */
#define fd_cacheflush(addr, size) /* nothing... */
#define fd_request_irq()          sun_fd_request_irq()
#define fd_free_irq()             /* nothing... */
#if 0  /* P3: added by Alain, these cause a MMU corruption. 19960524 XXX */
#define fd_dma_mem_alloc(size)    ((unsigned long) vmalloc(size))
#define fd_dma_mem_free(addr,size) (vfree((void *)(addr)))
#endif

#define FLOPPY_MOTOR_MASK         0x10

/* XXX This isn't really correct. XXX */
#define get_dma_residue(x)        (0)

#define FLOPPY0_TYPE  4
#define FLOPPY1_TYPE  0

/* Super paranoid... */
#undef HAVE_DISABLE_HLT

/* Here is where we catch the floppy driver trying to initialize,
 * therefore this is where we call the PROM device tree probing
 * routine etc. on the Sparc.
 */
#define FDC1                      sun_floppy_init()

static int FDC2=-1;

#define N_FDC    1
#define N_DRIVE  8

/* No 64k boundary crossing problems on the Sparc. */
#define CROSS_64KB(a,s) (0)

/* Routines unique to each controller type on a Sun. */
static unsigned char sun_82072_fd_inb(int port)
{
	udelay(5);
	switch(port & 7) {
	default:
		printk("floppy: Asked to read unknown port %d\n", port);
		panic("floppy: Port bolixed.");
	case 4: /* FD_STATUS */
		return sun_fdc->status_82072 & ~STATUS_DMA;
	case 5: /* FD_DATA */
		return sun_fdc->data_82072;
	case 7: /* FD_DIR */
		return (*AUXREG & AUXIO_FLPY_DCHG)? 0x80: 0;
	};
	panic("sun_82072_fd_inb: How did I get here?");
}

static void sun_82072_fd_outb(unsigned char value, int port)
{
	udelay(5);
	switch(port & 7) {
	default:
		printk("floppy: Asked to write to unknown port %d\n", port);
		panic("floppy: Port bolixed.");
	case 2: /* FD_DOR */
		/* Oh geese, 82072 on the Sun has no DOR register,
		 * the functionality is implemented via the AUXIO
		 * I/O register.  So we must emulate the behavior.
		 *
		 * ASSUMPTIONS:  There will only ever be one floppy
		 *               drive attached to a Sun controller
		 *               and it will be at drive zero.
		 */
		{
			unsigned bits = 0;
			if (value & 0x10) bits |= AUXIO_FLPY_DSEL;
			if ((value & 0x80) == 0) bits |= AUXIO_FLPY_EJCT;
			set_auxio(bits, (~bits) & (AUXIO_FLPY_DSEL|AUXIO_FLPY_EJCT));
		}
		break;
	case 5: /* FD_DATA */
		sun_fdc->data_82072 = value;
		break;
	case 7: /* FD_DCR */
		sun_fdc->dcr_82072 = value;
		break;
	case 4: /* FD_STATUS */
		sun_fdc->status_82072 = value;
		break;
	};
	return;
}

static unsigned char sun_82077_fd_inb(int port)
{
	udelay(5);
	switch(port & 7) {
	default:
		printk("floppy: Asked to read unknown port %d\n", port);
		panic("floppy: Port bolixed.");
	case 4: /* FD_STATUS */
		return sun_fdc->status_82077 & ~STATUS_DMA;
	case 5: /* FD_DATA */
		return sun_fdc->data_82077;
	case 7: /* FD_DIR */
		/* XXX: Is DCL on 0x80 in sun4m? */
		return sun_fdc->dir_82077;
	};
	panic("sun_82072_fd_inb: How did I get here?");
}

static void sun_82077_fd_outb(unsigned char value, int port)
{
	udelay(5);
	switch(port & 7) {
	default:
		printk("floppy: Asked to write to unknown port %d\n", port);
		panic("floppy: Port bolixed.");
	case 2: /* FD_DOR */
		/* Happily, the 82077 has a real DOR register. */
		sun_fdc->dor_82077 = value;
		break;
	case 5: /* FD_DATA */
		sun_fdc->data_82077 = value;
		break;
	case 7: /* FD_DCR */
		sun_fdc->dcr_82077 = value;
		break;
	case 4: /* FD_STATUS */
		sun_fdc->status_82077 = value;
		break;
	};
	return;
}

/* For pseudo-dma (Sun floppy drives have no real DMA available to
 * them so we must eat the data fifo bytes directly ourselves) we have
 * three state variables.  doing_pdma tells our inline low-level
 * assembly floppy interrupt entry point whether it should sit and eat
 * bytes from the fifo or just transfer control up to the higher level
 * floppy interrupt c-code.  I tried very hard but I could not get the
 * pseudo-dma to work in c-code without getting many overruns and
 * underruns.  If non-zero, doing_pdma encodes the direction of
 * the transfer for debugging.  1=read 2=write
 */
char *pdma_vaddr;
unsigned long pdma_size;
volatile int doing_pdma = 0;

/* This is software state */
char *pdma_base = 0;
unsigned long pdma_areasize;

/* Common routines to all controller types on the Sparc. */
static __inline__ void virtual_dma_init(void)
{
	/* nothing... */
}

static __inline__ void sun_fd_disable_dma(void)
{
	doing_pdma = 0;
	if (pdma_base) {
		mmu_unlockarea(pdma_base, pdma_areasize);
		pdma_base = 0;
	}
}

static __inline__ void sun_fd_set_dma_mode(int mode)
{
	switch(mode) {
	case DMA_MODE_READ:
		doing_pdma = 1;
		break;
	case DMA_MODE_WRITE:
		doing_pdma = 2;
		break;
	default:
		printk("Unknown dma mode %d\n", mode);
		panic("floppy: Giving up...");
	}
}

static __inline__ void sun_fd_set_dma_addr(char *buffer)
{
	pdma_vaddr = buffer;
}

static __inline__ void sun_fd_set_dma_count(int length)
{
	pdma_size = length;
}

static __inline__ void sun_fd_enable_dma(void)
{
	pdma_vaddr = mmu_lockarea(pdma_vaddr, pdma_size);
	pdma_base = pdma_vaddr;
	pdma_areasize = pdma_size;
}

/* Our low-level entry point in arch/sparc/kernel/entry.S */
extern void floppy_hardint(int irq, void *unused, struct pt_regs *regs);

static int sun_fd_request_irq(void)
{
	static int once = 0;
	int error;

	if(!once) {
		once = 1;
		error = request_fast_irq(FLOPPY_IRQ, floppy_hardint, SA_INTERRUPT, "floppy");
		return ((error == 0) ? 0 : -1);
	} else return 0;
}

static struct linux_prom_registers fd_regs[2];

static int sun_floppy_init(void)
{
	char state[128];
	int tnode, fd_node, num_regs;
	struct resource r;

	use_virtual_dma = 1;
	
	FLOPPY_IRQ = 11;
	/* Forget it if we aren't on a machine that could possibly
	 * ever have a floppy drive.
	 */
	if((sparc_cpu_model != sun4c && sparc_cpu_model != sun4m) ||
	   ((idprom->id_machtype == (SM_SUN4C | SM_4C_SLC)) ||
	    (idprom->id_machtype == (SM_SUN4C | SM_4C_ELC)))) {
		/* We certainly don't have a floppy controller. */
		goto no_sun_fdc;
	}
	/* Well, try to find one. */
	tnode = prom_getchild(prom_root_node);
	fd_node = prom_searchsiblings(tnode, "obio");
	if(fd_node != 0) {
		tnode = prom_getchild(fd_node);
		fd_node = prom_searchsiblings(tnode, "SUNW,fdtwo");
	} else {
		fd_node = prom_searchsiblings(tnode, "fd");
	}
	if(fd_node == 0) {
		goto no_sun_fdc;
	}

	/* The sun4m lets us know if the controller is actually usable. */
	if(sparc_cpu_model == sun4m) {
		prom_getproperty(fd_node, "status", state, sizeof(state));
		if(!strcmp(state, "disabled")) {
			goto no_sun_fdc;
		}
	}
	num_regs = prom_getproperty(fd_node, "reg", (char *) fd_regs, sizeof(fd_regs));
	num_regs = (num_regs / sizeof(fd_regs[0]));
	prom_apply_obio_ranges(fd_regs, num_regs);
	memset(&r, 0, sizeof(r));
	r.flags = fd_regs[0].which_io;
	r.start = fd_regs[0].phys_addr;
	sun_fdc = (struct sun_flpy_controller *)
	    sbus_ioremap(&r, 0, fd_regs[0].reg_size, "floppy");

	/* Last minute sanity check... */
	if(sun_fdc->status_82072 == 0xff) {
		sun_fdc = NULL;
		goto no_sun_fdc;
	}

        if(sparc_cpu_model == sun4c) {
                sun_fdops.fd_inb = sun_82072_fd_inb;
                sun_fdops.fd_outb = sun_82072_fd_outb;
                fdc_status = &sun_fdc->status_82072;
                /* printk("AUXIO @0x%p\n", auxio_register); */ /* P3 */
        } else {
                sun_fdops.fd_inb = sun_82077_fd_inb;
                sun_fdops.fd_outb = sun_82077_fd_outb;
                fdc_status = &sun_fdc->status_82077;
                /* printk("DOR @0x%p\n", &sun_fdc->dor_82077); */ /* P3 */
	}

	/* Success... */
	allowed_drive_mask = 0x01;
	return (int) sun_fdc;

no_sun_fdc:
	return -1;
}

static int sparc_eject(void)
{
	set_dor(0x00, 0xff, 0x90);
	udelay(500);
	set_dor(0x00, 0x6f, 0x00);
	udelay(500);
	return 0;
}

#define fd_eject(drive) sparc_eject()

#endif /* !(__ASM_SPARC_FLOPPY_H) */
