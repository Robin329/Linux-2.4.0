#
#
#

O_TARGET			:= math-emu.o

obj-y				:= math.o fmr.o lfd.o stfd.o

obj-$(CONFIG_MATH_EMULATION)	+= fabs.o fadd.o fadds.o fcmpo.o fcmpu.o \
					fctiw.o fctiwz.o fdiv.o fdivs.o \
					fmadd.o fmadds.o fmsub.o fmsubs.o \
					fmul.o fmuls.o fnabs.o fneg.o types.o \
					fnmadd.o fnmadds.o fnmsub.o fnmsubs.o \
					fres.o frsp.o frsqrte.o fsel.o lfs.o \
					fsqrt.o	fsqrts.o fsub.o fsubs.o \
					mcrfs.o mffs.o mtfsb0.o mtfsb1.o \
					mtfsf.o mtfsfi.o stfiwx.o stfs.o \
					udivmodti4.o

include $(TOPDIR)/Rules.make
