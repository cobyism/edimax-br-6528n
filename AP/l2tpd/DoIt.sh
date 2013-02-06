#!/bin/sh
set -e
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
		"ap")
			echo "AP dosn't need this!!"
			exit 0
		;;
		"ga")
			echo "GA dosn't need this!!"
			exit 0
		;;
	esac
fi

. ../../define/PATH
PATH=${CROSSDIR}:$PATH

if [ $MAKECLEAR = 1 ]; then
	rm -f .depend
	make clean
	if [ $? != 0 ]; then
		exit 1
	fi						
fi

if [ $MAKING = 1 ]; then
	make CC=${CROSS_COMPILE}gcc STRIP=${CROSS_COMPILE}strip
	if [ $? != 0 ]; then
		exit 1
	fi						
fi

