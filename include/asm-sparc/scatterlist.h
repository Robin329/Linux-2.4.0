/* $Id: scatterlist.h,v 1.5 2000/01/29 16:27:07 jj Exp $ */
#ifndef _SPARC_SCATTERLIST_H
#define _SPARC_SCATTERLIST_H

#include <linux/types.h>

struct scatterlist {
    char *  address;    /* Location data is to be transferred to */
    char * alt_address; /* Location of actual if address is a 
			 * dma indirect buffer.  NULL otherwise */
    unsigned int length;

    __u32 dvma_address; /* A place to hang host-specific addresses at. */
    __u32 dvma_length;
};

#define sg_dma_address(sg) ((sg)->dvma_address)
#define sg_dma_len(sg)     ((sg)->dvma_length)

#define ISA_DMA_THRESHOLD (~0UL)

#endif /* !(_SPARC_SCATTERLIST_H) */
