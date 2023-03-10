/*
 *  arch/s390/lib/memset.S
 *    S390 fast memset routine
 *
 *  S390 version
 *    Copyright (C) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *    Author(s): Martin Schwidefsky (schwidefsky@de.ibm.com),
 */

/*
 * R2 = address to memory area
 * R3 = byte to fill memory with
 * R4 = number of bytes to fill
 */
        .globl  memset
memset:
        LTR     4,4
        JZ      memset_end
        LR      0,2                    # save pointer to memory area
        LR      1,3                    # move pad byte to R1
        LR      3,4
        SR      4,4                    # no source for MVCLE, only a pad byte
        SR      5,5
        MVCLE   2,4,0(1)               # thats it, MVCLE is your friend
        JO      .-4
        LR      2,0                    # return pointer to mem.
memset_end:
        BR      14
        

