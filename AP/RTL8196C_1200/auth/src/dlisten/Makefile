#
# Makefile for WPA for the Linux OS
#
#
# $Id: Makefile,v 1.1.1.1 2007/08/06 10:04:42 root Exp $
#


WPA_ROOT_DIR = ../..

ARCH	=
NAME	= dlisten

###################
CC	= mips-uclibc-gcc
STRIP	= mips-uclibc-strip
LD	= mips-uclibc-ld
AR	= mips-uclibc-ar
###################

dlisten_SRCS = iwcontrol.c iwconfig.c iwcommon.c iwreq.c
dlisten_OBJS = ${dlisten_SRCS:.c=.o}

RTL_WPA_IFLAGS 	= -I${WPA_ROOT_DIR}/include \
				-I${WPA_ROOT_DIR}/Package/Libnet-1.0.2a/include \
				-I${WPA_ROOT_DIR}/Package/libpcap-0.7.2 \
				-I${WPA_ROOT_DIR}/Package/openssl-0.9.7b/include

#RTL_WPA_LIBS = ${WPA_ROOT_DIR}/libnet.a \
#				${WPA_ROOT_DIR}/libpcap.a \
#				${WPA_ROOT_DIR}/libcrypto.a

RTL_WPA_DEFFLAGS += -DGLIBC22_HEADERS

RTL_WPA_CFLAG = -Wall -Os

ifeq ($(SLINK),1)
RTL_WPA_CFLAG += -ffunction-sections -fdata-sections
RTL_WPA_LDFLAG += --static -Wl,--gc-sections
endif

#
#	Transition rules (add -o to put object in right directory)
#
.c.o:
	$(CC) $(RTL_WPA_CFLAG) $(RTL_WPA_IFLAGS) $(RTL_WPA_DEFFLAGS) -c -o $*.o $<


###################
all: iwcontrol


iwcontrol: $(dlisten_OBJS)
	$(CC) ${RTL_WPA_LDFLAG} -o iwcontrol $(dlisten_OBJS) $(RTL_WPA_LIBS)
	$(STRIP) $(STRIPFLAGS) iwcontrol



##-------------------------------------------------

clean:
	rm -f *.o; rm -f iwcontrol

