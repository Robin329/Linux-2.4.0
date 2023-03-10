/* $Id$
 *
 * vic.h: Various VIC controller defines.  The VIC is an interrupt controller
 *        used in Baget/MIPS series.
 *
 * Copyright (C) 1998 Gleb Raiko & Vladimir Roganov
 */
#ifndef _MIPS_VIC_H
#define _MIPS_VIC_H

#define VIC_VME_II       0x3
#define VIC_VME_INT1     0x7
#define VIC_VME_INT2     0xB
#define VIC_VME_INT3     0xF
#define VIC_VME_INT4     0x13
#define VIC_VME_INT5     0x17
#define VIC_VME_INT6     0x1B
#define VIC_VME_INT7     0x1F
#define VIC_DMA_INT      0x23
#define VIC_LINT1        0x27
#define VIC_LINT2        0x2B
#define VIC_LINT3        0x2F
#define VIC_LINT4        0x33
#define VIC_LINT5        0x37
#define VIC_LINT6        0x3B
#define VIC_LINT7        0x3F
#define VIC_ICGS_INT     0x43
#define VIC_ICMS_INT     0x47
#define    VIC_INT_IPL(lev)  ((~(lev))&0x7)
#define    VIC_INT_ACTIVE    (1<<3)
#define    VIC_INT_AUTO      (0<<4)
#define    VIC_INT_NOAUTO    (1<<4)
#define    VIC_INT_LEVEL     (0<<5)
#define    VIC_INT_EDGE      (1<<5)
#define    VIC_INT_LOW       (0<<6)
#define    VIC_INT_HIGH      (1<<6)
#define    VIC_INT_ENABLE    (0<<7)
#define    VIC_INT_DISABLE   (1<<7)
#define    VIC_INT_SWITCH(x) (1<<(((x)&0x3)+4))
#define VIC_ERR_INT      0x4B
#define    VIC_ERR_INT_SYSFAIL_ACTIVE  (1<<3)
#define    VIC_ERR_INT_SYSFAIL  (1<<4)
#define    VIC_ERR_INT_TIMO     (1<<5)
#define    VIC_ERR_INT_WRPOST   (1<<6)
#define    VIC_ERR_INT_ACFAIL   (1<<7)
#define VIC_ICGS_BASE    0x4F
#define VIC_ICMS_BASE    0x53
#define    VIC_ICxS_BASE_GSWITCH_MASK 0x3
#define    VIC_ICxS_BASE_ID(x)  (((x)&0x3f)<<2)
#define VIC_LOCAL_BASE   0x57
#define    VIC_LOCAL_BASE_LINT_MASK 0x7
#define    VIC_LOCAL_BASE_ID(x)  (((x)&0x1f)<<3)
#define VIC_ERR_BASE     0x5B
#define    VIC_ERR_BASE_ACFAIL   0
#define    VIC_ERR_BASE_WRPOST   1
#define    VIC_ERR_BASE_TIMO     2
#define    VIC_ERR_BASE_SYSFAIL  3
#define    VIC_ERR_BASE_VMEACK   4
#define    VIC_ERR_BASE_DMA      5
#define    VIC_ERR_BASE_ID(x)  (((x)&0x1f)<<3)
#define VIC_ICS          0x5F
#define VIC_IC0          0x63
#define VIC_IC1          0x67
#define VIC_IC2          0x6B
#define VIC_IC3          0x6F
#define VIC_IC4          0x73
#define VIC_ID           0x77
#define VIC_IC6          0x7B
#define    VIC_IC6_IRESET_STATUS (1<<7)
#define    VIC_IC6_HALT_STATUS   (1<<6)
#define    VIC_IC6_SYSRESET   (3<<0)
#define    VIC_IC6_RESET      (2<<0)
#define    VIC_IC6_HALT       (1<<0)
#define    VIC_IC6_RUN        (0<<0)
#define VIC_IC7          0x7F
#define    VIC_IC7_SYSFAIL     (1<<7)
#define    VIC_IC7_RESET       (1<<6)
#define    VIC_IC7_VME_MASTER  (1<<5)
#define    VIC_IC7_SEMSET(x)   ((1<<(x))&0x1f)
#define VIC_VME_REQ      0x83
#define VIC_VME_BASE1    0x87
#define VIC_VME_BASE2    0x8B
#define VIC_VME_BASE3    0x8F
#define VIC_VME_BASE4    0x93
#define VIC_VME_BASE5    0x97
#define VIC_VME_BASE6    0x9B
#define VIC_VME_BASE7    0x9F
#define VIC_XFER_TIMO    0xA3
#define    VIC_XFER_TIMO_VME_PERIOD_INF (7<<5)
#define    VIC_XFER_TIMO_VME_PERIOD_512 (6<<5)
#define    VIC_XFER_TIMO_VME_PERIOD_256 (5<<5)
#define    VIC_XFER_TIMO_VME_PERIOD_128 (4<<5)
#define    VIC_XFER_TIMO_VME_PERIOD_64 (3<<5)
#define    VIC_XFER_TIMO_VME_PERIOD_32 (2<<5)
#define    VIC_XFER_TIMO_VME_PERIOD_16 (1<<5)
#define    VIC_XFER_TIMO_VME_PERIOD_4  (0<<5)
#define    VIC_XFER_TIMO_VME_PERIOD_VAL(x) (((x)>>5)&7)
#define    VIC_XFER_TIMO_LOCAL_PERIOD_INF (7<<2)
#define    VIC_XFER_TIMO_LOCAL_PERIOD_512 (6<<2)
#define    VIC_XFER_TIMO_LOCAL_PERIOD_256 (5<<2)
#define    VIC_XFER_TIMO_LOCAL_PERIOD_128 (4<<2)
#define    VIC_XFER_TIMO_LOCAL_PERIOD_64 (3<<2)
#define    VIC_XFER_TIMO_LOCAL_PERIOD_32 (2<<2)
#define    VIC_XFER_TIMO_LOCAL_PERIOD_16 (1<<2)
#define    VIC_XFER_TIMO_LOCAL_PERIOD_4  (0<<2)
#define    VIC_XFER_TIMO_LOCAL_PERIOD_VAL(x) (((x)>>2)&7)
#define    VIC_XFER_TIMO_ARB  (1<<1)
#define    VIC_XFER_TIMO_VME  (1<<0)
#define VIC_LOCAL_TIM    0xA7
#define    VIC_LOCAL_TIM_PAS_ASSERT(x)   (((x)-2)&0xf)
#define    VIC_LOCAL_TIM_PAS_ASSERT_VAL(x) (((x)&0xf)+2)
#define    VIC_LOCAT_TIM_DS_DEASSERT(x)  ((((x)-1)&1)<<4)
#define    VIC_LOCAT_TIM_DS_DEASSERT_VAL(x)  ((((x)>>4)&1)+1)
#define    VIC_LOCAL_TIM_PAS_DEASSERT(x) ((((x)-1)&0x7)<<5)
#define    VIC_LOCAL_TIM_PAS_DEASSERT_VAL(x) ((((x)>>5)&0x7)+1)
#define VIC_BXFER_DEF    0xAB
#define    VIC_BXFER_DEF_VME_CROSS    (1<<3)
#define    VIC_BXFER_DEF_LOCAL_CROSS  (1<<2)
#define    VIC_BXFER_DEF_AMSR   (1<<1)
#define    VIC_BXFER_DEF_DUAL   (1<<0)
#define VIC_IFACE_CFG    0xAF
#define    VIC_IFACE_CFG_RMC3    (1<<7)
#define    VIC_IFACE_CFG_RMC2    (1<<6)
#define    VIC_IFACE_CFG_RMC1    (1<<5)
#define    VIC_IFACE_CFG_HALT    (1<<4)
#define    VIC_IFACE_CFG_NOHALT  (0<<4)
#define    VIC_IFACE_CFG_NORMC   (1<<3)
#define    VIC_IFACE_CFG_DEADLOCK_VAL(x) (((x)>>3)&3)
#define    VIC_IFACE_CFG_MSTAB   (1<<2)
#define    VIC_IFACE_CFG_TURBO   (1<<1)
#define    VIC_IFACE_CFG_NOTURBO (0<<1)
#define    VIC_IFACE_CFG_VME     (1<<0)
#define VIC_REQ_CFG      0xB3
#define    VIC_REQ_CFG_FAIRNESS_DISABLED  0
#define    VIC_REQ_CFG_FAIRNESS_ENABLED   1
#define    VIC_REQ_CFG_TIMO_DISABLED      0xf
#define    VIC_REQ_CFG_DRAM_REFRESH       (1<<4)
#define    VIC_REQ_CFG_LEVEL(x)           (((x)&3)<<5)
#define    VIC_REQ_CFG_PRIO_ARBITRATION   (1<<7)
#define    VIC_REQ_CFG_RR_ARBITRATION     (0<<7)
#define VIC_AMS          0xB7
#define    VIC_AMS_AM_2_0   (1<<7)
#define    VIC_AMS_AM_5_3   (1<<6)
#define    VIC_AMS_CODE(x)  ((x)&0x1f)
#define VIC_BERR_STATUS  0xBB
#define VIC_DMA_STATUS   0xBF
#define VIC_SS0CR0       0xC3
#define VIC_SS1CR0       0xCB
#define    VIC_SSxCR0_LOCAL_XFER_ACCEL  (2)
#define    VIC_SSxCR0_LOCAL_XFER_SINGLE (1)
#define    VIC_SSxCR0_LOCAL_XFER_NONE   (0)
#define    VIC_SSxCR0_A32       (0<<2)
#define    VIC_SSxCR0_A24       (1<<2)
#define    VIC_SSxCR0_A16       (2<<2)
#define    VIC_SSxCR0_USER      (3<<2)
#define    VIC_SSxCR0_D32       (1<<4)
#define    VIC_SSxCR0_SUPER     (1<<5)
#define    VIC_SS0CR0_TIMER_FREQ_MASK   (3<<6)
#define    VIC_SS0CR0_TIMER_FREQ_NONE   (0<<6)
#define    VIC_SS0CR0_TIMER_FREQ_50HZ   (1<<6)
#define    VIC_SS0CR0_TIMER_FREQ_1000HZ (2<<6)
#define    VIC_SS0CR0_TIMER_FREQ_100HZ  (3<<6)
#define    VIC_SS1CR0_MASTER_WRPOST (1<<6)
#define    VIC_SS1CR0_SLAVE_WRPOST  (1<<7)
#define VIC_SS0CR1       0xC7
#define VIC_SS1CR1       0xCF
#define    VIC_SSxCR1_TF2(x)  (((x)&0xf)<<4)
#define    VIC_SSxCR1_TF1(x)  ((x)&0xf)
#define VIC_RELEASE      0xD3
#define    VIC_RELEASE_BLKXFER_BLEN(x) ((x)&0x1f)
#define    VIC_RELEASE_ROR             (0<<6) 
#define    VIC_RELEASE_RWD             (1<<6) 
#define    VIC_RELEASE_ROC             (2<<6) 
#define    VIC_RELEASE_BCAP            (3<<6) 
#define VIC_BXFER_CTRL   0xD7
#define    VIC_BXFER_CTRL_MODULE     (1<<7)
#define    VIC_BXFER_CTRL_LOCAL      (1<<6)
#define    VIC_BXFER_CTRL_MOVEM      (1<<5)
#define    VIC_BXFER_CTRL_READ       (1<<4)
#define    VIC_BXFER_CTRL_WRITE      (0<<4)
#define    VIC_BXFER_CTRL_INTERLEAVE(x)  ((x)&0xf)
#define VIC_BXFER_LEN_LO    0xDB
#define VIC_BXFER_LEN_HI    0xDF
#define VIC_SYS_RESET    0xE3

#ifndef __LANGUAGE_ASSEMBLY__

#define vic_inb(p)    (*(volatile unsigned char *)(VIC_BASE + (p)))
#define vic_outb(v,p) (*((volatile unsigned char *)(VIC_BASE + (p))) = v)

#endif /* __LANGUAGE_ASSEMBLY__ */

#endif /* !(_MIPS_VIC_H) */
