/* lance.c: An AMD LANCE/PCnet ethernet driver for Linux. */
/*
	Written/copyright 1993-1998 by Donald Becker.

	Copyright 1993 United States Government as represented by the
	Director, National Security Agency.
	This software may be used and distributed according to the terms
	of the GNU Public License, incorporated herein by reference.

	This driver is for the Allied Telesis AT1500 and HP J2405A, and should work
	with most other LANCE-based bus-master (NE2100/NE2500) ethercards.

	The author may be reached as becker@CESDIS.gsfc.nasa.gov, or C/O
	Center of Excellence in Space Data and Information Sciences
	   Code 930.5, Goddard Space Flight Center, Greenbelt MD 20771

	Andrey V. Savochkin:
	- alignment problem with 1.3.* kernel and some minor changes.
	Thomas Bogendoerfer (tsbogend@bigbug.franken.de):
	- added support for Linux/Alpha, but removed most of it, because
        it worked only for the PCI chip. 
      - added hook for the 32bit lance driver
      - added PCnetPCI II (79C970A) to chip table
	Paul Gortmaker (gpg109@rsphy1.anu.edu.au):
	- hopefully fix above so Linux/Alpha can use ISA cards too.
    8/20/96 Fixed 7990 autoIRQ failure and reversed unneeded alignment -djb
    v1.12 10/27/97 Module support -djb
    v1.14  2/3/98 Module support modified, made PCI support optional -djb
    v1.15 5/27/99 Fixed bug in the cleanup_module(). dev->priv was freed
                  before unregister_netdev() which caused NULL pointer
                  reference later in the chain (in rtnetlink_fill_ifinfo())
                  -- Mika Kuoppala <miku@iki.fi>
    
    Forward ported v1.14 to 2.1.129, merged the PCI and misc changes from
    the 2.1 version of the old driver - Alan Cox
*/

static const char *version = "lance.c:v1.15ac 1999/11/13 dplatt@3do.com, becker@cesdis.gsfc.nasa.gov\n";

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/ptrace.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/malloc.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/dma.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

static unsigned int lance_portlist[] __initdata = { 0x300, 0x320, 0x340, 0x360, 0};
int lance_probe(struct net_device *dev);
static int lance_probe1(struct net_device *dev, int ioaddr, int irq, int options);

#ifdef LANCE_DEBUG
int lance_debug = LANCE_DEBUG;
#else
int lance_debug = 1;
#endif

/*
				Theory of Operation

I. Board Compatibility

This device driver is designed for the AMD 79C960, the "PCnet-ISA
single-chip ethernet controller for ISA".  This chip is used in a wide
variety of boards from vendors such as Allied Telesis, HP, Kingston,
and Boca.  This driver is also intended to work with older AMD 7990
designs, such as the NE1500 and NE2100, and newer 79C961.  For convenience,
I use the name LANCE to refer to all of the AMD chips, even though it properly
refers only to the original 7990.

II. Board-specific settings

The driver is designed to work the boards that use the faster
bus-master mode, rather than in shared memory mode.	 (Only older designs
have on-board buffer memory needed to support the slower shared memory mode.)

Most ISA boards have jumpered settings for the I/O base, IRQ line, and DMA
channel.  This driver probes the likely base addresses:
{0x300, 0x320, 0x340, 0x360}.
After the board is found it generates a DMA-timeout interrupt and uses
autoIRQ to find the IRQ line.  The DMA channel can be set with the low bits
of the otherwise-unused dev->mem_start value (aka PARAM1).  If unset it is
probed for by enabling each free DMA channel in turn and checking if
initialization succeeds.

The HP-J2405A board is an exception: with this board it is easy to read the
EEPROM-set values for the base, IRQ, and DMA.  (Of course you must already
_know_ the base address -- that field is for writing the EEPROM.)

III. Driver operation

IIIa. Ring buffers
The LANCE uses ring buffers of Tx and Rx descriptors.  Each entry describes
the base and length of the data buffer, along with status bits.	 The length
of these buffers is set by LANCE_LOG_{RX,TX}_BUFFERS, which is log_2() of
the buffer length (rather than being directly the buffer length) for
implementation ease.  The current values are 2 (Tx) and 4 (Rx), which leads to
ring sizes of 4 (Tx) and 16 (Rx).  Increasing the number of ring entries
needlessly uses extra space and reduces the chance that an upper layer will
be able to reorder queued Tx packets based on priority.	 Decreasing the number
of entries makes it more difficult to achieve back-to-back packet transmission
and increases the chance that Rx ring will overflow.  (Consider the worst case
of receiving back-to-back minimum-sized packets.)

The LANCE has the capability to "chain" both Rx and Tx buffers, but this driver
statically allocates full-sized (slightly oversized -- PKT_BUF_SZ) buffers to
avoid the administrative overhead. For the Rx side this avoids dynamically
allocating full-sized buffers "just in case", at the expense of a
memory-to-memory data copy for each packet received.  For most systems this
is a good tradeoff: the Rx buffer will always be in low memory, the copy
is inexpensive, and it primes the cache for later packet processing.  For Tx
the buffers are only used when needed as low-memory bounce buffers.

IIIB. 16M memory limitations.
For the ISA bus master mode all structures used directly by the LANCE,
the initialization block, Rx and Tx rings, and data buffers, must be
accessible from the ISA bus, i.e. in the lower 16M of real memory.
This is a problem for current Linux kernels on >16M machines. The network
devices are initialized after memory initialization, and the kernel doles out
memory from the top of memory downward.	 The current solution is to have a
special network initialization routine that's called before memory
initialization; this will eventually be generalized for all network devices.
As mentioned before, low-memory "bounce-buffers" are used when needed.

IIIC. Synchronization
The driver runs as two independent, single-threaded flows of control.  One
is the send-packet routine, which enforces single-threaded use by the
dev->tbusy flag.  The other thread is the interrupt handler, which is single
threaded by the hardware and other software.

The send packet thread has partial control over the Tx ring and 'dev->tbusy'
flag.  It sets the tbusy flag whenever it's queuing a Tx packet. If the next
queue slot is empty, it clears the tbusy flag when finished otherwise it sets
the 'lp->tx_full' flag.

The interrupt handler has exclusive control over the Rx ring and records stats
from the Tx ring. (The Tx-done interrupt can't be selectively turned off, so
we can't avoid the interrupt overhead by having the Tx routine reap the Tx
stats.)	 After reaping the stats, it marks the queue entry as empty by setting
the 'base' to zero. Iff the 'lp->tx_full' flag is set, it clears both the
tx_full and tbusy flags.

*/

/* Set the number of Tx and Rx buffers, using Log_2(# buffers).
   Reasonable default values are 16 Tx buffers, and 16 Rx buffers.
   That translates to 4 and 4 (16 == 2^^4).
   This is a compile-time option for efficiency.
   */
#ifndef LANCE_LOG_TX_BUFFERS
#define LANCE_LOG_TX_BUFFERS 4
#define LANCE_LOG_RX_BUFFERS 4
#endif

#define TX_RING_SIZE			(1 << (LANCE_LOG_TX_BUFFERS))
#define TX_RING_MOD_MASK		(TX_RING_SIZE - 1)
#define TX_RING_LEN_BITS		((LANCE_LOG_TX_BUFFERS) << 29)

#define RX_RING_SIZE			(1 << (LANCE_LOG_RX_BUFFERS))
#define RX_RING_MOD_MASK		(RX_RING_SIZE - 1)
#define RX_RING_LEN_BITS		((LANCE_LOG_RX_BUFFERS) << 29)

#define PKT_BUF_SZ		1544

/* Offsets from base I/O address. */
#define LANCE_DATA 0x10
#define LANCE_ADDR 0x12
#define LANCE_RESET 0x14
#define LANCE_BUS_IF 0x16
#define LANCE_TOTAL_SIZE 0x18

#define TX_TIMEOUT	20

/* The LANCE Rx and Tx ring descriptors. */
struct lance_rx_head {
	s32 base;
	s16 buf_length;			/* This length is 2s complement (negative)! */
	s16 msg_length;			/* This length is "normal". */
};

struct lance_tx_head {
	s32 base;
	s16 length;				/* Length is 2s complement (negative)! */
	s16 misc;
};

/* The LANCE initialization block, described in databook. */
struct lance_init_block {
	u16 mode;		/* Pre-set mode (reg. 15) */
	u8  phys_addr[6]; /* Physical ethernet address */
	u32 filter[2];			/* Multicast filter (unused). */
	/* Receive and transmit ring base, along with extra bits. */
	u32  rx_ring;			/* Tx and Rx ring base pointers */
	u32  tx_ring;
};

struct lance_private {
	/* The Tx and Rx ring entries must be aligned on 8-byte boundaries. */
	struct lance_rx_head rx_ring[RX_RING_SIZE];
	struct lance_tx_head tx_ring[TX_RING_SIZE];
	struct lance_init_block	init_block;
	const char *name;
	/* The saved address of a sent-in-place packet/buffer, for skfree(). */
	struct sk_buff* tx_skbuff[TX_RING_SIZE];
	/* The addresses of receive-in-place skbuffs. */
	struct sk_buff* rx_skbuff[RX_RING_SIZE];
	unsigned long rx_buffs;		/* Address of Rx and Tx buffers. */
	/* Tx low-memory "bounce buffer" address. */
	char (*tx_bounce_buffs)[PKT_BUF_SZ];
	int cur_rx, cur_tx;			/* The next free ring entry */
	int dirty_rx, dirty_tx;		/* The ring entries to be free()ed. */
	int dma;
	struct net_device_stats stats;
	unsigned char chip_version;	/* See lance_chip_type. */
	spinlock_t devlock;
};

#define LANCE_MUST_PAD          0x00000001
#define LANCE_ENABLE_AUTOSELECT 0x00000002
#define LANCE_MUST_REINIT_RING  0x00000004
#define LANCE_MUST_UNRESET      0x00000008
#define LANCE_HAS_MISSED_FRAME  0x00000010

/* A mapping from the chip ID number to the part number and features.
   These are from the datasheets -- in real life the '970 version
   reportedly has the same ID as the '965. */
static struct lance_chip_type {
	int id_number;
	const char *name;
	int flags;
} chip_table[] = {
	{0x0000, "LANCE 7990",				/* Ancient lance chip.  */
		LANCE_MUST_PAD + LANCE_MUST_UNRESET},
	{0x0003, "PCnet/ISA 79C960",		/* 79C960 PCnet/ISA.  */
		LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
			LANCE_HAS_MISSED_FRAME},
	{0x2260, "PCnet/ISA+ 79C961",		/* 79C961 PCnet/ISA+, Plug-n-Play.  */
		LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
			LANCE_HAS_MISSED_FRAME},
	{0x2420, "PCnet/PCI 79C970",		/* 79C970 or 79C974 PCnet-SCSI, PCI. */
		LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
			LANCE_HAS_MISSED_FRAME},
	/* Bug: the PCnet/PCI actually uses the PCnet/VLB ID number, so just call
		it the PCnet32. */
	{0x2430, "PCnet32",					/* 79C965 PCnet for VL bus. */
		LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
			LANCE_HAS_MISSED_FRAME},
        {0x2621, "PCnet/PCI-II 79C970A",        /* 79C970A PCInetPCI II. */
                LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
                        LANCE_HAS_MISSED_FRAME},
	{0x0, 	 "PCnet (unknown)",
		LANCE_ENABLE_AUTOSELECT + LANCE_MUST_REINIT_RING +
			LANCE_HAS_MISSED_FRAME},
};

enum {OLD_LANCE = 0, PCNET_ISA=1, PCNET_ISAP=2, PCNET_PCI=3, PCNET_VLB=4, PCNET_PCI_II=5, LANCE_UNKNOWN=6};


/* Non-zero if lance_probe1() needs to allocate low-memory bounce buffers.
   Assume yes until we know the memory size. */
static unsigned char lance_need_isa_bounce_buffers = 1;

static int lance_open(struct net_device *dev);
static int lance_open_fail(struct net_device *dev);
static void lance_init_ring(struct net_device *dev, int mode);
static int lance_start_xmit(struct sk_buff *skb, struct net_device *dev);
static int lance_rx(struct net_device *dev);
static void lance_interrupt(int irq, void *dev_id, struct pt_regs *regs);
static int lance_close(struct net_device *dev);
static struct net_device_stats *lance_get_stats(struct net_device *dev);
static void set_multicast_list(struct net_device *dev);
static void lance_tx_timeout (struct net_device *dev);



#ifdef MODULE
#define MAX_CARDS		8	/* Max number of interfaces (cards) per module */

static struct net_device dev_lance[MAX_CARDS];
static int io[MAX_CARDS];
static int dma[MAX_CARDS];
static int irq[MAX_CARDS];

MODULE_PARM(io, "1-" __MODULE_STRING(MAX_CARDS) "i");
MODULE_PARM(dma, "1-" __MODULE_STRING(MAX_CARDS) "i");
MODULE_PARM(irq, "1-" __MODULE_STRING(MAX_CARDS) "i");

int init_module(void)
{
	int this_dev, found = 0;

	for (this_dev = 0; this_dev < MAX_CARDS; this_dev++) {
		struct net_device *dev = &dev_lance[this_dev];
		dev->irq = irq[this_dev];
		dev->base_addr = io[this_dev];
		dev->dma = dma[this_dev];
		dev->init = lance_probe;
		if (io[this_dev] == 0)  {
			if (this_dev != 0) break; /* only complain once */
			printk(KERN_NOTICE "lance.c: Module autoprobing not allowed. Append \"io=0xNNN\" value(s).\n");
			return -EPERM;
		}
		if (register_netdev(dev) != 0) {
			printk(KERN_WARNING "lance.c: No PCnet/LANCE card found (i/o = 0x%x).\n", io[this_dev]);
			if (found != 0) return 0;	/* Got at least one. */
			return -ENXIO;
		}
		found++;
	}

	return 0;
}

void cleanup_module(void)
{
	int this_dev;

	for (this_dev = 0; this_dev < MAX_CARDS; this_dev++) {
		struct net_device *dev = &dev_lance[this_dev];
		if (dev->priv != NULL) {
			unregister_netdev(dev);	
			free_dma(dev->dma);
			release_region(dev->base_addr, LANCE_TOTAL_SIZE);
			kfree(dev->priv);
			dev->priv = NULL;
		}
	}
}
#endif /* MODULE */

/* Starting in v2.1.*, the LANCE/PCnet probe is now similar to the other
   board probes now that kmalloc() can allocate ISA DMA-able regions.
   This also allows the LANCE driver to be used as a module.
   */
int lance_probe(struct net_device *dev)
{
	int *port, result;

	if (high_memory <= phys_to_virt(16*1024*1024))
		lance_need_isa_bounce_buffers = 0;

	for (port = lance_portlist; *port; port++) {
		int ioaddr = *port;

		if ( check_region(ioaddr, LANCE_TOTAL_SIZE) == 0) {
			/* Detect "normal" 0x57 0x57 and the NI6510EB 0x52 0x44
			   signatures w/ minimal I/O reads */
			char offset15, offset14 = inb(ioaddr + 14);
			
			if ((offset14 == 0x52 || offset14 == 0x57) &&
				((offset15 = inb(ioaddr + 15)) == 0x57 || offset15 == 0x44)) {
				result = lance_probe1(dev, ioaddr, 0, 0);
				if ( !result ) return 0;
			}
		}
	}
	return -ENODEV;
}

static int __init lance_probe1(struct net_device *dev, int ioaddr, int irq, int options)
{
	struct lance_private *lp;
	short dma_channels;					/* Mark spuriously-busy DMA channels */
	int i, reset_val, lance_version;
	const char *chipname;
	/* Flags for specific chips or boards. */
	unsigned char hpJ2405A = 0;			/* HP ISA adaptor */
	int hp_builtin = 0;					/* HP on-board ethernet. */
	static int did_version = 0;			/* Already printed version info. */
	unsigned long flags;

	/* First we look for special cases.
	   Check for HP's on-board ethernet by looking for 'HP' in the BIOS.
	   There are two HP versions, check the BIOS for the configuration port.
	   This method provided by L. Julliard, Laurent_Julliard@grenoble.hp.com.
	   */
	if (isa_readw(0x000f0102) == 0x5048)  {
		static const short ioaddr_table[] = { 0x300, 0x320, 0x340, 0x360};
		int hp_port = (isa_readl(0x000f00f1) & 1)  ? 0x499 : 0x99;
		/* We can have boards other than the built-in!  Verify this is on-board. */
		if ((inb(hp_port) & 0xc0) == 0x80
			&& ioaddr_table[inb(hp_port) & 3] == ioaddr)
			hp_builtin = hp_port;
	}
	/* We also recognize the HP Vectra on-board here, but check below. */
	hpJ2405A = (inb(ioaddr) == 0x08 && inb(ioaddr+1) == 0x00
				&& inb(ioaddr+2) == 0x09);

	/* Reset the LANCE.	 */
	reset_val = inw(ioaddr+LANCE_RESET); /* Reset the LANCE */

	/* The Un-Reset needed is only needed for the real NE2100, and will
	   confuse the HP board. */
	if (!hpJ2405A)
		outw(reset_val, ioaddr+LANCE_RESET);

	outw(0x0000, ioaddr+LANCE_ADDR); /* Switch to window 0 */
	if (inw(ioaddr+LANCE_DATA) != 0x0004)
		return -ENODEV;

	/* Get the version of the chip. */
	outw(88, ioaddr+LANCE_ADDR);
	if (inw(ioaddr+LANCE_ADDR) != 88) {
		lance_version = 0;
	} else {							/* Good, it's a newer chip. */
		int chip_version = inw(ioaddr+LANCE_DATA);
		outw(89, ioaddr+LANCE_ADDR);
		chip_version |= inw(ioaddr+LANCE_DATA) << 16;
		if (lance_debug > 2)
			printk("  LANCE chip version is %#x.\n", chip_version);
		if ((chip_version & 0xfff) != 0x003)
			return -ENODEV;
		chip_version = (chip_version >> 12) & 0xffff;
		for (lance_version = 1; chip_table[lance_version].id_number; lance_version++) {
			if (chip_table[lance_version].id_number == chip_version)
				break;
		}
	}

	/* We can't use init_etherdev() to allocate dev->priv because it must
	   a ISA DMA-able region. */
	dev = init_etherdev(dev, 0);
	if (!dev)
		return -ENOMEM;
	SET_MODULE_OWNER(dev);
	dev->open = lance_open_fail;
	chipname = chip_table[lance_version].name;
	printk("%s: %s at %#3x,", dev->name, chipname, ioaddr);

	/* There is a 16 byte station address PROM at the base address.
	   The first six bytes are the station address. */
	for (i = 0; i < 6; i++)
		printk(" %2.2x", dev->dev_addr[i] = inb(ioaddr + i));

	dev->base_addr = ioaddr;
	request_region(ioaddr, LANCE_TOTAL_SIZE, chip_table[lance_version].name);

	/* Make certain the data structures used by the LANCE are aligned and DMAble. */
		
	lp = (struct lance_private *)(((unsigned long)kmalloc(sizeof(*lp)+7,
										   GFP_DMA | GFP_KERNEL)+7) & ~7);
	if(lp==NULL)
		return -ENODEV;
	if (lance_debug > 6) printk(" (#0x%05lx)", (unsigned long)lp);
	memset(lp, 0, sizeof(*lp));
	dev->priv = lp;
	lp->name = chipname;
	lp->rx_buffs = (unsigned long)kmalloc(PKT_BUF_SZ*RX_RING_SIZE,
										  GFP_DMA | GFP_KERNEL);
	if (lance_need_isa_bounce_buffers)
		lp->tx_bounce_buffs = kmalloc(PKT_BUF_SZ*TX_RING_SIZE,
									  GFP_DMA | GFP_KERNEL);
	else
		lp->tx_bounce_buffs = NULL;

	lp->chip_version = lance_version;
	lp->devlock = SPIN_LOCK_UNLOCKED;

	lp->init_block.mode = 0x0003;		/* Disable Rx and Tx. */
	for (i = 0; i < 6; i++)
		lp->init_block.phys_addr[i] = dev->dev_addr[i];
	lp->init_block.filter[0] = 0x00000000;
	lp->init_block.filter[1] = 0x00000000;
	lp->init_block.rx_ring = ((u32)virt_to_bus(lp->rx_ring) & 0xffffff) | RX_RING_LEN_BITS;
	lp->init_block.tx_ring = ((u32)virt_to_bus(lp->tx_ring) & 0xffffff) | TX_RING_LEN_BITS;

	outw(0x0001, ioaddr+LANCE_ADDR);
	inw(ioaddr+LANCE_ADDR);
	outw((short) (u32) virt_to_bus(&lp->init_block), ioaddr+LANCE_DATA);
	outw(0x0002, ioaddr+LANCE_ADDR);
	inw(ioaddr+LANCE_ADDR);
	outw(((u32)virt_to_bus(&lp->init_block)) >> 16, ioaddr+LANCE_DATA);
	outw(0x0000, ioaddr+LANCE_ADDR);
	inw(ioaddr+LANCE_ADDR);

	if (irq) {					/* Set iff PCI card. */
		dev->dma = 4;			/* Native bus-master, no DMA channel needed. */
		dev->irq = irq;
	} else if (hp_builtin) {
		static const char dma_tbl[4] = {3, 5, 6, 0};
		static const char irq_tbl[4] = {3, 4, 5, 9};
		unsigned char port_val = inb(hp_builtin);
		dev->dma = dma_tbl[(port_val >> 4) & 3];
		dev->irq = irq_tbl[(port_val >> 2) & 3];
		printk(" HP Vectra IRQ %d DMA %d.\n", dev->irq, dev->dma);
	} else if (hpJ2405A) {
		static const char dma_tbl[4] = {3, 5, 6, 7};
		static const char irq_tbl[8] = {3, 4, 5, 9, 10, 11, 12, 15};
		short reset_val = inw(ioaddr+LANCE_RESET);
		dev->dma = dma_tbl[(reset_val >> 2) & 3];
		dev->irq = irq_tbl[(reset_val >> 4) & 7];
		printk(" HP J2405A IRQ %d DMA %d.\n", dev->irq, dev->dma);
	} else if (lance_version == PCNET_ISAP) {		/* The plug-n-play version. */
		short bus_info;
		outw(8, ioaddr+LANCE_ADDR);
		bus_info = inw(ioaddr+LANCE_BUS_IF);
		dev->dma = bus_info & 0x07;
		dev->irq = (bus_info >> 4) & 0x0F;
	} else {
		/* The DMA channel may be passed in PARAM1. */
		if (dev->mem_start & 0x07)
			dev->dma = dev->mem_start & 0x07;
	}

	if (dev->dma == 0) {
		/* Read the DMA channel status register, so that we can avoid
		   stuck DMA channels in the DMA detection below. */
		dma_channels = ((inb(DMA1_STAT_REG) >> 4) & 0x0f) |
			(inb(DMA2_STAT_REG) & 0xf0);
	}
	if (dev->irq >= 2)
		printk(" assigned IRQ %d", dev->irq);
	else if (lance_version != 0)  {	/* 7990 boards need DMA detection first. */
		/* To auto-IRQ we enable the initialization-done and DMA error
		   interrupts. For ISA boards we get a DMA error, but VLB and PCI
		   boards will work. */
		autoirq_setup(0);

		/* Trigger an initialization just for the interrupt. */
		outw(0x0041, ioaddr+LANCE_DATA);

		dev->irq = autoirq_report(2);
		if (dev->irq)
			printk(", probed IRQ %d", dev->irq);
		else {
			printk(", failed to detect IRQ line.\n");
			return -ENODEV;
		}

		/* Check for the initialization done bit, 0x0100, which means
		   that we don't need a DMA channel. */
		if (inw(ioaddr+LANCE_DATA) & 0x0100)
			dev->dma = 4;
	}

	if (dev->dma == 4) {
		printk(", no DMA needed.\n");
	} else if (dev->dma) {
		if (request_dma(dev->dma, chipname)) {
			printk("DMA %d allocation failed.\n", dev->dma);
			return -ENODEV;
		} else
			printk(", assigned DMA %d.\n", dev->dma);
	} else {			/* OK, we have to auto-DMA. */
		for (i = 0; i < 4; i++) {
			static const char dmas[] = { 5, 6, 7, 3 };
			int dma = dmas[i];
			int boguscnt;

			/* Don't enable a permanently busy DMA channel, or the machine
			   will hang. */
			if (test_bit(dma, &dma_channels))
				continue;
			outw(0x7f04, ioaddr+LANCE_DATA); /* Clear the memory error bits. */
			if (request_dma(dma, chipname))
				continue;
				
			flags=claim_dma_lock();
			set_dma_mode(dma, DMA_MODE_CASCADE);
			enable_dma(dma);
			release_dma_lock(flags);

			/* Trigger an initialization. */
			outw(0x0001, ioaddr+LANCE_DATA);
			for (boguscnt = 100; boguscnt > 0; --boguscnt)
				if (inw(ioaddr+LANCE_DATA) & 0x0900)
					break;
			if (inw(ioaddr+LANCE_DATA) & 0x0100) {
				dev->dma = dma;
				printk(", DMA %d.\n", dev->dma);
				break;
			} else {
				flags=claim_dma_lock();
				disable_dma(dma);
				release_dma_lock(flags);
				free_dma(dma);
			}
		}
		if (i == 4) {			/* Failure: bail. */
			printk("DMA detection failed.\n");
			return -ENODEV;
		}
	}

	if (lance_version == 0 && dev->irq == 0) {
		/* We may auto-IRQ now that we have a DMA channel. */
		/* Trigger an initialization just for the interrupt. */
		autoirq_setup(0);
		outw(0x0041, ioaddr+LANCE_DATA);

		dev->irq = autoirq_report(4);
		if (dev->irq == 0) {
			printk("  Failed to detect the 7990 IRQ line.\n");
			return -ENODEV;
		}
		printk("  Auto-IRQ detected IRQ%d.\n", dev->irq);
	}

	if (chip_table[lp->chip_version].flags & LANCE_ENABLE_AUTOSELECT) {
		/* Turn on auto-select of media (10baseT or BNC) so that the user
		   can watch the LEDs even if the board isn't opened. */
		outw(0x0002, ioaddr+LANCE_ADDR);
		/* Don't touch 10base2 power bit. */
		outw(inw(ioaddr+LANCE_BUS_IF) | 0x0002, ioaddr+LANCE_BUS_IF);
	}

	if (lance_debug > 0  &&  did_version++ == 0)
		printk(version);

	/* The LANCE-specific entries in the device structure. */
	dev->open = lance_open;
	dev->hard_start_xmit = lance_start_xmit;
	dev->stop = lance_close;
	dev->get_stats = lance_get_stats;
	dev->set_multicast_list = set_multicast_list;
	dev->tx_timeout = lance_tx_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;

	return 0;
}

static int
lance_open_fail(struct net_device *dev)
{
	return -ENODEV;
}



static int
lance_open(struct net_device *dev)
{
	struct lance_private *lp = dev->priv;
	int ioaddr = dev->base_addr;
	int i;

	if (dev->irq == 0 ||
		request_irq(dev->irq, &lance_interrupt, 0, lp->name, dev)) {
		return -EAGAIN;
	}

	/* We used to allocate DMA here, but that was silly.
	   DMA lines can't be shared!  We now permanently allocate them. */

	/* Reset the LANCE */
	inw(ioaddr+LANCE_RESET);

	/* The DMA controller is used as a no-operation slave, "cascade mode". */
	if (dev->dma != 4) {
		unsigned long flags=claim_dma_lock();
		enable_dma(dev->dma);
		set_dma_mode(dev->dma, DMA_MODE_CASCADE);
		release_dma_lock(flags);
	}

	/* Un-Reset the LANCE, needed only for the NE2100. */
	if (chip_table[lp->chip_version].flags & LANCE_MUST_UNRESET)
		outw(0, ioaddr+LANCE_RESET);

	if (chip_table[lp->chip_version].flags & LANCE_ENABLE_AUTOSELECT) {
		/* This is 79C960-specific: Turn on auto-select of media (AUI, BNC). */
		outw(0x0002, ioaddr+LANCE_ADDR);
		/* Only touch autoselect bit. */
		outw(inw(ioaddr+LANCE_BUS_IF) | 0x0002, ioaddr+LANCE_BUS_IF);
 	}

	if (lance_debug > 1)
		printk("%s: lance_open() irq %d dma %d tx/rx rings %#x/%#x init %#x.\n",
			   dev->name, dev->irq, dev->dma,
		           (u32) virt_to_bus(lp->tx_ring),
		           (u32) virt_to_bus(lp->rx_ring),
			   (u32) virt_to_bus(&lp->init_block));

	lance_init_ring(dev, GFP_KERNEL);
	/* Re-initialize the LANCE, and start it when done. */
	outw(0x0001, ioaddr+LANCE_ADDR);
	outw((short) (u32) virt_to_bus(&lp->init_block), ioaddr+LANCE_DATA);
	outw(0x0002, ioaddr+LANCE_ADDR);
	outw(((u32)virt_to_bus(&lp->init_block)) >> 16, ioaddr+LANCE_DATA);

	outw(0x0004, ioaddr+LANCE_ADDR);
	outw(0x0915, ioaddr+LANCE_DATA);

	outw(0x0000, ioaddr+LANCE_ADDR);
	outw(0x0001, ioaddr+LANCE_DATA);

	netif_start_queue (dev);

	i = 0;
	while (i++ < 100)
		if (inw(ioaddr+LANCE_DATA) & 0x0100)
			break;
	/* 
	 * We used to clear the InitDone bit, 0x0100, here but Mark Stockton
	 * reports that doing so triggers a bug in the '974.
	 */
 	outw(0x0042, ioaddr+LANCE_DATA);

	if (lance_debug > 2)
		printk("%s: LANCE open after %d ticks, init block %#x csr0 %4.4x.\n",
			   dev->name, i, (u32) virt_to_bus(&lp->init_block), inw(ioaddr+LANCE_DATA));

	return 0;					/* Always succeed */
}

/* The LANCE has been halted for one reason or another (busmaster memory
   arbitration error, Tx FIFO underflow, driver stopped it to reconfigure,
   etc.).  Modern LANCE variants always reload their ring-buffer
   configuration when restarted, so we must reinitialize our ring
   context before restarting.  As part of this reinitialization,
   find all packets still on the Tx ring and pretend that they had been
   sent (in effect, drop the packets on the floor) - the higher-level
   protocols will time out and retransmit.  It'd be better to shuffle
   these skbs to a temp list and then actually re-Tx them after
   restarting the chip, but I'm too lazy to do so right now.  dplatt@3do.com
*/

static void 
lance_purge_ring(struct net_device *dev)
{
	struct lance_private *lp = dev->priv;
	int i;

	/* Free all the skbuffs in the Rx and Tx queues. */
	for (i = 0; i < RX_RING_SIZE; i++) {
		struct sk_buff *skb = lp->rx_skbuff[i];
		lp->rx_skbuff[i] = 0;
		lp->rx_ring[i].base = 0;		/* Not owned by LANCE chip. */
		if (skb)
			dev_kfree_skb_any(skb);
	}
	for (i = 0; i < TX_RING_SIZE; i++) {
		if (lp->tx_skbuff[i]) {
			dev_kfree_skb_any(lp->tx_skbuff[i]);
			lp->tx_skbuff[i] = NULL;
		}
	}
}


/* Initialize the LANCE Rx and Tx rings. */
static void
lance_init_ring(struct net_device *dev, int gfp)
{
	struct lance_private *lp = dev->priv;
	int i;

	lp->cur_rx = lp->cur_tx = 0;
	lp->dirty_rx = lp->dirty_tx = 0;

	for (i = 0; i < RX_RING_SIZE; i++) {
		struct sk_buff *skb;
		void *rx_buff;

		skb = alloc_skb(PKT_BUF_SZ, GFP_DMA | gfp);
		lp->rx_skbuff[i] = skb;
		if (skb) {
			skb->dev = dev;
			rx_buff = skb->tail;
		} else
			rx_buff = kmalloc(PKT_BUF_SZ, GFP_DMA | gfp);
		if (rx_buff == NULL)
			lp->rx_ring[i].base = 0;
		else
			lp->rx_ring[i].base = (u32)virt_to_bus(rx_buff) | 0x80000000;
		lp->rx_ring[i].buf_length = -PKT_BUF_SZ;
	}
	/* The Tx buffer address is filled in as needed, but we do need to clear
	   the upper ownership bit. */
	for (i = 0; i < TX_RING_SIZE; i++) {
		lp->tx_skbuff[i] = 0;
		lp->tx_ring[i].base = 0;
	}

	lp->init_block.mode = 0x0000;
	for (i = 0; i < 6; i++)
		lp->init_block.phys_addr[i] = dev->dev_addr[i];
	lp->init_block.filter[0] = 0x00000000;
	lp->init_block.filter[1] = 0x00000000;
	lp->init_block.rx_ring = ((u32)virt_to_bus(lp->rx_ring) & 0xffffff) | RX_RING_LEN_BITS;
	lp->init_block.tx_ring = ((u32)virt_to_bus(lp->tx_ring) & 0xffffff) | TX_RING_LEN_BITS;
}

static void
lance_restart(struct net_device *dev, unsigned int csr0_bits, int must_reinit)
{
	struct lance_private *lp = dev->priv;

	if (must_reinit ||
		(chip_table[lp->chip_version].flags & LANCE_MUST_REINIT_RING)) {
		lance_purge_ring(dev);
		lance_init_ring(dev, GFP_ATOMIC);
	}
	outw(0x0000,    dev->base_addr + LANCE_ADDR);
	outw(csr0_bits, dev->base_addr + LANCE_DATA);
}


static void lance_tx_timeout (struct net_device *dev)
{
	struct lance_private *lp = (struct lance_private *) dev->priv;
	int ioaddr = dev->base_addr;

	outw (0, ioaddr + LANCE_ADDR);
	printk ("%s: transmit timed out, status %4.4x, resetting.\n",
		dev->name, inw (ioaddr + LANCE_DATA));
	outw (0x0004, ioaddr + LANCE_DATA);
	lp->stats.tx_errors++;
#ifndef final_version
	if (lance_debug > 3) {
		int i;
		printk (" Ring data dump: dirty_tx %d cur_tx %d%s cur_rx %d.",
		  lp->dirty_tx, lp->cur_tx, netif_queue_stopped(dev) ? " (full)" : "",
			lp->cur_rx);
		for (i = 0; i < RX_RING_SIZE; i++)
			printk ("%s %08x %04x %04x", i & 0x3 ? "" : "\n ",
			 lp->rx_ring[i].base, -lp->rx_ring[i].buf_length,
				lp->rx_ring[i].msg_length);
		for (i = 0; i < TX_RING_SIZE; i++)
			printk ("%s %08x %04x %04x", i & 0x3 ? "" : "\n ",
			     lp->tx_ring[i].base, -lp->tx_ring[i].length,
				lp->tx_ring[i].misc);
		printk ("\n");
	}
#endif
	lance_restart (dev, 0x0043, 1);

	dev->trans_start = jiffies;
	netif_start_queue (dev);
}


static int lance_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct lance_private *lp = dev->priv;
	int ioaddr = dev->base_addr;
	int entry;
	unsigned long flags;

	spin_lock_irqsave(&lp->devlock, flags);

	if (lance_debug > 3) {
		outw(0x0000, ioaddr+LANCE_ADDR);
		printk("%s: lance_start_xmit() called, csr0 %4.4x.\n", dev->name,
			   inw(ioaddr+LANCE_DATA));
		outw(0x0000, ioaddr+LANCE_DATA);
	}

	/* Fill in a Tx ring entry */

	/* Mask to ring buffer boundary. */
	entry = lp->cur_tx & TX_RING_MOD_MASK;

	/* Caution: the write order is important here, set the base address
	   with the "ownership" bits last. */

	/* The old LANCE chips doesn't automatically pad buffers to min. size. */
	if (chip_table[lp->chip_version].flags & LANCE_MUST_PAD) {
		lp->tx_ring[entry].length =
			-(ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN);
	} else
		lp->tx_ring[entry].length = -skb->len;

	lp->tx_ring[entry].misc = 0x0000;

	/* If any part of this buffer is >16M we must copy it to a low-memory
	   buffer. */
	if ((u32)virt_to_bus(skb->data) + skb->len > 0x01000000) {
		if (lance_debug > 5)
			printk("%s: bouncing a high-memory packet (%#x).\n",
				   dev->name, (u32)virt_to_bus(skb->data));
		memcpy(&lp->tx_bounce_buffs[entry], skb->data, skb->len);
		lp->tx_ring[entry].base =
			((u32)virt_to_bus((lp->tx_bounce_buffs + entry)) & 0xffffff) | 0x83000000;
		dev_kfree_skb(skb);
	} else {
		lp->tx_skbuff[entry] = skb;
		lp->tx_ring[entry].base = ((u32)virt_to_bus(skb->data) & 0xffffff) | 0x83000000;
	}
	lp->cur_tx++;
	lp->stats.tx_bytes += skb->len;

	/* Trigger an immediate send poll. */
	outw(0x0000, ioaddr+LANCE_ADDR);
	outw(0x0048, ioaddr+LANCE_DATA);

	dev->trans_start = jiffies;

	if ((lp->cur_tx - lp->dirty_tx) >= TX_RING_SIZE)
		netif_stop_queue(dev);

	spin_unlock_irqrestore(&lp->devlock, flags);
	return 0;
}

/* The LANCE interrupt handler. */
static void
lance_interrupt(int irq, void *dev_id, struct pt_regs * regs)
{
	struct net_device *dev = dev_id;
	struct lance_private *lp;
	int csr0, ioaddr, boguscnt=10;
	int must_restart;

	if (dev == NULL) {
		printk ("lance_interrupt(): irq %d for unknown device.\n", irq);
		return;
	}

	ioaddr = dev->base_addr;
	lp = dev->priv;
	
	spin_lock (&lp->devlock);

	outw(0x00, dev->base_addr + LANCE_ADDR);
	while ((csr0 = inw(dev->base_addr + LANCE_DATA)) & 0x8600
		   && --boguscnt >= 0) {
		/* Acknowledge all of the current interrupt sources ASAP. */
		outw(csr0 & ~0x004f, dev->base_addr + LANCE_DATA);

		must_restart = 0;

		if (lance_debug > 5)
			printk("%s: interrupt  csr0=%#2.2x new csr=%#2.2x.\n",
				   dev->name, csr0, inw(dev->base_addr + LANCE_DATA));

		if (csr0 & 0x0400)			/* Rx interrupt */
			lance_rx(dev);

		if (csr0 & 0x0200) {		/* Tx-done interrupt */
			int dirty_tx = lp->dirty_tx;

			while (dirty_tx < lp->cur_tx) {
				int entry = dirty_tx & TX_RING_MOD_MASK;
				int status = lp->tx_ring[entry].base;
			
				if (status < 0)
					break;			/* It still hasn't been Txed */

				lp->tx_ring[entry].base = 0;

				if (status & 0x40000000) {
					/* There was an major error, log it. */
					int err_status = lp->tx_ring[entry].misc;
					lp->stats.tx_errors++;
					if (err_status & 0x0400) lp->stats.tx_aborted_errors++;
					if (err_status & 0x0800) lp->stats.tx_carrier_errors++;
					if (err_status & 0x1000) lp->stats.tx_window_errors++;
					if (err_status & 0x4000) {
						/* Ackk!  On FIFO errors the Tx unit is turned off! */
						lp->stats.tx_fifo_errors++;
						/* Remove this verbosity later! */
						printk("%s: Tx FIFO error! Status %4.4x.\n",
							   dev->name, csr0);
						/* Restart the chip. */
						must_restart = 1;
					}
				} else {
					if (status & 0x18000000)
						lp->stats.collisions++;
					lp->stats.tx_packets++;
				}

				/* We must free the original skb if it's not a data-only copy
				   in the bounce buffer. */
				if (lp->tx_skbuff[entry]) {
					dev_kfree_skb_irq(lp->tx_skbuff[entry]);
					lp->tx_skbuff[entry] = 0;
				}
				dirty_tx++;
			}

#ifndef final_version
			if (lp->cur_tx - dirty_tx >= TX_RING_SIZE) {
				printk("out-of-sync dirty pointer, %d vs. %d, full=%s.\n",
					   dirty_tx, lp->cur_tx,
					   netif_queue_stopped(dev) ? "yes" : "no");
				dirty_tx += TX_RING_SIZE;
			}
#endif

			/* if the ring is no longer full, accept more packets */
			if (netif_queue_stopped(dev) &&
			    dirty_tx > lp->cur_tx - TX_RING_SIZE + 2)
				netif_wake_queue (dev);

			lp->dirty_tx = dirty_tx;
		}

		/* Log misc errors. */
		if (csr0 & 0x4000) lp->stats.tx_errors++; /* Tx babble. */
		if (csr0 & 0x1000) lp->stats.rx_errors++; /* Missed a Rx frame. */
		if (csr0 & 0x0800) {
			printk("%s: Bus master arbitration failure, status %4.4x.\n",
				   dev->name, csr0);
			/* Restart the chip. */
			must_restart = 1;
		}

		if (must_restart) {
			/* stop the chip to clear the error condition, then restart */
			outw(0x0000, dev->base_addr + LANCE_ADDR);
			outw(0x0004, dev->base_addr + LANCE_DATA);
			lance_restart(dev, 0x0002, 0);
		}
	}

	/* Clear any other interrupt, and set interrupt enable. */
	outw(0x0000, dev->base_addr + LANCE_ADDR);
	outw(0x7940, dev->base_addr + LANCE_DATA);

	if (lance_debug > 4)
		printk("%s: exiting interrupt, csr%d=%#4.4x.\n",
			   dev->name, inw(ioaddr + LANCE_ADDR),
			   inw(dev->base_addr + LANCE_DATA));

	spin_unlock (&lp->devlock);
}

static int
lance_rx(struct net_device *dev)
{
	struct lance_private *lp = dev->priv;
	int entry = lp->cur_rx & RX_RING_MOD_MASK;
	int i;
		
	/* If we own the next entry, it's a new packet. Send it up. */
	while (lp->rx_ring[entry].base >= 0) {
		int status = lp->rx_ring[entry].base >> 24;

		if (status != 0x03) {			/* There was an error. */
			/* There is a tricky error noted by John Murphy,
			   <murf@perftech.com> to Russ Nelson: Even with full-sized
			   buffers it's possible for a jabber packet to use two
			   buffers, with only the last correctly noting the error. */
			if (status & 0x01)	/* Only count a general error at the */
				lp->stats.rx_errors++; /* end of a packet.*/
			if (status & 0x20) lp->stats.rx_frame_errors++;
			if (status & 0x10) lp->stats.rx_over_errors++;
			if (status & 0x08) lp->stats.rx_crc_errors++;
			if (status & 0x04) lp->stats.rx_fifo_errors++;
			lp->rx_ring[entry].base &= 0x03ffffff;
		}
		else 
		{
			/* Malloc up new buffer, compatible with net3. */
			short pkt_len = (lp->rx_ring[entry].msg_length & 0xfff)-4;
			struct sk_buff *skb;
			
			if(pkt_len<60)
			{
				printk("%s: Runt packet!\n",dev->name);
				lp->stats.rx_errors++;
			}
			else
			{
				skb = dev_alloc_skb(pkt_len+2);
				if (skb == NULL) 
				{
					printk("%s: Memory squeeze, deferring packet.\n", dev->name);
					for (i=0; i < RX_RING_SIZE; i++)
						if (lp->rx_ring[(entry+i) & RX_RING_MOD_MASK].base < 0)
							break;

					if (i > RX_RING_SIZE -2) 
					{
						lp->stats.rx_dropped++;
						lp->rx_ring[entry].base |= 0x80000000;
						lp->cur_rx++;
					}
					break;
				}
				skb->dev = dev;
				skb_reserve(skb,2);	/* 16 byte align */
				skb_put(skb,pkt_len);	/* Make room */
				eth_copy_and_sum(skb,
					(unsigned char *)bus_to_virt((lp->rx_ring[entry].base & 0x00ffffff)),
					pkt_len,0);
				lp->stats.rx_bytes+=skb->len;
				skb->protocol=eth_type_trans(skb,dev);
				lp->stats.rx_packets++;
				netif_rx(skb);
			}
		}
		/* The docs say that the buffer length isn't touched, but Andrew Boyd
		   of QNX reports that some revs of the 79C965 clear it. */
		lp->rx_ring[entry].buf_length = -PKT_BUF_SZ;
		lp->rx_ring[entry].base |= 0x80000000;
		entry = (++lp->cur_rx) & RX_RING_MOD_MASK;
	}

	/* We should check that at least two ring entries are free.	 If not,
	   we should free one and mark stats->rx_dropped++. */

	return 0;
}

static int
lance_close(struct net_device *dev)
{
	int ioaddr = dev->base_addr;
	struct lance_private *lp = dev->priv;

	netif_stop_queue (dev);

	if (chip_table[lp->chip_version].flags & LANCE_HAS_MISSED_FRAME) {
		outw(112, ioaddr+LANCE_ADDR);
		lp->stats.rx_missed_errors = inw(ioaddr+LANCE_DATA);
	}
	outw(0, ioaddr+LANCE_ADDR);

	if (lance_debug > 1)
		printk("%s: Shutting down ethercard, status was %2.2x.\n",
			   dev->name, inw(ioaddr+LANCE_DATA));

	/* We stop the LANCE here -- it occasionally polls
	   memory if we don't. */
	outw(0x0004, ioaddr+LANCE_DATA);

	if (dev->dma != 4)
	{
		unsigned long flags=claim_dma_lock();
		disable_dma(dev->dma);
		release_dma_lock(flags);
	}
	free_irq(dev->irq, dev);

	lance_purge_ring(dev);

	return 0;
}

static struct net_device_stats *lance_get_stats(struct net_device *dev)
{
	struct lance_private *lp = dev->priv;

	if (chip_table[lp->chip_version].flags & LANCE_HAS_MISSED_FRAME) {
		short ioaddr = dev->base_addr;
		short saved_addr;
		unsigned long flags;

		spin_lock_irqsave(&lp->devlock, flags);
		saved_addr = inw(ioaddr+LANCE_ADDR);
		outw(112, ioaddr+LANCE_ADDR);
		lp->stats.rx_missed_errors = inw(ioaddr+LANCE_DATA);
		outw(saved_addr, ioaddr+LANCE_ADDR);
		spin_unlock_irqrestore(&lp->devlock, flags);
	}

	return &lp->stats;
}

/* Set or clear the multicast filter for this adaptor.
 */

static void set_multicast_list(struct net_device *dev)
{
	short ioaddr = dev->base_addr;

	outw(0, ioaddr+LANCE_ADDR);
	outw(0x0004, ioaddr+LANCE_DATA); /* Temporarily stop the lance.	 */

	if (dev->flags&IFF_PROMISC) {
		/* Log any net taps. */
		printk("%s: Promiscuous mode enabled.\n", dev->name);
		outw(15, ioaddr+LANCE_ADDR);
		outw(0x8000, ioaddr+LANCE_DATA); /* Set promiscuous mode */
	} else {
		short multicast_table[4];
		int i;
		int num_addrs=dev->mc_count;
		if(dev->flags&IFF_ALLMULTI)
			num_addrs=1;
		/* FIXIT: We don't use the multicast table, but rely on upper-layer filtering. */
		memset(multicast_table, (num_addrs == 0) ? 0 : -1, sizeof(multicast_table));
		for (i = 0; i < 4; i++) {
			outw(8 + i, ioaddr+LANCE_ADDR);
			outw(multicast_table[i], ioaddr+LANCE_DATA);
		}
		outw(15, ioaddr+LANCE_ADDR);
		outw(0x0000, ioaddr+LANCE_DATA); /* Unset promiscuous mode */
	}

	lance_restart(dev, 0x0142, 0); /*  Resume normal operation */

}

