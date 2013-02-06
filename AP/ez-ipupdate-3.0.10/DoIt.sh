#!/bin/sh
set -e
. ../../define/PATH
if [ "$1" = "make" ] && [ -f ez-ipupdate ]; then exit; fi
if [ -f Makefile ]; then
make clean
fi
if [ "$1" = "clean" ]; then exit; fi
./configure
if [ "$1" = "configure" ]; then exit; fi
make CC=${CROSS_COMPILE}gcc CFLAGS="-Os" CPP="${CROSS_COMPILE}gcc -E" STRIP=${CROSS_COMPILE}strip ez_ipupdate_SOURCES="ez-ipupdate.c conf_file.c conf_file.h md5.c md5.h cache_file.c cache_file.h error.h pid_file.c pid_file.h dprintf.h md5_pci.c" ez_ipupdate_OBJECTS="ez-ipupdate.o conf_file.o md5.o cache_file.o pid_file.o md5_pci.o"
${CROSS_COMPILE}strip ez-ipupdate
