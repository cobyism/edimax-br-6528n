##/*-------------------------------------------------------------------
##		Created by REALTEK  
##--------------------------------------------------------------------*/
include ../.config
CROSS = mips-linux-
#CROSS = rsdk-elf-

OUTDIR	= ./Output

SRC	= ./monitor

CC	= $(CROSS)gcc

AS	= $(CROSS)as

AR	= $(CROSS)ar crs

LD	= $(CROSS)ld

OBJCOPY = $(CROSS)objcopy
#OBJCOPY = objcopy

OBJDUMP	= $(CROSS)objdump

IMG2BIN	= $(CROSS)img2bin

NM	= $(CROSS)nm

RM	= rm


TOOLCFLAGS	=

TOOLLDFLAGS	= -n

OPT	=  -G 0

TEXT	=

INCLUDES	= -I. -I./include



ifdef RTL8196B
CFLAGS	= -march=4181 -g  -fomit-frame-pointer -nostdinc -fno-pic -mno-abicalls $(TOOLCFLAGS) $(OPT) $(INCLUDES)\
          -D__KERNEL__\
          -Dlinux\
          -O 
endif
ifdef RTL8196C
CFLAGS	= -march=4181 -g  -Os -fomit-frame-pointer -fno-pic -mno-abicalls $(TOOLCFLAGS) $(OPT) $(INCLUDES)\
          -D__KERNEL__\
          -Dlinux\
          -O 
endif
ifdef RTL8198
CFLAGS	= -march=5281 -g  -Os -fomit-frame-pointer -fno-pic -mno-abicalls $(TOOLCFLAGS) $(OPT) $(INCLUDES)\
          -D__KERNEL__\
          -Dlinux\
          -O 
endif
       
ifeq ($(CONFIG_DHCP_SERVER),y)
CFLAGS += -DDHCP_SERVER
endif
ifeq ($(CONFIG_HTTP_SERVER),y)
CFLAGS += -DHTTP_SERVER
endif
ifeq ($(CONFIG_POST_ENABLE),y)
CFLAGS += -DCONFIG_POST_ENABLE
endif    

ifeq ($(CONFIG_BOOT_DEBUG_ENABLE),y)
CFLAGS += -DCONFIG_BOOT_DEBUG_ENABLE
endif

#------------------------------------------------------------------------------------------
ifneq "$(strip $(JUMP_ADDR))" ""
CFLAGS += -DJUMP_ADDR=$(JUMP_ADDR)
endif



ifeq ($(RTL865X),1)
CFLAGS += -DRTL865X=1 -DCONFIG_RTL865X=y -DCONFIG_RTL865XC=1 
CFLAGSW = $(CFLAGS) -DWRAPPER -DRTL865X
endif

ifeq ($(RTL8196B),1)
CFLAGS += -DRTL8196B=1 -DCONFIG_RTL865XC=1
CFLAGSW = $(CFLAGS) -DWRAPPER -DRTL8196B
endif

ifeq ($(RTL8198),1)
CFLAGS += -DRTL8198=1 -DCONFIG_RTL865XC=1
CFLAGSW = $(CFLAGS) -DWRAPPER -DRTL8198
endif

ifeq ($(RTL8196C),1)
EXTRA_CFLAGS += -DRTL8196C=1
endif
ifeq ($(RTL8196C),1)
CFLAGS += -DRTL8196C=1 -DCONFIG_RTL865XC=1
CFLAGSW = $(CFLAGS) -DWRAPPER -DRTL8196C
endif

ifeq ($(CONFIG_FPGA_PLATFORM),1)
CFLAGS += -DCONFIG_FPGA_PLATFORM=1
endif

ifdef CONFIG_RTL89xxC
CFLAGS-y += -DCONFIG_RTK_VOIP
endif

#--------------------------------------
ifeq ($(CONFIG_DDR_SDRAM),y)
CFLAGS += -DDDR_SDRAM
endif
ifeq ($(CONFIG_DDR1_SDRAM),y)
CFLAGS += -DDDR1_SDRAM
endif
ifeq ($(CONFIG_DDR2_SDRAM),y)
CFLAGS += -DDDR2_SDRAM
endif
#--------------------------------------
ifeq ($(CONFIG_SW_8366GIGA),y)
CFLAGS += -DSW_8366GIGA
endif
ifeq ($(CONFIG_SERIAL_SC16IS7X0),y)
ifdef CONFIG_RTL89xxC
CFLAGS-y += -DCONFIG_RTK_VOIP_DRIVERS_PCM89xxC -D__BOOTCODE__
CFLAGS-y += -DCONFIG_RTK_VOIP_GPIO_8954C_V200
else
CFLAGS-y += -DCONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY -D__BOOTCODE__
endif
CFLAGS-$(CONFIG_SERIAL_SC16IS7X0) += -DCONFIG_SERIAL_SC16IS7X0
CFLAGS-$(CONFIG_SERIAL_SC16IS7X0_CONSOLE) += -DCONFIG_SERIAL_SC16IS7X0_CONSOLE
CFLAGS-$(CONFIG_SERIAL_SC16IS7X0_XTAL1_CLK_1843200) += -DCONFIG_SERIAL_SC16IS7X0_XTAL1_CLK_1843200
CFLAGS-$(CONFIG_SERIAL_SC16IS7X0_XTAL1_CLK_14746500) += -DCONFIG_SERIAL_SC16IS7X0_XTAL1_CLK_14746500
CFLAGS += $(CFLAGS-y)
endif
#--------------------------------------

ifeq ($(CONFIG_SPI_STD_MODE),y)
CFLAGS += -DCONFIG_SPI_STD_MODE
endif

LDFLAGS = -nostdlib  -T./ld.script  -EB   --static
WLDFLAGS = -nostdlib  -T./ldw.script  -EB   --static
ASFLAGS	=  -D__ASSEMBLY__  -x assembler-with-cpp -G 0

CRT	=

LIBS	=

.SUFFIXES : .s .S .c .o .out .nm .img .sr .sre .text .bin .scr

#wboot(wrapped) means boot code will be wrapped in this code
all:
	@echo "No rules to make target. You should choose:"
	@echo "make { boot | wboot }  boot for general, wboot for wrapped boot code"

.PHONY: banner
banner:	cr-output
	@echo  \#define BOOT_CODE_TIME \"`date "+%Y.%m.%d-%H:%M%z"`\" > ./banner/mk_time	 	
	
cr-output:
	@if [ ! -d Output ]; then \
	mkdir Output; \
	fi


wboot: banner $(OUTDIR)/wboot.out
	$(NM)	./Output/wboot.out | sort > ./Output/wboot.nm
	$(OBJCOPY) -g -Obinary ./Output/wboot.out ./Output/wboot.img
	mips-linux-objdump -h -S ./Output/wboot.out > ./Output/wboot.text


#all :	"$(OUTDIR)" boot.out
boot: banner $(OUTDIR)/boot.out
	$(NM)	./Output/boot.out | sort > ./Output/boot.nm
	# $(OBJCOPY) -g -Obinary ./Output/boot.out ./Output/boot.img
	mips-linux-objcopy -g -Obinary ./Output/boot.out ./Output/boot.img
	mips-linux-objdump -h -S ./Output/boot.out > ./Output/boot.text
#david
	cp ./Output/boot.img ../btcode/boot.img -f

#"$(OUTDIR)" :

##########################head.o must be the first.

OBJFILES	= ./Output/head.o ./Output/eth_tftpd.o ./Output/irq.o ./Output/inthandler.o ./Output/setup.o ./Output/utility.o  ./Output/main.o ./Output/traps.o $(OUTDIR)/init.o $(OUTDIR)/string.o  $(OUTDIR)/strtol.o $(OUTDIR)/strtoul.o $(OUTDIR)/ctool.o $(OUTDIR)/misc.o  $(OUTDIR)/calloc.o

#--------------------------------------
ifdef CONFIG_SPI_FLASH
OBJFILES += $(OUTDIR)/spi_flash.o
OBJFILES += $(OUTDIR)/spi_common.o
else
OBJFILES += $(OUTDIR)/flash.o
endif
#--------------------------------------

#--------------------------------------
ifeq ($(CONFIG_DHCP_SERVER),y)
OBJFILES += ./Output/dhcpd.o
endif
#--------------------------------------
ifeq ($(CONFIG_HTTP_SERVER),y)
OBJFILES += ./Output/utcp.o 
endif
#--------------------------------------


ifdef RTL8196B
OBJFILES += ./Output/ethInt_865x.o \
./Output/swCore.o \
./Output/swNic_poll.o \
./Output/swTable.o \
./Output/vlanTable.o
endif
ifdef RTL8198
OBJFILES += ./Output/ethInt_865x.o \
./Output/swCore.o \
./Output/swNic_poll.o \
./Output/swTable.o \
./Output/vlanTable.o
endif
#--------------------------------------
ifdef CONFIG_PCIE_MODULE
OBJFILES += ./Output/test_hw.o
OBJFILES += ./Output/test_8168n.o
endif
#------------------------------------
ifeq ($(CONFIG_SPE_MODULE),y)
ifeq ($(CONFIG_SPE_HOST),y)
CFLAGS += -DCONFIG_R8198EP_HOST
else
CFLAGS += -DCONFIG_R8198EP_DEVICE
endif
OBJFILES += ./Output/test_slvpcie.o
endif

ifeq ($(CONFIG_RTL8198_TAROKO),y)
TAROKO_TEST = \
$(SRC)/rtl8198_taroko/rtl8198_cpu_test.c	\
$(SRC)/rtl8198_taroko/idmem_dma_op.c		\

TAROKO_TEST_ASM = \
$(SRC)/rtl8198_taroko/lexra_load_store.S

endif

ifeq ($(CONFIG_SERIAL_SC16IS7X0),y)
ifdef CONFIG_RTL89xxC
OBJFILES += $(OUTDIR)/i2c.o $(OUTDIR)/8250_sc16is7x0.o
else
OBJFILES += $(OUTDIR)/gpio_8972b.o $(OUTDIR)/i2c.o $(OUTDIR)/8250_sc16is7x0.o
endif
endif

ifeq ($(CONFIG_PCM_TEST),y)
PCM_IIS_TEST = \
$(SRC)/pcm/pcm_isr.c			\
$(SRC)/pcm/iis_isr.c			\
$(SRC)/pcm/ALC5621.c			\
$(SRC)/pcm/fpga_gpio.c			\

endif

SRCLIST = \
$(PCM_IIS_TEST)				\
$(TAROKO_TEST)				\

OBJS	= $(addprefix $(OUTDIR)/,$(addsuffix .o,$(notdir $(basename $(SRCLIST))) ))

$(OBJS):$(SRCLIST)
	$(CC) -c $(CFLAGS) $(filter %$(addsuffix .c,$(notdir $(basename $@))),$(SRCLIST) ) -o $@

ASMLIST = \
$(TAROKO_TEST_ASM)			\

ASMOBJS = $(addprefix $(OUTDIR)/,$(addsuffix .o,$(notdir $(basename $(ASMLIST))) ))

$(ASMOBJS):$(ASMLIST)
	$(CC) -c $(CFLAGS) $(ASFLAGS) $(filter %$(addsuffix .S,$(notdir $(basename $@))),$(ASMLIST) ) -o $@



#------------------------------------

#------------------------------------

#--------------------------------------
ifeq ($(CONFIG_HISTORY_KEYIN),y)
CFLAGS += -DCONFIG_HISTORY_KEYIN
endif
#--------------------------------------

##Assembly file######################################################
$(OUTDIR)/inthandler.o: ./init/inthandler.S 
	$(CC) -c $(CFLAGS) $(ASFLAGS) -o $(OUTDIR)/inthandler.o ./init/inthandler.S
$(OUTDIR)/head.o: ./arch/mips/kernel/head.S 
	$(CC) -c $(CFLAGS) $(ASFLAGS) -o $(OUTDIR)/head.o ./arch/mips/kernel/head.S
#####################################################################
##C file#############################################################
$(OUTDIR)/irq.o: ./init/irq.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/irq.o ./init/irq.c

#$(OUTDIR)/time.o: ./init/time.c 
#	$(CC) -c $(CFLAGS) -o $(OUTDIR)/time.o ./init/time.c

#$(OUTDIR)/timer.o: ./timer/timer.c 
#	$(CC) -c $(CFLAGS) -o $(OUTDIR)/timer.o ./timer/timer.c

$(OUTDIR)/eth_tftpd.o: ./init/eth_tftpd.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/eth_tftpd.o ./init/eth_tftpd.c


#-----------------------------------------------------------------------------

#----------------------------------------------------------------------------
ifeq ($(CONFIG_DHCP_SERVER),y)
$(OUTDIR)/dhcpd.o: ./init/dhcpd.c
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/dhcpd.o ./init/dhcpd.c
endif

ifeq ($(CONFIG_HTTP_SERVER),y)
$(OUTDIR)/utcp.o: ./init/utcp.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/utcp.o ./init/utcp.c
endif
#----------------------------------------------------------------------------
$(OUTDIR)/ethInt_865x.o: ./init/ethInt_865x.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/ethInt_865x.o ./init/ethInt_865x.c
	
$(OUTDIR)/swCore.o: ./rtl8196x/swCore.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/swCore.o ./rtl8196x/swCore.c
	
$(OUTDIR)/swNic_poll.o: ./rtl8196x/swNic_poll.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/swNic_poll.o ./rtl8196x/swNic_poll.c
	
$(OUTDIR)/swTable.o: ./rtl8196x/swTable.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/swTable.o ./rtl8196x/swTable.c
	
$(OUTDIR)/vlanTable.o: ./rtl8196x/vlanTable.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/vlanTable.o ./rtl8196x/vlanTable.c
	
#----------------------------------------------------------------------------		
ifdef CONFIG_SPI_FLASH	
$(OUTDIR)/spi_common.o: ./flash/spi_common.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/spi_common.o ./flash/spi_common.c
$(OUTDIR)/spi_flash.o: ./flash/spi_flash.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/spi_flash.o ./flash/spi_flash.c
else
$(OUTDIR)/flash.o: ./flash/flash.c 
	$(CC) -c $(CFLAGS)  -o $(OUTDIR)/flash.o ./flash/flash.c		
endif
#----------------------------------------------------------------------------
ifdef CONFIG_PCIE_MODULE
$(OUTDIR)/test_8168n.o: ./monitor/test_8168n.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/test_8168n.o ./monitor/test_8168n.c

endif

ifdef CONFIG_SPE_MODULE
$(OUTDIR)/test_slvpcie.o: ./monitor/test_slvpcie.c                            
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/test_slvpcie.o ./monitor/test_slvpcie.c
endif

ifdef CONFIG_PCIE_MODULE	
$(OUTDIR)/test_hw.o: ./monitor/test_hw_96c.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/test_hw.o ./monitor/test_hw_96c.c
endif	
#----------------------------------------------------------------------------		
#$(OUTDIR)/sdram.o: ./init/sdram.c 
#	$(CC) -c $(CFLAGS) -o $(OUTDIR)/sdram.o ./init/sdram.c
#####################################################################

$(OUTDIR)/setup.o: ./arch/mips/kernel/setup.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/setup.o ./arch/mips/kernel/setup.c

$(OUTDIR)/utility.o: ./init/utility.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/utility.o ./init/utility.c

$(OUTDIR)/main.o: ./init/main.c
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/main.o ./init/main.c

$(OUTDIR)/calloc.o: ./init/calloc.c			#wei add 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/calloc.o ./init/calloc.c

$(OUTDIR)/traps.o: ./arch/mips/kernel/traps.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/traps.o ./arch/mips/kernel/traps.c

$(OUTDIR)/vsprintf.o: ./io/vsprintf.c 
	$(CC) -c $(CFLAGS)  -o $(OUTDIR)/vsprintf.o ./io/vsprintf.c

$(OUTDIR)/init.o: ./io/init.c 
	$(CC) -c $(CFLAGS)  -o $(OUTDIR)/init.o ./io/init.c

$(OUTDIR)/string.o: ./io/string.c 
	$(CC) -c $(CFLAGS)  -o $(OUTDIR)/string.o ./io/string.c

$(OUTDIR)/strtol.o: ./io/strtol.c 
	$(CC) -c $(CFLAGS)  -o $(OUTDIR)/strtol.o ./io/strtol.c

$(OUTDIR)/strtoul.o: ./io/strtoul.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/strtoul.o ./io/strtoul.c

$(OUTDIR)/ctool.o: ./io/ctool.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/ctool.o ./io/ctool.c

$(OUTDIR)/misc.o: ./io/misc.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/misc.o ./io/misc.c

$(OUTDIR)/monitor.o: ./monitor/monitor.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/monitor.o ./monitor/monitor.c

$(OUTDIR)/wmonitor.o: ./monitor/monitor.c 
	$(CC) -c $(CFLAGSW) -o $(OUTDIR)/wmonitor.o ./monitor/monitor.c

#$(OUTDIR)/flash.o: ./flash/flash.c 
#	$(CC) -c $(CFLAGS)  -o $(OUTDIR)/flash.o ./flash/flash.c
$(OUTDIR)/nand.o: ./nand/nand.c 
	$(CC) -c $(CFLAGS)  -o $(OUTDIR)/nand.o ./nand/nand.c
$(OUTDIR)/ctype.o: ./io/ctype.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/ctype.o ./io/ctype.c

$(OUTDIR)/boot.out :	 $(OBJFILES) $(OUTDIR)/monitor.o $(OBJS) $(ASMOBJS)
	$(LD) $(LDFLAGS) $(OBJFILES) $(OUTDIR)/monitor.o $(OBJS) $(ASMOBJS) $(LIBS) -o $(OUTDIR)/boot.out

$(OUTDIR)/temp.o: temp.c
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/temp.o temp.c
	mips-linux-objcopy --add-section .btimg=$(OUTDIR)/boot $(OUTDIR)/temp.o

$(OUTDIR)/wboot.out :	  $(OBJFILES) $(OUTDIR)/temp.o $(OUTDIR)/wmonitor.o $(OBJS) $(ASMOBJS)
	$(LD) $(WLDFLAGS) $(OBJFILES) $(OUTDIR)/temp.o $(OUTDIR)/wmonitor.o $(OBJS) $(ASMOBJS) $(LIBS) -o $(OUTDIR)/wboot.out

ifeq ($(CONFIG_SERIAL_SC16IS7X0),y)
$(OUTDIR)/gpio_8954c.o: ./serial_sc16is7x0/gpio/gpio_8954c.c
	$(CC) -c $(CFLAGS) -o $@ $<
else
$(OUTDIR)/gpio_8954c.o: ./init/gpio_8954c.c 
	$(CC) -c $(CFLAGS) -o $(OUTDIR)/gpio_8954c.o ./init/gpio_8954c.c 
endif

$(OUTDIR)/gpio_8972b.o: ./serial_sc16is7x0/gpio/gpio_8972b.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(OUTDIR)/i2c.o: ./serial_sc16is7x0/i2c.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(OUTDIR)/8250_sc16is7x0.o: ./serial_sc16is7x0/8250_sc16is7x0.c
	$(CC) -c $(CFLAGS) -o $@ $<

compile :	 $(OBJFILES)

.scr.bin:
	$(IMG2BIN) "$(OUTDIR)\$<" "$(OUTDIR)\$@"

.s.out:
	$(CC) $(CFLAGS) $(ASFLAGS) $(LDFLAGS) -o "$(OUTDIR)\$@" $<

.S.out:
	$(CC) $(CFLAGS) $(LDFLAGS) -o "$(OUTDIR)\$@" $< $(LIBS)

.c.out:
	$(CC) $(CFLAGS) $(LDFLAGS) -o "$(OUTDIR)\$@" $< $(LIBS)

.s.o:
	$(CC) -c $(CFLAGS) $(ASFLAGS) -o "$(OUTDIR)\$@" $<

.S.o:
	$(CC) -c $(CFLAGS) -o "$(OUTDIR)\$@" $<

.c.o:
	$(CC) -c $(CFLAGS) -o "$(OUTDIR)\$@" $<

.cpp.o:
	$(CC) -c $(CFLAGS) -o "$(OUTDIR)\$@" $<

.out.nm:
	$(NM) -B -n "$(OUTDIR)\$<" > "$(OUTDIR)\$@"

.out.img:
	$(OBJCOPY) -O binary "$(OUTDIR)\$<" "$(OUTDIR)\$@"

.out.text:
	$(OBJDUMP) -h -S -l --show-raw-insn "$(OUTDIR)\$<" > "$(OUTDIR)\$@"

.img.bin:
	$(IMG2BIN) "$(OUTDIR)\$<" "$(OUTDIR)\$@"

clean :
	$(RM) -f $(OBJFILES)
	$(RM) -f $(OUTDIR)/boot.out $(OUTDIR)/boot.text $(OUTDIR)/boot.nm $(OUTDIR)/boot.img
	$(RM) -f $(OUTDIR)/boot.cdb $(OUTDIR)/monitor.o $(OUTDIR)/wmonitor.o $(OUTDIR)/wboot.*
	$(RM) -f $(OUTDIR)/ethInt*.o $(OUTDIR)/temp.o $(OUTDIR)/boot
	@find $(OUTDIR) -type f -name '*.o' -exec rm -f {} \;
	
	
	
test:
	OBJFILES	= ./Output/head.o 
$(OUTDIR)/test.out :	 $(OBJFILES) $(OUTDIR)/test.o
	$(LD) $(LDFLAGS) $(OBJFILES) $(OUTDIR)/test.o $(LIBS) -o $(OUTDIR)/test.out	


