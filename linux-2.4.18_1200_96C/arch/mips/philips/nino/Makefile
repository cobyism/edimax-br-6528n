#
# Makefile for the Philips Nino specific parts of the kernel
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

.S.s:
	$(CPP) $(AFLAGS) $< -o $@
.S.o:
	$(CC) $(AFLAGS) -c $< -o $@

O_TARGET	:= nino.o
export-objs := pci_865x.o
all: nino.o

#obj-y		:= int-handler.o irq.o setup.o prom.o power.o
obj-y		:= int-handler.o irq.o setup.o prom.o
obj-$(CONFIG_RTL_EB8186G) += pci.o
obj-$(CONFIG_RTL865X) += pci_865x.o

obj-$(CONFIG_REMOTE_DEBUG)	+= gdb_hook.o

int-handler.o: int-handler.S

clean:
		rm -f *.o

include $(TOPDIR)/Rules.make
