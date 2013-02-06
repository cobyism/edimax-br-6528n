#!/bin/sh

. ../define/PATH
. ../define/FUNCTION_SCRIPT
echo "Upgrade Firmware ${_MODEL_}_${_MODE_}_${_VERSION_}.bin"
cd ${_MODEL_}/${_MODE_}/Tester/


#ifconfig eth0 192.168.1.189

tftp 192.168.1.6 -m binary -c put ${_MODEL_}_${_MODE_}_${_VERSION_}.bin

if  [ "$1" != "" ]; then
	ifconfig eth0 $1
	ifconfig eth0 down
	ifconfig eth0 up
fi

