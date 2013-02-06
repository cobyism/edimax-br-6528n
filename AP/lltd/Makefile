include ../../make.def
include ../../define/FUNCTION_COMPILER
include ../../define/PATH
include ../../target.def
VPATH = src

#CROSS_COMPILE = /opt/crosstool/uClibc_v5te_le_gcc_4_1_1-nofpu/bin/arm-linux-
#LD            = $(CROSS_COMPILE)gcc
#CC            = $(CROSS_COMPILE)gcc
#CPP           = $(CC) -E
#STRIP         = $(CROSS_COMPILE)strip
CFLAGS =  -pipe -Wall -Wno-unused -O3 -g
CFLAGS += $(MODEL) $(FUNCTION)
#CFLAGS =  -pipe -Wall -Wstrict-prototypes -Wno-trigraphs -O2 \
	  -fno-strict-aliasing -fno-common -G 0 -mlong-calls
LDFLAGS = -DHAVE_WIRELESS

OS_LAYER = osl-linux.c

all: lld2d
 
include src/common.mk

depend:
	$(CC) $(CFLAGS) -M $(DCFILES) >.depend
	$(CC) $(CFLAGS) -M $(TCFILES) >>.depend

romfs:
	$(ROMFSINST) lld2d    /bin/lld2d
	$(ROMFSINST) ./src/gateway.ico  /etc/icon.ico
-include .depend

