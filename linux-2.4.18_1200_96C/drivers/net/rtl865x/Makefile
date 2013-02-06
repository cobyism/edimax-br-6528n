O_TARGET := rtl865x.o

obj-y    += eth865x.o 
obj-y    += swCore.o

ifeq ($(CONFIG_RTL_KERNEL_MIPS16_DRVETH),y)
	obj-y    += swNic_poll.o16
	obj-y    += swTable.o16
	obj-y    += swUtil.o16
	obj-y    += vlanTable.o16
else
	obj-y    += swNic_poll.o
obj-y    += swTable.o 
obj-y    += swUtil.o
obj-y    += vlanTable.o
endif

export-objs := eth865x.o rtl_glue.o

ifeq ($(CONFIG_RTL8196_RTL8366),y)
subdir-y := RTL8366RB_DRIVER
EXTRA_CFLAGS += -I./RTL8366RB_DRIVER
endif

## ifeq ($(CONFIG_RTL865X_HW_TABLES),y)	
ifeq ($(CONFIG_RTL865X_LIGHT_ROMEDRV),y)

	obj-y    += rtl_glue.o
	obj-y += tblDrv/tblDrv.o
	subdir-y := tblDrv
	
ifeq ($(CONFIG_RTL865X_CLE),y)
	obj-y += cle/cle.o cle_cmdRoot.o
	subdir-y += cle 
	export-objs += cle_cmdRoot.o
endif

	EXTRA_CFLAGS +=  -DRTL_TBLDRV -D__linux__  -mno-memcpy -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX

else
	ifeq ($(CONFIG_HW_MULTICAST_TBL),y)
		obj-y += tblDrv/partialTblDrv.o
		subdir-y := tblDrv
	else

		ifeq ($(CONFIG_HW_PROTOCOL_VLAN_TBL),y)
			obj-y += tblDrv/partialTblDrv.o
			subdir-y := tblDrv
		else

			ifeq ($(CONFIG_RTK_VLAN_SUPPORT),y)
				obj-y += tblDrv/partialTblDrv.o
				subdir-y := tblDrv
			endif
		
		endif
		
	endif

endif

EXTRA_CFLAGS +=  -I. 

ifeq ($(CONFIG_MESH_ENABLE),y)
        EXTRA_CFLAGS += -DCONFIG_RTK_MESH #-DMESH_USE_METRICOP
        ifeq ($(CONFIG_11S_TEST_MODE),y)
        EXTRA_CFLAGS += -D_11s_TEST_MODE_
        endif
endif


O_OBJS   := $(obj-y)
OX_OBJS  := 

include $(TOPDIR)/Rules.make
