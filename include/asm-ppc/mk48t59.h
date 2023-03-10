/*
 * Registers for the mk48t59 real-time-clock
 */

#ifndef _PPC_MK48T59_H
#define _PPC_MK48T59_H

/* RTC Offsets */

#define MK48T59_RTC_SECONDS		0x1FF9
#define MK48T59_RTC_MINUTES		0x1FFA
#define MK48T59_RTC_HOURS		0x1FFB
#define MK48T59_RTC_DAY_OF_WEEK		0x1FFC
#define MK48T59_RTC_DAY_OF_MONTH	0x1FFD
#define MK48T59_RTC_MONTH		0x1FFE
#define MK48T59_RTC_YEAR		0x1FFF

#define MK48T59_RTC_CONTROLA		0x1FF8
#define MK48T59_RTC_CA_WRITE		0x80
#define MK48T59_RTC_CA_READ		0x40
#define MK48T59_RTC_CA_CALIB_SIGN	0x20
#define MK48T59_RTC_CA_CALIB_MASK	0x1f

#define MK48T59_RTC_CONTROLB		0x1FF9
#define MK48T59_RTC_CB_STOP		0x80

#ifndef BCD_TO_BIN
#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)
#endif

#ifndef BIN_TO_BCD
#define BIN_TO_BCD(val) ((val)=(((val)/10)<<4) + (val)%10)
#endif

#endif /* _PPC_MK48T59_H */
