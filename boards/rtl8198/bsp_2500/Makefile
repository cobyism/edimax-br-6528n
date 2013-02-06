# Realtek Semiconductor Corp.
#
# Tony Wu (tonywu@realtek.com)
# Oct. 18, 2008
#

#
# RTL8198
#
include $(DIR_LINUX)/.config

core-y   += arch/rlx/bsp/
cflags-y += -Iinclude/asm-rlx -Iarch/rlx/bsp/
load-y   += 0x80000000

extra-y  += vmlinux.lds

obj-y := prom.o setup.o irq.o timer.o serial.o
obj-$(CONFIG_PCI) += pci.o

EXTRA_AFLAGS := $(CFLAGS)
EXTRA_CFLAGS += -I$(DIR_LINUX)/drivers/net/rtl819x/AsicDriver
EXTRA_CFLAGS += -I$(DIR_LINUX)/drivers/net/rtl819x/common
ifdef CONFIG_RTL_819X_SWCORE
EXTRA_CFLAGS += -DCONFIG_RTL_819X_SWCORE
endif
