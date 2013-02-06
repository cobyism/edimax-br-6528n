#!/bin/sh
. ../define/PATH
. ../define/FUNCTION_SCRIPT
pwd=`pwd`
cd $ROOTDIR/boards/rtl8196c
rm -f config.in
ln -s config.in_2300 config.in
rm -r bsp
ln -s bsp_1200 bsp
cd $pwd
function SEARCH_AND_REPLACE {
	if [ -f "$1" ]; then
		filename=$1
		target_old=$2
		target_new=$3
		target_line=`cat $filename | grep -n "$2" | cut -f 1 -d :`
		if [ "$target_line" ]; then
			total_line=`cat $filename | wc -l`
			cat ${filename} | head -n `expr $target_line - 1` > ${filename}_temp
			echo "$target_new" >> ${filename}_temp
			cat ${filename} | tail -n `expr $total_line - $target_line` >> ${filename}_temp
			cat ${filename}_temp > ${filename}
			rm -rf ${filename}_temp
		fi
	fi
}
rm -rf .config
cp .config.Normal .config
if [ "$_VLAN_CONFIG_SUPPORTED_" ] || [ "$_CONFIG_RTK_VLAN_SUPPORT_" ]; then
	SEARCH_AND_REPLACE .config "CONFIG_RTK_VLAN_SUPPORT" "CONFIG_RTK_VLAN_SUPPORT=y"
fi
if [ "$_WanAutoDetect_" ]; then
	SEARCH_AND_REPLACE .config "CONFIG_IP_NF_TARGET_REDIRECT" "CONFIG_IP_NF_TARGET_REDIRECT=y"
fi
if [ "$_IPV6_BRIDGE_" ]; then
	SEARCH_AND_REPLACE .config "CONFIG_RTK_VLAN_SUPPORT" "CONFIG_RTK_VLAN_SUPPORT=y"
	SEARCH_AND_REPLACE .config "CONFIG_RTK_IPV6_PASSTHRU_SUPPORT" "CONFIG_RTK_IPV6_PASSTHRU_SUPPORT=y"
	SEARCH_AND_REPLACE .config "CONFIG_RTK_VLC_SPEEDUP_SUPPORT" "CONFIG_RTK_VLC_SPEEDUP_SUPPORT=y"
	SEARCH_AND_REPLACE .config "CONFIG_RTL865X_SUPPORT_IPV6_MLD" "CONFIG_RTL865X_SUPPORT_IPV6_MLD=y"
fi
if [ "$_TX_POWER_CONTROL_" ]; then
	SEARCH_AND_REPLACE .config "CONFIG_HIGH_POWER_EXT_PA" "CONFIG_HIGH_POWER_EXT_PA=y"
	SEARCH_AND_REPLACE .config "CONFIG_HIGH_POWER_EXT_LNA" "CONFIG_HIGH_POWER_EXT_LNA=y"
fi
if [ ! -f "${LINUXDIR}/last_compile" ] || [ "`diff ../define/FUNCTION_COMPILER ${LINUXDIR}/last_compile`" ]; then
	rm -rf ${LINUXDIR}/last_compile
	make clean
fi
if [ "$1" = "clean" ]; then
	make V=1 ARCH=mips CROSS_COMPILE=${CROSS_COMPILE} clean
	exit
fi
rm -f include/asm-mips/asm-mips include/asm
ln -s asm-mips include/asm
if [ "`diff .config .config.Normal`" ]; then make menuconfig; fi
make dep
make ARCH=mips CROSS_COMPILE=${CROSS_COMPILE} -j1 V=1 2> compile.log
if [ $? != 0 ]; then
	cat compile.log
	exit
fi
cd ../AP/goahead-2.1.1/
./DoIt.sh clean
./DoIt.sh make
cd $pwd/rtkload
make ARCH=mips NM=${CROSS_COMPILE}nm CC=${CROSS_COMPILE}gcc STRIP=${CROSS_COMPILE}strip OBJCOPY=${CROSS_COMPILE}objcopy LD=${CROSS_COMPILE}ld V=1
if [ $? != 0 ]; then
	cat error
	exit
fi
cd ..
cp ../define/FUNCTION_COMPILER ${LINUXDIR}/last_compile
