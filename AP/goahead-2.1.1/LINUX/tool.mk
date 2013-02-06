#!/bin/sh
. ../../../target.def
. ../../../define/FUNCTION_SCRIPT
. ../../../define/PATH

CONF_FILE="config-GW-EdimaxOBM.txt"
rm -rf ${IMAGEDIR}/config.bin
./cvcfg -r ../default_value/${CONF_FILE} ${IMAGEDIR}/config.bin

rm -rf config-pc.bin setting.bin
./cvcfg -r ../default_value/${CONF_FILE} -op config-pc.bin
cp config-pc.bin setting.bin
echo "end copy ${CONF_FILE}"
	
