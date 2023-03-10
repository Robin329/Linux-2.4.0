#ifndef __ALPHA_IRONGATE__H__
#define __ALPHA_IRONGATE__H__

#include <linux/types.h>
#include <asm/compiler.h>

/*
 * IRONGATE is the internal name for the AMD-751 K7 core logic chipset
 * which provides memory controller and PCI access for NAUTILUS-based
 * EV6 (21264) systems.
 *
 * This file is based on:
 *
 * IronGate management library, (c) 1999 Alpha Processor, Inc.
 * Copyright (C) 1999 Alpha Processor, Inc.,
 *	(David Daniel, Stig Telfer, Soohoon Lee)
 */

/*
 * The 21264 supports, and internally recognizes, a 44-bit physical
 * address space that is divided equally between memory address space
 * and I/O address space. Memory address space resides in the lower
 * half of the physical address space (PA[43]=0) and I/O address space
 * resides in the upper half of the physical address space (PA[43]=1).
 */

/*
 * Irongate CSR map.  Some of the CSRs are 8 or 16 bits, but all access
 * through the routines given is 32-bit.
 *
 * The first 0x40 bytes are standard as per the PCI spec.
 */

typedef volatile __u32	igcsr32;

typedef struct {
	igcsr32 dev_vendor;		/* 0x00 - device ID, vendor ID */
	igcsr32 stat_cmd;		/* 0x04 - status, command */
	igcsr32 class;			/* 0x08 - class code, rev ID */
	igcsr32 latency;		/* 0x0C - header type, PCI latency */
	igcsr32 bar0;			/* 0x10 - BAR0 - AGP */
	igcsr32 bar1;			/* 0x14 - BAR1 - GART */
	igcsr32 bar2;			/* 0x18 - Power Management reg block */

	igcsr32 rsrvd0[6];		/* 0x1C-0x33 reserved */

	igcsr32 capptr;			/* 0x34 - Capabilities pointer */

	igcsr32 rsrvd1[2];		/* 0x38-0x3F reserved */

	igcsr32 bacsr10;		/* 0x40 - base address chip selects */
	igcsr32 bacsr32;		/* 0x44 - base address chip selects */
	igcsr32 bacsr54;		/* 0x48 - base address chip selects */

	igcsr32 rsrvd2[1];		/* 0x4C-0x4F reserved */

	igcsr32 drammap;		/* 0x50 - address mapping control */
	igcsr32 dramtm;			/* 0x54 - timing, driver strength */
	igcsr32 dramms;			/* 0x58 - ECC, mode/status */

	igcsr32 rsrvd3[1];		/* 0x5C-0x5F reserved */

	igcsr32 biu0;			/* 0x60 - bus interface unit */
	igcsr32 biusip;			/* 0x64 - Serial initialisation pkt */

	igcsr32 rsrvd4[2];		/* 0x68-0x6F reserved */

	igcsr32 mro;			/* 0x70 - memory request optimiser */

	igcsr32 rsrvd5[3];		/* 0x74-0x7F reserved */

	igcsr32 whami;			/* 0x80 - who am I */
	igcsr32 pciarb;			/* 0x84 - PCI arbitration control */
	igcsr32 pcicfg;			/* 0x88 - PCI config status */

	igcsr32 rsrvd6[5];		/* 0x8C-0x9F reserved */

	/* AGP (bus 1) control registers */
	igcsr32 agpcap;			/* 0xA0 - AGP Capability Identifier */
	igcsr32 agpstat;		/* 0xA4 - AGP status register */
	igcsr32 agpcmd;			/* 0xA8 - AGP control register */
	igcsr32 agpva;			/* 0xAC - AGP Virtual Address Space */
	igcsr32 agpmode;		/* 0xB0 - AGP/GART mode control */
} Irongate0;


typedef struct {

	igcsr32 dev_vendor;		/* 0x00 - Device and Vendor IDs */
	igcsr32 stat_cmd;		/* 0x04 - Status and Command regs */
	igcsr32 class;			/* 0x08 - subclass, baseclass etc */
	igcsr32 htype;			/* 0x0C - header type (at 0x0E) */
	igcsr32 rsrvd0[2];		/* 0x10-0x17 reserved */
	igcsr32 busnos;			/* 0x18 - Primary, secondary bus nos */
	igcsr32 io_baselim_regs;	/* 0x1C - IO base, IO lim, AGP status */
	igcsr32	mem_baselim;		/* 0x20 - memory base, memory lim */
	igcsr32 pfmem_baselim;		/* 0x24 - prefetchable base, lim */
	igcsr32 rsrvd1[2];		/* 0x28-0x2F reserved */
	igcsr32 io_baselim;		/* 0x30 - IO base, IO limit */
	igcsr32 rsrvd2[2];		/* 0x34-0x3B - reserved */
	igcsr32 interrupt;		/* 0x3C - interrupt, PCI bridge ctrl */

} Irongate1;


/*
 * Memory spaces:
 */

/* Irongate is consistent with a subset of the Tsunami memory map */
#ifdef USE_48_BIT_KSEG
#define IRONGATE_BIAS 0x80000000000UL
#else
#define IRONGATE_BIAS 0x10000000000UL
#endif


#define IRONGATE_MEM		(IDENT_ADDR | IRONGATE_BIAS | 0x000000000UL)
#define IRONGATE_IACK_SC	(IDENT_ADDR | IRONGATE_BIAS | 0x1F8000000UL)
#define IRONGATE_IO		(IDENT_ADDR | IRONGATE_BIAS | 0x1FC000000UL)
#define IRONGATE_CONF		(IDENT_ADDR | IRONGATE_BIAS | 0x1FE000000UL)

/*
 * PCI Configuration space accesses are formed like so:
 *
 * 0x1FE << 24 |  : 2 2 2 2 1 1 1 1 : 1 1 1 1 1 1 0 0 : 0 0 0 0 0 0 0 0 :
 *                : 3 2 1 0 9 8 7 6 : 5 4 3 2 1 0 9 8 : 7 6 5 4 3 2 1 0 :
 *                  ---bus numer---   -device-- -fun-   ---register----
 */

#define IGCSR(dev,fun,reg)	( IRONGATE_CONF | \
				((dev)<<11) | \
				((fun)<<8) | \
				(reg) )

#define IRONGATE0		((Irongate0 *) IGCSR(0, 0, 0))
#define IRONGATE1		((Irongate1 *) IGCSR(1, 0, 0))

/*
 * Data structure for handling IRONGATE machine checks:
 * This is the standard OSF logout frame
 */

#define SCB_Q_SYSERR	0x620			/* OSF definitions */
#define SCB_Q_PROCERR	0x630
#define SCB_Q_SYSMCHK	0x660
#define SCB_Q_PROCMCHK	0x670

struct el_IRONGATE_sysdata_mcheck {
	__u32 FrameSize;                 /* Bytes, including this field */
	__u32 FrameFlags;                /* <31> = Retry, <30> = Second Error */
	__u32 CpuOffset;                 /* Offset to CPU-specific into */
	__u32 SystemOffset;              /* Offset to system-specific info */
	__u32 MCHK_Code;
	__u32 MCHK_Frame_Rev;
	__u64 I_STAT;
	__u64 DC_STAT;
	__u64 C_ADDR;
	__u64 DC1_SYNDROME;
	__u64 DC0_SYNDROME;
	__u64 C_STAT;
	__u64 C_STS;
	__u64 RESERVED0;
	__u64 EXC_ADDR;
	__u64 IER_CM;
	__u64 ISUM;
	__u64 MM_STAT;
	__u64 PAL_BASE;
	__u64 I_CTL;
	__u64 PCTX;
};


#ifdef __KERNEL__

#ifndef __EXTERN_INLINE
#define __EXTERN_INLINE extern inline
#define __IO_EXTERN_INLINE
#endif

/*
 * I/O functions:
 *
 * IRONGATE (AMD-751) PCI/memory support chip for the EV6 (21264) and
 * K7 can only use linear accesses to get at PCI memory and I/O spaces.
 */

#define vucp	volatile unsigned char *
#define vusp	volatile unsigned short *
#define vuip	volatile unsigned int *
#define vulp	volatile unsigned long *

__EXTERN_INLINE unsigned int irongate_inb(unsigned long addr)
{
	return __kernel_ldbu(*(vucp)(addr + IRONGATE_IO));
}

__EXTERN_INLINE void irongate_outb(unsigned char b, unsigned long addr)
{
        __kernel_stb(b, *(vucp)(addr + IRONGATE_IO));
	mb();
}

__EXTERN_INLINE unsigned int irongate_inw(unsigned long addr)
{
	return __kernel_ldwu(*(vusp)(addr + IRONGATE_IO));
}

__EXTERN_INLINE void irongate_outw(unsigned short b, unsigned long addr)
{
        __kernel_stw(b, *(vusp)(addr + IRONGATE_IO));
	mb();
}

__EXTERN_INLINE unsigned int irongate_inl(unsigned long addr)
{
	return *(vuip)(addr + IRONGATE_IO);
}

__EXTERN_INLINE void irongate_outl(unsigned int b, unsigned long addr)
{
        *(vuip)(addr + IRONGATE_IO) = b;
	mb();
}

/*
 * Memory functions.  All accesses are done through linear space.
 */

__EXTERN_INLINE unsigned long irongate_readb(unsigned long addr)
{
	return __kernel_ldbu(*(vucp)addr);
}

__EXTERN_INLINE unsigned long irongate_readw(unsigned long addr)
{
	return __kernel_ldwu(*(vusp)addr);
}

__EXTERN_INLINE unsigned long irongate_readl(unsigned long addr)
{
	return *(vuip)addr;
}

__EXTERN_INLINE unsigned long irongate_readq(unsigned long addr)
{
	return *(vulp)addr;
}

__EXTERN_INLINE void irongate_writeb(unsigned char b, unsigned long addr)
{
	__kernel_stb(b, *(vucp)addr);
}

__EXTERN_INLINE void irongate_writew(unsigned short b, unsigned long addr)
{
	__kernel_stw(b, *(vusp)addr);
}

__EXTERN_INLINE void irongate_writel(unsigned int b, unsigned long addr)
{
	*(vuip)addr = b;
}

__EXTERN_INLINE void irongate_writeq(unsigned long b, unsigned long addr)
{
	*(vulp)addr = b;
}

__EXTERN_INLINE unsigned long irongate_ioremap(unsigned long addr)
{
	return addr + IRONGATE_MEM;
}

__EXTERN_INLINE int irongate_is_ioaddr(unsigned long addr)
{
	return addr >= IRONGATE_MEM;
}

#undef vucp
#undef vusp
#undef vuip
#undef vulp

#ifdef __WANT_IO_DEF

#define __inb(p)		irongate_inb((unsigned long)(p))
#define __inw(p)		irongate_inw((unsigned long)(p))
#define __inl(p)		irongate_inl((unsigned long)(p))
#define __outb(x,p)		irongate_outb((x),(unsigned long)(p))
#define __outw(x,p)		irongate_outw((x),(unsigned long)(p))
#define __outl(x,p)		irongate_outl((x),(unsigned long)(p))
#define __readb(a)		irongate_readb((unsigned long)(a))
#define __readw(a)		irongate_readw((unsigned long)(a))
#define __readl(a)		irongate_readl((unsigned long)(a))
#define __readq(a)		irongate_readq((unsigned long)(a))
#define __writeb(x,a)		irongate_writeb((x),(unsigned long)(a))
#define __writew(x,a)		irongate_writew((x),(unsigned long)(a))
#define __writel(x,a)		irongate_writel((x),(unsigned long)(a))
#define __writeq(x,a)		irongate_writeq((x),(unsigned long)(a))
#define __ioremap(a)		irongate_ioremap((unsigned long)(a))
#define __is_ioaddr(a)		irongate_is_ioaddr((unsigned long)(a))

#define inb(p)			__inb(p)
#define inw(p)			__inw(p)
#define inl(p)			__inl(p)
#define outb(x,p)		__outb((x),(p))
#define outw(x,p)		__outw((x),(p))
#define outl(x,p)		__outl((x),(p))
#define __raw_readb(a)		__readb(a)
#define __raw_readw(a)		__readw(a)
#define __raw_readl(a)		__readl(a)
#define __raw_readq(a)		__readq(a)
#define __raw_writeb(v,a)	__writeb((v),(a))
#define __raw_writew(v,a)	__writew((v),(a))
#define __raw_writel(v,a)	__writel((v),(a))
#define __raw_writeq(v,a)	__writeq((v),(a))

#endif /* __WANT_IO_DEF */

#ifdef __IO_EXTERN_INLINE
#undef __EXTERN_INLINE
#undef __IO_EXTERN_INLINE
#endif

#endif /* __KERNEL__ */

#endif /* __ALPHA_IRONGATE__H__ */
