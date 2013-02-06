# Makefile for Linux driver

#CFLAGS	+=	-DHW_SEM -DVW_DEBUG
EXTRA_CFLAGS	+=	-DHW_SEM

O_TARGET := codec.o

obj-y       :=
obj-m       :=
obj-n       :=
obj-        :=

obj-$(CONFIG_VW2010)       += vw2010_rtk.o vw2010_ioctl.o vw2010_hw.o srec.o vw2010_mmap.o          

include $(TOPDIR)/Rules.make
