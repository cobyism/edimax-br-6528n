#
# drivers/net/wireless/Makefile
#
# Makefile for the Linux Wireless network device drivers.
#
CFLAGS += -DMODULE
O_TARGET := rtl8187.o

obj-y		:=
obj-m		:=
obj-n		:=
obj-		:=

# Things that need to export symbols
export-objs	:= 

subdir-y	+= rtl8187
obj-y		+= rtl8187/r8187.o

subdir-y	+= ieee80211
obj-y		+= ieee80211/r8187_ieee80211.o

include $(TOPDIR)/Rules.make
