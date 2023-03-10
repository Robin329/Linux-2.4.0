/*
 * linux/include/asm-arm/arch-cl7500/uncompress.h
 *
 * Copyright (C) 1999 Nexus Electronics Ltd.
 */

#define BASE 0x03010000
#define SERBASE (BASE + (0x3f8 << 2))

static __inline__ void putc(char c)
{
	while (!(*((volatile unsigned int *)(SERBASE + 0x14)) & 0x20));
	*((volatile unsigned int *)(SERBASE)) = c;
}

/*
 * This does not append a newline
 */
static void puts(const char *s)
{
	while (*s) {
		putc(*s);
		if (*s == '\n')
			putc('\r');
		s++;
	}
}

static __inline__ void arch_decomp_setup(void)
{
	int baud = 3686400 / (9600 * 32);

	*((volatile unsigned int *)(SERBASE + 0xC)) = 0x80;
	*((volatile unsigned int *)(SERBASE + 0x0)) = baud & 0xff;
	*((volatile unsigned int *)(SERBASE + 0x4)) = (baud & 0xff00) >> 8;
	*((volatile unsigned int *)(SERBASE + 0xC)) = 3; /* 8 bits */
	*((volatile unsigned int *)(SERBASE + 0x10)) = 3; /* DTR, RTS */
}

/*
 * nothing to do
 */
#define arch_decomp_wdog()
