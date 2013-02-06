# The kernel tree you do builds in.
# Uncomment if you're building for the emulator
include ../.config
DOQUIET = false
#EMULATOR = true
#KERNEL_ROOT = /home/john/RealTek/linux-2.5/linux
KERNEL_ROOT = ..
#EDX
#CVIMG=../../AP/mkimg/tools/cvimg
CVIMG=../../AP/goahead-2.1.1/LINUX/cvimg
#EDX

# ---- shouldn't need to modify below this line.

ifdef EMULATOR
EMUOPTS = -DEMULATOR
LDSCRIPT = ld-emu.script
else
EMUOPTS =
LDSCRIPT = ld.script
endif

COPTIONS = -DROM_MEMORY -DCOMPRESSED_KERNEL

ifdef CONFIG_RTL_EB8186
COPTIONS += -DRTL8186
endif
#SOURCES = vsprintf.c string.c ctype.c prom_printf.c misc.c hfload.c start.S read_memory.c
ifdef BZ2_COMPRESS
	SOURCES = misc.c hfload.c start.S cache.c
else
ifdef GZIP_COMPRESS
	SOURCES = string.c ctype.c misc.c hfload.c start.S read_memory.c cache.c
else
	SOURCES = string.c ctype.c misc.c hfload.c start.S read_memory.c cache.c LzmaDecode.c
endif	
endif

LOADER_FILES = hfload.o read_memory.o
#LOADER_FILES = hfload.o
ifdef BZ2_COMPRESS
	SUPPORT_FILES = misc.o cache.o
else
ifdef GZIP_COMPRESS
	SUPPORT_FILES = vsprintf.o prom_printf.o string.o ctype.o misc.o cache.o	
else
	SUPPORT_FILES = vsprintf.o prom_printf.o string.o ctype.o misc.o cache.o LzmaDecode.o
endif	
endif

CFLAGS =-Os -g -fno-pic -mno-abicalls $(EMUOPTS) -DEMBEDDED -I$(KERNEL_ROOT)/include/linux -I$(KERNEL_ROOT)/include -I$(KERNEL_ROOT)/lib $(COPTIONS) -G 0
ASFLAGS = -g $(EMUOPTS) -DEMBEDDED -I$(KERNEL_ROOT)/include
LDFLAGS=-static -nostdlib


ifdef BZ2_COMPRESS
CFLAGS += -DBZ2_COMPRESS
ASFLAGS += -DBZ2_COMPRESS
else
ifndef GZIP_COMPRESS
CFLAGS += -DLZMA_COMPRESS
ASFLAGS += -DLZMA_COMPRESS
endif
endif

START_FILE = start.o

ifeq ($(strip $(DOQUIET)),true)
    #CFLAGS  += $(WARNINGS)  -D__DO_QUIET__
    CFLAGS  += $(WARNINGS)  -D__DO_QUIET__
	SUPPORT_FILES = string.o ctype.o misc.o
endif


ifeq ($(CONFIG_RTL_LINUX_IMAGE_OFFSET),)
FLASH_OFFSET=20000
else
FLASH_OFFSET=$(CONFIG_RTL_LINUX_IMAGE_OFFSET)
endif

ifdef CONFIG_RTL8186_AP
LOAD_START_ADDR=0x80500000
FLASH_OFFSET=30000
endif

ifdef CONFIG_RTL8186_GW
LOAD_START_ADDR=0x80500000
FLASH_OFFSET=30000
endif

ifdef CONFIG_RTL8186_GW_8M
LOAD_START_ADDR=0x80300000
FLASH_OFFSET=30000
endif


ifdef CONFIG_RTL8186_GW_VPN
LOAD_START_ADDR=0x80800000
FLASH_OFFSET=30000
endif

ifdef CONFIG_RTL8196B_GW
#ifdef CONFIG_RTL8196_RTL8366
#LOAD_START_ADDR=0x80A00000
#else
LOAD_START_ADDR=0x80500000
#endif
ifdef CONFIG_RTL8196B_GW_8M
FLASH_OFFSET=10000
else
ifeq ($(CONFIG_RTL_LINUX_IMAGE_OFFSET),)
FLASH_OFFSET=30000
else
FLASH_OFFSET=$(CONFIG_RTL_LINUX_IMAGE_OFFSET)
endif
endif
endif


ifdef CONFIG_RTL8196B
CVIMG=../../AP/goahead-2.1.1/LINUX/cvimg
endif


ifdef CONFIG_RTL8196B_AP_ROOT
LOAD_START_ADDR=0x80500000
FLASH_OFFSET=30000
endif

ifdef CONFIG_RTL8196C_AP_ROOT
LOAD_START_ADDR=0x80500000
endif

ifdef CONFIG_RTL8198_AP_ROOT
LOAD_START_ADDR=0x80500000
endif

ifdef CONFIG_RTL8186_DMA
LOAD_START_ADDR=0x80600000
endif

ifdef CONFIG_RTL8196C_CLIENT_ONLY
LOAD_START_ADDR=0x80500000
endif

ifdef CONFIG_SQUASHFS
#EDX
#CV_OPTION=linux-ro
CV_OPTION=linux
#EDX
else
CV_OPTION=linux
endif

ifdef CONFIG_RTK_VOIP
LOAD_START_ADDR=0x80900000
FLASH_OFFSET=30000
endif

ifdef CONFIG_RTL865X_CMO
LOAD_START_ADDR=0x80300000
FLASH_OFFSET=20000
endif
ifdef CONFIG_RTL8196C
ifdef CONFIG_RTL8196B_GW_MP
 USE_RSDK_1_2_11=1
 export USE_RSDK_1_2_11
else
 ifdef CONFIG_RTL8198
 USE_RSDK_98=1
 export USE_RSDK_98
 else
 USE_RSDK_96C=1
 export USE_RSDK_96C
endif
endif
endif

SEDFLAGS	= s/LOAD_ADDR/$(LOAD_START_ADDR)/;


O_TARGET := rtk
obj-y		:= vmlinux_img.o $(START_FILE) $(LOADER_FILES) $(SUPPORT_FILES)


all: rtk-clean rtk-vmlinux_img $(START_FILE) $(LOADER_FILES) $(SUPPORT_FILES)

#memload-partial
	$(CC) -fno-pic -mno-abicalls -Os -fomit-frame-pointer -D__KERNEL__ -c vmlinux_img.c -o vmlinux_img.o	
	$(OBJCOPY) --add-section .vmlinux=vmlinux_img.gz vmlinux_img.o
	@sed "$(SEDFLAGS)" < ld.script.in > $(LDSCRIPT)
	$(LD) $(LDFLAGS) -G 0 -T $(LDSCRIPT) -o memload-partial $(START_FILE) $(LOADER_FILES) $(SUPPORT_FILES) vmlinux_img.o
	$(NM) memload-partial | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aU] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | sort > system.map
	
	cp memload-partial memload-full
	$(OBJCOPY) -Obinary memload-full nfjrom
	$(CVIMG) $(CV_OPTION) nfjrom linux.bin $(LOAD_START_ADDR) $(FLASH_OFFSET)
#EDX
	cp nfjrom ../../image/linux
#	@./size_chk vmlinux_img $(LOAD_START_ADDR)
#	@$(CVIMG) size_chk vmlinux_img $(LOAD_START_ADDR)
#EDX
rtk-clean:
	rm -f *.o memload system.map nfjrom memload-partial memload-full vmlinux_img.gz target target.img strip1 linux.bin vmlinux-stripped  $(LDSCRIPT) vmlinux_img
	
rtk-vmlinux_img: 
	cp $(KERNEL_ROOT)/vmlinux vmlinux-stripped
	$(STRIP) vmlinux-stripped
	$(OBJCOPY) -Obinary vmlinux-stripped vmlinux_img
	rm -f vmlinux_img.gz
ifdef BZ2_COMPRESS
	cat vmlinux_img | bzip2 -9v > vmlinux_img.gz
else
ifdef GZIP_COMPRESS
	cat vmlinux_img | gzip -9v > vmlinux_img.gz	
else
	./lzma e vmlinux_img vmlinux_img.gz
endif	
endif
ifdef CONFIG_RTL8197B_PANA
	$(CVIMG) vmlinux vmlinux_img.gz vmlinux_img.gz
endif

target:	nfjrom $(BOOT_CODE) $(EXT2_IMG)
# Pading boot code to 512 bytes long (but only 256 bytes are allowed, 512 is to make it page alignment)
	$(OBJCOPY) -Obinary --pad-to=0x80000200 $(BOOT_CODE) bootcode
	$(OBJCOPY) -Obinary --remove-section=.bss --remove-section=.data --remove-section=.mdebug  --pad-to=0x806FFE00 memload-full nandrom
	$(LD) -G0 -Ttarget.script -o target  -bbinary bootcode -bbinary nandrom -bbinary $(EXT2_IMG)
	$(OBJCOPY) -Obinary target target.img		 
	cp -f ./target.img /tftpboot
depend:
	rm -f .depend
	$(CC) $(CFLAGS) -MM $(SOURCES) >.depend

bz2:
	@make BZ2_COMPRESS=1

gzip:
	@make GZIP_COMPRESS=1

include ../Rules.make
