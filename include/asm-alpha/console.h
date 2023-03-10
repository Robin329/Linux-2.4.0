#ifndef __AXP_CONSOLE_H
#define __AXP_CONSOLE_H

/*
 * Console callback routine numbers
 */
#define CCB_GETC		0x01
#define CCB_PUTS		0x02
#define CCB_RESET_TERM		0x03
#define CCB_SET_TERM_INT	0x04
#define CCB_SET_TERM_CTL	0x05
#define CCB_PROCESS_KEYCODE	0x06

#define CCB_OPEN		0x10
#define CCB_CLOSE		0x11
#define CCB_IOCTL		0x12
#define CCB_READ		0x13
#define CCB_WRITE		0x14

#define CCB_SET_ENV		0x20
#define CCB_RESET_ENV		0x21
#define CCB_GET_ENV		0x22
#define CCB_SAVE_ENV		0x23

#define CCB_PSWITCH            0x30
#define CCB_BIOS_EMUL          0x32

/*
 * Environment variable numbers
 */
#define ENV_AUTO_ACTION		0x01
#define ENV_BOOT_DEV		0x02
#define ENV_BOOTDEF_DEV		0x03
#define ENV_BOOTED_DEV		0x04
#define ENV_BOOT_FILE		0x05
#define ENV_BOOTED_FILE		0x06
#define ENV_BOOT_OSFLAGS	0x07
#define ENV_BOOTED_OSFLAGS	0x08
#define ENV_BOOT_RESET		0x09
#define ENV_DUMP_DEV		0x0A
#define ENV_ENABLE_AUDIT	0x0B
#define ENV_LICENSE		0x0C
#define ENV_CHAR_SET		0x0D
#define ENV_LANGUAGE		0x0E
#define ENV_TTY_DEV		0x0F

#ifdef __KERNEL__
#ifndef __ASSEMBLY__
extern long callback_puts(long unit, const char *s, long length);
extern long callback_open(const char *device, long length);
extern long callback_close(long unit);
extern long callback_read(long channel, long count, const char *buf, long lbn);
extern long callback_getenv(long id, const char *buf, unsigned long buf_size);

extern int srm_fixup(unsigned long new_callback_addr,
		     unsigned long new_hwrpb_addr);
extern long srm_puts(const char *, long);
extern long srm_printk(const char *, ...)
	__attribute__ ((format (printf, 1, 2)));

struct crb_struct;
struct hwrpb_struct;
extern int callback_init_done;
extern void * callback_init(void *);
#endif /* __ASSEMBLY__ */
#endif /* __KERNEL__ */

#endif /* __AXP_CONSOLE_H */
