#
# Makefile for the linux m68k-specific parts of the memory manager.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET := mm.o

obj-y		:= init.o fault.o extable.o hwtest.o

ifndef CONFIG_SUN3
obj-y		+= kmap.o memory.o motorola.o
else
obj-y		+= sun3mmu.o
endif


include $(TOPDIR)/Rules.make
