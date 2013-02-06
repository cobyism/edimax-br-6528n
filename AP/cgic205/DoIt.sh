#!/bin/sh
set -e
. ../../define/PATH
PATH=${CROSSDIR}:${PATH}

if [ $# = 1 ]; then
	case "$1" in
		"clean")
			MAKECLEAR=1
			MAKING=0
			;;
		"make")
			MAKECLEAR=0
			MAKING=1
			;;
    esac
fi  
																							
if [ $MAKECLEAR = 1 ]; then
	rm -rf _install
	make clean
    if [ $? != 0 ]; then
        exit 1
    fi						
fi
if [ $MAKING = 1 ]; then
	#make install
	./go
    if [ $? != 0 ]; then
        exit 1
    fi
fi
