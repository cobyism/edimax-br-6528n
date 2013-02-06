DEFAULT_JUMP=0x80500000


ROOTDIR  = $(shell pwd)
PATH     := $(PATH):$(ROOTDIR)/tools
HOSTCC   = cc
IMAGEDIR = $(ROOTDIR)/images
RELDIR   = $(ROOTDIR)/release
ROMFSDIR = $(ROOTDIR)/romfs
ROMFSINST= romfs-inst.sh
SCRIPTSDIR = $(ROOTDIR)/config/scripts
#TFTPDIR    = /tftpboot
GDBSERVERDIR = $(ROOTDIR)/user

LINUXDIR = $(CONFIG_LINUXDIR)
ifeq ($(LINUXDIR),)
LINUXDIR := linux-2.6.x
endif

LINUX_CONFIG  = $(ROOTDIR)/.config

############################################################################
#
# Check if linux-2.4.x/.config exist (for packer)
#
ifeq ($(LINUX_CONFIG),$(wildcard $(LINUX_CONFIG)))
else
all: kernel_config_error
endif


CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
          else if [ -x /bin/bash ]; then echo /bin/bash; \
          else echo sh; fi ; fi)


#ARCH = mips
#CROSS_COMPILE = /toolchain/rsdk/linux/uclibc/bin/mips-linux-
#CROSS_KERNEL_COMPILE = /toolchain/rsdk-1.3.5/linux/uclibc/bin/mips-linux-
#TOOLCHAIN_PATH = `echo ${CROSS_KERNEL_COMPILE} | sed -e 's/\/mips-linux-/ /g'`
#PATH     := $(PATH):$(TOOLCHAIN_PATH)
MAKE = make




ifneq ($(SUBARCH),)
# Using UML, so make the kernel and non-kernel with different ARCHs
MAKEARCH = $(MAKE) ARCH=$(SUBARCH) CROSS_COMPILE=$(CROSS_COMPILE)
MAKEARCH_KERNEL = $(MAKE)  ARCH=$(ARCH) SUBARCH=$(SUBARCH) CROSS_COMPILE=$(CROSS_KERNEL_COMPILE)
else
MAKEARCH = $(MAKE) V=1 ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
MAKEARCH_KERNEL =$(MAKE) V=1 ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_KERNEL_COMPILE)
endif


export VENDOR PRODUCT ROOTDIR LINUXDIR VENDDIR HOSTCC CONFIG_SHELL
export CONFIG_CONFIG LINUX_CONFIG ROMFSDIR SCRIPTSDIR
export VERSIONPKG VERSIONSTR ROMFSINST PATH IMAGEDIR RELDIR RELFILES TFTPDIR
export CROSS_KERNEL_COMPILE
###########################################################################
#
# Lets work out what the user wants, and if they have configured us yet
#
ARCH+=RTL865X=1


ifeq (.config,$(wildcard .config))
include .config
include $(LINUX_CONFIG)

export PATH
ifeq ($(CONFIG_RTL8196C),y)
PATH:= $(shell pwd)/../rlxlinux-v2.2/toolchain/rsdk-1.3.6-4181-EB-2.6.30-0.9.30/bin:$(PATH)
endif
ifeq ($(CONFIG_RTL8198),y)
PATH:= $(shell pwd)/../rlxlinux-v2.2/toolchain/rsdk-1.3.6-5281-EB-2.6.30-0.9.30/bin:$(PATH)
endif

export CONFIG_SPI_STD_MODE

ifeq ($(CONFIG_NFBI),y)
    DEFAULT_JUMP=0x806ffff0
#	ARCH=RTL8196B=1 RTL8197B=1
#	ARCH=RTL8196B=1 RTL8197B=1 CONFIG_FPGA_PLATFORM=1
endif



all:         
	make -C boot boot JUMP_ADDR=$(DEFAULT_JUMP) $(ARCH)
	make -C btcode $(ARCH)
	make -C boot wboot $(ARCH)

else # else of ifeq (.config,$(wildcard .config))
all: config_error
endif # end of ifeq (.config,$(wildcard .config))


boot:
	make -C boot



16bit:	
	make -C boot boot JUMP_ADDR=$(DEFAULT_JUMP) $(ARCH)
	make -C btcode 16B=1  $(ARCH)

clean:
	make -C boot clean
	make -C btcode clean
		

			
nfbi:
	make -C boot boot JUMP_ADDR=$(DEFAULT_JUMP) $(ARCH)
	make -C btcode $(ARCH)
	
wboot16: clean
	make -C boot boot JUMP_ADDR=$(DEFAULT_JUMP) $(ARCH) 
	make -C btcode 16B=1 $(ARCH)
	make -C boot wboot $(ARCH)

	
test:
	make -C boot test	



menuconfig: 

	cp config.in config.in.old -f	
	$(MAKE) -C $(SCRIPTSDIR)/lxdialog clean all

	$(CONFIG_SHELL) $(SCRIPTSDIR)/Menuconfig config.in
	@if [ ! -f .config ]; then \
		echo; \
		echo "You have not saved your config, please re-run make config"; \
		echo; \
		exit 1; \
	fi
	cp autoconf.h boot/include/linux/autoconf2.h
_opfiles:
	@echo "ln sc16is7x0 file from voip folder"
	@if [ "$(src)" == "" ] || [ ! -d $(src) ] || [ ! -f $(src)/voip_drivers/i2c.c ];  then		\
		echo "Can't find files in '$(src)'";								\
		echo -e "Syntax:\n\tmake $(OPcmd) src=.../rtk_voiop/";		\
		exit 1;														\
	fi
	$(OP) -f $(src)/voip_drivers/gpio/gpio_8972b.c				./boot/serial_sc16is7x0/gpio/gpio_8972b.c
	$(OP) -f $(src)/voip_drivers/gpio/gpio_8972b.h				./boot/serial_sc16is7x0/gpio/gpio_8972b.h
	$(OP) -f $(src)/voip_drivers/gpio/gpio.h					./boot/serial_sc16is7x0/gpio/gpio.h
	$(OP) -f $(src)/voip_drivers/i2c.c							./boot/serial_sc16is7x0/i2c.c
	$(OP) -f $(src)/voip_drivers/i2c.h							./boot/serial_sc16is7x0/i2c.h
	$(OP) -f $(src)/../drivers/serial/8250_sc16is7x0.c			./boot/serial_sc16is7x0/8250_sc16is7x0.c

lnfiles:
	@make _opfiles OP="ln -s" src="$(src)" OPcmd="lnfiles"

cpfiles:
	@make _opfiles OP="cp" src="$(src)" OPcmd="cpfiles"
