#
# Makefile for common code of NEC Osprey board
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

.S.s:
	$(CPP) $(CFLAGS) $< -o $*.s
.S.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

O_TARGET := osprey.o

obj-y	 := setup.o prom.o reset.o

obj-$(CONFIG_REMOTE_DEBUG)	+= dbg_io.o

include $(TOPDIR)/Rules.make
