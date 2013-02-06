#
# Makefile for the IEEE 802.1d ethernet bridging layer.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

ifeq ($(CONFIG_MESH_ENABLE),y)
        EXTRA_CFLAGS += -DCONFIG_RTK_MESH
endif

O_TARGET	:= bridge.o

ifdef CONFIG_RTL_KERNEL_MIPS16_BRIDGE
obj-y		:= br.o16 br_device.o16 br_fdb.o16 br_forward.o16 br_if.o br_input.o16 \
			br_ioctl.o br_notify.o16 br_stp.o16 br_stp_bpdu.o16 \
			br_stp_if.o16 br_stp_timer.o16
else
obj-y		:= br.o br_device.o br_fdb.o br_forward.o br_if.o br_input.o \
			br_ioctl.o br_notify.o br_stp.o br_stp_bpdu.o \
			br_stp_if.o br_stp_timer.o
endif			
			
obj-$(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) += pocket_filter.o			
obj-m		:= $(O_TARGET)

include $(TOPDIR)/Rules.make
