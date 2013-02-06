.S.s:
	$(CPP) $(AFLAGS) $< -o $@
.S.o:
	$(CC) $(AFLAGS) -c $< -o $@

O_TARGET := sb1250.o

obj-y := setup.o irq.o irq_handler.o time.o

obj-$(CONFIG_PCI)		+= pci.o
obj-$(CONFIG_SMP)		+= smp.o
obj-$(CONFIG_BCM1250_TBPROF)	+= bcm1250_tbprof.o
obj-$(CONFIG_MIPS32)		+= lib_hssubr.o

include $(TOPDIR)/Rules.make
