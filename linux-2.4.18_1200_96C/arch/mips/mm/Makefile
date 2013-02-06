#
# Makefile for the Linux/MIPS-specific parts of the memory manager.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

.S.s:
	$(CPP) $(AFLAGS) $< -o $@
.S.o:
	$(CC) $(AFLAGS) -c $< -o $@

O_TARGET := mm.o

export-objs			+= ioremap.o loadmmu.o umap.o
obj-y				+= extable.o init.o ioremap.o fault.o loadmmu.o

############################
# export symbol in c-r3k.o, for wlan driver module, david+2006-1-19
# 
#obj-$(CONFIG_CPU_R3000)		+= pg-r3k.o c-r3k.o tlb-r3k.o \
#				   tlbex-r3k.o
obj-$(CONFIG_CPU_R3000)		+= pg-r3k.o tlb-r3k.o tlbex-r3k.o
ifeq ($(CONFIG_CPU_R3000),y)
export-objs	+= c-r3k.o
obj-y += c-r3k.o
endif
############################

obj-$(CONFIG_CPU_TX39XX)	+= pg-r3k.o c-tx39.o tlb-r3k.o tlbex-r3k.o
obj-$(CONFIG_CPU_TX49XX)	+= pg-r4k.o c-tx49.o tlb-r4k.o tlbex-r4k.o
obj-$(CONFIG_CPU_R4300)		+= pg-r4k.o c-r4k.o tlb-r4k.o tlbex-r4k.o
obj-$(CONFIG_CPU_R4X00)		+= pg-r4k.o c-r4k.o tlb-r4k.o tlbex-r4k.o
obj-$(CONFIG_CPU_VR41XX)	+= pg-r4k.o c-r4k.o tlb-r4k.o tlbex-r4k.o
obj-$(CONFIG_CPU_R5000)		+= pg-r4k.o c-r4k.o tlb-r4k.o tlbex-r4k.o
obj-$(CONFIG_CPU_NEVADA)	+= pg-r4k.o c-r4k.o tlb-r4k.o tlbex-r4k.o
obj-$(CONFIG_CPU_R5432)		+= pg-r5432.o c-r5432.o tlb-r4k.o tlbex-r4k.o
obj-$(CONFIG_CPU_RM7000)	+= pg-rm7k.o c-rm7k.o tlb-r4k.o tlbex-r4k.o
obj-$(CONFIG_CPU_R10000)	+= pg-andes.o c-andes.o tlb-r4k.o tlbex-r4k.o
obj-$(CONFIG_CPU_MIPS32)	+= pg-mips32.o c-mips32.o tlb-r4k.o tlbex-r4k.o
obj-$(CONFIG_CPU_MIPS64)	+= pg-mips32.o c-mips32.o tlb-r4k.o tlbex-r4k.o
obj-$(CONFIG_CPU_SB1)		+= pg-sb1.o c-sb1.o tlb-sb1.o tlbex-r4k.o

obj-$(CONFIG_SGI_IP22)		+= umap.o
obj-$(CONFIG_BAGET_MIPS)	+= umap.o

include $(TOPDIR)/Rules.make
