#
# linux/drivers/chips/Makefile
#
# $Id: Makefile,v 1.3 2009/07/29 08:51:27 joeylin Exp $

O_TARGET	:= chipslink.o

export-objs	:= chipreg.o gen_probe.o

#                       *** BIG UGLY NOTE ***
#
# The removal of get_module_symbol() and replacement with
# inter_module_register() et al has introduced a link order dependency
# here where previously there was none.  We now have to ensure that
# the CFI command set drivers are linked before cfi_probe.o

obj-$(CONFIG_MTD)		+= chipreg.o
obj-$(CONFIG_MTD_AMDSTD)	+= amd_flash.o 
obj-$(CONFIG_MTD_CFI)		+= cfi_probe.o
obj-$(CONFIG_MTD_CFI_AMDSTD)	+= cfi_cmdset_0002.o
obj-$(CONFIG_MTD_CFI_INTELEXT)	+= cfi_cmdset_0001.o
obj-$(CONFIG_MTD_GEN_PROBE)	+= gen_probe.o
obj-$(CONFIG_MTD_INTELPROBE)	+= intel_probe.o
obj-$(CONFIG_MTD_JEDEC)		+= jedec.o
obj-$(CONFIG_MTD_JEDECPROBE)	+= jedec_probe.o
obj-$(CONFIG_MTD_RAM)		+= map_ram.o
obj-$(CONFIG_MTD_ROM)		+= map_rom.o
obj-$(CONFIG_MTD_SHARP)		+= sharp.o
obj-$(CONFIG_MTD_ABSENT)	+= map_absent.o
subdir-$(CONFIG_MTD_RTL8196_SPI)       := rtl8196
obj-$(CONFIG_MTD_RTL8196_SPI)	+= rtl8196/spi_rtl8196.o
include $(TOPDIR)/Rules.make
