#
# Makefile for the 8366 switch driver
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...
#Add mips16 Support
O_TARGET := rtl8366.o

obj-y += rtl8366rb_api.o rtl8368s_asicdrv.o smi.o gpio.o
export-objs = rtl8368s_asicdrv.o
include $(TOPDIR)/Rules.make
EXTRA_CFLAGS += -I../
#obj-y := rtl8366rb_api.o rtl8368s_asicdrv.o rtl_multicast_glue.o smi.o rtl_multicast_snooping.o


