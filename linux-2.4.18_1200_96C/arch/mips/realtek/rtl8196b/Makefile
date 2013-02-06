#
# Makefile for Realtek 8196B specific parts of the kernel
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

.S.s:
	$(CPP) $(AFLAGS) $< -o $@
.S.o:
	$(CC) $(AFLAGS) -c $< -o $@

O_TARGET	:= rtl8196b.o
export-objs := pci-rtl8196b.o
all: rtl8196b.o

#obj-y		:= int-handler.o irq.o setup.o prom.o power.o
obj-y		:= int-handler.o irq.o setup.o prom.o
ifeq ($(CONFIG_PCI),y)
obj-$(CONFIG_RTL8196B) += pci-rtl8196b.o
endif

obj-$(CONFIG_REMOTE_DEBUG)	+= gdb_hook.o

int-handler.o: int-handler.S

clean:
		rm -f *.o

include $(TOPDIR)/Rules.make
