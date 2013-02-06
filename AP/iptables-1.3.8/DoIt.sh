#!/bin/sh
set -e
if [ "$1" != "make" ] && [ "$1" != "clean" ] && [ "$1" != "configure" ]; then
	echo "$0 [make|clean|configure]"
fi

. ../../define/PATH
PATH=${CROSSDIR}:$PATH

if [ "$1" = "configure" ]; then
:
fi

if [ "$1" = "clean" ]; then
	make clean
fi

if [ "$1" = "make" ]; then
	make KERNEL_DIR=${LINUXDIR} DO_IPV6=0 NO_SHARED_LIBS=1 DO_MULTI=0 CC=${CROSS_COMPILE}gcc AR=${CROSS_COMPILE}ar
	${CROSS_COMPILE}strip iptables
fi
