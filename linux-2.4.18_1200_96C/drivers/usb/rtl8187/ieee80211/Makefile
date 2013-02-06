#
# drivers/net/wireless/Makefile
#
# Makefile for the Linux Wireless network device drivers.
#

O_TARGET := r8187_ieee80211.o

export-objs     := ieee80211_crypt.o  ieee80211_module.o ieee80211_rx.o  ieee80211_softmac.o \
		ieee80211_softmac_wx.o ieee80211_tx.o ieee80211_wx.o
obj-y	:= ieee80211_rx.o ieee80211_tx.o ieee80211_wx.o ieee80211_module.o ieee80211_softmac_wx.o \
	ieee80211_softmac.o \
	ieee80211_crypt.o \
	#arc4.o api.o autoload.o cipher.o compress.o digest.o scatterwalk.o proc.o \
	michael_mic.o \
	aes.o\
	ieee80211_crypt_wep.o\
	ieee80211_crypt_tkip.o\
	ieee80211_crypt_ccmp.o

obj-m   := $(O_TARGET)
include $(TOPDIR)/Rules.make
