#!/bin/sh
set -e
. ../define/PATH
. ../define/FUNCTION_SCRIPT
. ../define/COMPILER_CFG.dat
echo "" > jobs.txt
case "$1" in
"make")
export MAKING=1
export MAKECLEAR=0
MAKING_IMG=1
;;
"image")
export MAKING=0
export MAKECLEAR=0
MAKING_IMG=1
;;
"clean")
export MAKING=0
export MAKECLEAR=1
MAKING_IMG=0
;;
*)
echo "usage $0 [make|clean]"
exit 1
;;
esac
function check_condition
{
IS_COMPILER="n"
COMPILER_VAR=""
IF_FIND="n"
if [ "$1" != "" ]; then
for arg in $INDEX
do
_DIR=`echo "$arg" | cut -d";" -f 1`
COMPILER_VAR=""
if [ "$1" = "$_DIR" ]; then
COMPILER_VAR=`echo "$arg" | cut -d";" -f 2`
fi
if [ "$COMPILER_VAR" != "" ]; then
IF_FIND="y"
if [ "$IS_COMPILER" != "y" ]; then
IS_COMPILER="$COMPILER_VAR"
fi
fi
done
if [ "$IF_FIND" = "n" ]; then
IS_COMPILER="y"
fi
fi
}
if [ "$MAKING" = "1" ] || [ "$MAKECLEAR" = "1" ]; then
for DIR in $(ls); do
if [ "$MAKING_IMG" = "1" ]; then
check_condition "$DIR"
fi
if [ -d $DIR ] && [ "$IS_COMPILER" != "n" ]; then	
echo "$DIR" >> jobs.txt
cd $DIR
if [ -f DoIt.sh ]; then
echo -e "\033[33;1m"
echo ""
echo "******************** Compiling $DIR ********************"
echo ""
echo -e "\033[0m"
if [ "$MAKING_IMG" = "1" ]; then
./DoIt.sh make
else
make clean
./DoIt.sh clean
fi
if [ $? = 0 ]; then
echo -e "\033[33;1m"
echo ""
echo "******************** End compile $DIR ********************"
echo ""
echo -e "\033[0m"
else
echo -e "\033[35;1m"
echo ""
echo "******************** compile ERROR in $DIR !! ********************"
echo ""
echo -e "\033[0m"
exit 1
fi
fi
cd ..
fi
done
fi	
if [ "$MAKING_IMG" = "1" ]; then
echo -e "\033[33;1m"
echo ""
echo "******************** Collecting Applications ********************"
echo ""
echo -e "\033[0m"
cd ${APPDIR}/mkimg
./app_collecting_script.sh
cd ${APPDIR}
echo -e "\033[33;1m"
echo ""
echo "******************** Making appimg ********************"
echo ""
echo -e "\033[0m"
rm -f ${IMAGEDIR}/appimg
cd ${ROMFSDIR}/..
if [ "$_PLATFORM_" = "RTL8196_2300" ]; then
./${_PLATFORM_}_tools/mksquashfs-lzma ${ROMFSDIR} ${IMAGEDIR}/appimg -be -always-use-fragments
else
if [ "$_KERNEL_VERSION_26_" = "y" ];then
./${_PLATFORM_}_tools/mksquashfs ${ROMFSDIR} ${IMAGEDIR}/appimg -comp lzma -always-use-fragments
else
./tools/mksquashfs-lzma ${ROMFSDIR} ${IMAGEDIR}/appimg -be
fi
fi
cd ..
chmod +r ${IMAGEDIR}/appimg
ls -l ${IMAGEDIR}/appimg
if [ $? = 0 ]; then
echo -e "\033[33;1m"
echo ""
echo "******************** Make appimg OK !! ********************"
echo ""
echo -e "\033[0m"
else
echo -e "\033[35;1m"
echo ""
echo "******************** Make appimg FAIL !! ********************"
echo ""
echo -e "\033[0m"
exit 1
fi
fi	
echo -e "\033[33;1m"
echo ""
echo "******************** Script finished !! ********************"
echo ""
echo -e "\033[0m"
