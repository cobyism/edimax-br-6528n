#
# Makefile for the RTL865x hardware tables driver
#

ifeq ($(CONFIG_RTL865X_LIGHT_ROMEDRV),y)
O_TARGET := tblDrv.o

light_lib    :=  mbuf.o \
                	rtl_utils.o \
               	rtl865xC_tblBasicDrv.o \
                	rtl865xC_tblAsicDrv.o \
                	rtl865xC_layer2asic.o \
                	rtl865x_lightrome.o \
                	light_rome.o \
			rtl865xC_hs.o \
	       	rtl865x_lightromeext.o 
else

	ifeq ($(CONFIG_HW_MULTICAST_TBL),y)
		O_TARGET := partialTblDrv.o
		light_lib    :=  rtl865xC_tblAsicDrv.o rtl865xC_tblBasicDrv.o

	else

		ifeq ($(CONFIG_HW_PROTOCOL_VLAN_TBL),y)
			O_TARGET := partialTblDrv.o
			light_lib    :=  rtl865xC_tblAsicDrv.o rtl865xC_tblBasicDrv.o
		else

			ifeq ($(CONFIG_RTK_VLAN_SUPPORT),y)
				O_TARGET := partialTblDrv.o
				light_lib    :=  rtl865xC_tblAsicDrv.o rtl865xC_tblBasicDrv.o
			endif
		
		endif
		
	endif

endif

ifeq ($(CONFIG_RTL865X_CLE),y)	
	light_lib += rtl8651_tblAsicDrv_cmd.o\
			    light_cmd.o
endif

#ROME_OBJS    := romedrv_linux.o light_lib.o
ROME_OBJS    := light_lib.o

obj-y   :=$(ROME_OBJS)

# for toolchain rsdk-1.3.6
#EXTRA_CFLAGS += -O1 -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Wno-implicit -Werror
EXTRA_CFLAGS += -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX -Wno-implicit
EXTRA_CFLAGS += -I. -I.. -I../cle


include $(TOPDIR)/Rules.make

light_lib.o : $(light_lib)
	rm -f $@
	$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -r -o light_lib.o $(light_lib)
