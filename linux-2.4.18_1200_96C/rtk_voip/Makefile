O_TARGET := rtk_voip.o

mod-subdirs := voip_rx voip_tx voip_manager voip_dsp voip_drivers
obj-y :=

subdir-$(CONFIG_RTK_VOIP_RX)      += voip_rx
obj-$(CONFIG_RTK_VOIP_RX) += voip_rx/voip_rx.o
subdir-$(CONFIG_RTK_VOIP_TX)      += voip_tx
obj-$(CONFIG_RTK_VOIP_TX) += voip_tx/voip_tx.o

ifeq "$(CONFIG_AUDIOCODES_VOIP)" "y"
ifeq "$(CONFIG_VOIP_SDK)" "y"
obj-y += voip_dsp/dsp_r1/t38.obj
else
subdir-y              += voip_dsp/dsp_r1/t38
obj-y += voip_dsp/dsp_r1/t38/t38.o
endif
subdir-y              += voip_dsp/ivr
obj-y += voip_dsp/ivr/ivr_sup.o
subdir-y              += voip_dsp/cp3
obj-y += voip_dsp/cp3/cp3.o
endif

ifneq "$(CONFIG_AUDIOCODES_VOIP)" "y"
subdir-$(CONFIG_RTK_VOIP_DSP)     += voip_dsp
obj-$(CONFIG_RTK_VOIP_DSP) += voip_dsp/voip_dsp.o
subdir-$(CONFIG_RTK_VOIP_SRTP)    += srtp
obj-$(CONFIG_RTK_VOIP_SRTP) += srtp/srtp.o
endif

subdir-$(CONFIG_RTK_VOIP_MANAGER)     += voip_manager
obj-$(CONFIG_RTK_VOIP_MANAGER) += voip_manager/voip_manager.o
subdir-$(CONFIG_RTK_VOIP_DRIVERS)     += voip_drivers
obj-$(CONFIG_RTK_VOIP_DRIVERS) += voip_drivers/voip_drivers.o

EXTRA_CFLAGS += include

ifeq ($(CONFIG_RTK_VOIP),m)
EXTRA_CFLAGS += -mlong-calls
EXTRA_CFLAGS += -DMODULE
endif

#obj-y   += $(join $(subdir-y), $(patsubst %,/%.o,$(notdir $(subdir-y))))
include $(TOPDIR)/Rules.make
