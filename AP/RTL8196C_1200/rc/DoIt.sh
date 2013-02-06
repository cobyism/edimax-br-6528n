#!/bin/sh
. ../../../define/PATH
if [ "$1" = "make" ] && [ -f reload ]; then exit; fi
make clean
if [ "$1" = "clean" ]; then exit; fi
make
${CROSS_COMPILE}strip reload
