#!/bin/sh
set -e
. ../../define/FUNCTION_SCRIPT
. ../../define/PATH
PATH=${CROSSDIR}:${PATH}
function SEARCH_AND_REPLACE {
	if [ -f "$1" ]; then
		filename=$1
		target_old=$2
		target_new=$3
		target_line=`cat $filename | grep -n "$2" | cut -f 1 -d :`
		if [ "$target_line" ]; then
			total_line=`cat $filename | wc -l`
			cat $filename | head -n `expr $target_line - 1` > ${filename}_temp
			echo "$target_new" >> ${filename}_temp
			cat $filename | tail -n `expr $total_line - $target_line` >> ${filename}_temp
			cat ${filename}_temp > $filename
			rm -rf ${filename}_temp
		fi
	fi
}
if [ "$_EDIMAX30_" = "y" ]; then
	cp .config_edimax30 .config -f
elif [ "$_DHCP_RELAY_" = "y" ]; then
	cp .config_dhchprelay .config -f
elif [ "$_MODE_" = "Customer" ]; then
	cp .config_Buffalo .config -f
else
	cp .config_normal .config -f
fi
if [ "$_MODE_" = "Customer" ]; then
	SEARCH_AND_REPLACE .config "CONFIG_KLOGD" "CONFIG_KLOGD=y"
fi
if [ "$_SUPPORT_REMOTE_SYSLOGD_" = "y" ]; then
	SEARCH_AND_REPLACE .config "CONFIG_FEATURE_REMOTE_LOG" "CONFIG_FEATURE_REMOTE_LOG=y"
fi
if [ "$_LANGUAGE_PACK_" ]; then
	SEARCH_AND_REPLACE .config "CONFIG_GUNZIP" "CONFIG_GUNZIP=y"
	SEARCH_AND_REPLACE .config "CONFIG_GZIP" "CONFIG_GZIP=y"
fi
if [ "$_USB_SERVER_" = "y" ]; then
	SEARCH_AND_REPLACE .config "CONFIG_HOSTNAME" "CONFIG_HOSTNAME=y"
fi
if [ "$_IPV6_SUPPORT_" = "y" ]; then
	SEARCH_AND_REPLACE .config "CONFIG_FEATURE_IPV6" "CONFIG_FEATURE_IPV6=y"
	SEARCH_AND_REPLACE .config "CONFIG_FEATURE_PREFER_IPV4_ADDRESS" "CONFIG_FEATURE_PREFER_IPV4_ADDRESS=y"
	SEARCH_AND_REPLACE .config "CONFIG_FEATURE_IP_TUNNEL" "CONFIG_FEATURE_IP_TUNNEL=y"
	# SEARCH_AND_REPLACE .config "CONFIG_PING6" "CONFIG_PING6=y"
	SEARCH_AND_REPLACE .config "CONFIG_AWK" "CONFIG_AWK=y"
	SEARCH_AND_REPLACE .config "CONFIG_FEATURE_AWK_MATH" "CONFIG_FEATURE_AWK_MATH=y"
	SEARCH_AND_REPLACE .config "CONFIG_SED" "CONFIG_SED=y"
	# SEARCH_AND_REPLACE .config "CONFIG_IP=y" "# CONFIG_IP is not set"
	# SEARCH_AND_REPLACE .config "CONFIG_FEATURE_IP_ADDRESS=y" "# CONFIG_FEATURE_IP_ADDRESS is not set"
	# SEARCH_AND_REPLACE .config "CONFIG_FEATURE_IP_LINK=y" "# CONFIG_FEATURE_IP_LINK is not set"
	# SEARCH_AND_REPLACE .config "CONFIG_FEATURE_IP_ROUTE=y" "# CONFIG_FEATURE_IP_ROUTE is not set"
	# SEARCH_AND_REPLACE .config "CONFIG_FEATURE_IP_TUNNEL=y" "# CONFIG_FEATURE_IP_TUNNEL is not set"
	# SEARCH_AND_REPLACE .config "CONFIG_FEATURE_IP_RULE=y" "# CONFIG_FEATURE_IP_RULE is not set"
	if [ "$_IPV6_6RD_SUPPORT_" = "y" ]; then
		SEARCH_AND_REPLACE .config "CONFIG_NSLOOKUP" "CONFIG_NSLOOKUP=y"
	fi
fi
if [ "$_TELNET_DAEMON_" = "y" ]; then
	SEARCH_AND_REPLACE .config "CONFIG_TELNETD" "CONFIG_TELNETD=y"
	SEARCH_AND_REPLACE .config "CONFIG_FEATURE_TELNETD_STANDALONE" "CONFIG_FEATURE_TELNETD_STANDALONE=y"
	SEARCH_AND_REPLACE .config "CONFIG_FEATURE_DEVPTS" "# CONFIG_FEATURE_DEVPTS is not set"
fi
if [ "$_WanAutoDetect_" = "y" ] || [ "$_IQSETUP_" = "y" ]; then
	SEARCH_AND_REPLACE .config "CONFIG_HTTPD" "CONFIG_HTTPD=y"
	SEARCH_AND_REPLACE .config "CONFIG_FEATURE_HTTPD_CGI" "CONFIG_FEATURE_HTTPD_CGI=y"
fi
if [ "$_WAKEONLAN_" = "y" ]; then
	SEARCH_AND_REPLACE .config "CONFIG_ETHER_WAKE" "CONFIG_ETHER_WAKE=y"
fi
if [ $# = 1 ]; then
	case "$1" in
	"clean")
		MAKECLEAR=1
		MAKING=0
	;;
	"make")
		MAKECLEAR=0
		MAKING=1
	;;
	esac
fi
if [ $MAKECLEAR = 1 ]; then
	rm -rf _install
	make clean
	if [ $? != 0 ]; then
		exit 1
	fi
fi
if [ $MAKING = 1 ]; then
	./go
	if [ $? != 0 ]; then
		exit 1
	fi
fi
