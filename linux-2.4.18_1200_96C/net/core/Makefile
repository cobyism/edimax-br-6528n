#
# Makefile for the Linux networking core.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET := core.o

export-objs := netfilter.o profile.o skbuff.o dev.o neighbour.o

obj-y := iovec.o

ifdef CONFIG_RTL_KERNEL_MIPS16_NETCORE
	obj-y += sock.o16 skbuff.o16 scm.o16

ifeq ($(CONFIG_RTL8196C),y)	
	obj-y += datagram.o
else
  obj-y += datagram.o16
endif

else
	obj-y += sock.o skbuff.o datagram.o scm.o
endif

ifeq ($(CONFIG_SYSCTL),y)
ifeq ($(CONFIG_NET),y)
ifdef CONFIG_RTL_KERNEL_MIPS16_NETCORE
	obj-y += sysctl_net_core.o16
else
obj-y += sysctl_net_core.o
endif
endif
endif

obj-$(CONFIG_FILTER) += filter.o

ifdef CONFIG_RTL_KERNEL_MIPS16_NETCORE
	obj-$(CONFIG_NET) += dev.o16 dev_mcast.o16 dst.o16 neighbour.o16 rtnetlink.o16 utils.o16
	obj-$(CONFIG_NETFILTER) += netfilter.o16	
else
obj-$(CONFIG_NET) += dev.o dev_mcast.o dst.o neighbour.o rtnetlink.o utils.o
	obj-$(CONFIG_NETFILTER) += netfilter.o
endif	

obj-$(CONFIG_NET_DIVERT) += dv.o
obj-$(CONFIG_NET_PROFILE) += profile.o

ifeq ($(CONFIG_WIRELESS_EXT_V18),y)
obj-$(CONFIG_NET_RADIO) += wireless.o
# Ugly. I wish all wireless drivers were moved in drivers/net/wireless
obj-$(CONFIG_NET_PCMCIA_RADIO) += wireless.o
endif

include $(TOPDIR)/Rules.make
