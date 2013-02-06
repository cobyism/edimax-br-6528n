# Generated automatically from Makefile.in by configure.
#***********************************************************************
#
# Makefile
#
# Makefile for Roaring Penguin's Linux PPPoE plugin.
#
# Copyright (C) 2001 Roaring Penguin Software Inc.
#
# This program may be distributed according to the terms of the GNU
# General Public License, version 2 or (at your option) any later version.
#
# $Id: Makefile,v 1.1.1.1 2004/08/12 10:33:31 ysc Exp $
#***********************************************************************

# Version is set ONLY IN THE MAKEFILE!  Don't delete this!
VERSION=3.3

include ../../../../../make.def
include ../../../../../define/FUNCTION_COMPILER
CFLAGS+=-I../../../include/linux --static -s -DCOMPACK_SIZE $(FUNCTION)
all: libplugin.a

rp-pppoe.so: libplugin.a plugin.o
	gcc -o rp-pppoe.so -shared plugin.o libplugin.a

install: all
	$(INSTALL) -d -m 755 $(LIBDIR)
	$(INSTALL) -s -c -m 4550 rp-pppoe.so $(LIBDIR)

clean:
	rm -f *.o *.so

plugin.o: plugin.c
	$(CC) '-DRP_VERSION="$(VERSION)"' $(CFLAGS) -I../../..  -c -o plugin.o  plugin.c

libplugin.a: discovery.o if.o common.o debug.o plugin.o
	$(AR) -rc $@ $^

discovery.o: discovery.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o discovery.o  discovery.c

if.o: if.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o if.o  if.c

debug.o: debug.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o debug.o  debug.c

common.o: common.c
	$(CC) $(CFLAGS) '-DVERSION="$(VERSION)"' -c -o common.o  common.c

