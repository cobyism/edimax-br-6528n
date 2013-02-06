#
# drivers/net/rtl8192cd
#
# Makefile for the Linux Wireless network device drivers.
#

O_TARGET := rtl8192cd.o

#EXTRA_CFLAGS += -Werror

# for rules.make
WIRELESS_COMPILER=1

#subdir-$(CONFIG_RTL8192SE)	+= rtl8192Pci

#add for MESH
#ifeq ($(CONFIG_MESH_ENABLE),y)
#	EXTRA_CFLAGS += -DCONFIG_RTK_MESH #-DMESH_USE_METRICOP
#	ifeq ($(CONFIG_11S_TEST_MODE),y)
#	EXTRA_CFLAGS += -D_11s_TEST_MODE_
#	endif	
#endif

obj-y := 8192cd_ioctl.o\
 	$(join $(subdir-y), $(patsubst %,/%.o,$(notdir $(subdir-y))))

ifeq ($(CONFIG_RTL_KERNEL_MIPS16_WLAN),y)

ifeq ($(CONFIG_RTL8196C),y)
obj-y += 8192cd_osdep.o
else
obj-y += 8192cd_osdep.o16
endif

ifeq ($(CONFIG_RTL8196C_CLIENT_ONLY),y)
obj-y += 8192cd_rx.o
else
obj-y += 8192cd_rx.o16
endif

obj-y += 8192cd_sme.o

ifeq ($(CONFIG_RTL8196C_AP_ROOT),y)
obj-y += 8192cd_br_ext.o
else

ifeq ($(CONFIG_RTL8196C_EC),y)
obj-y += 8192cd_br_ext.o
else
obj-y += 8192cd_br_ext.o16
endif


endif

obj-y +=\
	8192cd_tx.o16\
	8192cd_util.o16\
	8192cd_hw.o16\
	8192cd_security.o16\
	8192cd_tkip.o16\
	8192cd_aes.o16\
	8192cd_proc.o16\
	8192cd_eeprom.o16\
	8192cd_mp.o16\
	8192cd_psk.o16\
	1x_kmsm_aes.o16\
	1x_kmsm_hmac.o16\
	1x_md5c.o16\
	1x_rc4.o16\
	8192cd_mib.o16\
	8192cd_dmem.o16\
	romeperf.o16
ifeq ($(CONFIG_RTL_COMAPI_CFGFILE),y)
obj-y += 8192cd_comapi.o16
endif
else
obj-y +=\
	8192cd_tx.o\
	8192cd_rx.o\
	8192cd_osdep.o\
	8192cd_sme.o\
	8192cd_util.o\
	8192cd_hw.o\
	8192cd_security.o\
	8192cd_tkip.o\
	8192cd_aes.o\
	8192cd_proc.o\
	8192cd_br_ext.o\
	8192cd_eeprom.o\
	8192cd_mp.o\
	8192cd_psk.o\
	1x_kmsm_aes.o\
	1x_kmsm_hmac.o\
	1x_md5c.o\
	1x_rc4.o\
	8192cd_mib.o\
	8192cd_dmem.o\
	romeperf.o
ifeq ($(CONFIG_RTL_COMAPI_CFGFILE),y)
obj-y += 8192cd_comapi.o
endif
endif	

#add for MESH
#ifeq ($(CONFIG_MESH_ENABLE),y)
#obj-y +=\
#	mesh_ext/mesh_proc.o\
#	mesh_ext/mesh_route.o\
#	mesh_ext/mesh_rx.o\
#	mesh_ext/mesh_sme.o\
#	mesh_ext/mesh_security.o\
#	mesh_ext/mesh_tx.o\
#	mesh_ext/mesh_util.o\
#	mesh_ext/mesh_11kv.o\
#	mesh_ext/hash_table.o
#endif

#add for WAPI
ifeq ($(CONFIG_RTL_WAPI_SUPPORT),y)
//obj-y += SMS4.o wapi_wai.o wapiCrypto.o	
obj-y += wapi_wai.o wapiCrypto.o	
endif	

#CONFIG_SINUX_SUPPORT=1
ifeq ($(CONFIG_SINUX_SUPPORT),1)
EXTRA_CFLAGS  += -DOPENSSL_FIPS -D__linux__ -DRSDK_BUILT -DOPENSSL_NO_SPEED -DOPENSSL_THREADS -D_REENTRANT \
                        -DDSO_DLFCN -DHAVE_DLFCN_H -DOPENSSL_NO_KRB5 -DB_ENDIAN -DTERMIO \
			-fomit-frame-pointer 
# -save-temps                      
                        
obj-y  += wps/ssl/mem.o wps/ssl/mem_clr.o wps/ssl/bn_add.o wps/ssl/bn_lib.o \
			wps/ssl/bn_asm.o wps/ssl/bn_const.o wps/ssl/bn_ctx.o \
                wps/ssl/bn_div.o wps/ssl/bn_exp.o wps/ssl/bn_gcd.o \
				wps/ssl/bn_mod.o wps/ssl/bn_mont.o \
                wps/ssl/bn_mul.o wps/ssl/bn_prime.o wps/ssl/bn_rand.o \
				wps/ssl/bn_recp.o wps/ssl/bn_shift.o wps/ssl/bn_sqr.o \
                wps/ssl/bn_word.o wps/ssl/dh_check.o wps/ssl/dh_gen.o \
				wps/ssl/dh_key.o wps/ssl/dh_lib.o wps/ssl/digest.o \
                wps/ssl/m_sha1.o wps/ssl/hmac.o wps/ssl/md_rand.o \
				wps/ssl/rand_lib.o wps/ssl/sha1dgst.o wps/ssl/sha256.o \
                wps/ssl/aes_cbc.o wps/ssl/fips_aes_core.o
				
#EXTRA_CFLAGS += -I../../../lib
obj-y += wps/8192cd_wscd.o
obj-y += wps/sercomm_intf.o
endif

# rock: link binary data via bin2c.pl
.SECONDARY: $(SRCS_TXT) $(SRCS_BIN)
SRCS_TXT = $(addprefix data_, $(notdir $(patsubst %.txt,%.c,$(wildcard data/*.txt))))
SRCS_BIN = $(addprefix data_, $(notdir $(patsubst %.bin,%.c,$(wildcard data/*.bin))))
#obj-y += $(SRCS_TXT:.c=.o) $(SRCS_BIN:.c=.o)
data_%.c: data/%.txt
	./bin2c.pl $(basename $@) < $< > $@
data_%.c: data/%.bin
	./bin2c.pl $(basename $@) < $< > $@
# create our target
all: $(SRCS_TXT) $(SRCS_BIN) $(O_TARGET)

include $(TOPDIR)/Rules.make
