#!/bin/sh
set -e
. ../../define/PATH
if [ "$1" = "make" ] && [ -f libpcap.a ]; then exit; fi
make clean
if [ "$1" = "clean" ]; then exit; fi
make CC=${CROSS_COMPILE}gcc
