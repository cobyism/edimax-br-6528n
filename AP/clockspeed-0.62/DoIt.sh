#!/bin/sh
set -e
. ../../define/PATH
export PATH=$CROSSDIR:$PATH
rm -f sntpclock load conf-cc systype conf-ld make-makelib make-compile auto-ccld.sh make-load find-systype compile
rm -f *.o
rm -f *.a
if [ "$1" = "clean" ]; then exit; fi
echo "${CROSS_COMPILE}gcc -O2" > conf-cc
echo "${CROSS_COMPILE}gcc -s" > conf-ld
make sntpclock
