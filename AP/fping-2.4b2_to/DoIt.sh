#!/bin/sh
set -e
. ../../define/PATH
if [ "$1" = "make" ] && [ -f fping ]; then exit; fi
if [ -f Makefile ]; then
make clean
fi
if [ "$1" = "clean" ]; then exit; fi
CC=${CROSS_COMPILE}gcc CFLAGS="-Os" ./configure --host=mips-linux
if [ "$1" = "configure" ]; then exit; fi
make
${CROSS_COMPILE}strip fping
