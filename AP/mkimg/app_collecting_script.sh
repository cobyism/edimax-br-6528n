#!/bin/sh
#!/bin/sh
. ../../define/PATH
. ../../define/FUNCTION_SCRIPT
. ../../target.def
if [ "${ROMFSDIR}" = "" ]; then
echo "NO Defined PATH!"
exit 0
fi
rm -rf ${ROMFSDIR}
mkdir ${ROMFSDIR}
mkdir ${ROMFSDIR}/dev
mkdir ${ROMFSDIR}/bin
mkdir ${ROMFSDIR}/sbin
mkdir ${ROMFSDIR}/usr
mkdir ${ROMFSDIR}/etc
mkdir ${ROMFSDIR}/var
mkdir ${ROMFSDIR}/proc
ln -s /var ${ROMFSDIR}/tmp
if [ "$_WAN3G_" = "y" ]; then
mkdir ${ROMFSDIR}/sys
fi
mknod -m666 ${ROMFSDIR}/dev/mtdblock0 b 31 0
mknod -m666 ${ROMFSDIR}/dev/mtdblock1 b 31 1
if [ "$_EDIMAX30_" = "y" ]; then
mknod -m666 ${ROMFSDIR}/dev/mtdblock2 b 31 2			#cypress 100714 edimax30, image1
mknod -m666 ${ROMFSDIR}/dev/mtdblock3 b 31 3			#cypress 100714 edimax30, image2
mknod -m666 ${ROMFSDIR}/dev/mtdblock4 b 31 4			#cypress 100714 edimax30, image3
mknod -m666 ${ROMFSDIR}/dev/mtdblock5 b 31 5			#cypress 100714 edimax30, icons pos
fi
mknod -m666 ${ROMFSDIR}/dev/kmem 	c 	1 	2
mknod -m666 ${ROMFSDIR}/dev/null 	c 	1 	3
mknod -m666 ${ROMFSDIR}/dev/random c 1 8
mknod -m666 ${ROMFSDIR}/dev/urandom 	c 1 9
mknod -m600 ${ROMFSDIR}/dev/ptmx c 5 2
mknod -m666 ${ROMFSDIR}/dev/ttyS0 c 4 64
mknod -m666 ${ROMFSDIR}/dev/console c 5 1
mknod -m666 ${ROMFSDIR}/dev/flash0	c 	200 	0
mknod -m666 ${ROMFSDIR}/dev/rdm0 c   254   0  #for reg tool
mknod  -m666 ${ROMFSDIR}/dev/ttyp0 c 3 0
mknod  -m666 ${ROMFSDIR}/dev/ttyS1 c 4 65
mknod  -m666 ${ROMFSDIR}/dev/ppp c 108 0
mknod  -m666 ${ROMFSDIR}/dev/ptyp3 c 2 3
mknod  -m666 ${ROMFSDIR}/dev/ttyp1 c 3 1
mknod  -m666 ${ROMFSDIR}/dev/ttyp2 c 3 2
mknod  -m666 ${ROMFSDIR}/dev/ttyp3 c 3 3
mkdir -p ${ROMFSDIR}/dev/pts
for minor in 0 1 2 3 4 5 6 7 8 9 10 ; do
mknod -m666 ${ROMFSDIR}/dev/pts/$minor c 136 $minor
done
ln -s /var/dev/log ${ROMFSDIR}/dev/log
ln -s /var/dev/ptyp0 ${ROMFSDIR}/dev/ptyp0
ln -s /var/dev/ptyp1 ${ROMFSDIR}/dev/ptyp1
ln -s /var/dev/ptyp2 ${ROMFSDIR}/dev/ptyp2
mkdir ${ROMFSDIR}/lib
cp ${APPLIB}/libuClibc-${LIB_VER}.so ${ROMFSDIR}/lib/libc.so.0
cp ${APPLIB}/ld-uClibc-${LIB_VER}.so ${ROMFSDIR}/lib/ld-uClibc.so.0
cp ${APPLIB}/libpthread-${LIB_VER}.so ${ROMFSDIR}/lib/libpthread.so.0
cp ${APPLIB}/libcrypt-${LIB_VER}.so ${ROMFSDIR}/lib/libcrypt.so.0
cp ${APPLIB}/libdl-${LIB_VER}.so ${ROMFSDIR}/lib/libdl.so.0
cp ${APPLIB}/libresolv-${LIB_VER}.so ${ROMFSDIR}/lib/libresolv.so.0 #for "rdisc
cp ${APPLIB}/libm-${LIB_VER}.so ${ROMFSDIR}/lib/libm.so.0
cp -R -p ${APPDIR}/busybox-1.11.1/_install/* ${ROMFSDIR}
cp ${APPDIR}/bridge-utils/brctl/brctl ${ROMFSDIR}/bin
cp -R -p ${APPDIR}/etc.rootfs/* ${ROMFSDIR}/etc
if [ "$_IS_GATEWAY_" = "y" ]; then
rm ${ROMFSDIR}/etc/profile.ap -f
else
mv -f ${ROMFSDIR}/etc/profile.ap ${ROMFSDIR}/etc/profile
fi

cp -f ${APPDIR}/${_PLATFORM_}/rc/reload ${ROMFSDIR}/bin/
cp ${APPDIR}/${_PLATFORM_}/auth/src/auth $ROMFSDIR/bin
cp ${APPDIR}/${_PLATFORM_}/auth/src/dlisten/iwcontrol $ROMFSDIR/bin
cp ${APPDIR}/${_PLATFORM_}/IAPP/iapp $ROMFSDIR/bin
cp ${APPDIR}/${_PLATFORM_}/lltdd/native-linux/lld2d $ROMFSDIR/bin
cp ${APPDIR}/${_PLATFORM_}/lltdd/src/wrt54g.large.ico $ROMFSDIR/etc/icon.ico
if [ "$_WINDOWS7_LOGO_" = "y" ]; then
cp ${APPDIR}/${_PLATFORM_}/wsc/src/wscd_win7 $ROMFSDIR/bin/wscd
cp ${APPDIR}/${_PLATFORM_}/mini_upnp/mini_upnp_win7.so $ROMFSDIR/lib/mini_upnp.so
cp ${APPDIR}/${_PLATFORM_}/mini_upnp/mini_upnpd_win7 $ROMFSDIR/bin/mini_upnpd
else
cp ${APPDIR}/${_PLATFORM_}/wsc/src/wscd $ROMFSDIR/bin
if [ "$_KERNEL_VERSION_26_" != "y" ]; then
cp ${APPDIR}/${_PLATFORM_}/mini_upnp/mini_upnp.so $ROMFSDIR/lib
cp ${APPDIR}/${_PLATFORM_}/mini_upnp/mini_upnpd $ROMFSDIR/bin
fi
fi
if [ "$_MODE_" = "Customer" ]; then
if [ "$_RFTYPE_" = "1T1R" ]; then
cp ${APPDIR}/${_PLATFORM_}/wsc/src/wscd_Customer_250R.conf $ROMFSDIR/etc/wscd.conf
elif [ "$_RFTYPE_" = "2T2R" ]; then
cp ${APPDIR}/${_PLATFORM_}/wsc/src/wscd_Customer_350R.conf $ROMFSDIR/etc/wscd.conf
fi
elif [ "$_MODE_" = "EdimaxOBM" ] || [ "$_MODE_" = "Customer" ]; then
cp ${APPDIR}/${_PLATFORM_}/wsc/src/wscd_Edimax.conf $ROMFSDIR/etc/wscd.conf
else
cp ${APPDIR}/${_PLATFORM_}/wsc/src/wscd.conf $ROMFSDIR/etc/wscd.conf
fi
ln -s /bin/wps.sh $ROMFSDIR/bin/wpstool
cp ${APPDIR}/${_PLATFORM_}/script/simplecfg* $ROMFSDIR/etc
ln -s /var/wps $ROMFSDIR/etc/simplecfg
cp ${APPDIR}/${_PLATFORM_}/miniigd/miniigd $ROMFSDIR/bin
ln -s /var/linuxigd $ROMFSDIR/etc/
mkdir -p $ROMFSDIR/etc/tmp
cp ${APPDIR}/${_PLATFORM_}/miniigd/pics* $ROMFSDIR/etc/tmp
if [ "$_MODE_" = "EdimaxOBM" ]; then
sed -i "s/Internet Gateway Device/Edimax/g" $ROMFSDIR/etc/tmp/pics*
sed -i "s/Wireless Router/Edimax/g" $ROMFSDIR/etc/tmp/pics*
if [ "$_MODEL_" = "BR6428GN" ]; then
sed -i "s/IGD/BR-6428n/g" $ROMFSDIR/etc/tmp/pics*
elif [ "$_MODEL_" = "BR6228GN" ]; then
sed -i "s/IGD/BR-6228n/g" $ROMFSDIR/etc/tmp/pics*
elif [ "$_MODEL_" = "BR6258GN" ]; then
sed -i "s/IGD/BR-6258n/g" $ROMFSDIR/etc/tmp/pics*
elif [ "$_MODEL_" = "BR6458GN" ]; then
sed -i "s/IGD/BR-6458n/g" $ROMFSDIR/etc/tmp/pics*
fi
elif [ "$_MODE_" = "Customer" ]; then
if [ "$_MODEL_" = "BR6228GN" ]; then
sed -i "s/BR6228GN/WN-250R/g" $ROMFSDIR/etc/tmp/picsdesc.skl
sed -i "s/IGD/WN-250R/g" $ROMFSDIR/etc/tmp/picsdesc.xml
elif [ "$_MODEL_" = "BR6428GN" ]; then
sed -i "s/BR6428GN/WN-350R/g" $ROMFSDIR/etc/tmp/picsdesc.skl
sed -i "s/IGD/WN-350R/g" $ROMFSDIR/etc/tmp/picsdesc.xml
fi
elif [ "$_MODE_" = "Customer" ]; then
if [ "$_MODEL_" = "BR6408HAN" ]; then
sed -i "s/Internet Gateway Device/Customer Wireless-300N Smart Dish Repeater/g" $ROMFSDIR/etc/tmp/pics*
sed -i "s/IGD/HAW2DR/g" $ROMFSDIR/etc/tmp/pics*
sed -i "s/Wireless Router/Customer/g" $ROMFSDIR/etc/tmp/pics*
fi
fi
cp -f ${APPDIR}/wireless_tools.25/iwpriv ${ROMFSDIR}/bin
if  [ "$_MODE_" = "Customer" ] && [ "$_USB_8712_" = "y" ]; then
cp -f ${APPDIR}/wireless_tools.29/iwlist ${ROMFSDIR}/bin
cp -f ${APPDIR}/wireless_tools.29/iwconfig ${ROMFSDIR}/bin
cp -f ${APPDIR}/wireless_tools.29/iwlib.so ${ROMFSDIR}/lib
cp -f ${APPDIR}/wireless_tools.29/libiw.so.29 ${ROMFSDIR}/lib
cp -f ${APPDIR}/wpa_supplicant-0.6.9/wpa_supplicant/wpa.conf ${ROMFSDIR}/etc
cp -f ${APPDIR}/wpa_supplicant-0.6.9/wpa_supplicant/wpa_cli ${ROMFSDIR}/bin
cp -f ${APPDIR}/wpa_supplicant-0.6.9/wpa_supplicant/wpa_supplicant ${ROMFSDIR}/bin
cp -f ${APPDIR}/openssl-0.9.8k/ssh_install/lib/libcrypto.so.0.9.8 ${ROMFSDIR}/lib
cp -f ${APPDIR}/openssl-0.9.8k/ssh_install/lib/libssl.so.0.9.8 ${ROMFSDIR}/lib
fi
cp ${APPDIR}/clockspeed-0.62/sntpclock ${ROMFSDIR}/bin
cp ${APPDIR}/ez-ipupdate-3.0.10/ez-ipupdate ${ROMFSDIR}/bin
if [ "$_DDNS_NOIP_" = "y" ]; then
cp ${APPDIR}/updatedd-2.6/src/updatedd ${ROMFSDIR}/bin
mkdir -p ${ROMFSDIR}/usr/local/lib/updatedd
cp ${APPDIR}/updatedd-2.6/src/plugins/.libs/libnoip.so.0.0.0 ${APPDIR}/updatedd-2.6/src/plugins/.libs/libnoip.la ${ROMFSDIR}/usr/local/lib/updatedd
ln -s libnoip.so.0.0.0 ${ROMFSDIR}/usr/local/lib/updatedd/libnoip.so.0
ln -s libnoip.so.0.0.0 ${ROMFSDIR}/usr/local/lib/updatedd/libnoip.so
fi
cp ${IMAGEDIR}/config.bin ${ROMFSDIR}/etc/config.bin
echo ${_VERSION_} > ${ROMFSDIR}/etc/version
echo ${_DATE_} >  ${ROMFSDIR}/etc/compiler_date
cp -R -p ${APPDIR}/var/* ${ROMFSDIR}/var
if [ "$_IS_GATEWAY_" = "y" ]; then
if [ "$_START_BOA_" = "y" ]; then
cp -f ${GOAHEADDIR}/src/flash ${ROMFSDIR}/bin/
else
cp -f ${GOAHEADDIR}/LINUX/flash-gw ${ROMFSDIR}/bin/flash
fi
else
cp -f ${GOAHEADDIR}/LINUX/flash-ap ${ROMFSDIR}/bin/flash
fi
if [ "$_TX_POWER_CONTROL_" = "y" ] || [ "$_MODE_" = "Customer" ]; then
cp ${APPDIR}/hex_dec_convert/hex_dec_convert ${ROMFSDIR}/bin
fi
mkdir ${ROMFSDIR}/web
if [ "$_EDIMAX30_" = "y" ]; then
mkdir ${ROMFSDIR}/web/mnt					#cypress edimax30 100714
fi
cp -Rf ${GOAHEADDIR}/web/* ${ROMFSDIR}/web
if [ "$_MODE_" = "Customer" ]; then
rm -rf ${ROMFSDIR}/web/file_BR6425N ${ROMFSDIR}/web/file_BR6225N
fi
if [ "$_IS_GATEWAY_" = "y" ]; then
if [ "$_START_BOA_" = "y" ]; then
cp -f ${GOAHEADDIR}/src/boa ${ROMFSDIR}/bin/webs
else
cp -f ${GOAHEADDIR}/LINUX/webs-gw ${ROMFSDIR}/bin/webs
fi	
else
cp -f ${GOAHEADDIR}/LINUX/webs-ap ${ROMFSDIR}/bin/webs
fi
if [ "$_START_BOA_" = "y" ]; then
cp -rf ${GOAHEADDIR}/etc/boa ${ROMFSDIR}/etc/
fi
if [ "$_LANGUAGE_PACK_" ]; then
rm -rf ${ROMFSDIR}/web/language_pack ${ROMFSDIR}/web/language_pack.txt
ln -s /tmp/language_pack.txt ${ROMFSDIR}/web/language_pack.txt
fi
if [ "$_MODE_" = "Customer" ]; then
if [ "$_RFTYPE_" = "1T1R" ]; then
cp ${APPDIR}/goahead-2.1.1/web-gw-Customer/graphic/250R.gif ${ROMFSDIR}/web/graphic/index_4.gif
elif [ "$_RFTYPE_" = "2T2R" ]; then
cp ${APPDIR}/goahead-2.1.1/web-gw-Customer/graphic/350R.gif ${ROMFSDIR}/web/graphic/index_4.gif
fi
fi
if [ "$_MODE_" = "Customer" ]; then
rm -rf ${ROMFSDIR}/web/file/db_banner_l.gif
if [ "$_MODEL_" = "BR6258GN" ]; then
mv ${ROMFSDIR}/web/file/MZK-RP150N.gif ${ROMFSDIR}/web/file/db_banner_l.gif
elif [ "$_MODEL_" = "BR6428GNL" ]; then
mv ${ROMFSDIR}/web/file/MZK-MF300N2.gif ${ROMFSDIR}/web/file/db_banner_l.gif
fi
rm -rf ${ROMFSDIR}/web/file/MZK-MF300N2.gif ${ROMFSDIR}/web/file/MZK-RP150N.gif
fi
if [ "$_MODE_" = "Customer" ]; then
if [ "$_MODEL_" = "BR6228GNV2" ]; then
cp ${APPDIR}/goahead-2.1.1/web-gw-Icidu/file/6228gn.jpg ${ROMFSDIR}/web/file/Icidu_logo.jpg
elif [ "$_MODEL_" = "BR6428GNV2" ]; then
cp ${APPDIR}/goahead-2.1.1/web-gw-Icidu/file/6428gn.jpg ${ROMFSDIR}/web/file/Icidu_logo.jpg
elif [ "$_MODEL_" = "BR6258GNV2" ]; then
cp ${APPDIR}/goahead-2.1.1/web-gw-Icidu/file/6258gn.jpg ${ROMFSDIR}/web/file/Icidu_logo.jpg
elif [ "$_MODEL_" = "BR6478GNV2" ]; then
cp ${APPDIR}/goahead-2.1.1/web-gw-Icidu/file/6478gn.jpg ${ROMFSDIR}/web/file/Icidu_logo.jpg
fi
fi
if [ "$_NBTSCAN2_" = "y" ]; then
cp -Rf ${APPDIR}/nbtscan2/nbtscan ${ROMFSDIR}/bin
else
cp -Rf ${APPDIR}/nbtscan-1.5.1a/nbtscan ${ROMFSDIR}/bin
fi
if [ "$_IS_GATEWAY_" = "y" ]; then
if [ "$_KERNEL_VERSION_26_" = "y" ]; then
cp ${APPDIR}/iproute2-2.6.39/tc/tc ${ROMFSDIR}/bin
else
cp ${APPDIR}/iproute2-2.4.7/tc/tc ${ROMFSDIR}/bin
fi
if [ "$_USB_SERVER_" = "y" ]; then
mkdir -p ${ROMFSDIR}/lib/modules/sxuptp
cp -rf ${APPDIR}/sxuptp/*.ko ${ROMFSDIR}/lib/modules/sxuptp
cp -f ${APPDIR}/probeUSB/probeUSB/probeUSB ${ROMFSDIR}/bin
if [ "$_USB_8712_" = "y" ]; then
cp -f ${APPDIR}/USB_8712/8712u.ko ${ROMFSDIR}/lib/modules/sxuptp
fi
fi
if [ "$_CONNECTION_CTRL_" = "y" ]; then
mkdir -p ${ROMFSDIR}/lib/modules/2.6.20/netfilter
cp -f ${APPDIR}/module/netfilter/ipt_connlimit.ko   ${ROMFSDIR}/lib/modules/2.6.20/netfilter
fi
mkdir ${ROMFSDIR}/usr/share
mkdir ${ROMFSDIR}/var/lib
mkdir ${ROMFSDIR}/var/lib/misc
cp ${APPDIR}/dnrd-2.10/src/dnrd ${ROMFSDIR}/bin
mkdir ${ROMFSDIR}/etc/dnrd
mkdir -p ${ROMFSDIR}/usr/sbin
ln -s /bin/pppd ${ROMFSDIR}/usr/sbin/pppd
cp ${APPDIR}/ppp-2.4.2/chat/chat ${ROMFSDIR}/sbin
cp ${APPDIR}/ppp-2.4.2/pppd/pppd ${ROMFSDIR}/bin
cp ${APPDIR}/rp-pppoe-3.5/src/pppoe ${ROMFSDIR}/bin
if [ "$_PPPOE_PASSTHROUGH_" = "y" ]; then
cp -Rf ${APPDIR}/rp-pppoe-3.5/src/pppoe-relay ${ROMFSDIR}/bin
fi
if [ "$_L2TPD_" != "" ]; then
mkdir -p ${ROMFSDIR}/etc/l2tpd
cp ${APPDIR}/l2tpd/l2tpd ${ROMFSDIR}/bin
else
cp ${APPDIR}/rp-l2tp-0.4/l2tpd ${ROMFSDIR}/bin
mkdir ${ROMFSDIR}/etc/l2tp
mkdir ${ROMFSDIR}/bin/handlers/
cp ${APPDIR}/rp-l2tp-0.4/handlers/sync-pppd.so ${ROMFSDIR}/bin/handlers/
cp ${APPDIR}/rp-l2tp-0.4/handlers/l2tp-control ${ROMFSDIR}/bin/handlers/
cp ${APPDIR}/rp-l2tp-0.4/handlers/cmd.so ${ROMFSDIR}/bin/handlers/
fi
if [ "${_MODE_}" != "Customer" ]; then
cp ${APPDIR}/bpalogin-2.0.2/bpalogin ${ROMFSDIR}/bin
fi
cp ${APPDIR}/pptp-1.31/pptp ${ROMFSDIR}/bin
mkdir ${ROMFSDIR}/var/lock
if [ "$_IGMP_PROXY_" = "y" ]; then
if [ "$_KERNEL_VERSION_26_" = "y" ]; then
cp ${APPDIR}/igmpproxy_k26/igmpproxy ${ROMFSDIR}/bin
else
cp ${APPDIR}/igmpproxy/igmpproxy ${ROMFSDIR}/bin
fi
fi
if [ "$_ENSNMP_" = "y" ]; then
mkdir -p ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/SNMPv2-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/SNMPv2-SMI.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/SNMPv2-TC.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/IP-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/IANAifType-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/IF-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/TCP-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/UDP-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/HOST-RESOURCES-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/NOTIFICATION-LOG-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/SNMP-VIEW-BASED-ACM-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/SNMP-COMMUNITY-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/SNMP-FRAMEWORK-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/SNMP-MPD-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/SNMP-USER-BASED-SM-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/SNMP-TARGET-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a ${APPDIR}/net-snmp-5.1.4/install/usr/local/share/snmp/mibs/UCD-SNMP-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a  ${APPDIR}/net-snmp-5.1.4/trap_mib_edimax/NOTIFICATION-TEST-MIB.txt  ${ROMFSDIR}/usr/local/share/snmp/mibs
cp -a  ${APPDIR}/net-snmp-5.1.4/snmpd.conf  ${ROMFSDIR}/etc
cp -a  ${APPDIR}/net-snmp-5.1.4/snmp.sh ${ROMFSDIR}/bin
cp ${APPDIR}/net-snmp-5.1.4/install/usr/local/sbin/snmpd ${ROMFSDIR}/bin
cp ${APPDIR}/snmptrap/snmptrap ${ROMFSDIR}/bin
fi
if [ "$_ENRIP_" = "y" ]; then
cp ${APPDIR}/zebra-0.94/ripd/ripd ${ROMFSDIR}/bin
cp ${APPDIR}/zebra-0.94/zebra/zebra ${ROMFSDIR}/bin
fi
if [ "$_KERNEL_VERSION_26_" = "y" ]; then
cp -rf ${APPDIR}/iptables-1.4.4/.libs/iptables ${ROMFSDIR}/bin
if [ "$_IPV6_SUPPORT_" = "y" ]; then
cp -rf ${APPDIR}/iptables-1.4.4/.libs/ip6tables ${ROMFSDIR}/bin
fi
cp -rf ${APPDIR}/iptables-1.4.4/libiptc/.libs/libiptc.so.0.0.0 ${ROMFSDIR}/lib/libiptc.so.0
cp -rf ${APPDIR}/iptables-1.4.4/.libs/libxtables.so.2.0.0 ${ROMFSDIR}/lib/libxtables.so.2
else
cp -rf ${APPDIR}/iptables-1.3.8/iptables ${ROMFSDIR}/bin
fi
mkdir ${ROMFSDIR}/usr/lib
if [ "$_NATPMP_" = "y" ]; then
cp ${APPDIR}/natpmp-0.2.3/natpmp ${ROMFSDIR}/bin
fi
if [ "$_PC_DATABASE_" = "y" ]; then
cp ${APPDIR}/pc_database_tool/addTime ${ROMFSDIR}/bin
fi
if [ "$_IPV6_SUPPORT_" = "y" ]; then
cp ${APPDIR}/dnsmasq-2.57/src/dnsmasq ${ROMFSDIR}/bin
cp ${APPDIR}/gogoc-1_2-RELEASE/gogoc-tsp/bin/gogoc ${ROMFSDIR}/bin
cp ${APPDIR}/gogoc-1_2-RELEASE/gogoc-tsp/template/linux.sh ${ROMFSDIR}/bin/gogoc.sh
cp ${APPDIR}/iproute2-2.6.39/ip/ip ${ROMFSDIR}/bin/ip2
cp ${APPDIR}/mldc/mldc ${ROMFSDIR}/bin
cp ${APPDIR}/radvd-1.7/radvd ${ROMFSDIR}/bin
cp ${APPDIR}/wide-dhcpv6-20080615/dhcp6s ${ROMFSDIR}/bin
cp ${APPDIR}/wide-dhcpv6-20080615/dhcp6c ${ROMFSDIR}/bin
cp ${APPDIR}/iputils-s20101006/ping6 ${ROMFSDIR}/bin
if [ "$_IPV6_READY_LOGO_" = "y" ]; then
cp $APPDIR/CHT_IPv6_Ready_Logo/P2_Core_IOT_Router_Cmd_Linux_all_in_one.txt $ROMFSDIR/etc
cp $APPDIR/CHT_IPv6_Ready_Logo/P2_Core_IOT_Router_Cmd_Linux_radvdconfig/* $ROMFSDIR/etc
fi
fi
if [ "$_AUTO_WAN_PROB_" = "y" ] || [ "$_WanAutoDetect_" = "y" ]; then
cp ${APPDIR}/fping-2.4b2_to/fping ${ROMFSDIR}/bin
fi
fi
if [ "$_IS_GATEWAY_" != "y" ]; then
cp -rf ${APPDIR}/freeradius-1.0.2/tmp/* ${ROMFSDIR}/
fi
if [ "$_STRACE_" = "y" ]; then
cp ${APPDIR}/strace-4.6/strace ${ROMFSDIR}/bin
fi
if [ "$_EZVIEW_" = "y" ]; then
cp -Rf ${APPDIR}/upnpscan/src/upnpscan ${ROMFSDIR}/bin
fi
if [ "$_EZ_XML_TAG_" = "y" ]; then
mkdir ${ROMFSDIR}/web/upnp
ln -s /var/APDesc.xml ${ROMFSDIR}/web/upnp/APDesc.xml
cp ${APPDIR}/ezview_upnpd/upnpd ${ROMFSDIR}/bin
fi
if [ "$_RDISC_" = "y" ]; then
cp ${APPDIR}/iputils/rdisc ${ROMFSDIR}/bin
fi
if [ "$_AUTOWPA_" = "y" ] || [ "$_WEP_MAC_" = "y" ] || [ "$_WPA_KEY_BY_MAC_" = "y" ] || [ "$_AUTOWPA_BY_DEFAULT_" = "y" ]; then
cp -f ${APPDIR}/AutoWPA/AutoWPA ${ROMFSDIR}/bin
fi
if [ "${_MODE_}" = "Customer" ]; then
cp -f ${APPDIR}/Agent-Edimax/agent ${ROMFSDIR}/bin
fi
if [ "$_DNS_HIJACK_" = "y" ]; then
cp -av ${APPDIR}/dsniff-2.4/dnsspoof ${ROMFSDIR}/bin
cp -av ${APPDIR}/libnids-1.19/build/lib/libnids.so.1.19 ${ROMFSDIR}/lib
cp -av ${APPDIR}/Libnet-1.0.2a/build/lib/libnet.so.0.0.0 ${ROMFSDIR}/lib/libnet.so.0
cp -av ${APPDIR}/gdbm-1.8.3/build/lib/libgdbm.so.3.0.0 ${ROMFSDIR}/lib/libgdbm.so.3
fi
if [ "$_WAN3G_" = "y" ]; then
mknod -m666 ${ROMFSDIR}/dev/ttyUSB0 c 188 0
mknod -m666 ${ROMFSDIR}/dev/ttyUSB1 c 188 1
mknod -m666 ${ROMFSDIR}/dev/ttyUSB2 c 188 2
mknod -m666 ${ROMFSDIR}/dev/ttyUSB3 c 188 3
mknod -m666 ${ROMFSDIR}/dev/ttyUSB4 c 188 4
cp -f ${LINUXDIR}/drivers/usb/serial/usbserial.ko ${ROMFSDIR}/bin/
cp -ra ${APPDIR}/3G_WAN/hotplug ${ROMFSDIR}/etc/
cp -ra ${APPDIR}/3G_WAN/pccard_template ${ROMFSDIR}/etc/
cp -a ${APPDIR}/3G_WAN/usb_hotplug.sh ${ROMFSDIR}/bin/
cp -a ${APPDIR}/3G_WAN/stop_3g.sh ${ROMFSDIR}/bin/
cp -a ${APPDIR}/3G_WAN/sierra_cdma ${ROMFSDIR}/etc/
cp -a ${APPDIR}/3G_WAN/sierra_cdma_chat ${ROMFSDIR}/etc/
cp -a ${APPDIR}/3G_WAN/sierra_cdma_chat_disconnect ${ROMFSDIR}/etc/
cp -a ${APPDIR}/3G_WAN/usb_modeswitch-0.9.5/usb_modeswitch ${ROMFSDIR}/bin/
cp -a ${APPDIR}/3G_WAN/usr_bin/runppp.sh ${ROMFSDIR}/usr/bin/
cp -a ${APPDIR}/3G_WAN/SendATCmd/sendatcmd ${ROMFSDIR}/bin/tip_sendat
cp -a ${APPDIR}/3G_WAN/ttcp/ttcp ${ROMFSDIR}/bin/
cp -a ${APPDIR}/3G_WAN/comgt.0.32/comgt ${ROMFSDIR}/bin/comgt
echo "none	/proc/bus/usb	usbfs	defaults	0	0" >> ${ROMFSDIR}/etc/fstab
mkdir -p ${ROMFSDIR}/var/wan
else
rm -f ${ROMFSDIR}/bin/getcard.sh
rm -f ${ROMFSDIR}/web/3gwwan.asp
fi
cp ${ROOTDIR}/define/FUNCTION_SCRIPT ${ROMFSDIR}/web/
cp ${ROOTDIR}/AP/mkimg/l7-protocols ${ROMFSDIR}/etc/ -rf
if [ "$_PLATFORM_" = "RTL8196C_1200" ]; then
PATH=${CROSSDIR}/bin:$PATH
cp -f ${_PLATFORM_}_tools/libstrip/libgcc_s_4181.so.1 $ROMFSDIR/lib/libgcc_s_4181.so.1
${CROSS_COMPILE}lstrip $ROMFSDIR
elif [ "$_PLATFORM_" = "RTL8196C_2300" ] || [ "$_PLATFORM_" = "RTL8196C_2400" ] || [ "$_PLATFORM_" = "RTL8196C_2500" ]; then
PATH=${CROSSDIR}/bin:$PATH
cp -f ${_PLATFORM_}_tools/libstrip/libgcc_s_4181.so.1 $ROMFSDIR/lib/libgcc_s_4181.so.1
${CROSS_COMPILE}lstrip $ROMFSDIR
elif [ "$_PLATFORM_" = "RTL8198_2300" ] || [ "$_PLATFORM_" = "RTL8198_2400" ] || [ "$_PLATFORM_" = "RTL8198_2500" ]; then
PATH=${CROSSDIR}/bin:$PATH
cp -f ${_PLATFORM_}_tools/libstrip/libgcc_s_5281.so.1 $ROMFSDIR/lib/libgcc_s_5281.so.1
${CROSS_COMPILE}lstrip $ROMFSDIR
fi
if [ "$_EDIMAX30_" = "y" ]; then
cp -f libc.so.0 $ROMFSDIR/lib/libc.so.0			#cypress edimax30 100714
fi
if [ "$_USB_8712_" = "y" -a "$_MODE_" = "Customer" ]; then	
cp -Rf ${ROMFSDIR}/etc ${ROMFSDIR}/etc.tmp
fi
echo "clean SVN....."
find ${ROMFSDIR} -name .svn | xargs -i rm -rf {}
chmod 777 ${ROMFSDIR}/bin/*.*
if [ ! -f ${ROOTDIR}/toolchain/clean-space/clean-space ]; then
cd ${ROOTDIR}/toolchain/clean-space
gcc -o clean-space clean-space.c
fi
${ROOTDIR}/toolchain/clean-space/clean-space ${ROMFSDIR}
find ${ROMFSDIR}/* | xargs -i file {}  | grep "strip" | cut -f1 -d":" | xargs -r ${CROSS_COMPILE}strip -R .comment -R .note -g --strip-unneeded;
if [ "$_EDIMAX30_" = "y" ]; then
ln -s /tmp/room1 ${ROMFSDIR}/web/room1
ln -s /tmp/room2 ${ROMFSDIR}/web/room2
ln -s /tmp/room3 ${ROMFSDIR}/web/room3
cp ../Edimax_3_0/edimax_3_0 ${ROMFSDIR}/bin/edimax_3_0
cp ../crossdomain/crossdomain ${ROMFSDIR}/bin/crossdomain
cp ../readJPG/readJPG ${ROMFSDIR}/bin/readJPG
ln -s /var/log/syslog ${ROMFSDIR}/web/E3_sys_log
ln -s /var/log/security ${ROMFSDIR}/web/E3_sec_log
ln -s /var/run/wanstat ${ROMFSDIR}/web/E3_wan_log
ln -s /tmp/device_list.xml ${ROMFSDIR}/web/openport.xml
ln -s /tmp/edimax3_wrong_gateway.xml ${ROMFSDIR}/web/edimax3_wrong_gateway.xml
cp ../ddns_update/ddns_update ${ROMFSDIR}/bin/ddns_update
fi
cp ${APPDIR}/script/*.sh ${ROMFSDIR}/bin
rm -rf ${ROMFSDIR}/bin/config-vlan.sh
rm -rf ${ROMFSDIR}/bin/scriptlib_3G.sh
rm -rf ${ROMFSDIR}/bin/upnp_protocol_getinfo.sh
rm -rf ${ROMFSDIR}/bin/wan_3070sta.sh
rm -rf ${ROMFSDIR}/bin/wan_3070.sh
rm -rf ${ROMFSDIR}/bin/2880bridge.sh
rm -rf ${ROMFSDIR}/bin/repeater_status.sh
rm -rf ${ROMFSDIR}/bin/rftest_old.sh
rm -rf ${ROMFSDIR}/bin/wlan_sta.sh
if [ "$_INTERNET_SCHEDULE_" != "y" ]; then
rm -rf ${ROMFSDIR}/bin/internet_sch.sh
fi
if [ "$_DNS_HIJACK_" = "y" ]; then
rm -rf ${ROMFSDIR}/bin/autoWanDetect.sh
rm -rf ${ROMFSDIR}/bin/detectlog.sh
rm -rf ${ROMFSDIR}/bin/detect_wan.sh
rm -rf ${ROMFSDIR}/bin/Customer_threeway_switch.sh
fi
if [ "$_EZ_QOS_" != "y" ]; then
rm -rf ${ROMFSDIR}/bin/bandwidth_calc.sh
fi
if [ "$_EZVIEW_" = "y" ]; then
cp ${APPDIR}/ezview-flash-converter/ez ${ROMFSDIR}/bin
else
rm -rf ${ROMFSDIR}/bin/ez.sh
rm -rf ${ROMFSDIR}/bin/ez1.sh
fi
if [ "$_IPV6_SUPPORT_" != "y" ]; then
rm -rf ${ROMFSDIR}/bin/ipv6*
fi
