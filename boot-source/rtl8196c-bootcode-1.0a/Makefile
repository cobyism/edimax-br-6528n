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

############################################################################
#
# Lets work out what the user wants, and if they have configured us yet
#
ARCH+=RTL865X=1


ifeq (.config,$(wildcard .config))
include .config
include $(LINUX_CONFIG)



ifeq ($(CONFIG_DEFAULTS_REALTEK_RTL8197B),y)
    DEFAULT_JUMP=0x806ffff0
#	ARCH=RTL8196B=1 RTL8197B=1

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
		
#16-bit 16M SDRAM for KLD 
kld:
	make -C boot boot JUMP_ADDR=$(DEFAULT_JUMP) $(ARCH) KLD=1 MD=0
	make -C btcode 16B=1 $(ARCH) KLD=1 LZMA=1 MD=0	
	make -C boot wboot $(ARCH) JUMP_ADDR=$(DEFAULT_JUMP) $(ARCH) KLD=1 MD=0

#16-bit 8M SDRAM for EC
ec:
	make -C boot boot JUMP_ADDR=$(DEFAULT_JUMP) $(ARCH) EC=1 MD=0
	make -C btcode 16B=1 $(ARCH) EC=1 LZMA=1 MD=0	
			
rtl8197b:
	make -C boot boot JUMP_ADDR=$(DEFAULT_JUMP) $(ARCH)
	make -C btcode $(ARCH)
#	cp btcode/boot.bin ../AP/panahost/rtl
	
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
