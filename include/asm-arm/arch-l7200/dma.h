/*
 * linux/include/asm-arm/arch-l7200/dma.h
 */
#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H

/*
 * This is the maximum DMA address that can be DMAd to.
 * There should not be more than (0xd0000000 - 0xc0000000)
 * bytes of RAM.
 */
#define MAX_DMA_ADDRESS		0xd0000000
#define MAX_DMA_CHANNELS	8

#define DMA_0			0
#define DMA_1			1
#define DMA_2			2
#define DMA_3			3
#define DMA_S0			4
#define DMA_S1			5
#define DMA_VIRTUAL_FLOPPY	6
#define DMA_VIRTUAL_SOUND	7

#define DMA_FLOPPY		DMA_VIRTUAL_FLOPPY

#endif
