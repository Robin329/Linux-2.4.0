#
# Makefile for the linux kernel.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

O_TARGET := sbus_all.o
ifneq ($(ARCH),m68k)
obj-y    := sbus.o dvma.o
endif

subdir-y += char
subdir-m += char
obj-$(CONFIG_SBUSCHAR) += char/sunchar.o

subdir-y += audio
subdir-m += audio
obj-$(CONFIG_SPARCAUDIO) += audio/sparcaudio.o

include $(TOPDIR)/Rules.make
