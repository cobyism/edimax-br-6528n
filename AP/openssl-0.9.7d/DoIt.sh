#!/bin/sh
set -e
. ../../define/PATH
if [ "$1" = "make" ] && [ -f libcrypto.a ] && [ -f libssl.a ]; then exit; fi
make clean
if [ "$1" = "clean" ]; then exit; fi
./Configure linux:${CROSS_COMPILE}gcc
if [ "$1" = "configure" ]; then exit; fi
make
