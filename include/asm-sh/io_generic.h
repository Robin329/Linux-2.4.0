/*
 * include/asm-sh/io_generic.h
 *
 * Copyright 2000 Stuart Menefy (stuart.menefy@st.com)
 *
 * May be copied or modified under the terms of the GNU General Public
 * License.  See linux/COPYING for more information.
 *
 * Generic IO functions
 */

#ifndef _ASM_SH_IO_GENERIC_H
#define _ASM_SH_IO_GENERIC_H

extern unsigned long generic_io_base;

extern unsigned long generic_inb(unsigned int port);
extern unsigned long generic_inw(unsigned int port);
extern unsigned long generic_inl(unsigned int port);

extern void generic_outb(unsigned long value, unsigned int port);
extern void generic_outw(unsigned long value, unsigned int port);
extern void generic_outl(unsigned long value, unsigned int port);

extern unsigned long generic_inb_p(unsigned int port);
extern unsigned long generic_inw_p(unsigned int port);
extern unsigned long generic_inl_p(unsigned int port);
extern void generic_outb_p(unsigned long value, unsigned int port);
extern void generic_outw_p(unsigned long value, unsigned int port);
extern void generic_outl_p(unsigned long value, unsigned int port);

extern void generic_insb(unsigned int port, void *addr, unsigned long count);
extern void generic_insw(unsigned int port, void *addr, unsigned long count);
extern void generic_insl(unsigned int port, void *addr, unsigned long count);
extern void generic_outsb(unsigned int port, const void *addr, unsigned long count);
extern void generic_outsw(unsigned int port, const void *addr, unsigned long count);
extern void generic_outsl(unsigned int port, const void *addr, unsigned long count);

extern unsigned long generic_readb(unsigned long addr);
extern unsigned long generic_readw(unsigned long addr);
extern unsigned long generic_readl(unsigned long addr);
extern void generic_writeb(unsigned char b, unsigned long addr);
extern void generic_writew(unsigned short b, unsigned long addr);
extern void generic_writel(unsigned int b, unsigned long addr);

extern void *generic_ioremap(unsigned long offset, unsigned long size);
extern void *generic_ioremap_nocache (unsigned long offset, unsigned long size);
extern void generic_iounmap(void *addr);

extern unsigned long generic_isa_port2addr(unsigned long offset);

#endif /* _ASM_SH_IO_GENERIC_H */
