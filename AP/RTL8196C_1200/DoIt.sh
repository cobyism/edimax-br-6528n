#!/bin/sh
pwd=`pwd`
for i in `ls`; do
	if [ -d $i ]; then
		cd "$pwd/""$i"
		if [ -f DoIt.sh ]; then
			./DoIt.sh $1
		fi
		cd $pwd
	fi
done
