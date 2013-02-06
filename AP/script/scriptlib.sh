#!/bin/sh
if [ "$__FUNCTION_SCRIPT" = "" ]; then
. /web/FUNCTION_SCRIPT
__FUNCTION_SCRIPT="y"
fi
WLAN_IF2=wlan0
findCurrentFlashValue()
{
flash all2 | grep $1 | cut -d= -f 2
}
flashDefault()
{
flash default
if [ "$_WIFI_SUPPORT_" = "y" ]; then
flash set WLAN_CTS 0
flash set WLAN_N_CHAN_WIDTH 1 #20Mhz
flash set WIFI_TEST 1
flash set WPS_PROXY_ENABLE 1
fi
echo flash default
if [ "$_MODE_" = "Customer" ]; then
setSSID=`findCurrentFlashValue HW_NIC0_ADDR`
flash set SSID_1 $setSSID
flash set USER_PASSWORD `echo $setSSID | cut -c 7-`
if [ "$1" != "0" ]; then
kill -USR1 `pidof webs` 2> /dev/null
fi
if [ "`findCurrentFlashValue IS_RESET_DEFAULT`" = "0" ]; then
flash set IS_RESET_DEFAULT 1
fi
fi
}
reloadFlash()
{
if [ "$_MODE_" = "Customer" ] && [ "`findCurrentFlashValue IS_RESET_DEFAULT`" = "0" ]; then
flashDefault
fi
if [ "$_SPECIAL_CHAR_FILTER_IN_SCRIPT_" = "y" ]; then
/bin/flash all2 > /var/flash.inc
else
/bin/flash all > /var/flash.inc
fi
if [ "`flash all2 | grep OP_MODE | cut -f 2 -d =`" = "1" ]; then
if [ "$_USB_8712_" = "y" ]; then
if [ "`flash all2 | grep QUICK_TYPE | cut -f 2 -d =`" = "0" ]; then
echo "_WAN_IF_=\"wlan1\"" >> /var/flash.inc
elif [ "`flash all2 | grep QUICK_TYPE | cut -f 2 -d =`" = "1" ]; then
echo "_WAN_IF_=\"eth1\"" >> /var/flash.inc
fi
else
echo "_WAN_IF_=\"$WLAN_IF2\"" >> /var/flash.inc
fi
elif [ "`flash all2 | grep OP_MODE | cut -f 2 -d =`" = "2" ]; then
echo "_WAN_IF_=\"wlan0\"" >> /var/flash.inc
elif [ "$_DEFAULT_WAN_IF_" != "" ]; then
echo "_WAN_IF_=\""$_DEFAULT_WAN_IF_"\"" >> /var/flash.inc
else
echo "_WAN_IF_=\"eth1\"" >> /var/flash.inc
fi
if [ "$_DEFAULT_LAN_IF_" != "" ]; then
echo "_LAN_IF_=\""$_DEFAULT_LAN_IF_"\"" >> /var/flash.inc
else
echo "_LAN_IF_=\"eth0\"" >> /var/flash.inc
fi
}
if [ "$__flash" = "" ]; then
if [ ! -f /var/flash.inc ]; then
reloadFlash
fi
. /var/flash.inc
__flash="y"
fi
__TRUE="1"
__FALSE="0"
__LINK_UP="1"
__LINK_DOWN="0"
__UNDEFINED="-1"
reloadWirelessDriver()
{
ifconfig wlan0 down
ifconfig wlan0 up
brctl addif br0 wlan0
}
getWdsNum()
{
COUNT=0
ADDR_NUM=0
while [ $COUNT -lt 6 ]
do
eval "WL_LINKMAC="\$WL_LINKMAC$COUNT""
if [ "$WL_LINKMAC" != "000000000000" ]; then
ADDR_NUM=`expr $ADDR_NUM + 1`
fi
COUNT=`expr $COUNT + 1`
done
echo $ADDR_NUM
}
setMacFilterByHw()
{
echo "LED ON" > /dev/WPS_LED
WAIT_TIME=$1
NSSID=\$SSID_"$2"
flash="/bin/flash" #flash tool
WLAN="ra"$2
if [ "$2" -ge "1" ]; then
MACAC="WLAN_MACAC"`expr $2 + 1`
else
MACAC="WLAN_MACAC"
fi
eval "$MACAC"_NUM="`eval echo \$"$MACAC"_NUM`"
echo  start to collect macs
iwpriv $WLAN set HideSSID=0
if [ "$2" -ge "1" ]; then
iwpriv $WLAN set SSID=`eval echo $NSSID`
else
iwpriv $WLAN set SSID=$SSID
fi
iwpriv $WLAN set AccessPolicy=0
ALL_MAC=`iwpriv $WLAN show_mac_table | grep -v $WLAN`
echo now mac = $ALL_MAC
count=1
BREAK=0
while [ "$BREAK" = "0" ]; do
sleep 1
NOW_MAC=`iwpriv $WLAN show_mac_table | grep -v $WLAN`
echo now mac =  $NOW_MAC
for arg in $NOW_MAC
do
if [ "`echo $ALL_MAC | grep -i $arg`" = "" ]; then
if [ "`$flash all | grep \"$MACAC\"_ADDR | grep -i $arg`" = "" ]; then
if [ "`eval echo \$\"$MACAC\"_NUM`" = "20" ]; then
$flash set "$MACAC"_ADDR del del # delete first mac in mac filter
fi
$flash set "$MACAC"_ADDR add $arg
sleep 1
BREAK=1
fi
fi
done
if [ "$count" = "$WAIT_TIME" ]; then
BREAK=1
fi
count=`expr $count + 1`
echo $count
ALL_MAC=$NOW_MAC
done
echo finish collecting mac
if [ "$2" -ge "1" ]; then
aclList="aclList""`expr $2 + 1`"
MAC_LIST=`flash $aclList | cut -d '=' -f 2`
else
MAC_LIST=`flash aclList | cut -d '=' -f 2`
fi
echo "MAC LIST="$MAC_LIST
if [ "$MAC_LIST" != "" ]; then
iwpriv $WLAN set ACLClearAll=1
iwpriv $WLAN set ACLAddEntry="$MAC_LIST"
else
echo no mac in mac list
fi
iwpriv $WLAN set AccessPolicy=1
iwpriv $WLAN set HideSSID=1
if [ "$2" -ge "1" ]; then
iwpriv $WLAN set SSID=`eval echo $NSSID`
flash set HIDDEN_SSID_$2 1
flash set WLAN_MACAC"`expr $2 + 1`"_ENABLED 1
else
iwpriv $WLAN set SSID=$SSID
flash set HIDDEN_SSID 1
flash set WLAN_MACAC_ENABLED 1
fi
reloadFlash
kill -USR1 `pidof webs`
echo "LED OFF" > /dev/WPS_LED
}
setUpWirelessDriver()
{
WCOUNT=0
while [ $WCOUNT -le 7 ];
do
ifconfig wlan$WCOUNT down 2> /dev/null
brctl delif br0 wlan$WCOUNT 2> /dev/null
WCOUNT=`expr $WCOUNT + 1`
done
WCOUNT=""
ifconfig $WLAN_IF2 down 2> /dev/null
ifconfig wds0 down 2> /dev/null
ifconfig wds1 down 2> /dev/null
ifconfig wds2 down 2> /dev/null
ifconfig wds3 down 2> /dev/null
brctl delif br0 $WLAN_IF2 2> /dev/null
brctl delif br0 wds0 2> /dev/null
brctl delif br0 wds1 2> /dev/null
brctl delif br0 wds2 2> /dev/null
brctl delif br0 wds3 2> /dev/null
if [ "$_DFS_" = "y" ] || [ "$_CARRIER_" = "y" ]; then
insmod /bin/rt_timer.ko
fi
rmmod rt2860v2_ap 2> /dev/null
rmmod rt2860v2_sta 2> /dev/null
sleep 1
if [ "$1" = "" ] || [ "$1" = "0" ] || [ "$1" = "1" ]; then
insmod /bin/rt2860v2_ap.ko
elif [ "$1" = "2" ]; then
insmod /bin/rt2860v2_sta.ko
fi
}
wirelessSchedule()
{
flash="/bin/flash" #flash tool
SCH_NUM=`$flash get WLAN_SCH_NUM | cut -d '=' -f 2`
profile="/var/cron/crontabs/root"
rm -rf /var/cron
mkdir /var/cron/
mkdir /var/cron/crontabs
touch /var/cron/crontabs/root
count=0
while [ "$count" -lt "$SCH_NUM" ]
do
SCH=`$flash schList $count | cut -d '=' -f 2`
SCHT=`echo $SCH | cut -d ';' -f 1`
COM=`echo $SCH | cut -d ';' -f 2 | cut -d '|' -f1`
for arg in $SCHT
do
if [ "$arg" = "100" ]; then
echo -n  \* >> $profile
echo -n " " >> $profile
else
echo -n $arg >> $profile
echo -n " " >> $profile
fi
done
echo $COM >> $profile
count=`expr $count + 1`
done
killall -9 crond
crond -L /var/cron/crond.log -l 0 &
}
setUpWireless()
{
if [ $1 = "1" ]; then
ifconfig wlan0 down
if [ "$AP_MODE" = 3 ]; then
ifconfig wlan0-wds0 down
elif [ "$AP_MODE" = 4 ]; then
ifconfig wlan0-wds0 down
elif [ "$AP_MODE" = 5 ]; then
ifconfig wlan0-wds0 down
elif [ "$AP_MODE" = 6 ]; then
ifconfig wlan0-vxd down
fi
echo 3 > /proc/wlan_led
else
ifconfig wlan0 up
if [ "$AP_MODE" = 3 ]; then
ifconfig wlan0-wds0 up
elif [ "$AP_MODE" = 4 ]; then
ifconfig wlan0-wds0 up
elif [ "$AP_MODE" = 5 ]; then
ifconfig wlan0-wds0 up
elif [ "$AP_MODE" = 6 ]; then
ifconfig wlan0-vxd up
fi
echo 2 > /proc/wlan_led
fi
}
setPassthroughLanToWan()
{
MAC=`cat /proc/net/arp | grep $1 | cut -b42-59`
iptables -t nat -I PREROUTING -m physdev --physdev-in wlan1  -m mac --mac-source $MAC -j RETURN
echo -n $MAC >> /var/MacAddr
}
removePassthroughLanToWan()
{
OLD_MAC=`cat /var/MacAddr`
ALL_MAC=`iwpriv wlan0 show_mac_table | grep -v wlan0`
rm -f /var/MacAddr
touch /var/MacAddr
for arg in $OLD_MAC
do
if [ "`echo $ALL_MAC | grep -i $arg`" = "" ]; then
iptables -t nat -D PREROUTING -m physdev --physdev-in wlan1  -m mac --mac-source $arg -j RETURN
else
echo -n $arg >> /var/MacAddr
fi
done
}
PREVIOUS_STATE=$__UNDEFINED
checkWANStatus()
{
if [ "$WAN_MODE" = "0" ]; then
LINK_STATE=`/bin/linkdetect.sh 4 | cut -f 2 -d =`
if [ "$PREVIOUS_STATE" != __UNDEFINED ]; then
if [ "$PREVIOUS_STATE" = "0" ] && [ "$LINK_STATE" = "1" ]; then
echo "WAN DOWN to UP"
/bin/dhcpc.sh $_WAN_IF_ wait
elif  [ "$PREVIOUS_STATE" = "1" ] && [ "$LINK_STATE" = "0" ]; then
echo "WAN UP to DOWN"
ifconfig $_WAN_IF_ 0.0.0.0
fi
fi
PREVIOUS_STATE=$LINK_STATE
else
PREVIOUS_STATE=$__UNDEFINED
fi
}
pingtool()
{
killall -2 ping
ping -W 5 -c $2 $1 > /tmp/$3
echo "---   END   ---" >> /tmp/$3
}
Schedule_PortForwarding()
{
/bin/flash set PORTFW_ENABLED $1
reloadFlash
/bin/firewall.sh
kill -USR1 `pidof webs`
}
Schedule_UrlBlocking()
{
/bin/flash set URLB_ENABLED $1
reloadFlash
/bin/firewall.sh
kill -USR1 `pidof webs`
}
getCurrentWANIP()
{
CURRENT_WAN="$_WAN_IF_"
if [ "$OP_MODE" = '1' ];then
if [ "$_USB_8712_" = "y" ]; then
CURRENT_WAN="wlan1"
else
CURRENT_WAN="$WLAN_IF2"
fi
fi
if [ "$WAN_MODE" = "2" ] ||  [ "$WAN_MODE" = "3" ] || [ "$WAN_MODE" = "6" ]; then
CURRENT_WAN="ppp0"
if [ "$MPPPOE" != "" ]; then
CURRENT_WAN1="ppp1"
fi
fi
CURRENT_WAN_IP=`ifconfig $CURRENT_WAN | grep -i "addr:" | cut -f2 -d: | cut -f1 -d " "`
}
setSuperDMZ()
{
killall udhcpd
getCurrentWANIP
echo "============SET Super DMZ================"
echo "==== "DMZ_MAC=$DMZ_MAC_ADDR   LANMAC=$HW_NIC0_ADDR   LANIP=$IP_ADDR  WANIP=$CURRENT_WAN_IP"  ===="
echo $DMZ_MAC_ADDR $HW_NIC0_ADDR $IP_ADDR $CURRENT_WAN_IP $1 > /proc/sys/net/ipv4/super_dmz_control
dhcpd.sh br0
}
wait_time_pass()
{
COUNT=1
TIMEOUT=$1
while [ "$COUNT" -le "$TIMEOUT" ]; do
echo -e "$COUNT / $TIMEOUT\r\c"
COUNT=`expr $COUNT + 1`
done
}
checkIsConnect() {
echo "checkIsConnect"
eval `flash get WAN_MODE`
wanStatus=/tmp/wanStatus
if [ "`cat $wanStatus`" = 4 ]; then
echo "wan port no link!!"
exit
fi
if [ "$_MODE_" = "Customer" ]; then
sleep 24
else
sleep 50
fi
result=0
gateway=`route -n | grep ^0.0.0.0 | tr -s " " | cut -d " " -f 2`
if [ "$gateway" ]; then
if [ "`fping $gateway | grep alive | wc -l`" = 1 ]; then result=1; fi # Default Gateway
if [ "$result" = 0 ] && [ "`fping 8.8.8.8 | grep alive | wc -l`" = 1 ]; then result=1; fi # Google DNS
if [ "$result" = 0 ] && [ "`fping 8.8.4.4 | grep alive | wc -l`" = 1 ]; then result=1; fi # Google DNS
if [ "$result" = 0 ] && [ "`fping 208.67.222.222 | grep alive | wc -l`" = 1 ]; then result=1; fi # Open DNS
if [ "$result" = 0 ] && [ "`fping 208.67.220.220 | grep alive | wc -l`" = 1 ]; then result=1; fi # Open DNS
fi
if [ "$result" = 1 ]; then
echo Pass
echo 1 > $wanStatus
else
echo Fail
case "$WAN_MODE" in
"0")
echo 2 > $wanStatus # DHCP Fail
;;
"1" | "2")
echo 3 > $wanStatus # PPPoE Fail
;;
esac
fi
}
led_control()
{
DEV_WALN_LED=/dev/WLAN_LED0
DEV_POW_LED=/dev/PowerLED
if [ "$1" = "ON" ]; then
reg s 0xb0110000
reg w a4 5
reg w a8 5
reg w ac 5
reg w b0 5
reg w b4 5
if [ ! -f $DEV_WALN_LED ]; then
mknod /dev/WLAN_LED0 c 166 $_HW_LED_WIRELESS_
echo "LED BLINK RANDOM" > $DEV_WALN_LED
else
echo "LED BLINK RANDOM" > $DEV_WALN_LED
fi
elif [ "$1" = "OFF" ]; then
reg s 0xb0110000
reg w a4 7
reg w a8 7
reg w ac 7
reg w b0 7
reg w b4 7
echo "LED OFF" > $DEV_WALN_LED
rm -f $DEV_WALN_LED
fi
}
lockusingfile()
{
lockfile=/var/lockusingfile.var
while [ true ]; do
if ( set -o noclobber; echo "$1 `pidof $1`" > "$lockfile") 2> /dev/null; then
trap 'rm -f "$lockfile"; exit $?' INT TERM EXIT
$1 $2 $3 $4 $5 $6 $7
trap - INT TERM EXIT
break
fi
done
}
unlockfile()
{
lockfile=/var/lockusingfile.var
STARTLOCK=1
ENDLOCK=$1
while [ "$STARTLOCK" -le "$ENDLOCK" ]; do
sleep 1
STARTLOCK=`expr $STARTLOCK + 1`
if [ ! -f "$lockfile" ]; then
break
fi
done
rm -f "$lockfile" 2> /dev/null
}
create_pppoe_secretfile()
{
echo "#################################################" > $3
echo "\"$1\"	*	\"$2\" *" >> $3
}
create_pppoe_optionfile()
{
if [ "$1" != "add" ]; then
echo "name \"$1\"" > $5
echo "noipdefault" >> $5
echo "hide-password" >> $5
echo "ipcp-accept-remote" >> $5
echo "ipcp-accept-local" >> $5
echo "nodetach" >> $5
echo "usepeerdns" >> $5
echo "lcp-echo-interval 2" >> $5
echo "lcp-echo-failure 20" >> $5
echo "lock" >> $5
echo "mtu $2" >> $5
echo "mru $3" >> $5
echo "sync" >> $5
echo "$4" >> $5
elif [ "$1" = "add" ]; then
echo "$2" >> "$3"
fi
}
eth_port_control()
{
if [ "$1" = "0" ] || [ "$1" = "1" ] || [ "$1" = "2" ] || [ "$1" = "3" ] || [ "$1" = "4" ]; then
if [ "$2" = "ON" ] || [ "$2" = "1" ]; then
mii_mgr -s -p $1 -r 0 -v 0x3100 2> /dev/null
elif [ "$2" = "OFF" ] || [ "$2" = "0" ]; then
mii_mgr -s -p $1 -r 0 -v 0x3900 2> /dev/null
else
echo "Input Error Var2 != ON or OFF"
fi
else
echo "Input Error Var1 !=  0-4"
fi
}
start_usb_server()
{
if [ "$_USB_SERVER_" != "" ]; then
if [ "$_USB_8712_" = "y" ]; then
export SXUPTP=/lib/modules/sxuptp
insmod ${SXUPTP}/8712u.ko
mount -t usbfs none /proc/bus/usb
else
export SXUPTP=/lib/modules/sxuptp
insmod ${SXUPTP}/sxuptp.ko netif=br0
insmod ${SXUPTP}/sxuptp_driver.ko
insmod ${SXUPTP}/jcp.ko
insmod ${SXUPTP}/jcp_cmd.ko
mount -t usbfs none /proc/bus/usb
killall probeUSB
probeUSB &
fi
fi
}
stop_usb_server()
{
if [ "$_USB_SERVER_" != "" ]; then
rmmod jcp_cmd
rmmod jcp
rmmod sxuptp_driver
rmmod sxuptp
umount /proc/bus/usb
killall probeUSB
fi
}
get_sntp_status()
{
if [ "$_POWERSAVINGCustomer_" != "" ]; then
/bin/sntp.sh
sntpStatus=`cat /var/log/sntp.log | grep -i "ERROR"`
if [ "$sntpStatus" != "" ]; then
echo "0" > /tmp/sntpStatus.txt
else
echo "1" > /tmp/sntpStatus.txt
fi
fi
}
dhcpc_up_down()
{
if [ "$1" = "0" ]; then
echo "_____udhcpc down_____"
ifconfig $_WAN_IF_ 0.0.0.0
kill -9 `pidof udhcpc` 2> /dev/null
else
echo "_____udhcpc up_____"
/bin/dhcpc.sh $_WAN_IF_ wait
fi
}
ststic_up_down()
{
if [ "$1" = "0" ]; then
echo "_____ststic IP down_____"
ifconfig $_WAN_IF_ 0.0.0.0
else
echo "_____ststic IP up_____"
/bin/fixedip.sh $_WAN_IF_ $WAN_IP_ADDR $WAN_SUBNET_MASK $WAN_DEFAULT_GATEWAY
/bin/ipup.sh static
fi
}
set_groupID()
{
if [ "$_MSSID_" = "y" ]; then
ID=0
while [ $ID -lt $_MSSIDNUM_ ]; do
eval "EN=\$ENGROUPID_$ID"
if [ $ID = 0 ]; then
if [ $ENGROUPID = 1 ]; then
iwpriv wlan0 set_mib groupID=`expr $ID + 1`
else
iwpriv wlan0 set_mib groupID=0
fi
elif [ $EN  = 1 ]; then
NUM=`expr $ID - 1`
iwpriv wlan0-va$NUM set_mib groupID=`expr $ID + 1`
else
NUM=`expr $ID - 1`
iwpriv wlan0-va$NUM set_mib groupID=0
fi
ID=`expr $ID + 1`
done
else
iwpriv wlan0 set groupID=1
fi
}
set_block_relay()
{
if [ "$_MSSID_" = "y" ]; then
ID=0
while [ $ID -lt $_MSSIDNUM_ ]; do
eval "EN=\$ENBLOCKRELAY_$ID"
if [ $ID = 0 ]; then
if [ $ENBLOCKRELAY = 1 ]; then
iwpriv wlan0 set_mib block_relay=1
else
iwpriv wlan0 set_mib block_relay=0
fi
elif [ $EN = 1 ]; then
NUM=`expr $ID - 1`
iwpriv wlan0-va$NUM set_mib block_relay=1
else
NUM=`expr $ID - 1`
iwpriv wlan0-va$NUM set_mib block_relay=0
fi
ID=`expr $ID + 1`
done
else
iwpriv wlan0 set block_relay=1
fi
}
if [ "$_STAR_CF_3G_" = "y" ]; then
. /bin/scriptlib_3G.sh
fi
debug_print()
{
echo -e "[\033[1m$SCRIPT_TITLE\033[0m] $1"
}
debug_print_and_execute()
{
debug_print "# $1"
eval "$1"
}
wan_mtu()
{
case "$1" in
"1")
case "$WAN_MODE" in
"2")
if [ "$PPP_MTU" = "0" ]; then
PPP_MTU=1392
fi
echo "`expr $PPP_MTU - 40`"
;;
"3")
if [ "$PPTP_MTU" = "0" ]; then
PPTP_MTU=1392
fi
echo "`expr $PPTP_MTU - 32`"
;;
"6")
if [ "$L2TP_MTU" = "0" ]; then
L2TP_MTU=1392
fi
echo "`expr $L2TP_MTU - 32`"
;;
*)
;;
esac
;;
"2")
case "$WAN_MODE" in
"2")
if [ "$PPP_MTU1" = "0" ]; then
PPP_MTU1=1392
fi
if [ "$PPP_MTU1" != "" ]; then
echo "`expr $PPP_MTU1 - 40`"
fi
;;
*)
;;
esac
;;
"3")
case "$WAN_MODE" in
"2")
if [ "$PPP_MTU2" = "0" ]; then
PPP_MTU2=1392
fi
if [ "$PPP_MTU2" != "" ]; then
echo "`expr $PPP_MTU2 - 40`"
fi
;;
*)
;;
esac
;;
*)
;;
esac
}
wan_interface()
{
case "$1" in
"1")
case "$WAN_MODE" in
"2" | "3" | "6" )
echo "ppp0"
;;
*)
case "$OP_MODE" in
"1")
if [ "$_USB_8712_" = "y" ]; then
echo "wlan1"
else
echo "wlan0"
fi
;;
*)
echo "eth1"
;;
esac
;;
esac
;;
"2")
case "$WAN_MODE" in
"2")
if [ "$_MPPPOE_" != "" ] && [ "$PPPoEMODE" = "2" ] && [ -f /etc/ppp/link1 ]; then
echo "ppp1"
fi
;;
*)
;;
esac
;;
"3")
case "$WAN_MODE" in
"2")
if [ "$_TRIPPPOE_" != "" ] && [ "$PPPoEMODE" = "2" ] && [ -f /etc/ppp/link2 ]; then
echo "ppp2"
fi
;;
*)
;;
esac
;;
"R")
echo "eth1"
;;
*)
;;
esac
}
lan_interface()
{
echo "br0"
}
ip_of_interface()
{
if [ "$1" = "" ]; then
echo "0.0.0.0"
else
result="`ifconfig $1 | grep \"inet addr:\" | cut -f 2 -d : | cut -f 1 -d \" \"`"
if [ "$result" = "" ]; then
echo "0.0.0.0"
else
echo $result
fi
fi
}
security_log()
{
echo [`date +"%F %T"`]: [$SCRIPT_TITLE]: $1 >> /var/log/security
}
dhcp_discover()
{
result="`udhcpc -n -q -t 2 -T 2 -i $_WAN_IF_ | grep \"^Lease\" | cut -f 3 -d \" \"`"
if [ "$result" ]; then
echo "$result"
fi
}
pppoe_discover()
{
if [ "`pppoe -d -I $_WAN_IF_`" ]; then
echo "y"
fi
}
iQsetup_detect()
{
wanStatus=/tmp/wanStatus
if [ "`dhcp_discover`" ]; then
packetloss=`ping 8.8.8.8 -c 3 -w 3 | grep "packet loss" | tr -s " " | cut -d " " -f 7 | cut -d "%" -f 1`
if [ "$packetloss" -lt 100 ] && [ "$packetloss" != "" ]; then
echo "dhcp successful!"
echo "1" > $wanStatus
else
echo "2" > $wanStatus
fi
elif [ "`pppoe_discover`" ]; then
packetloss=`ping 8.8.8.8 -c 3 -w 3 | grep "packet loss" | tr -s " " | cut -d " " -f 7 | cut -d "%" -f 1`
if [ "$packetloss" -lt 100 ] && [ "$packetloss" != "" ]; then
echo "pppoe successful!"
echo "3" > $wanStatus
else
echo "4" > $wanStatus
fi
else
echo "fail!!"
echo "2" > $wanStatus
fi
}
iQsetup_set()
{
mkdir -p /tmp/www
httpd -h /tmp/www -p 8000
echo "<html>" > /tmp/www/index.html
echo "<head>" >> /tmp/www/index.html
echo "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">" >> /tmp/www/index.html
echo "<meta http-equiv=\"Pragma\" content=\"no-cache\">" >> /tmp/www/index.html
echo "<meta http-equiv=\"cache-control\" content=\"no-cache\">" >> /tmp/www/index.html
echo "<meta http-equiv=\"cache-control\" content=\"no-store\">" >> /tmp/www/index.html
echo "<meta http-equiv=\"Expires\" content=\"-1\">" >> /tmp/www/index.html
echo "</head>" >> /tmp/www/index.html
echo "<script>" >> /tmp/www/index.html
echo "function asd()" >> /tmp/www/index.html
echo "{" >> /tmp/www/index.html
echo "window.location.assign('http://${IP_ADDR}/iQsetup_main.asp');" >> /tmp/www/index.html
echo "}" >> /tmp/www/index.html
echo "</script>" >> /tmp/www/index.html
echo "<body onload=\"asd();\">" >> /tmp/www/index.html
echo "</body>" >> /tmp/www/index.html
echo "</html>" >> /tmp/www/index.html 
chmod 755 /tmp/www/index.html
echo "#!/bin/sh" > /tmp/iptables_build.sh
echo ". /var/flash.inc" >> /tmp/iptables_build.sh
echo "iptables -t nat -D PREROUTING -i br0 ! -d 192.168.2.1 --proto tcp --dport 80 -j DNAT --to 192.168.2.1:8000" >> /tmp/iptables_build.sh
echo "iptables -t nat -I PREROUTING -i br0 ! -d 192.168.2.1 --proto tcp --dport 80 -j DNAT --to 192.168.2.1:8000" >> /tmp/iptables_build.sh
chmod 755 /tmp/iptables_build.sh
echo "#!/bin/sh" > /tmp/iptables_release.sh     
echo ". /var/flash.inc" >> /tmp/iptables_release.sh
echo "iptables -t nat -D PREROUTING -i br0 ! -d 192.168.2.1 --proto tcp --dport 80 -j DNAT --to 192.168.2.1:8000" >> /tmp/iptables_release.sh
chmod 755 /tmp/iptables_release.sh
sh /tmp/iptables_build.sh
}
iQsetup_ping()
{	packetloss="`ping 8.8.8.8 -c 3 -w 3 | grep \"packet loss\" | tr -s \" \" | cut -d \" \" -f 7 | cut -d \"%\" -f 1`"
rm -rf /tmp/wanStatus 2> /dev/null
if [ "$packetloss" != "" ] && [ "$packetloss" -lt 100 ]; then
echo "1" > /tmp/wanStatus
else
echo "2" > /tmp/wanStatus
fi
}
