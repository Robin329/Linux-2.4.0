/*
 *	linux/arch/alpha/kernel/sys_rx164.c
 *
 *	Copyright (C) 1995 David A Rusling
 *	Copyright (C) 1996 Jay A Estabrook
 *	Copyright (C) 1998, 1999 Richard Henderson
 *
 * Code supporting the RX164 (PCA56+POLARIS).
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/pci.h>
#include <linux/init.h>

#include <asm/ptrace.h>
#include <asm/system.h>
#include <asm/dma.h>
#include <asm/irq.h>
#include <asm/bitops.h>
#include <asm/mmu_context.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/core_polaris.h>

#include "proto.h"
#include "irq_impl.h"
#include "pci_impl.h"
#include "machvec_impl.h"


/* Note mask bit is true for ENABLED irqs.  */
static unsigned long cached_irq_mask;

/* Bus 0, Device 0.  Nothing else matters, since we invoke the
   POLARIS routines directly.  */
static struct pci_dev rx164_system;

static inline void
rx164_update_irq_hw(unsigned long mask)
{
	unsigned int temp;
	polaris_write_config_dword(&rx164_system, 0x74, mask);
	polaris_read_config_dword(&rx164_system, 0x74, &temp);
}

static inline void
rx164_enable_irq(unsigned int irq)
{
	rx164_update_irq_hw(cached_irq_mask |= 1UL << (irq - 16));
}

static void
rx164_disable_irq(unsigned int irq)
{
	rx164_update_irq_hw(cached_irq_mask &= ~(1UL << (irq - 16)));
}

static unsigned int
rx164_startup_irq(unsigned int irq)
{
	rx164_enable_irq(irq);
	return 0;
}

static void
rx164_end_irq(unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED|IRQ_INPROGRESS)))
		rx164_enable_irq(irq);
}

static struct hw_interrupt_type rx164_irq_type = {
	typename:	"RX164",
	startup:	rx164_startup_irq,
	shutdown:	rx164_disable_irq,
	enable:		rx164_enable_irq,
	disable:	rx164_disable_irq,
	ack:		rx164_disable_irq,
	end:		rx164_end_irq,
};

static void 
rx164_device_interrupt(unsigned long vector, struct pt_regs *regs)
{
	unsigned int temp;
	unsigned long pld;
	long i;

	/* Read the interrupt summary register.  On Polaris, this is
	   the DIRR register in PCI config space (offset 0x84).  */
	polaris_read_config_dword(&rx164_system, 0x84, &temp);
	pld = temp;

	/*
	 * Now for every possible bit set, work through them and call
	 * the appropriate interrupt handler.
	 */
	while (pld) {
		i = ffz(~pld);
		pld &= pld - 1; /* clear least bit set */
		if (i == 20) {
			isa_no_iack_sc_device_interrupt(vector, regs);
		} else {
			handle_irq(16+i, regs);
		}
	}
}

static void __init
rx164_init_irq(void)
{
	long i;

	rx164_update_irq_hw(0);
	for (i = 16; i < 40; ++i) {
		irq_desc[i].status = IRQ_DISABLED | IRQ_LEVEL;
		irq_desc[i].handler = &rx164_irq_type;
	}

	init_i8259a_irqs();
	common_init_isa_dma();

	setup_irq(16+20, &isa_cascade_irqaction);
}


/*
 * The RX164 changed its interrupt routing between pass1 and pass2...
 *
 * PASS1:
 *
 *      Slot    IDSEL   INTA    INTB    INTC    INTD    
 *      0       6       5       10      15      20
 *      1       7       4       9       14      19
 *      2       5       3       8       13      18
 *      3       9       2       7       12      17
 *      4       10      1       6       11      16
 *
 * PASS2:
 *      Slot    IDSEL   INTA    INTB    INTC    INTD    
 *      0       5       1       7       12      17
 *      1       6       2       8       13      18
 *      2       8       3       9       14      19
 *      3       9       4       10      15      20
 *      4       10      5       11      16      6
 *      
 */

/*
 * IdSel       
 *   5  32 bit PCI option slot 0
 *   6  64 bit PCI option slot 1
 *   7  PCI-ISA bridge
 *   7  64 bit PCI option slot 2
 *   9  32 bit PCI option slot 3
 *  10  PCI-PCI bridge
 * 
 */

static int __init
rx164_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
#if 0
	static char irq_tab_pass1[6][5] __initdata = {
	  /*INT   INTA  INTB  INTC   INTD */
	  { 16+3, 16+3, 16+8, 16+13, 16+18},      /* IdSel 5,  slot 2 */
	  { 16+5, 16+5, 16+10, 16+15, 16+20},     /* IdSel 6,  slot 0 */
	  { 16+4, 16+4, 16+9, 16+14, 16+19},      /* IdSel 7,  slot 1 */
	  { -1,     -1,    -1,    -1,   -1},      /* IdSel 8, PCI/ISA bridge */
	  { 16+2, 16+2, 16+7, 16+12, 16+17},      /* IdSel 9,  slot 3 */
	  { 16+1, 16+1, 16+6, 16+11, 16+16},      /* IdSel 10, slot 4 */
	};
#else
	static char irq_tab[6][5] __initdata = {
	  /*INT   INTA  INTB  INTC   INTD */
	  { 16+0, 16+0, 16+6, 16+11, 16+16},      /* IdSel 5,  slot 0 */
	  { 16+1, 16+1, 16+7, 16+12, 16+17},      /* IdSel 6,  slot 1 */
	  { -1,     -1,    -1,    -1,   -1},      /* IdSel 7, PCI/ISA bridge */
	  { 16+2, 16+2, 16+8, 16+13, 16+18},      /* IdSel 8,  slot 2 */
	  { 16+3, 16+3, 16+9, 16+14, 16+19},      /* IdSel 9,  slot 3 */
	  { 16+4, 16+4, 16+10, 16+15, 16+5},      /* IdSel 10, PCI-PCI */
	};
#endif
	const long min_idsel = 5, max_idsel = 10, irqs_per_slot = 5;

	/* JRP - Need to figure out how to distinguish pass1 from pass2,
	   and use the correct table.  */
	return COMMON_TABLE_LOOKUP;
}


/*
 * The System Vector
 */

struct alpha_machine_vector rx164_mv __initmv = {
	vector_name:		"RX164",
	DO_EV5_MMU,
	DO_DEFAULT_RTC,
	DO_POLARIS_IO,
	DO_POLARIS_BUS,
	machine_check:		polaris_machine_check,
	max_dma_address:	ALPHA_MAX_DMA_ADDRESS,
	min_io_address:		DEFAULT_IO_BASE,
	min_mem_address:	DEFAULT_MEM_BASE,

	nr_irqs:		40,
	device_interrupt:	rx164_device_interrupt,

	init_arch:		polaris_init_arch,
	init_irq:		rx164_init_irq,
	init_rtc:		common_init_rtc,
	init_pci:		common_init_pci,
	kill_arch:		NULL,
	pci_map_irq:		rx164_map_irq,
	pci_swizzle:		common_swizzle,
};
ALIAS_MV(rx164)
