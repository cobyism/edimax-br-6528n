VERSION = 2
PATCHLEVEL = 4
SUBLEVEL = 18
EXTRAVERSION = -MIPS-01.00
#
KERNELRELEASE=$(VERSION).$(PATCHLEVEL).$(SUBLEVEL)$(EXTRAVERSION)

ARCH = mips
KERNELPATH=kernel-$(shell echo $(KERNELRELEASE) | sed -e "s/-//g")

CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)
TOPDIR	:= $(shell /bin/pwd)

include ../define/FUNCTION_COMPILER # EDX

HPATH   	= $(TOPDIR)/include
FINDHPATH	= $(HPATH)/asm $(HPATH)/linux $(HPATH)/scsi $(HPATH)/net

HOSTCC  	= gcc
HOSTCFLAGS	= -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer $(FUNCTION)

CROSS_COMPILE 	=

#
# Include the make variables (CC, etc...)
#

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
MAKEFILES	= $(TOPDIR)/.config
GENKSYMS	= /sbin/genksyms
DEPMOD		= /sbin/depmod
MODFLAGS	= -DMODULE
CFLAGS_KERNEL	= -nostdinc -g -DJACKSON_NET_WORK $(FUNCTION) #-gstabs+  #-DNAT_SPEEDUP -DDELAY_RX -DDRIVER_SPEEDUP
PERL		= perl

export	VERSION PATCHLEVEL SUBLEVEL EXTRAVERSION KERNELRELEASE ARCH \
	CONFIG_SHELL TOPDIR HPATH HOSTCC HOSTCFLAGS CROSS_COMPILE AS LD CC \
	CPP AR NM STRIP OBJCOPY OBJDUMP MAKE MAKEFILES GENKSYMS MODFLAGS PERL

all:	do-it-all

#
# Make "config" the default target if there is no configuration file or
# "depend" the target if there is no top-level dependency information.
#

ifeq (.config,$(wildcard .config))
include .config
ifeq (.depend,$(wildcard .depend))
include .depend
do-it-all:	Version vmlinux
else
CONFIGURATION = depend
do-it-all:	depend
endif
else
CONFIGURATION = config
do-it-all:	config
endif
#add for mesh
rtl8190_clean:
	rm -rf drivers/net/rtl8190/*.o
	rm -rf drivers/net/rtl8190/mesh_ext/*.o
popen:  rtl8190_clean do-it-all
#add for mesh
#
# INSTALL_PATH specifies where to place the updated kernel and system map
# images.  Uncomment if you want to place them anywhere other than root.
#

#export	INSTALL_PATH=/boot

#
# INSTALL_MOD_PATH specifies a prefix to MODLIB for module directory
# relocations required by build roots.  This is not defined in the
# makefile but the arguement can be passed to make if needed.
#

MODLIB	:= $(INSTALL_MOD_PATH)/lib/modules/$(KERNELRELEASE)
export MODLIB

#
# standard CFLAGS
#

CPPFLAGS := -D__KERNEL__ -I$(HPATH)

ifdef CONFIG_RTL8197B_PANA
USE_RSDK_1_2_11=1
export USE_RSDK_1_2_11
endif

ifdef CONFIG_RTL8196C
ifdef CONFIG_RTL8196B_GW_MP
USE_RSDK_1_2_11=1
export USE_RSDK_1_2_11
else
USE_RSDK_96C=1
export USE_RSDK_96C
endif
else
ifdef CONFIG_RTL8198

ifdef CONFIG_RTL8196B_GW_MP
USE_RSDK_1_2_11=1
export USE_RSDK_1_2_11
else
USE_RSDK_98=1
export USE_RSDK_98
endif

endif
endif

ifeq ($(USE_RSDK_1_2_11),1)
CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes -Wno-trigraphs -Os \
	  -fomit-frame-pointer -fno-strict-aliasing -fno-common -mlong-calls

COMMAND_SHELL := $(shell cp ./arch/mips/ld.script.in.1.2.x ./arch/mips/ld.script.in)
else
CFLAGS := $(CPPFLAGS) -Wall -Wstrict-prototypes -Wno-trigraphs -Os \
	  -fomit-frame-pointer -fno-strict-aliasing -fno-common -mlong-calls -DGCCPATCH_1_3_X

COMMAND_SHELL := $(shell cp ./arch/mips/ld.script.in.gccpatch1.3.x ./arch/mips/ld.script.in)
endif

AFLAGS := -D__ASSEMBLY__ $(CPPFLAGS)

#
# ROOT_DEV specifies the default root-device when making the image.
# This can be either FLOPPY, CURRENT, /dev/xxxx or empty, in which case
# the default of FLOPPY is used by 'build'.
# This is i386 specific.
#

export ROOT_DEV = CURRENT

#
# If you want to preset the SVGA mode, uncomment the next line and
# set SVGA_MODE to whatever number you want.
# Set it to -DSVGA_MODE=NORMAL_VGA if you just want the EGA/VGA mode.
# The number is the same as you would ordinarily press at bootup.
# This is i386 specific.
#

export SVGA_MODE = -DSVGA_MODE=NORMAL_VGA

#
# If you want the RAM disk device, define this to be the size in blocks.
# This is i386 specific.
#

#export RAMDISK = -DRAMDISK=512

CORE_FILES	=kernel/kernel.o mm/mm.o fs/fs.o ipc/ipc.o
NETWORKS	=net/network.o

ifeq ($(CONFIG_RTK_VOIP_DRIVERS_IP_PHONE),y)
LIBS		=$(TOPDIR)/lib/lib.a rtk_voip/voip_dsp/dsplib.obj
else
LIBS		=$(TOPDIR)/lib/lib.a
endif
SUBDIRS		=kernel drivers mm fs net ipc lib

ifdef CONFIG_VOIP_SDK
ifeq ($(CONFIG_RTK_VOIP),y)
SUBDIRS +=  rtk_voip
endif
else
ifneq ($(CONFIG_RTK_VOIP),)
SUBDIRS +=  rtk_voip
endif
endif

ifeq ($(CONFIG_AUDIOCODES_VOIP),y)
SUBDIRS +=  acmw_lx
ACLPRJDIR := $(TOPDIR)/acmw_lx
ACLPRJINCDIR := $(ACLPRJDIR)/include
ACLFLAGS :=
include $(ACLPRJDIR)/AclRules.make
export ACLPRJDIR ACLPRJINCDIR ACLFLAGS
endif
DRIVERS-n :=
DRIVERS-y :=
DRIVERS-m :=
DRIVERS-  :=

DRIVERS-$(CONFIG_ACPI) += drivers/acpi/acpi.o
DRIVERS-$(CONFIG_PARPORT) += drivers/parport/driver.o
DRIVERS-y += drivers/char/char.o \
	drivers/block/block.o \
	drivers/misc/misc.o \
	drivers/net/net.o \
	drivers/media/media.o
DRIVERS-$(CONFIG_AGP) += drivers/char/agp/agp.o
DRIVERS-$(CONFIG_DRM_NEW) += drivers/char/drm/drm.o
DRIVERS-$(CONFIG_DRM_OLD) += drivers/char/drm-4.0/drm.o
DRIVERS-$(CONFIG_NUBUS) += drivers/nubus/nubus.a
DRIVERS-$(CONFIG_ISDN) += drivers/isdn/isdn.a
DRIVERS-$(CONFIG_NET_FC) += drivers/net/fc/fc.o
DRIVERS-$(CONFIG_APPLETALK) += drivers/net/appletalk/appletalk.o
DRIVERS-$(CONFIG_TR) += drivers/net/tokenring/tr.o
DRIVERS-$(CONFIG_WAN) += drivers/net/wan/wan.o
DRIVERS-$(CONFIG_ARCNET) += drivers/net/arcnet/arcnetdrv.o
DRIVERS-$(CONFIG_ATM) += drivers/atm/atm.o
DRIVERS-$(CONFIG_IDE) += drivers/ide/idedriver.o
DRIVERS-$(CONFIG_FC4) += drivers/fc4/fc4.a
DRIVERS-$(CONFIG_SCSI) += drivers/scsi/scsidrv.o
DRIVERS-$(CONFIG_FUSION_BOOT) += drivers/message/fusion/fusion.o
DRIVERS-$(CONFIG_IEEE1394) += drivers/ieee1394/ieee1394drv.o

ifneq ($(CONFIG_CD_NO_IDESCSI)$(CONFIG_BLK_DEV_IDECD)$(CONFIG_BLK_DEV_SR)$(CONFIG_PARIDE_PCD),)
DRIVERS-y += drivers/cdrom/driver.o
endif

DRIVERS-$(CONFIG_SOUND) += drivers/sound/sounddrivers.o
DRIVERS-$(CONFIG_PCI) += drivers/pci/driver.o
#DRIVERS-$(CONFIG_MTD_RTL8196_SPI) += drivers/mtd/chips/rtl8196/spi_rtl8196.o

DRIVERS-$(CONFIG_MTD) += drivers/mtd/mtdlink.o
ifndef CONFIG_MTD
DRIVERS-$(CONFIG_FLASH) += drivers/flash/flash.o
endif
#DRIVERS-$(CONFIG_MTD_RTL8196_SPI) += drivers/mtd/maps/rtl8196_flash.o

DRIVERS-$(CONFIG_PCMCIA) += drivers/pcmcia/pcmcia.o
DRIVERS-$(CONFIG_NET_PCMCIA) += drivers/net/pcmcia/pcmcia_net.o
DRIVERS-$(CONFIG_NET_WIRELESS) += drivers/net/wireless/wireless_net.o
#rtl8366.o
DRIVERS-$(CONFIG_RTL8196_RTL8366) += drivers/net/rtl865x/RTL8366RB_DRIVER/rtl8366.o
ifndef CONFIG_WIRELESS_LAN_MODULE
DRIVERS-$(CONFIG_RTL8192CD) += drivers/net/rtl8192cd/rtl8192cd.o
endif
DRIVERS-$(CONFIG_PCMCIA_CHRDEV) += drivers/char/pcmcia/pcmcia_char.o
DRIVERS-$(CONFIG_DIO) += drivers/dio/dio.a
DRIVERS-$(CONFIG_SBUS) += drivers/sbus/sbus_all.o
DRIVERS-$(CONFIG_ZORRO) += drivers/zorro/driver.o
DRIVERS-$(CONFIG_FC4) += drivers/fc4/fc4.a
DRIVERS-$(CONFIG_ALL_PPC) += drivers/macintosh/macintosh.o
DRIVERS-$(CONFIG_MAC) += drivers/macintosh/macintosh.o
DRIVERS-$(CONFIG_ISAPNP) += drivers/pnp/pnp.o
DRIVERS-$(CONFIG_SGI_IP22) += drivers/sgi/sgi.a
DRIVERS-$(CONFIG_VT) += drivers/video/video.o
DRIVERS-$(CONFIG_PARIDE) += drivers/block/paride/paride.a
DRIVERS-$(CONFIG_HAMRADIO) += drivers/net/hamradio/hamradio.o
DRIVERS-$(CONFIG_TC) += drivers/tc/tc.a
DRIVERS-$(CONFIG_USB) += drivers/usb/usbdrv.o
DRIVERS-$(CONFIG_INPUT) += drivers/input/inputdrv.o
DRIVERS-$(CONFIG_I2O) += drivers/message/i2o/i2o.o
DRIVERS-$(CONFIG_IRDA) += drivers/net/irda/irda.o
DRIVERS-$(CONFIG_I2C) += drivers/i2c/i2c.o
DRIVERS-$(CONFIG_PHONE) += drivers/telephony/telephony.o
DRIVERS-$(CONFIG_MD) += drivers/md/mddev.o
DRIVERS-$(CONFIG_BLUEZ) += drivers/bluetooth/bluetooth.o
#DRIVERS-$(CONFIG_RTL8139CP) += drivers/net/rtl8181/rtl8181.o
#DRIVERS-$(CONFIG_RTL8186) += drivers/net/rtl8186/rtl8186.o
DRIVERS-$(CONFIG_RTL865X_ETH) += drivers/net/rtl865x/rtl865x.o
DRIVERS-$(CONFIG_HOTPLUG_PCI) += drivers/hotplug/vmlinux-obj.o
DRIVERS-$(CONFIG_VW2010) += drivers/codec/vw2010/codec.o
DRIVERS-$(CONFIG_RTK_VOIP) += rtk_voip/rtk_voip.o
DRIVERS-$(CONFIG_AUDIOCODES_VOIP) += acmw_lx/aclxx.o

ifeq ($(CONFIG_PANA_WLAN),y)
DRIVERS-y += drivers/net/wlan/wlan.o
endif

DRIVERS := $(DRIVERS-y)


# files removed with 'make clean'
CLEAN_FILES = \
	kernel/ksyms.lst include/linux/compile.h \
	vmlinux System.map \
	.tmp* \
	drivers/char/consolemap_deftbl.c drivers/video/promcon_tbl.c \
	drivers/char/conmakehash \
	drivers/char/drm/*-mod.c \
	drivers/pci/devlist.h drivers/pci/classlist.h drivers/pci/gen-devlist \
	drivers/zorro/devlist.h drivers/zorro/gen-devlist \
	drivers/sound/bin2hex drivers/sound/hex2hex \
	drivers/atm/fore200e_mkfirm drivers/atm/{pca,sba}*{.bin,.bin1,.bin2} \
	drivers/scsi/aic7xxx/aicasm/aicasm_gram.c \
	drivers/scsi/aic7xxx/aicasm/aicasm_scan.c \
	drivers/scsi/aic7xxx/aicasm/y.tab.h \
	drivers/scsi/aic7xxx/aicasm/aicasm \
	drivers/scsi/53c700_d.h \
	net/khttpd/make_times_h \
	net/khttpd/times.h \
	submenu* \
       rtkload/linux_checksum \
       rtkload/memload-full \
       rtkload/memload-partial \
       rtkload/nfjrom \
       rtkload/system.map \
       rtkload/vmlinux-stripped \
       rtkload/vmlinux_img \
       rtkload/vmlinux_img.gz \
       drivers/net/rtl8192cd/data_*.c
# directories removed with 'make clean'
CLEAN_DIRS = \
	modules

# files removed with 'make mrproper'
MRPROPER_FILES = \
	include/linux/autoconf.h include/linux/version.h \
	drivers/net/hamradio/soundmodem/sm_tbl_{afsk1200,afsk2666,fsk9600}.h \
	drivers/net/hamradio/soundmodem/sm_tbl_{hapn4800,psk4800}.h \
	drivers/net/hamradio/soundmodem/sm_tbl_{afsk2400_7,afsk2400_8}.h \
	drivers/net/hamradio/soundmodem/gentbl \
	drivers/sound/*_boot.h drivers/sound/.*.boot \
	drivers/sound/msndinit.c \
	drivers/sound/msndperm.c \
	drivers/sound/pndsperm.c \
	drivers/sound/pndspini.c \
	drivers/atm/fore200e_*_fw.c drivers/atm/.fore200e_*.fw \
	.version .config* config.in config.old \
	scripts/tkparse scripts/kconfig.tk scripts/kconfig.tmp \
	scripts/lxdialog/*.o scripts/lxdialog/lxdialog \
	.menuconfig.log \
	include/asm \
	.hdepend scripts/mkdep scripts/split-include scripts/docproc \
	$(TOPDIR)/include/linux/modversions.h \
	kernel.spec

# directories removed with 'make mrproper'
MRPROPER_DIRS = \
	include/config \
	$(TOPDIR)/include/linux/modules


include arch/$(ARCH)/Makefile

export	CPPFLAGS CFLAGS CFLAGS_KERNEL AFLAGS AFLAGS_KERNEL

export	NETWORKS DRIVERS LIBS HEAD LDFLAGS LINKFLAGS MAKEBOOT ASFLAGS

.S.s:
	$(CPP) $(AFLAGS) $(AFLAGS_KERNEL) -traditional -o $*.s $<
.S.o:
	$(CC) $(AFLAGS) $(AFLAGS_KERNEL) -traditional -c -o $*.o $<

Version: dummy
	@rm -f include/linux/compile.h

boot: vmlinux
	@$(MAKE) CFLAGS="$(CFLAGS) $(CFLAGS_KERNEL)" -C arch/$(ARCH)/boot

clean_o16:
			find . \( -name '*.o16' \) -type f -print \
		| grep -v lxdialog/ | xargs rm -f
			
vmlinux: clean_o16 include/linux/version.h $(CONFIGURATION) init/main.o init/version.o linuxsubdirs
	$(LD) $(LINKFLAGS) $(HEAD) init/main.o init/version.o \
		--start-group \
		$(CORE_FILES) \
		$(DRIVERS) \
		$(NETWORKS) \
		$(LIBS) \
		--end-group \
		-o vmlinux
	$(NM) vmlinux | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | sort > System.map

symlinks:
	rm -f include/asm
	( cd include ; ln -sf asm-$(ARCH) asm)
	@if [ ! -d include/linux/modules ]; then \
		mkdir include/linux/modules; \
	fi
ifeq ($(CONFIG_PANA_WLAN),y)
	rm -f drivers/net/wlan
	cd drivers/net; ln -sf ../../../AP/pana/wlan wlan
endif

oldconfig: symlinks
	$(CONFIG_SHELL) scripts/Configure -d arch/$(ARCH)/config.in

xconfig: symlinks
	$(MAKE) -C scripts kconfig.tk
	wish -f scripts/kconfig.tk

menuconfig: include/linux/version.h symlinks
	$(MAKE) -C scripts/lxdialog all
	$(CONFIG_SHELL) scripts/Menuconfig arch/$(ARCH)/config.in

config: symlinks
	$(CONFIG_SHELL) scripts/Configure arch/$(ARCH)/config.in

include/config/MARKER: scripts/split-include include/linux/autoconf.h
	scripts/split-include include/linux/autoconf.h include/config
	@ touch include/config/MARKER

linuxsubdirs: $(patsubst %, _dir_%, $(SUBDIRS))

$(patsubst %, _dir_%, $(SUBDIRS)) : dummy include/linux/version.h include/config/MARKER
	$(MAKE) CFLAGS="$(CFLAGS) $(CFLAGS_KERNEL)" -C $(patsubst _dir_%, %, $@)

$(TOPDIR)/include/linux/version.h: include/linux/version.h
$(TOPDIR)/include/linux/compile.h: include/linux/compile.h

newversion:
	. scripts/mkversion > .tmpversion
	@mv -f .tmpversion .version

include/linux/compile.h: $(CONFIGURATION) include/linux/version.h newversion
	@echo -n \#define UTS_VERSION \"\#`cat .version` > .ver
	@if [ -n "$(CONFIG_SMP)" ] ; then echo -n " SMP" >> .ver; fi
	@if [ -f .name ]; then  echo -n \-`cat .name` >> .ver; fi
	@LANG=en; echo ' '`date`'"' >> .ver
	@echo \#define LINUX_COMPILE_TIME \"`date +%T`\" >> .ver
	@echo \#define LINUX_COMPILE_BY \"`whoami`\" >> .ver
	@echo \#define LINUX_COMPILE_HOST \"`hostname`\" >> .ver
	@if [ -x /bin/dnsdomainname ]; then \
	   echo \#define LINUX_COMPILE_DOMAIN \"`dnsdomainname`\"; \
	 elif [ -x /bin/domainname ]; then \
	   echo \#define LINUX_COMPILE_DOMAIN \"`domainname`\"; \
	 else \
	   echo \#define LINUX_COMPILE_DOMAIN ; \
	 fi >> .ver
	@echo \#define LINUX_COMPILER \"`$(CC) $(CFLAGS) -v 2>&1 | tail -1`\" >> .ver
	@mv -f .ver $@

include/linux/version.h: ./Makefile
	@echo \#define UTS_RELEASE \"$(KERNELRELEASE)\" > .ver
	@echo \#define LINUX_VERSION_CODE `expr $(VERSION) \\* 65536 + $(PATCHLEVEL) \\* 256 + $(SUBLEVEL)` >> .ver
	@echo '#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))' >>.ver
	@mv -f .ver $@

comma	:= ,

init/version.o: init/version.c include/linux/compile.h include/config/MARKER
	$(CC) $(CFLAGS) $(CFLAGS_KERNEL) -DUTS_MACHINE='"$(ARCH)"' -DKBUILD_BASENAME=$(subst $(comma),_,$(subst -,_,$(*F))) -c -o init/version.o init/version.c

init/main.o: init/main.c include/config/MARKER
	$(CC) $(CFLAGS) $(CFLAGS_KERNEL) $(PROFILING) -DKBUILD_BASENAME=$(subst $(comma),_,$(subst -,_,$(*F))) -c -o $*.o $<

fs lib mm ipc kernel drivers net: dummy
	$(MAKE) CFLAGS="$(CFLAGS) $(CFLAGS_KERNEL)" $(subst $@, _dir_$@, $@)

TAGS: dummy
	etags `find include/asm-$(ARCH) -name '*.h'`
	find include -type d \( -name "asm-*" -o -name config \) -prune -o -name '*.h' -print | xargs etags -a
	find $(SUBDIRS) init -name '*.[ch]' | xargs etags -a

# Exuberant ctags works better with -I
tags: dummy
	CTAGSF=`ctags --version | grep -i exuberant >/dev/null && echo "-I __initdata,__exitdata,EXPORT_SYMBOL,EXPORT_SYMBOL_NOVERS"`; \
	ctags $$CTAGSF `find include/asm-$(ARCH) -name '*.h'` && \
	find include -type d \( -name "asm-*" -o -name config \) -prune -o -name '*.h' -print | xargs ctags $$CTAGSF -a && \
	find $(SUBDIRS) init -name '*.[ch]' | xargs ctags $$CTAGSF -a

ifdef CONFIG_MODULES
ifdef CONFIG_MODVERSIONS
MODFLAGS += -DMODVERSIONS -include $(HPATH)/linux/modversions.h
endif

.PHONY: modules
modules: $(patsubst %, _mod_%, $(SUBDIRS))

.PHONY: $(patsubst %, _mod_%, $(SUBDIRS))
$(patsubst %, _mod_%, $(SUBDIRS)) : include/linux/version.h include/config/MARKER
	$(MAKE) -C $(patsubst _mod_%, %, $@) CFLAGS="$(CFLAGS) $(MODFLAGS)" MAKING_MODULES=1 modules

.PHONY: modules_install
modules_install: _modinst_ $(patsubst %, _modinst_%, $(SUBDIRS)) _modinst_post

.PHONY: _modinst_
_modinst_:
	@rm -rf $(MODLIB)/kernel
	@rm -f $(MODLIB)/build
	@mkdir -p $(MODLIB)/kernel
	@ln -s $(TOPDIR) $(MODLIB)/build

# If System.map exists, run depmod.  This deliberately does not have a
# dependency on System.map since that would run the dependency tree on
# vmlinux.  This depmod is only for convenience to give the initial
# boot a modules.dep even before / is mounted read-write.  However the
# boot script depmod is the master version.
ifeq "$(strip $(INSTALL_MOD_PATH))" ""
depmod_opts	:=
else
depmod_opts	:= -b $(INSTALL_MOD_PATH) -r
endif
.PHONY: _modinst_post
_modinst_post: _modinst_post_pcmcia
	if [ -r System.map ]; then $(DEPMOD) -ae -F System.map $(depmod_opts) $(KERNELRELEASE); fi

# Backwards compatibilty symlinks for people still using old versions
# of pcmcia-cs with hard coded pathnames on insmod.  Remove
# _modinst_post_pcmcia for kernel 2.4.1.
.PHONY: _modinst_post_pcmcia
_modinst_post_pcmcia:
	cd $(MODLIB); \
	mkdir -p pcmcia; \
	find kernel -path '*/pcmcia/*' -name '*.o' | xargs -i -r ln -sf ../{} pcmcia

.PHONY: $(patsubst %, _modinst_%, $(SUBDIRS))
$(patsubst %, _modinst_%, $(SUBDIRS)) :
	$(MAKE) -C $(patsubst _modinst_%, %, $@) modules_install

# modules disabled....

else
modules modules_install: dummy
	@echo
	@echo "The present kernel configuration has modules disabled."
	@echo "Type 'make config' and enable loadable module support."
	@echo "Then build a kernel with module support enabled."
	@echo
	@exit 1
endif

clean:	archclean
	find . \( -name '*.[oas]' -o -name '*.o16' -o -name core -o -name '.*.flags' \) -type f -print \
		| grep -v lxdialog/ | xargs rm -f
	rm -f $(CLEAN_FILES)
	rm -rf $(CLEAN_DIRS)
	$(MAKE) -C Documentation/DocBook clean
ifeq ($(CONFIG_PANA_WLAN),y)
	$(MAKE) -C drivers/net/wlan clean
endif

mrproper: clean archmrproper
	find . \( -size 0 -o -name .depend \) -type f -print | xargs rm -f
	rm -f $(MRPROPER_FILES)
	rm -rf $(MRPROPER_DIRS)
	$(MAKE) -C Documentation/DocBook mrproper

distclean: mrproper
	find . \( -not -type d \) -and \
		\( -name core -o -name '*.orig' -o -name '*.rej' \
		-o -name '*~' -o -name '*.bak' -o -name '#*#' \
		-o -name '.*.rej' -o -name '.SUMS' -o -size 0 \
		-o -name TAGS -o -name tags \) -print | env -i xargs rm -f

backup: mrproper
	cd .. && tar cf - linux/ | gzip -9 > backup.gz
	sync

sgmldocs: 
	chmod 755 $(TOPDIR)/scripts/docgen
	chmod 755 $(TOPDIR)/scripts/gen-all-syms
	chmod 755 $(TOPDIR)/scripts/kernel-doc
	$(MAKE) -C $(TOPDIR)/Documentation/DocBook books

psdocs: sgmldocs
	$(MAKE) -C Documentation/DocBook ps

pdfdocs: sgmldocs
	$(MAKE) -C Documentation/DocBook pdf

htmldocs: sgmldocs
	$(MAKE) -C Documentation/DocBook html

sums:
	find . -type f -print | sort | env -i xargs sum > .SUMS

dep-files: scripts/mkdep archdep include/linux/version.h
	scripts/mkdep -- init/*.c > .depend
	find $(FINDHPATH) -name SCCS -prune -o -follow -name \*.h ! -name modversions.h -print | env -i PATH="$(PATH)" HPATH="$(HPATH)" xargs scripts/mkdep -- > .hdepend
	$(MAKE) $(patsubst %,_sfdep_%,$(SUBDIRS)) _FASTDEP_ALL_SUB_DIRS="$(SUBDIRS)"
ifdef CONFIG_MODVERSIONS
	$(MAKE) update-modverfile
endif

ifdef CONFIG_MODVERSIONS
MODVERFILE := $(TOPDIR)/include/linux/modversions.h
else
MODVERFILE :=
endif
export	MODVERFILE

depend dep: dep-files
ifeq ($(CONFIG_AUDIOCODES_VOIP),y)
	ln -fs $(TOPDIR)/acmw_lx/include/userdef.h $(TOPDIR)/rtk_voip/include/acmw_userdef.h
endif
checkconfig:
	find * -name '*.[hcS]' -type f -print | sort | xargs $(PERL) -w scripts/checkconfig.pl

checkhelp:
	find * -name [cC]onfig.in -print | sort | xargs $(PERL) -w scripts/checkhelp.pl

checkincludes:
	find * -name '*.[hcS]' -type f -print | sort | xargs $(PERL) -w scripts/checkincludes.pl

ifdef CONFIGURATION
..$(CONFIGURATION):
	@echo
	@echo "You have a bad or nonexistent" .$(CONFIGURATION) ": running 'make" $(CONFIGURATION)"'"
	@echo
	$(MAKE) $(CONFIGURATION)
	@echo
	@echo "Successful. Try re-making (ignore the error that follows)"
	@echo
	exit 1

#dummy: ..$(CONFIGURATION)
dummy:

else

dummy:

endif

include Rules.make

#
# This generates dependencies for the .h files.
#

scripts/mkdep: scripts/mkdep.c
	$(HOSTCC) $(HOSTCFLAGS) -o scripts/mkdep scripts/mkdep.c

scripts/split-include: scripts/split-include.c
	$(HOSTCC) $(HOSTCFLAGS) -o scripts/split-include scripts/split-include.c

#
# RPM target
#
#	If you do a make spec before packing the tarball you can rpm -ta it
#
spec:
	. scripts/mkspec >kernel.spec

#
#	Build a tar ball, generate an rpm from it and pack the result
#	There arw two bits of magic here
#	1) The use of /. to avoid tar packing just the symlink
#	2) Removing the .dep files as they have source paths in them that
#	   will become invalid
#
rpm:	clean spec
	find . \( -size 0 -o -name .depend -o -name .hdepend \) -type f -print | xargs rm -f
	set -e; \
	cd $(TOPDIR)/.. ; \
	ln -sf $(TOPDIR) $(KERNELPATH) ; \
	tar -cvz --exclude CVS -f $(KERNELPATH).tar.gz $(KERNELPATH)/. ; \
	rm $(KERNELPATH) ; \
	cd $(TOPDIR) ; \
	. scripts/mkversion > .version ; \
	rpm -ta $(TOPDIR)/../$(KERNELPATH).tar.gz ; \
	rm $(TOPDIR)/../$(KERNELPATH).tar.gz
