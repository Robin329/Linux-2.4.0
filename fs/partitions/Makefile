#
# Makefile for the linux kernel.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

O_TARGET := partitions.o

export-objs := check.o

obj-y := check.o

obj-$(CONFIG_ACORN_PARTITION) += acorn.o
obj-$(CONFIG_AMIGA_PARTITION) += amiga.o
obj-$(CONFIG_ATARI_PARTITION) += atari.o
obj-$(CONFIG_MAC_PARTITION) += mac.o
obj-$(CONFIG_MSDOS_PARTITION) += msdos.o
obj-$(CONFIG_OSF_PARTITION) += osf.o
obj-$(CONFIG_SGI_PARTITION) += sgi.o
obj-$(CONFIG_SUN_PARTITION) += sun.o
obj-$(CONFIG_ULTRIX_PARTITION) += ultrix.o
obj-$(CONFIG_IBM_PARTITION) += ibm.o

include $(TOPDIR)/Rules.make

