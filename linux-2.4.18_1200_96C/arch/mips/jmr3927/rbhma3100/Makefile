#
# Makefile for TOSHIBA JMR-TX3927 board
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

.S.s:
	$(CPP) $(CFLAGS) $< -o $*.s
.S.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

O_TARGET:= jmr3927.o

obj-y	 += init.o int-handler.o irq.o setup.o rtc.o pci_fixup.o pci_ops.o

obj-$(CONFIG_LL_DEBUG) 		+= debug.o
obj-$(CONFIG_REMOTE_DEBUG)	+= kgdb_io.o

include $(TOPDIR)/Rules.make
