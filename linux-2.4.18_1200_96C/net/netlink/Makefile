#
# Makefile for the netlink driver.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET := netlink.o

export-objs := af_netlink.o

ifdef CONFIG_RTL_KERNEL_MIPS16_NETLINK
obj-y  := af_netlink.o16
else
obj-y  := af_netlink.o
endif

obj-m  :=

ifdef CONFIG_RTL_KERNEL_MIPS16_NETLINK
obj-$(CONFIG_NETLINK_DEV) += netlink_dev.o16
else
obj-$(CONFIG_NETLINK_DEV) += netlink_dev.o
endif

include $(TOPDIR)/Rules.make
