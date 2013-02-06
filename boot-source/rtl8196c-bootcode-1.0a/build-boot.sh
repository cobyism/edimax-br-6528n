ROOT=$PWD
toolPath=$ROOT/../../toolchain/rtl8196c-toolchain-1.1/rsdk-96c/bin

export PATH=$toolPath:$PATH
touch autoconf.h
make clean 
make

echo ""
#ls -l boot-*
#echo "Default Flash layout, BootCode Size = 24k = 24576."
