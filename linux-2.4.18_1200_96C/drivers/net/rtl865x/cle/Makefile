#
# Makefile for RTL865X command line engine
#

O_TARGET := cle.o

CLE_OBJS := \
	cle_ciscoParser.o \
	cle_ciscoParserParamChecker.o \
	cle_parser.o cle_utility.o \
	cmd_keyword.o \
	cmd_cmdEveryWhere.o \
	cle_linuxglue.o

EXTRA_CFLAGS += -I.. -DRTL865X_OVER_KERNEL -DRTL865X_OVER_LINUX
obj-$(CONFIG_RTL865X_CLE) :=  $(CLE_OBJS)


# Hand off to Rules.make.

include $(TOPDIR)/Rules.make
