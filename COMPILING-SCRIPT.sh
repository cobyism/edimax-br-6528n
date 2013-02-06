#!/bin/sh
if [ "$5" = "v6" ]; then
cp -f ./define/PATH_${4}_${5} ./define/PATH
else
cp -f ./define/PATH_${4} ./define/PATH
fi
if [ ! -f "./define/PATH_${4}" ]; then
echo "Error can't find ./define/PATH_${4}"
exit 0
fi
. ./define/PATH
if [ `echo $LINUXDIR |grep "2.6" |wc -l` == 1 ];then
KERNEL_V26=1
else
KERNEL_V26=0
fi
_PLATFORM_=`cat define/FUNCTION_SCRIPT |grep ${4}`
if [ "$_PLATFORM_" == "" ] || [ -e cleanfile ];then
echo -e "\033[33;1m"
echo "********************************************************************************"
echo "*                       Application need clean                                 *"
echo "********************************************************************************"
echo -e "\033[0m"
echo 1 > cleanfile
rm ./toolchain/rtl8196c-toolchain-1.1/rsdk-96c
cd toolchain/rtl8196c-toolchain-1.1/
if [ $KERNEL_V26 == 1 ];then
ln -s rsdk-1.3.6-4181-EB-2.6.30-0.9.30 rsdk-96c
else
ln -s rsdk-1.3.6-4181-EB-2.4.25-0.9.30 rsdk-96c
fi
cd ../../AP
cd ..
rm cleanfile
fi
LANG=eng
STR_DATE1=`date`
echo -e "\033[33;1m"
echo "********************************************************************************"
echo "*                       Function Define                                        *"
echo "********************************************************************************"
echo -e "\033[0m"
#${ROOTDIR}/set_app_defined.sh ${1} ${2} ${3} ${4} ${5}
. ./define/FUNCTION_SCRIPT
if [ "$_START_BOA_" = "y" ]; then
sed -i "s/goahead-2.1.1/boa-0.94.14rc21/g" ./define/PATH
. ./define/PATH
fi
echo -e "\033[33;1m"
echo "********************************************************************************"
echo "*                       Generae target.def &  make.def                         *"
echo "********************************************************************************"
echo -e "\033[0m"
echo "PLATFORM=-D_RT305X_" > target.def
echo "MODEL=-D_${1}_" >> target.def
echo "ENDIAN=" >> target.def
echo "GATEWAY=${_IS_GATEWAY_}" >> target.def
echo "CROSS=${CROSS_COMPILE}" >> target.def
echo "CROSS_LINUX=${CROSS_COMPILE}" >> target.def
echo "CROSS_COMPILE=${CROSS_COMPILE}" >> target.def
echo "CROSS_PATH=${CROSSDIR}" >> target.def
echo "WLDEV=-D_RALINK_WL_DEVICE_" >> target.def
echo "PSDEV=-D_OFF_PS_DEVICE_" >> target.def
echo "SDEV=-D_OFF_S_DEVICE_" >> target.def
echo "" > make.def
echo "include ${ROOTDIR}/target.def" >> make.def
echo "include ${ROOTDIR}/define/FUNCTION_SCRIPT" >> make.def
echo "CC=\$(CROSS)gcc" >> make.def
echo "STRIP=\$(CROSS)strip" >> make.def
echo "LD=\$(CROSS)ld" >> make.def
echo "AR=\$(CROSS)ar" >> make.def
echo "RANLIB=\$(CROSS)ranlib" >> make.def
echo "CAS=\$(CROSS)gcc -c" >> make.def
echo "CPP=\$(CROSS)gcc -E" >> make.def
echo "PLATFORM=\${4}" >> make.def
echo -e "\033[33;1m"
echo ""
echo "********************************************************************************"
echo "*                            Setting WEB Directory                             *"
echo "********************************************************************************"
echo -e "\033[0m"
rm -f ${GOAHEADDIR}/web
if [ "$_MODE_" = "Customer" ] || [ "$_MODE_" = "Customer" ]; then
WEB_DIR="web-ap-Customer"
elif [ "$_MODE_" = "Customer" ]; then
WEB_DIR="web-gw-EdimaxOBM"
elif [ "$_MODE_" = "Customer" ] && [ "$_MODEL_" = "BR6408GNS" ]; then
WEB_DIR="web-gw-Customer6408GNS"
elif [ "$_MODE_" = "Customer" ] && [ "$_MODEL_" = "BR6408HAN" ]; then
WEB_DIR="web-gw-Customer6408HAN"
elif [ "$_MODE_" = "Customer" ] || [ "$_MODE_" = "Customer" ] || [ "$_MODE_" = "Customer" ]; then
WEB_DIR="web-gw-General"
elif [ "$_MODE_" = "Customer" ] || [ "$_MODE_" = "Customer" ]; then
WEB_DIR="web-gw-Customer_EZSetup"
elif [ "$_IS_GATEWAY_" = "y" ]; then
if [ "$_WEB_FILE_NAME_" != "" ]; then
WEB_DIR="web-gw-${_WEB_FILE_NAME_}-${_MODE_}"
else
WEB_DIR="web-gw-${_MODE_}"
fi
else
WEB_DIR="web-ap-${_MODE_}"
fi
if [ "$_LANGUAGE_PACK_" ]; then
WEB_DIR=${WEB_DIR}_with_LanguagePack
fi
if [ -d "${GOAHEADDIR}/${WEB_DIR}" ]; then
ln -s ${GOAHEADDIR}/${WEB_DIR} ${GOAHEADDIR}/web
case "$_MODE_" in
"Customer330" | "Customer331")
rm -rf ${GOAHEADDIR}/${WEB_DIR}/pictures/header_312.png
rm -rf ${GOAHEADDIR}/${WEB_DIR}/lang/de.txt
rm -rf ${GOAHEADDIR}/${WEB_DIR}/lang/es.txt
rm -rf ${GOAHEADDIR}/${WEB_DIR}/lang/fr.txt
rm -rf ${GOAHEADDIR}/${WEB_DIR}/lang/nl.txt
rm -rf ${GOAHEADDIR}/${WEB_DIR}/lang/pt.txt
rm -rf ${GOAHEADDIR}/${WEB_DIR}/lang/uk.txt
rm -rf ${GOAHEADDIR}/${WEB_DIR}/lang/dk.txt
rm -rf ${GOAHEADDIR}/${WEB_DIR}/lang/fi.txt
rm -rf ${GOAHEADDIR}/${WEB_DIR}/lang/it.txt
rm -rf ${GOAHEADDIR}/${WEB_DIR}/lang/no.txt
rm -rf ${GOAHEADDIR}/${WEB_DIR}/lang/se.txt
rm -rf ${GOAHEADDIR}/${WEB_DIR}/index.asp
rm -rf ${GOAHEADDIR}/${WEB_DIR}/getlanguage.js
rm -rf ${GOAHEADDIR}/${WEB_DIR}/wpsconfig.asp
if [ "$_MODE_" = "Customer" ]; then
tmpvar=330
fi
if [ "$_MODE_" = "Customer" ]; then
tmpvar=331
cp ${GOAHEADDIR}/${WEB_DIR}/wpsconfig331.asp ${GOAHEADDIR}/${WEB_DIR}/wpsconfig.asp
fi
cp ${GOAHEADDIR}/${WEB_DIR}/pictures/header_${tmpvar}.png ${GOAHEADDIR}/${WEB_DIR}/pictures/header_312.png
cp ${GOAHEADDIR}/${WEB_DIR}/lang/de${tmpvar}.txt ${GOAHEADDIR}/${WEB_DIR}/lang/de.txt
cp ${GOAHEADDIR}/${WEB_DIR}/lang/es${tmpvar}.txt ${GOAHEADDIR}/${WEB_DIR}/lang/es.txt
cp ${GOAHEADDIR}/${WEB_DIR}/lang/fr${tmpvar}.txt ${GOAHEADDIR}/${WEB_DIR}/lang/fr.txt
cp ${GOAHEADDIR}/${WEB_DIR}/lang/nl${tmpvar}.txt ${GOAHEADDIR}/${WEB_DIR}/lang/nl.txt
cp ${GOAHEADDIR}/${WEB_DIR}/lang/pt${tmpvar}.txt ${GOAHEADDIR}/${WEB_DIR}/lang/pt.txt
cp ${GOAHEADDIR}/${WEB_DIR}/lang/uk${tmpvar}.txt ${GOAHEADDIR}/${WEB_DIR}/lang/uk.txt
cp ${GOAHEADDIR}/${WEB_DIR}/lang/dk${tmpvar}.txt ${GOAHEADDIR}/${WEB_DIR}/lang/dk.txt
cp ${GOAHEADDIR}/${WEB_DIR}/lang/fi${tmpvar}.txt ${GOAHEADDIR}/${WEB_DIR}/lang/fi.txt
cp ${GOAHEADDIR}/${WEB_DIR}/lang/it${tmpvar}.txt ${GOAHEADDIR}/${WEB_DIR}/lang/it.txt
cp ${GOAHEADDIR}/${WEB_DIR}/lang/no${tmpvar}.txt ${GOAHEADDIR}/${WEB_DIR}/lang/no.txt
cp ${GOAHEADDIR}/${WEB_DIR}/lang/se${tmpvar}.txt ${GOAHEADDIR}/${WEB_DIR}/lang/se.txt
cp ${GOAHEADDIR}/${WEB_DIR}/index${tmpvar}.asp ${GOAHEADDIR}/${WEB_DIR}/index.asp
cp ${GOAHEADDIR}/${WEB_DIR}/getlanguage${tmpvar}.js ${GOAHEADDIR}/${WEB_DIR}/getlanguage.js
;;
*)
;;
esac
echo "Set WEB DIR to ${WEB_DIR} successfully..."
else
echo "Can not find ${WEB_DIR}..."
exit 0
fi
echo -e "\033[33;1m"
echo ""
echo "********************************************************************************"
echo "*                      Setting Wireless Driver Directory                       *"
echo "********************************************************************************"
echo -e "\033[0m"
echo -e "\033[33;1m"
echo ""
echo "********************************************************************************"
echo "*                      Building Linux Kernel and Modules                       *"
echo "********************************************************************************"
echo -e "\033[0m"
rm -f ${APPDIR}/module/led/*.* 2> /dev/null
cd ${LINUXDIR}
if [ $KERNEL_V26 != 1 ];then
./DoLinux.sh clean
fi
./DoLinux.sh
if [ $? != 0 ]; then
exit 1
fi
echo -e "\033[33;1m"
echo "********************************************************************************"
echo "*                            Building Applications                             *"
echo "********************************************************************************"
echo -e "\033[0m"
cd ${APPDIR}
./DoApp.sh make
if [ $? != 0 ]; then
exit 1
fi
echo -e "\033[33;1m"
echo "********************************************************************************"
echo "*                               Building Image                                 *"
echo "********************************************************************************"
echo -e "\033[0m"
cd ${IMAGEDIR}
./DoImage.sh
if [ $? != 0 ];then
exit 1
fi
STR_DATE2=`date`
echo "Start Time -->"${STR_DATE1}
echo "End Time   -->"${STR_DATE2}
