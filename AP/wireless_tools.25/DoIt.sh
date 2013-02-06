#!/bin/sh
set -e
. ../../define/PATH
export PATH=$CROSSDIR:$PATH
make realclean
if [ "$1" = "clean" ]; then exit; fi
make CC=${CROSS_COMPILE}gcc AR=${CROSS_COMPILE}ar RANLIB=${CROSS_COMPILE}ranlib STRIP=${CROSS_COMPILE}strip BUILD_STRIPPING=y BUILD_NOLIBM=y
