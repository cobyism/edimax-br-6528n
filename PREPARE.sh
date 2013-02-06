#!/bin/sh
set -e
NOW_PATH=`pwd`
cd toolchain
if [ -e rtl8196c-toolchain-1.1 ]; then
echo 96c-sdk-1.1 toolchain ok!
else
tar zxvf rtl8196c-toolchain-1.1.tar.gz
fi
cd ${NOW_PATH}
