#!/bin/sh
set -e
#cypress 100723
. ../../define/FUNCTION_SCRIPT

cd LINUX

./tool.mk
	if [ $? != 0 ]; then
		echo "make tool error!!"
		exit 1
	fi
cd ..
