ROOT=$PWD
toolPath=$ROOT/../../toolchain/rsdk-1.3.6-5281-EB-2.6.30-0.9.30/bin

export PATH=$toolPath:$PATH
touch autoconf.h
make clean 
make

echo ""
#ls -l boot-*
#echo "Default Flash layout, BootCode Size = 24k = 24576."
