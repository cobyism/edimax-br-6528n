<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<link rel="stylesheet" href="edimax.css">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<script type="text/javascript" src="file/multilanguage.var"></script>
<script language ='javascript' src ="file/fwwl-n.var"></script>
<script type="text/javascript" src="file/statustool-n.var"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<script type="text/javascript" src="file/ezview-n.var"></script>
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="FUNCTION_SCRIPT"></script>
<script type="text/javascript">
IPv6WANMode=<% getInfo("IPv6WANMode"); %>
IPv6LANMode=<% getInfo("IPv6LANMode"); %>
IPv6DNSMode=<% getInfo("IPv6DNSMode"); %>
<% write("pppConnectType="+getIndex("pppConnectType")); %>
<% write("pppConnectStatus="+getIndex("pppConnectStatus")); %>
function wanreload(x) {
document.getElementById("WANIPv6Address").style.display = "none"
document.getElementById("WANIPv6DSLite").style.display = "none"
document.getElementById("WANIPv6Static").style.display = "none"
document.getElementById("WANIPv6in4").style.display = "none"
document.getElementById("WANIPv66rd").style.display = "none"
document.getElementById("WANIPv6MTU").style.display = "none"
document.getElementById("WANIPv6TTL").style.display = "none"
document.getElementById("WANIPv6PPPoE").style.display = "none"
document.getElementById("WANIPv6TSP").style.display = "none"
document.getElementById("WANIPv6DNSMode").style.display = "none"
document.getElementById("WANIPv6DNS").style.display = "none"
dnsreload(1)
if (x == 1) {
document.getElementById("WANIPv6Address").style.display = "block"
document.getElementById("WANIPv6DNSMode").style.display = "block"
document.getElementById("WANIPv6DNS").style.display = "block"
if (document.formIPv6.IPv6DNSMode[0].checked == true)
dnsreload(0)
}
else if (x == 2) {
document.getElementById("WANIPv6Address").style.display = "block"
document.getElementById("WANIPv6DNSMode").style.display = "block"
document.getElementById("WANIPv6DNS").style.display = "block"
if (document.formIPv6.IPv6DNSMode[0].checked == true)
dnsreload(0)
}
else if (x == 3) {
document.getElementById("WANIPv6Static").style.display = "block"
document.getElementById("WANIPv6MTU").style.display = "block"
document.getElementById("WANIPv6DNS").style.display = "block"
}
else if (x == 4) {
document.getElementById("WANIPv6Address").style.display = "block"
document.getElementById("WANIPv6MTU").style.display = "block"
document.getElementById("WANIPv6TTL").style.display = "block"
document.getElementById("WANIPv6DNS").style.display = "block"
}
else if (x == 5) {
document.getElementById("WANIPv6in4").style.display = "block"
document.getElementById("WANIPv6MTU").style.display = "block"
document.getElementById("WANIPv6TTL").style.display = "block"
document.getElementById("WANIPv6DNS").style.display = "block"
}
else if (x == 6) {
document.getElementById("WANIPv6Address").style.display = "block"
document.getElementById("WANIPv6PPPoE").style.display = "block"
document.getElementById("WANIPv6MTU").style.display = "block"
document.getElementById("WANIPv6TTL").style.display = "block"
document.getElementById("WANIPv6DNS").style.display = "block"
}
else if (x == 7) {
document.getElementById("WANIPv6Address").style.display = "block"
document.getElementById("WANIPv6TSP").style.display = "block"
document.getElementById("WANIPv6DNS").style.display = "block"
}
else if (x == 8) {
document.getElementById("WANIPv6DSLite").style.display = "block"
document.getElementById("WANIPv6Static").style.display = "block"
document.getElementById("WANIPv6MTU").style.display = "block"
document.getElementById("WANIPv6DNS").style.display = "block"
}
else if (x == 9) {
document.getElementById("WANIPv6Address").style.display = "block"
document.getElementById("WANIPv66rd").style.display = "block"
document.getElementById("WANIPv6MTU").style.display = "block"
document.getElementById("WANIPv6TTL").style.display = "block"
document.getElementById("WANIPv6DNS").style.display = "block"
}
else if (x == 10) {
document.getElementById("WANIPv6Address").style.display = "block"
document.getElementById("WANIPv66rd").style.display = "block"
document.getElementById("WANIPv6PPPoE").style.display = "block"
document.getElementById("WANIPv6MTU").style.display = "block"
document.getElementById("WANIPv6TTL").style.display = "block"
document.getElementById("WANIPv6DNS").style.display = "block"
}
}
function lanreload(x) {
document.getElementById("LANIPv6Stateless").style.display = "none"
document.getElementById("LANIPv6DHCP").style.display = "none"
document.getElementById("LANIPv6Lifetime").style.display = "none"
if (x == 1) {
document.getElementById("LANIPv6Stateless").style.display = "block"
document.getElementById("LANIPv6Lifetime").style.display = "block"
}
else if (x == 2) {
document.getElementById("LANIPv6Stateless").style.display = "block"
document.getElementById("LANIPv6DHCP").style.display = "block"
document.getElementById("LANIPv6Lifetime").style.display = "block"
}
}
function dnsreload(x) {
if (x == 1) {
document.formIPv6.IPv6DNS1.disabled = false
document.formIPv6.IPv6DNS2.disabled = false
}
else {
document.formIPv6.IPv6DNS1.disabled = true
document.formIPv6.IPv6DNS2.disabled = true
}
}
function pppreload(x) {
document.formIPv6.pppConnect.disabled = true
document.formIPv6.pppDisconnect.disabled = true
document.formIPv6.pppIdleTime.disabled = true
if (x == 1)
document.formIPv6.pppIdleTime.disabled = false
else if (x == 2) {
if (pppConnectStatus == 1)
document.formIPv6.pppDisconnect.disabled = false
else
document.formIPv6.pppConnect.disabled = false
}
}
function init() {
document.formIPv6.IPv6DNSMode[IPv6DNSMode].checked = true
document.formIPv6.wanipv6.selectedIndex = IPv6WANMode
wanreload(IPv6WANMode)
document.formIPv6.lanipv6.selectedIndex = IPv6LANMode
lanreload(IPv6LANMode)
document.formIPv6.pppConnectType.selectedIndex = pppConnectType
pppreload(pppConnectType)
}
function dhcprange() {
if (document.formIPv6.IPv6LANIP.value.match(/^(?:[a-f0-9]{1,4}:){7}[a-f0-9]{1,4}$/i) && document.formIPv6.IPv6DHCPStart.value.match(/^(?:[a-f0-9]{1,4}:){7}[a-f0-9]{1,4}$/i) && document.formIPv6.IPv6DHCPEnd.value.match(/^(?:[a-f0-9]{1,4}:){7}[a-f0-9]{1,4}$/i)) {
ip = document.formIPv6.IPv6LANIP.value.split(":")
prefix = document.formIPv6.IPv6LANPrefix.value
start = document.formIPv6.IPv6DHCPStart.value.split(":")
end = document.formIPv6.IPv6DHCPEnd.value.split(":")
for ( i = 0; i < prefix / 16; i++ ) {
start[i] = ip[i]
end[i] = ip[i]
}
document.formIPv6.IPv6DHCPStart.value = start[0] + ":" + start[1] + ":" + start[2] + ":" + start[3] + ":" + start[4] + ":" + start[5] + ":" + start[6] + ":" + start[7]
document.formIPv6.IPv6DHCPEnd.value = end[0] + ":" + end[1] + ":" + end[2] + ":" + end[3] + ":" + end[4] + ":" + end[5] + ":" + end[6] + ":" + end[7]
}
}
function pppoe(x) {
if (document.formIPv6.pppUserName.value == "" || document.formIPv6.pppPassword.value == "") return false
document.formWanTcpipSetup.pppUserName.value = document.formIPv6.pppUserName.value
document.formWanTcpipSetup.pppPassword.value = document.formIPv6.pppPassword.value
document.formWanTcpipSetup.pppServName.value = document.formIPv6.pppServName.value
document.formWanTcpipSetup.pppConnectType.value = document.formIPv6.pppConnectType.selectedIndex
document.formWanTcpipSetup.pppIdleTime.value = document.formIPv6.pppIdleTime.value
document.formWanTcpipSetup.submit()
}
var cloud_sta = "<% getInfo("cloud-sta"); %>"
var tcpipwanmac = "<% getInfo("wanMac"); %>"
var macaddrvalue = "<% getInfo("cloneMac"); %>"
var dyiphostname = "<% getInfo("dynIPHostName"); %>"
var valdns1 = "<% getInfo("wan-dns1"); %>"
var valdns2 = "<% getInfo("wan-dns2"); %>"
var valdns3 = "<% getInfo("wan-dns3"); %>"
var internet_sta = "<% getInfo("sta-current"); %>"
var DUAL_WAN_IGMP="<% getInfo("DUAL_WAN_IGMP"); %>"
<% write("wizardEnabled = "+getIndex("wizardEnabled"));%>
<% write("wanmod = "+getIndex("wanMode"));%>
<% write("dnsmode = "+getIndex("wanDNS"));%>
<% write("ttlval = "+getIndex("pppEnTtl"));%>
<% write("dualaccessSW = "+getIndex("isDuallAccessEnable"));%>
<%write("duallAccessType= "+getIndex("duallAccessMode"));%>
/* function for main */
	var comment = new Array( new Array("com00","com01"),
							 new Array("com10","com11"),
							 new Array("com20","com21"),
							 new Array("com30","com31"),
							 new Array("com40","com41"),
							 new Array("com50","com51"))
	function c_fun(num)
	{
		document.getElementById('wan_dip').style.display = "none"
		document.getElementById('wan_sip').style.display = "none"
		document.getElementById('wan_ppp').style.display = "none"
		document.getElementById('wan_pptp').style.display = "none"
		document.getElementById('wan_l2tp').style.display = "none"
		document.getElementById('wan_wisp').style.display = "none"
		document.getElementById('wan_ipv6').style.display = "none"
		if (num == 0)
		{
			document.getElementById('wan_dip').style.display = "block"
			document.getElementsByName("sys")[0].checked = true
			autoDNSClicked(document.dip)
		}
		else if (num == 1)
		{
			document.getElementById('wan_sip').style.display = "block"
			document.getElementsByName("sys")[1].checked = true
		}
		else if (num == 2)
		{
			document.getElementsByName("sys")[2].checked = true
			document.getElementById('wan_ppp').style.display = "block"
			pppTypeSelection();
			autoDNSClicked(document.ppp)
		}
		else if (num == 3)
		{
			document.getElementsByName("sys")[3].checked = true
			document.getElementById('wan_pptp').style.display = "block"
			pptpTypeSelection();
			autoIpClicked();
			autoDNSClicked(document.pptp)
		}
		else if (num == 4)
		{
			document.getElementsByName("sys")[4].checked = true
			document.getElementById('wan_l2tp').style.display = "block"
			L2TPTypeSelection();
			autoIpClicked();
			autoDNSClicked(document.l2tp)
		}
		else if (num == 5)
		{
			document.getElementsByName("sys")[5].checked = true
			document.getElementById('wan_wisp').style.display = "block"
			updateFormat();
			setPskKeyValue();
			displayObj();
		}
		else if (num == 6)
		{
			document.getElementsByName("sys")[6].checked = true
			document.getElementById('wan_ipv6').style.display = "block"
			//updateFormat();
			//setPskKeyValue();
			//displayObj();
		}
	}
	function com_sw(cnum,com)
	{
		if (com == 1)
		{
			document.getElementById(comment[cnum][0]).style.display = "none"
			document.getElementById(comment[cnum][1]).style.display = "block"
		}
		else if ( com == 2)
		{
			document.getElementById(comment[cnum][0]).style.display = "block"
			document.getElementById(comment[cnum][1]).style.display = "none"
		}
	}
/* wan function*/
    function saveChanges()
    {
		if(document.getElementById('wan_dip').style.display == "block")
		{
			if ( document.dip.macAddr.value == "" ) document.dip.macAddr.value="000000000000";
			var str = document.dip.macAddr;
			if ( !macRule(str,showText(wandipAlert2), 1)) return false;
			var str1 = document.dip.dynIPHostName.value;
			if (str1.search('"') != -1 || str1.search("'") != -1)
			{
				alert(showText(wandipAlert3));
				document.dip.dynIPHostName.focus();
				return false;
			}
			document.dip.isApply.value = "ok"
			document.dip.submit();
		}
		else if(document.getElementById('wan_sip').style.display == "block")
		{
			if ( !ipRule( document.sip.ip, showText(IPAddress), "ip", 1)) return false;
			if ( !maskRule( document.sip.mask, showText(SubnetMask), 1)) return false;
			if ( !ipRule( document.sip.gateway, showText(DefaultGateway), "gw", 1)) return false;
			if ( !subnetRule(document.sip.ip, document.sip.mask, document.sip.gateway, showText(DefaultGateway), showText(IPAddress))) return false;
			if ( !ipRule( document.sip.dns1, showText(DNSAddess), "gw", 1)) return false;
			document.sip.isApply.value = "ok"
			document.sip.submit();
		}
		else if(document.getElementById('wan_wisp').style.display == "block")
		{
			switch(document.wispSetup.stadrv_encrypttype.selectedIndex) // Disable / WEP / WPA pre-shared key / WPA RADIUS
			{
				case 0:
					break;
				case 1:
					if(  document.getElementsByName("key"+(document.wispSetup.stadrv_defaultkey.selectedIndex+1))[0].value.length == 0 )
					{
						alert(showText(wlencryptAlertBlank));
						return false;
					}
					switch(document.wispSetup.stadrv_weplength.selectedIndex) // 64-bit / 128-bit
					{
						case 0:
							for(i=1;i<2;i++)
							{
								switch(document.getElementsByName("stadrv_wepformat")[0].selectedIndex) // ASCII / HEX
								{
									case 0:
										if(!validlength(document.getElementsByName("key"+i)[0].value, i, 5)) return false;
										break;
									case 1:
										if(!validlength(document.getElementsByName("key"+i)[0].value, i, 10)) return false;
										if(!isHex(document.getElementsByName("key"+i)[0].value)) return false;
										break;
									default:
										break;
								}
							}
							break;
						case 1:
							for(i=1;i<2;i++)
							{
								switch(document.getElementsByName("stadrv_wepformat")[0].selectedIndex) // ASCII / HEX
								{
									case 0:
										if(!validlength(document.getElementsByName("key"+i)[0].value, i, 13)) return false;
										break;
									case 1:
										if(!validlength(document.getElementsByName("key"+i)[0].value, i, 26)) return false;
										if(!isHex(document.getElementsByName("key"+i)[0].value)) return false;
										break;
									default:
										break;
								}
							}
							break;
						default:
							break;
					}
					break;
				case 2:
					switch(document.getElementsByName("stadrv_pskformat")[0].selectedIndex) // ASCII / HEX
					{
						case 0:
							if(document.getElementsByName("stadrv_pskkey")[0].value.length<8 )
							{
								alert(showText(wlencryptAlertKey8));
								return false;
							}
							break;
						case 1:
							if(!validlength(document.getElementsByName("stadrv_pskkey")[0].value, i, 64)) return false;
							if(!isHex(document.getElementsByName("stadrv_pskkey")[0].value)) return false;
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
			wisp_mode=0
			for(i=0;i<2;i++)
			{
				if( document.getElementsByName("stadrv_type")[i] && document.getElementsByName("stadrv_type")[i].checked==true)
				{
					wisp_mode=i;
					break;
				}
			}
			if (wisp_mode == 1)
			{
				if (document.wispSetup.stadrv_ssid.value=="")
				{
					alert(showText(SomethingCannotBeEmpty).replace(/#####/,showText(SSID)));
					document.wispSetup.stadrv_ssid.focus();
					return false;
				}
			}
			document.wispSetup.submit();
		}
		else if(document.getElementById('wan_ipv6').style.display == "block")
		{
			document.formIPv6.submit();
		}
		else saveChang(3)
        return true;
    }
	var pppConnectStatus=0;
	function setPPPConnected()
    {
        pppConnectStatus = 1;
    }
	function saveChang(connect)
    {
		var str
		if(document.getElementById('wan_ppp').style.display == "block")
		{
			if ( !(document.ppp.pppConnectType.selectedIndex == 2 && pppConnectStatus==connect) && !(connect==3)) return false;
			if (!strRule(document.ppp.pppUserName,showText(UserName))) return false;
			if (!strRule(document.ppp.pppPassword,showText(Password))) return false;
			if (document.ppp.pppServName.value != "" && !document.ppp.pppServName.value.match(/^[A-Za-z0-9_]{1,30}$/))
			{
				alert("Invalid Service Name");
				document.ppp.pppServName.value = document.ppp.pppServName.defaultValue;
				document.ppp.pppServName.focus();
				return false;
			}
			if ( document.ppp.pppConnectType.selectedIndex != 0 && !portRule(document.ppp.pppIdleTime,showText(IdleTimeOut), 0, "", 1, 1000, 1)) return false;
			if (!portRule(document.ppp.pppMTU,showText(MTU), 0, "", 512, 1492, 1)) return false;
			if(document.ppp.enableDuallAccess.checked==true && document.ppp.duallAccessMode[0].checked==true)
			{
				str = document.ppp.macAddr2;
				document.ppp.macAddr.value = " ";
			}
			else
			{
				str = document.ppp.macAddr;
				document.ppp.macAddr2.value = " ";
			}
			if ( str.value == "" ) str.value="000000000000";
			if ( !macRule(str,showText(wandipAlert2), 1)) return false;
			document.ppp.isApply.value = "ok"
			document.ppp.submit();
        }
		else if(document.getElementById('wan_pptp').style.display == "block")
		{
			strIp = document.pptp.pptpIPAddr;
			strMask = document.pptp.pptpIPMaskAddr;
			strDefGateway = document.pptp.pptpDfGateway;
			strPPTPGateway = document.pptp.pptpGateway;
			if ( !(document.pptp.pptpConnectType.selectedIndex == 2 && pppConnectStatus==connect) && !(connect==3)) return false;
			if (document.pptp.pptpIpMode[1].checked==true)
			{
				str =document.pptp.macAddr1;
				document.pptp.macAddr.value = "";
				if ( str.value == "" ) str.value="000000000000";
				if ( !ipRule( strIp, showText(IPAddress), "ip", 1) ||
					 !maskRule(strMask, showText(SubnetMask), 1) ||
					 !ipRule( strDefGateway,showText(DefaultGateway), "gw", 1) ||
					 !macRule(str,showText(wandipAlert2), 1))
					return false;
			}
			else
			{
				str =document.pptp.macAddr;
				document.pptp.macAddr1.value = "";
				if ( str.value == "" ) str.value="000000000000";
				if ( !macRule(str,showText(wandipAlert2), 1)) return false;
        		if (document.pptp.HostName.value != "" && !document.pptp.HostName.value.match(/^[A-Za-z0-9_]{1,30}$/))
				{
					alert("Invalid Host Name");
					document.pptp.HostName.value = document.pptp.HostName.defaultValue;
					document.pptp.HostName.focus();
					return false;
				}
			}
			if (!strRule(document.pptp.pptpUserName,showText(UserName)) || !strRule(document.pptp.pptpPassword,showText(Password)) ) return false;
			if ( !_PPTP_FQDN_ )
			{
				if ( !ipRule( strPPTPGateway,showText(pptpGateAddr), "gw", 1)) return false;
				if ( document.pptp.pptpIpMode[1].checked==true)
					if ( !checkSubnet( strIp.value, strMask.value, strPPTPGateway.value) )
					{
						if ( !checkSubnet( strIp.value, strMask.value, strDefGateway.value))
						{
							alert(showText(InvalidSomething).replace(/#####/,showText(DefaultGateway))+' '+showText(SomethingAndSomethingShouldInSameSubnet).replace(/#####/,showText(DefaultGateway)).replace(/####@/,showText(IPAddress))      );
							setFocus(strDefGateway);
							return false;
						}
					}
					else
					{
						if (strDefGateway.value!="" && strDefGateway.value!="0.0.0.0")
						{
							alert(showText(wanpptpAlert2));
							setFocus(strDefGateway);
							return false;
						}
					}
			}
			else
			{
				if (!strRule(strPPTPGateway,showText(pptpGate)))
					return false
			}
			if (document.pptp.pptpConntID.value != "" && !document.pptp.pptpConntID.value.match(/^[A-Za-z0-9_]{1,30}$/))
			{
				alert("Invalid Connection ID");
				document.pptp.pptpConntID.value = document.pptp.pptpConntID.defaultValue;
				document.pptp.pptpConntID.focus();
				return false;
			}
			if (document.pptp.pptpConnectType.value==1 && !portRule(document.pptp.pptpIdleTime,showText(IdleTimeOut), 0, "", 1, 1000, 1)) return false;
			if (!portRule(document.pptp.pptpMTU,showText(MTU), 0, "", 512, 1492, 1)) return false;
			var str2 = document.pptp.HostName.value;
			if (str2.search('"') != -1 || str2.search("'") != -1)
			{
				alert(showText(wandipAlert3));
				document.pptp.HostName.focus();
				return false;
			}
			document.pptp.isApply.value = "ok"
			document.pptp.submit();
		}
		else if(document.getElementById('wan_l2tp').style.display == "block")
		{
			strIp = document.l2tp.L2TPIPAddr;
			strMask = document.l2tp.L2TPMaskAddr;
			strDefGateway = document.l2tp.L2TPDefGateway;
			strL2TPGateway = document.l2tp.L2TPGateway;
			if ( !(document.l2tp.L2TPConnectType.selectedIndex == 2 && pppConnectStatus==connect) && !(connect==3)) return false;
			if ( document.l2tp.macAddr.value == "" ) document.l2tp.macAddr.value="000000000000";
			if (document.l2tp.L2TPIpMode[1].checked==true)
			{
				if ( !ipRule( strIp, showText(IPAddress), "ip", 1)
					|| !maskRule(strMask, showText(SubnetMask), 1)
					|| !ipRule( strDefGateway, showText(DefaultGateway), "gw", 1)) return false;
			}
			else
			{
				var str =document.l2tp.macAddr.value;
				if ( str.length < 12)
				{
					alert(showText(InvalidSomething).replace(/#####/,showText(MACAddress))+' '+showText(SomethingShouldconsist12Hex).replace(/#####/,showText(MACAddress))   );
					document.l2tp.macAddr.focus();
					return false;
				}
				for (var i=0; i<str.length; i++)
				{
					if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) >= 'a' && str.charAt(i) <= 'f') || (str.charAt(i) >= 'A' && str.charAt(i) <= 'F') ) continue;
					alert(showText(wandipAlert2));
					document.l2tp.macAddr.focus();
					return false;
				}
				if ( document.l2tp.HostName.value != "" && !document.l2tp.HostName.value.match(/^[A-Za-z0-9_]{1,30}$/))
				{
					alert("Invalid Host Name");
					document.l2tp.HostName.value = document.l2tp.HostName.defaultValue;
					document.l2tp.HostName.focus();
					return false;
				}
			}
			if (!strRule(document.l2tp.L2TPUserName, showText(UserName)) ||
				!strRule(document.l2tp.L2TPPassword, showText(Password)) ||
				!strRule(strL2TPGateway,showText(L2TPGateway)) ||
				!portRule(document.l2tp.L2TPMTU, showText(MTU), 0, "", 512, 1492, 1)) return false;
			if ( document.l2tp.L2TPConnectType.selectedIndex == 1 && !portRule(document.l2tp.L2TPIdleTime, showText(IdleTimeOut), 0, "", 1, 1000, 1)) return false;
			var str3 = document.l2tp.HostName.value;
			if (str3.search('"') != -1 || str3.search("'") != -1)
			{
				alert(showText(wandipAlert3));
				document.dip.dynIPHostName.focus();
				return false;
			}
			document.l2tp.isApply.value = "ok"
			document.l2tp.submit();
		}
		return true;
    }
	function pppTypeSelection()
	{
        if ( document.ppp.pppConnectType.selectedIndex == 2)
		{
			if (pppConnectStatus==0)
			{
				document.ppp.pppConnect.disabled=false;
				document.ppp.pppDisconnect.disabled = true;
			}
			else
			{
				document.ppp.pppConnect.disabled = true;
				document.ppp.pppDisconnect.disabled=false;
			}
			document.ppp.pppIdleTime.disabled = true;
        }
		else
		{
			document.ppp.pppConnect.disabled = true;
			document.ppp.pppDisconnect.disabled = true;
			if (document.ppp.pppConnectType.selectedIndex == 1) document.ppp.pppIdleTime.disabled=false;
			else document.ppp.pppIdleTime.disabled=true;
        }
    }
	function pptpTypeSelection()
	{
        if ( document.pptp.pptpConnectType.selectedIndex == 2)
		{
			if (pppConnectStatus==0)
			{
				document.pptp.pptpConnect.disabled=false;
				document.pptp.pptpDisconnect.disabled = true;
			}
			else
			{
				document.pptp.pptpConnect.disabled = true;
				document.pptp.pptpDisconnect.disabled=false;
			}
			document.pptp.pptpIdleTime.disabled = true;
        }
		else
		{
			document.pptp.pptpConnect.disabled = true;
			document.pptp.pptpDisconnect.disabled = true;
			if (document.pptp.pptpConnectType.selectedIndex == 1) document.pptp.pptpIdleTime.disabled=false;
			else document.pptp.pptpIdleTime.disabled = true;
        }
    }
	function L2TPTypeSelection()
    {
        if ( document.l2tp.L2TPConnectType.selectedIndex == 2)
		{
			if (pppConnectStatus==0)
			{
				document.l2tp.L2TPConnect.disabled=false;
				document.l2tp.L2TPDisconnect.disabled=true;
			}
			else
			{
				document.l2tp.L2TPConnect.disabled=true;
				document.l2tp.L2TPDisconnect.disabled=false;
			}
			document.l2tp.L2TPIdleTime.disabled=true;
        }
        else
		{
			document.l2tp.L2TPConnect.disabled=true;
			document.l2tp.L2TPDisconnect.disabled=true;
			if (document.l2tp.L2TPConnectType.selectedIndex == 1) document.l2tp.L2TPIdleTime.disabled=false;
			else document.l2tp.L2TPIdleTime.disabled=true;
        }
    }
	function autoIpClicked()
	{
		if(document.getElementById('wan_pptp').style.display == "block")
		{
			if(document.pptp.pptpIpMode[0].checked==true)
			{
				document.pptp.pptpIPAddr.disabled=true;
				document.pptp.pptpIPMaskAddr.disabled=true;
				document.pptp.pptpDfGateway.disabled = true;
				document.pptp.macAddr1.disabled = true;
				document.pptp.Clone1.disabled = true;
				document.pptp.HostName.disabled = false;
				document.pptp.macAddr.disabled = false;
				document.pptp.Clone.disabled = false;
			}
			else
			{
				document.pptp.pptpIPAddr.disabled=false;
				document.pptp.pptpIPMaskAddr.disabled=false;
				document.pptp.pptpDfGateway.disabled = false;
				document.pptp.macAddr1.disabled = false;
				document.pptp.Clone1.disabled = false;
				document.pptp.HostName.disabled = true;
				document.pptp.macAddr.disabled = true;
				document.pptp.Clone.disabled = true;
				document.pptp.DNSMode[0].checked = false;
				document.pptp.DNSMode[1].checked = true
				autoDNSClicked(document.pptp)
			}
		}
		else if(document.getElementById('wan_l2tp').style.display == "block")
		{
			if(document.l2tp.L2TPIpMode[0].checked==true)
			{
				document.l2tp.L2TPIPAddr.disabled = true;
				document.l2tp.L2TPMaskAddr.disabled = true;
				document.l2tp.L2TPDefGateway.disabled = true;
				document.l2tp.HostName.disabled = false;
				document.l2tp.macAddr.disabled = false;
				document.l2tp.Clone.disabled = false;
			}
			else
			{
				document.l2tp.L2TPIPAddr.disabled = false;
				document.l2tp.L2TPMaskAddr.disabled = false;
				document.l2tp.L2TPDefGateway.disabled = false;
				document.l2tp.HostName.disabled = true;
				document.l2tp.macAddr.disabled = true;
				document.l2tp.Clone.disabled = true;
				document.l2tp.DNSMode[0].checked = false;
				document.l2tp.DNSMode[1].checked = true
				autoDNSClicked(document.l2tp)
			}
		}
    }
	<!-------------------- showSiteSurvey() -------------------->
	function showSiteSurvey()
	{
		MyRef=window.open('wlsurvey.asp','SiteSurvey','channelmode=0, directories=0,fullscreen=0,height=400,location=0,menubar=0,resizable=1,scrollbars=1,status=0,titlebar=0,toolbar=0,width=850','false');
		MyRef.focus();
	}
	<!-------------------- set_wep_key_max_length(item, length) -------------------->
	function set_wep_key_max_length(item, length)
	{
		item.maxLength=length;
		item.value=item.value.substr(0,length);
	}
	function radiovalue()
	{
		if( typeof _WISP_WITH_STA_ != 'undefined' )
		{
			if ( document.getElementsByName("stadrv_type")[0].checked )
				document.getElementById("WISP").style.display = "none";
			else
				document.getElementById("WISP").style.display = "block";
			if ( document.getElementsByName("stadrv_type")[2].checked )
			{
				document.getElementById("clone").style.display = "block";
				document.getElementById("channel").style.display = "none";
			}
			else
			{
				document.getElementById("clone").style.display = "none";
				document.getElementById("channel").style.display = "block";
			}
		}
	}
	function lengthClick()
	{
		updateFormat();
	}
	<!-------------------- setPskKeyFormat() -------------------->
	function setPskKeyFormat()
	{
		if(document.wispSetup.stadrv_pskformat.selectedIndex==0)
		{
			document.wispSetup.stadrv_pskkey.value=document.wispSetup.stadrv_pskkey.value.substr(0,63);
			document.wispSetup.stadrv_pskkey.maxLength=63;
		}
		else if(document.wispSetup.stadrv_pskformat.selectedIndex==1)
		{
			document.wispSetup.stadrv_pskkey.value=document.wispSetup.stadrv_pskkey.value.substr(0,64);
			document.wispSetup.stadrv_pskkey.maxLength=64;
		}
	}
	<!-------------------- displayObj() -------------------->
	function displayObj()
	{
		i=document.wispSetup.stadrv_encrypttype.value;
		switch(i)
		{
			case "0":
				document.getElementById("WEP").style.display = "none";
				document.getElementById("WEPandWPA").style.display = "none";
				document.getElementById("WPA").style.display = "none";
				break;
			case "1":
				document.getElementById("WEP").style.display = "block";
				document.getElementById("WEPandWPA").style.display = "none";
				document.getElementById("WPA").style.display = "none";
				break;
			case "2":
				document.getElementById("WEP").style.display = "none";
				document.getElementById("WEPandWPA").style.display = "block";
				document.getElementById("WPA").style.display = "block";
				break;
			default: break;
		}
		lengthClick();
	}
	<!-------------------- isHex(item) -------------------->
	function isHex(item)
	{
		var str = new Array("","","","");
		var i=0;
		pattern_star_5 = new RegExp("^[*]\{5\}$");
		pattern_hex_5 = new RegExp("^[0-9a-fA-F]\{5\}$");
		pattern_star_10 = new RegExp("^[*]\{10\}$");
		pattern_hex_10 = new RegExp("^[0-9a-fA-F]\{10\}$");
		pattern_star_13 = new RegExp("^[*]\{13\}$");
		pattern_hex_13 = new RegExp("^[0-9a-fA-F]\{13\}$");
		pattern_star_26 = new RegExp("^[*]\{26\}$");
		pattern_hex_26 = new RegExp("^[0-9a-fA-F]\{26\}$");
		for (a=1; a<2; a++)
		{
			str[a-1]=document.getElementsByName("key"+a)[0].value;
			if(
				!pattern_star_5.test(str[a-1]) &&
				!pattern_hex_5.test(str[a-1]) &&
				!pattern_star_10.test(str[a-1]) &&
				!pattern_hex_10.test(str[a-1]) &&
				!pattern_star_13.test(str[a-1]) &&
				!pattern_hex_13.test(str[a-1]) &&
				!pattern_star_26.test(str[a-1]) &&
				!pattern_hex_26.test(str[a-1])
				)
				{
				alert(showText(wlencryptAlertInvVal));
				return false;
				}
		}
		return true;
	}
	<!-------------------- validlength(item, length) -------------------->
	function validlength(item, number, length)
	{
		if(item.length != length && item.length != 0)
		{
			alert(showText(wlencryptStrInvKey) + number + showText(wlencryptStrShouldBe) + length + showText(wlencryptStrChara));
			return false;
		}
		return true;
	}
	function wlValidateKey(idx, str, len)
	{
		if ( str.length==0)
		{
			alert(showText(DefaultKeyCannotBeBlank));
			return 0;
		}
		if(vaildKeyFormat(str)==0) return 0;
		if (str.length ==0) return 1;
		if ( str.length != len)
		{
			idx++;
			alert( showText(InvalidSomething).replace(/#####/, showText(Key)+' '+idx)+' '+showText(LengthOfSomethingShouldBeHowmany).replace(/#####/, showText(Key)+' '+idx).replace(/####@/,len) );
			return 0;
		}
        if ( str == "*****" ||
             str == "**********" ||
             str == "*************" ||
             str == "****************" ||
             str == "**************************" ||
             str == "********************************" ) return 1;
        if (document.wispSetup.stadrv_wepformat.selectedIndex==0) return 1;
		for (var i=0; i<str.length; i++)
		{
		    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
					(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
					(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') || str.charAt(i) == '*') continue;
			alert(showText(InvalidSomething).replace(/#####/, showText(Key)+' '+(idx+1))+' '+showText(SomethingConsistOnlyHexadecimal).replace(/#####/, showText(Key)+' '+(idx+1)));
			return 0;
		}
        return 1;
    }
    function updateFormat()
    {
        if (document.wispSetup.stadrv_weplength.selectedIndex == 0)
        {
			document.wispSetup.stadrv_wepformat.options[0].text = 'ASCII (5 ' + showText(Characters) +')';
			document.wispSetup.stadrv_wepformat.options[1].text = 'Hex (10 ' + showText(Characters) +')';
        }
        else if (document.wispSetup.stadrv_weplength.selectedIndex == 1)
        {
			document.wispSetup.stadrv_wepformat.options[0].text = 'ASCII (13 ' + showText(Characters) +')';
			document.wispSetup.stadrv_wepformat.options[1].text = 'Hex (26 ' + showText(Characters) +')';
        }
        setDefaultKeyValue();
    }
    function setPskKeyValue()
    {
        if (document.wispSetup.stadrv_pskformat.selectedIndex == 0)
		{
			 document.wispSetup.stadrv_pskkey.maxLength = 63;
			 document.wispSetup.stadrv_pskkey.value=document.wispSetup.stadrv_pskkey.value.substr(0,63);
		}
        else document.wispSetup.stadrv_pskkey.maxLength = 64;
    }
    function setDefaultKeyValue()
    {
        if (document.wispSetup.stadrv_weplength.selectedIndex == 0)
        {
			if ( document.wispSetup.stadrv_wepformat.selectedIndex == 0)
			{
				document.wispSetup.key1.maxLength = 5;
				document.wispSetup.key1.value = "*****";
			}
			else
			{
				document.wispSetup.key1.maxLength = 10;
				document.wispSetup.key1.value = "**********";
			}
        }
        else if (document.wispSetup.stadrv_weplength.selectedIndex == 1)
        {
			if ( document.wispSetup.stadrv_wepformat.selectedIndex == 0)
			{
				document.wispSetup.key1.maxLength = 13;
				document.wispSetup.key1.value = "*************";
			}
			else
			{
				document.wispSetup.key1.maxLength = 26;
				document.wispSetup.key1.value = "**************************";
			}
        }
    }
	function wisp_sw()
	{
		if (document.wispSetup.stadrv_type[0].checked==true)
		{
			document.wispSetup.stadrv_ssid.disabled=true;
			document.wispSetup.stadrv_chan.disabled=true;
			document.wispSetup.selSurvey.disabled=true;
			document.wispSetup.stadrv_encrypttype.disabled=true;
		}
		else
		{
			document.wispSetup.stadrv_ssid.disabled=false;
			document.wispSetup.stadrv_chan.disabled=false;
			document.wispSetup.selSurvey.disabled=false;
			document.wispSetup.stadrv_encrypttype.disabled=false;
		}
	}
	function displayUpdate()
	{
		if (document.ppp.duallAccessMode[0].checked==true){
			document.getElementById('dynamicBtId').style.display="block";
			document.getElementById('staticBtId').style.display="none";
		}
		else if (document.ppp.duallAccessMode[1].checked==true){
			document.getElementById('dynamicBtId').style.display="none";
			document.getElementById('staticBtId').style.display="block";
		}
		if(document.ppp.enableDuallAccess.checked==true)
			document.getElementById('DualAccessEnableBtId').style.display="block";
		else
			document.getElementById('DualAccessEnableBtId').style.display="none";
		if(document.ppp.enableDuallAccess.checked==true)
		{
			if (document.ppp.duallAccessMode[0].checked==true)
			{
				document.ppp.macAddr.disabled=true
				document.ppp.Clone.disabled=true
			}
			else
			{
				document.ppp.macAddr.disabled=false
				document.ppp.Clone.disabled=false
			}
		}
		else
		{
			document.ppp.macAddr.disabled=false
			document.ppp.Clone.disabled=false
		}
	}
	function autoDNSClicked(dnsmodesw)
	{
		if(dnsmodesw.DNSMode[0].checked == true)
		{
			if( ( document.getElementsByName("sys")[4].checked == true && document.l2tp.L2TPIpMode[1].checked==true ) || ( document.pptp.pptpIpMode[1].checked==true && document.getElementsByName("sys")[3].checked == true ) )
			{
				alert("The dns mode is available only with manual input !");
				dnsmodesw.DNSMode[0].checked = false;
				dnsmodesw.DNSMode[1].checked = true
				return false;
			}
			dnsmodesw.dns1.disabled = true;
			dnsmodesw.dns2.disabled = true;
			dnsmodesw.dns3.disabled = true;
		}
		else
		{
			dnsmodesw.dns1.disabled = false;
			dnsmodesw.dns2.disabled = false;
			dnsmodesw.dns3.disabled = false;
		}
	}
	function copyto(dom)
    {
		dom.value = macaddrvalue
    }
</script>
<title></title>
</head>
<body onLoad="top.startclock()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>
		if(wizardEnabled == 1)
			Write_Header(1,0)
		else
			Write_Header(2,1)
	</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF"><br />
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666">
									<script>
										if( wanmod == 0) document.write('<input onclick="c_fun(0)" type="radio" name="sys" checked>')
										else document.write('<input onclick="c_fun(0)" type="radio" name="sys">')
									</script>
									<span class="style13" style="cursor:pointer;" onclick="c_fun(0);">&nbsp;<script>dw(DynamicIP)</script></span>
								</td>
							</tr>
							<tr>
								<td align="top" align="center">
									<div id="wan_dip" style="display:block;"><br>
									<form action=/goform/formWanTcpipSetup method=POST name="dip">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(hostName)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											document.write('<input type="text" name="dynIPHostName" value="'+dyiphostname+'" size="20" maxlength="30" class="text">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(macAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<script>
												document.write('<input type="text" name="macAddr" value="'+tcpipwanmac+'" size="20" maxlength="12">&nbsp;&nbsp;')
												document.write('<input type="button" value="'+showText(clone)+'" name="Clone" onClick="copyto(document.dip.macAddr);" class="btnsize">')
											</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<script>
												if (dnsmode)
												{
													document.write('<input type="radio" value="0" name="DNSMode" onClick="autoDNSClicked(document.dip);">'+showText(obtainIP))
													document.write('<br>')
													document.write('<input type="radio" value="1" name="DNSMode" onClick="autoDNSClicked(document.dip);" checked >'+showText(useIPAddr))
												}
												else
												{
													document.write('<input type="radio" value="0" name="DNSMode" onClick="autoDNSClicked(document.dip);" checked >'+showText(obtainIP))
													document.write('<br>')
													document.write('<input type="radio" value="1" name="DNSMode" onClick="autoDNSClicked(document.dip);">'+showText(useIPAddr))
												}
											</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr1)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns1)	document.write('<input type="text" name="dns1" size="15" maxlength="15" value="'+valdns1+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns1" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr2)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns2)	document.write('<input type="text" name="dns2" size="15" maxlength="15" value="'+valdns2+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns2" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr3)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns3)	document.write('<input type="text" name="dns3" size="15" maxlength="15" value="'+valdns3+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns3" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14">TTL :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(!ttlval)
											{
												document.write('<input type="radio" value="0" name="pppEnTtl" checked>'+showText(disable))
												document.write('<input type="radio" value="1" name="pppEnTtl">'+showText(enable))
											}
											else
											{
												document.write('<input type="radio" value="0" name="pppEnTtl">'+showText(disable))
												document.write('<input type="radio" value="1" name="pppEnTtl" checked>'+showText(enable))
											}
										</script>
										</td>
									</tr>
									</table>
									<input type=hidden value="/wan.asp" name="submit-url" id="submitUrl">
									<input type=hidden value="0" name="wanMode">
									<input type=hidden value="" name="isApply">
									</form>
									</div>
								</td>
							</tr>
						</table>
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666">
									<script>
										if( wanmod == 1) document.write('<input onclick="c_fun(1)" type="radio" name="sys" checked>')
										else document.write('<input onclick="c_fun(1)" type="radio" name="sys">')
									</script>
									<span class="style13"style="cursor:pointer;" onclick="c_fun(1);">&nbsp;<script>dw(StaticIP)</script></span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<div id="wan_sip" style="display:none;"><br>
									<form action=/goform/formWanTcpipSetup method=POST name="sip">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(macAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<script>
												document.write('<input type="text" name="macAddr" value="'+tcpipwanmac+'" size="20" maxlength="12">&nbsp;&nbsp;')
												document.write('<input type="button" value="'+showText(clone)+'" name="Clone" onClick="copyto(document.sip.macAddr);" class="btnsize">')
											</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(StaticIPAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="ip" size="15" maxlength="15" value=<% getInfo("wan-ip-rom"); %>>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(subnetMask)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="mask" size="15" maxlength="15" value="<% getInfo("wan-mask-rom"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr1)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns1)	document.write('<input type="text" name="dns1" size="15" maxlength="15" value="'+valdns1+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns1" size="15" maxlength="15" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr2)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns2)	document.write('<input type="text" name="dns2" size="15" maxlength="15" value="'+valdns2+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns2" size="15" maxlength="15" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr3)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns3)	document.write('<input type="text" name="dns3" size="15" maxlength="15" value="'+valdns3+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns3" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(DefaultGateway)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="gateway" size="15" maxlength="15" value="<% getInfo("wan-gateway-rom"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14">TTL :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(!ttlval)
											{
												document.write('<input type="radio" value="0" name="pppEnTtl" checked>'+showText(disable))
												document.write('<input type="radio" value="1" name="pppEnTtl">'+showText(enable))
											}
											else
											{
												document.write('<input type="radio" value="0" name="pppEnTtl">'+showText(disable))
												document.write('<input type="radio" value="1" name="pppEnTtl" checked>'+showText(enable))
											}
										</script>
										</td>
									</tr>
									<input type=hidden value="/wan.asp" name="submit-url" id="submitUrl">
									<input type=hidden value="1" name="wanMode" >
									<input type=hidden value="1" name="DNSMode">
									<input type=hidden value="fixedIp" name="ipMode" >
									<input type=hidden value="" name="isApply">
									</table>
									</form>
									</div>
								</td>
							</tr>
						</table>
						<br />
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666">
									<script>
										if( wanmod == 2) document.write('<input onclick="c_fun(2)" type="radio" name="sys" checked>')
										else document.write('<input onclick="c_fun(2)" type="radio" name="sys">')
									</script>
									<span class="style13"style="cursor:pointer;" onclick="c_fun(2);">&nbsp;<script>dw(PPPoE)</script></span>
								</td>
							</tr>
						</table>
						<table width="600" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td valign="top" align="center">
									<div id="wan_ppp" style="display:none;"><br>
									<form action=/goform/formWanTcpipSetup method=POST name="ppp">
									<input type="hidden" name="redirect" size="20" value="">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(userName)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="pppUserName" size="20" maxlength="64" value="<% getInfo("pppUserName"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(password)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="password" name="pppPassword" size="20" maxlength="64" value="<% getInfo("pppPassword"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(macAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<script>
												document.write('<input type="text" name="macAddr" value="'+tcpipwanmac+'" size="20" maxlength="12">&nbsp;&nbsp;')
												document.write('<input type="button" value="'+showText(clone)+'" name="Clone" onClick="copyto(document.ppp.macAddr);" class="btnsize">')
											</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<script>
												if (dnsmode)
												{
													document.write('<input type="radio" value="0" name="DNSMode" onClick="autoDNSClicked(document.ppp);">'+showText(obtainIP))
													document.write('<br>')
													document.write('<input type="radio" value="1" name="DNSMode" onClick="autoDNSClicked(document.ppp);" checked >'+showText(useIPAddr))
												}
												else
												{
													document.write('<input type="radio" value="0" name="DNSMode" onClick="autoDNSClicked(document.ppp);" checked >'+showText(obtainIP))
													document.write('<br>')
													document.write('<input type="radio" value="1" name="DNSMode" onClick="autoDNSClicked(document.ppp);">'+showText(useIPAddr))
												}
											</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr1)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns1)	document.write('<input type="text" name="dns1" size="15" maxlength="15" value="'+valdns1+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns1" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr2)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns2)	document.write('<input type="text" name="dns2" size="15" maxlength="15" value="'+valdns2+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns2" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr3)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns3)	document.write('<input type="text" name="dns3" size="15" maxlength="15" value="'+valdns3+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns3" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14">TTL :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(!ttlval)
											{
												document.write('<input type="radio" value="0" name="pppEnTtl" checked>'+showText(disable))
												document.write('<input type="radio" value="1" name="pppEnTtl">'+showText(enable))
											}
											else
											{
												document.write('<input type="radio" value="0" name="pppEnTtl">'+showText(disable))
												document.write('<input type="radio" value="1" name="pppEnTtl" checked>'+showText(enable))
											}
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(serviceName)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="pppServName" size="20" maxlength="30" value=<% getInfo("pppServName"); %>>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(mtu)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="pppMTU" size="10" maxlength="4" value=<% getInfo("pppMTU"); %>><font size=2>&nbsp;(512&lt;=MTU Value&lt;=1492)
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(connectType)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<select size="1" name="pppConnectType" onChange="pppTypeSelection();">
										<% var type = getIndex("pppConnectType");
											if ( type == 0 ) {
												write( "<option selected value=\"0\"><script>dw(continuous)</script></option>" );
												write( "<option value=\"1\"><script>dw(connectDemand)</script></option>" );
												write( "<option value=\"2\"><script>dw(manual)</script></option>" );
											}
											if ( type == 1 ) {
												write( "<option value=\"0\"><script>dw(continuous)</script></option>" );
												write( "<option selected value=\"1\"><script>dw(connectDemand)</script></option>" );
												write( "<option value=\"2\"><script>dw(manual)</script></option>" );
											}
											if ( type == 2 ) {
												write( "<option value=\"0\"><script>dw(continuous)</script></option>" );
												write( "<option value=\"1\"><script>dw(connectDemand)</script></option>" );
												write( "<option selected value=\"2\"><script>dw(manual)</script></option>" );
											}
										%>
										</select>&nbsp;
										<script>document.write('<input type="submit" value="'+showText(connect)+'" name="pppConnect" onClick="return saveChang(0)" class="btnsize">')</script>&nbsp;
										<script>document.write('<input type="submit" value="'+showText(disconnect)+'" name="pppDisconnect" onClick="return saveChang(1)" class="btnsize">')</script>
										<% if ( getIndex("pppConnectStatus") ) write("\n<script> setPPPConnected(); </script>\n"); %>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(idleTime)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="pppIdleTime" size="10" maxlength="4" value="<% getInfo("wan-ppp-idle"); %>">&nbsp;(1-1000 <script>dw(minute)</script>)
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14">
										<script>
											if(dualaccessSW) document.write('<input type="checkbox" name="enableDuallAccess" value="ON" onclick=displayUpdate(); checked>&nbsp;Enable Dual Wan Access :</br>')
											else  document.write('<input type="checkbox" name="enableDuallAccess" value="ON" onclick=displayUpdate();>&nbsp;Enable Dual Wan Access :</br>')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14">IGMP Source :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="radio" name="DUAL_WAN_IGMP" value="0" />ETH&nbsp;&nbsp;<input type="radio" name="DUAL_WAN_IGMP" value="1" />PPP
										<script>
											if (DUAL_WAN_IGMP == 0)
												document.ppp.DUAL_WAN_IGMP[0].checked = true
											else
												document.ppp.DUAL_WAN_IGMP[1].checked = true
										</script>
										</td>
									</tr>
									</table>
									<div id="DualAccessEnableBtId" style="display:block" >
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
										<td width="190" height="20" valign="middle" align="right" class="style14">
										<script>
											if(!duallAccessType) document.write('<input type="radio" name="duallAccessMode" value="0" onClick="displayUpdate();" checked><font size="2">Dynamic IP</font>')
											else document.write('<input type="radio" name="duallAccessMode" value="0" onClick="displayUpdate();"><font size="2">Dynamic IP</font>')
										</script>
										</td>
										<td width="410" height="20" valign="middle" align="left" class="style14">
										<script>
											if(duallAccessType) document.write('<input type="radio" name="duallAccessMode" value="1" onClick="displayUpdate();" checked><font size="2">Static IP</font>')
											else document.write('<input type="radio" name="duallAccessMode" value="1" onClick="displayUpdate();"><font size="2">Static IP</font>')
										</script>
										</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="dynamicBtId" style="display:none" >
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(hostName)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											document.write('<input type="text" name="dynIPHostName" value="'+dyiphostname+'" size="20" maxlength="30" class="text">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(macAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<script>
												document.write('<input type="text" name="macAddr2" value="'+tcpipwanmac+'" size="20" maxlength="12">&nbsp;&nbsp;')
												document.write('<input type="button" value="'+showText(clone)+'" name="Clone2" onClick="copyto(document.ppp.macAddr2);" class="btnsize">')
											</script>
										</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="staticBtId" style="display:none" >
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(StaticIPAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="ip" size="15" maxlength="15" value=<% getInfo("wan-ip-rom"); %>>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(subnetMask)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="mask" size="15" maxlength="15" value="<% getInfo("wan-mask-rom"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(gateAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="gateway" size="15" maxlength="15" value="<% getInfo("wan-gateway-rom"); %>">
										</td>
									</tr>
									</table>
									</div>
									<input type=hidden value="/wan.asp" name="submit-url" id="submitUrl">
									<input type=hidden value="2" name="wanMode">
									<input type=hidden value="ppp" name="ipMode" >
									<input type=hidden value="" name="isApply">
									<input type=hidden value="" name="pppmacaddr">
									</form>
									</div>
								</td>
							</tr>
						</table>
						<br />
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666">
									<script>
										if( wanmod == 3) document.write('<input onclick="c_fun(3)" type="radio" name="sys" checked>')
										else document.write('<input onclick="c_fun(3)" type="radio" name="sys">')
									</script>
									<span class="style13"style="cursor:pointer;" onclick="c_fun(3);"><script>dw(PPTP)</script></span>
								</td>
							</tr>
							<script>
								<% write("pptpMode = "+getIndex("pptpIpMode"));%>
							</script>
							<tr>
								<td valign="top" align="center">
									<div id="wan_pptp" style="display:none;"><br>
									<form action=/goform/formPPTPSetup method=POST name="pptp">
									<input type="hidden" name="redirect" size="20" value="">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
										<td colspan="2" align="left" height="25px" class="style14">
										<script>
											if(!pptpMode)document.write('<input type="radio" value="0" name="pptpIpMode" onClick="autoIpClicked()" checked>&nbsp;'+showText(obtainIP)+' :</br>')
											else document.write('<input type="radio" value="0" name="pptpIpMode" onClick="autoIpClicked()">&nbsp;'+showText(obtainIP)+' :</br>')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(hostName)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											document.write('<input type="text" name="HostName" value="'+dyiphostname+'" size="20" maxlength="30" class="text">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(macAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<script>
												document.write('<input type="text" name="macAddr" value="'+tcpipwanmac+'" size="20" maxlength="12">&nbsp;&nbsp;')
												document.write('<input type="button" value="'+showText(clone)+'" name="Clone" onClick="copyto(document.pptp.macAddr);" class="btnsize">')
											</script>
										</td>
									</tr>
									<tr>
										<td colspan="2" align="left" height="25px" class="style14">
										<script>
											if(pptpMode)document.write('<input type="radio" value="1" name="pptpIpMode" onClick="autoIpClicked()" checked>&nbsp;'+showText(useIPAddr)+' :</br>')
											else document.write('<input type="radio" value="1" name="pptpIpMode" onClick="autoIpClicked()">&nbsp;'+showText(useIPAddr)+' :</br>')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(IPAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="pptpIPAddr" size="15" maxlength="15" value="<% getInfo("pptpIPAddr"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(subnetMask)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="pptpIPMaskAddr" size="15" maxlength="15" value="<% getInfo("pptpIPMaskAddr"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(defaultGateway)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="pptpDfGateway" size="15" maxlength="15" value="<% getInfo("pptpDfGateway"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(macAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<script>
												document.write('<input type="text" name="macAddr1" value="'+tcpipwanmac+'" size="20" maxlength="12">&nbsp;&nbsp;')
												document.write('<input type="button" value="'+showText(clone)+'" name="Clone1" onClick="copyto(document.pptp.macAddr1);" class="btnsize">')
											</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<script>
												if (dnsmode)
												{
													document.write('<input type="radio" value="0" name="DNSMode" onClick="autoDNSClicked(document.pptp);">'+showText(obtainIP))
													document.write('<br>')
													document.write('<input type="radio" value="1" name="DNSMode" onClick="autoDNSClicked(document.pptp);" checked >'+showText(useIPAddr))
												}
												else
												{
													document.write('<input type="radio" value="0" name="DNSMode" onClick="autoDNSClicked(document.pptp);" checked >'+showText(obtainIP))
													document.write('<br>')
													document.write('<input type="radio" value="1" name="DNSMode" onClick="autoDNSClicked(document.pptp);">'+showText(useIPAddr))
												}
											</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr1)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns1)	document.write('<input type="text" name="dns1" size="15" maxlength="15" value="'+valdns1+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns1" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr2)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns2)	document.write('<input type="text" name="dns2" size="15" maxlength="15" value="'+valdns2+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns2" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr3)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns3)	document.write('<input type="text" name="dns3" size="15" maxlength="15" value="'+valdns3+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns3" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td colspan="2" align="left" height="25px" class="style14">
										<script>
											if(dualaccessSW) document.write('<input type="checkbox" name="enableDuallAccess" value="ON" onclick=displayUpdate(); checked>&nbsp;Enable Dual Wan Access :</br>')
											else  document.write('<input type="checkbox" name="enableDuallAccess" value="ON" onclick=displayUpdate();>&nbsp;Enable Dual Wan Access :</br>')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14">IGMP Source :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="radio" name="DUAL_WAN_IGMP" value="0" />ETH&nbsp;&nbsp;<input type="radio" name="DUAL_WAN_IGMP" value="1" />PPP
										<script>
											if (DUAL_WAN_IGMP == 0)
												document.pptp.DUAL_WAN_IGMP[0].checked = true
											else
												document.pptp.DUAL_WAN_IGMP[1].checked = true
										</script>
										</td>
									</tr>
									<tr>
										<td colspan="2" align="left" height="25px" class="style14">&nbsp;<script>dw(pptpSet)</script> :&nbsp;&nbsp;</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(userID)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="pptpUserName" size="20" maxlength="31" value="<% getInfo("pptpUserName"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(password)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="password" name="pptpPassword" size="20" maxlength="31" value="<% getInfo("pptpPassword"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(pptpGate)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="pptpGateway" size="20" maxlength="30" value="<% getInfo("pptpGateway"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(connectionID)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="pptpConntID" size="20" maxlength="30" value="<% getInfo("pptpConntID"); %>">&nbsp;(<script>dw(optional)</script>)
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(mtu)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="pptpMTU" size="10" maxlength="4" value=<% getInfo("pptpMTU"); %>>
											&nbsp;(512&lt;=<script>dw(mtuVal)</script>&lt;=1492)
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(BEZEQ)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="checkbox" name="pptpBEZEQEnable" value="ON" <% if (getIndex("pptpBEZEQEnable")) write("checked");%>><script>dw(useInISRAEL)</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(connectType)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<select size="1" name="pptpConnectType" onChange="pptpTypeSelection();">
										<script>
											<%	write("type = "+getIndex("pptpConnectType"));%>
											var typeTbl = new Array(showText(continuous), showText(connectDemand),showText(manual));
											for (i=0 ; i<3 ; i++)
											{
												if ( i == type )
													document.write("<option selected value='" +i+"'>" +typeTbl[i]+ "</option>");
												else
													document.write("<option value='" +i+"'>" +typeTbl[i]+ "</option>");
											}
										</script>
										</select>
										<script>
											document.write('<input type="submit" value="'+showText(connect)+'" name="pptpConnect" onClick="return saveChang(0)" class="btnsize">')
											document.write('<input type="submit" value="'+showText(disconnect)+'" name="pptpDisconnect" onClick="return saveChang(1)" class="btnsize">')
										</script>
										<% if ( getIndex("pppConnectStatus") ) write("\n<script> setPPPConnected(); </script>\n"); %>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(idleTime)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="pptpIdleTime" size="10" maxlength="4" value="<% getInfo("wan-pptp-idle"); %>">&nbsp;(1-1000 <script>dw(minute)</script>)<br>
										</td>
									</tr>
									<input type=hidden value="/wan.asp" name="submit-url" id="submitUrl">
									<input type=hidden value="3" name="wanMode" >
									<input type=hidden value="" name="isApply">
									</table>
									</form>
									</div>
								</td>
							</tr>
						</table>
						<br />
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666">
									<script>
										if( wanmod == 6) document.write('<input onclick="c_fun(4)" type="radio" name="sys" checked>')
										else document.write('<input onclick="c_fun(4)" type="radio" name="sys">')
									</script>
									<span class="style13"style="cursor:pointer;" onclick="c_fun(4);"><script>dw(L2TP)</script></span>
								</td>
							</tr>
							<script>
								<% write("l2tpMode = "+getIndex("L2TPIpMode"));%>
							</script>
							<tr>
								<td valign="top" align="center">
									<div id="wan_l2tp" style="display:none;"><br>
									<form action=/goform/formL2TPSetup method=POST name="l2tp">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<input type="hidden" name="redirect" size="20" value="">
									<tr>
										<td colspan="2" align="left" height="25px" class="style14">
										<script>
											if(!l2tpMode)document.write('<input type="radio" value="0" name="L2TPIpMode" onClick="autoIpClicked()" checked>&nbsp;'+showText(obtainIP)+' :</br>')
											else document.write('<input type="radio" value="0" name="L2TPIpMode" onClick="autoIpClicked()">&nbsp;'+showText(obtainIP)+' :</br>')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(hostName)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											document.write('<input type="text" name="HostName" value="'+dyiphostname+'" size="20" maxlength="30" class="text">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(macAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<script>
												document.write('<input type="text" name="macAddr" value="'+tcpipwanmac+'" size="20" maxlength="12">&nbsp;&nbsp;')
												document.write('<input type="button" value="'+showText(clone)+'" name="Clone" onClick="copyto(document.l2tp.macAddr);" class="btnsize">')
											</script>
										</td>
									</tr>
									<tr>
										<td colspan="2" align="left" height="25px" class="style14">
										<script>
											if(l2tpMode)document.write('<input type="radio" value="1" name="L2TPIpMode" onClick="autoIpClicked()" checked>&nbsp;'+showText(useIPAddr)+' :</br>')
											else document.write('<input type="radio" value="1" name="L2TPIpMode" onClick="autoIpClicked()">&nbsp;'+showText(useIPAddr)+' :</br>')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(IPAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="L2TPIPAddr" size="18" maxlength="15" value="<% getInfo("L2TPIPAddr"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(subnetMask)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="L2TPMaskAddr" size="18" maxlength="15" value="<% getInfo("L2TPMaskAddr"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(defaultGateway)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="L2TPDefGateway" size="18" maxlength="15" value="<% getInfo("L2TPDefGateway"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<script>
												if (dnsmode)
												{
													document.write('<input type="radio" value="0" name="DNSMode" onClick="autoDNSClicked(document.l2tp);">'+showText(obtainIP))
													document.write('<br>')
													document.write('<input type="radio" value="1" name="DNSMode" onClick="autoDNSClicked(document.l2tp);" checked >'+showText(useIPAddr))
												}
												else
												{
													document.write('<input type="radio" value="0" name="DNSMode" onClick="autoDNSClicked(document.l2tp);" checked >'+showText(obtainIP))
													document.write('<br>')
													document.write('<input type="radio" value="1" name="DNSMode" onClick="autoDNSClicked(document.l2tp);">'+showText(useIPAddr))
												}
											</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr1)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns1)	document.write('<input type="text" name="dns1" size="15" maxlength="15" value="'+valdns1+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns1" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr2)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns2)	document.write('<input type="text" name="dns2" size="15" maxlength="15" value="'+valdns2+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns2" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr3)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											if(valdns3)	document.write('<input type="text" name="dns3" size="15" maxlength="15" value="'+valdns3+'" size="15" maxlength="30">')
											else document.write('<input type="text" name="dns3" value="0.0.0.0" size="15" maxlength="30">')
										</script>
										</td>
									</tr>
									<tr>
										<td colspan="2" align="left" height="25px" class="style14">
										<script>
											if(dualaccessSW) document.write('<input type="checkbox" name="enableDuallAccess" value="ON" onclick=displayUpdate(); checked>&nbsp;Enable Dual Wan Access :</br>')
											else  document.write('<input type="checkbox" name="enableDuallAccess" value="ON" onclick=displayUpdate();>&nbsp;Enable Dual Wan Access :</br>')
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14">IGMP Source :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="radio" name="DUAL_WAN_IGMP" value="0" />ETH&nbsp;&nbsp;<input type="radio" name="DUAL_WAN_IGMP" value="1" />PPP
										<script>
											if (DUAL_WAN_IGMP == 0)
												document.l2tp.DUAL_WAN_IGMP[0].checked = true
											else
												document.l2tp.DUAL_WAN_IGMP[1].checked = true
										</script>
										</td>
									</tr>
									<tr>
										<td colspan="2" align="left" height="25px" class="style14"><script>dw(l2tpSet)</script></td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(userID)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="L2TPUserName" size="18" maxlength="64" value="<% getInfo("L2TPUserName"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(password)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="password" name="L2TPPassword" size="18" maxlength="64" value="<% getInfo("L2TPPassword"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(L2TPGateway)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="L2TPGateway" size="18" maxlength="30" value="<% getInfo("L2TPGateway"); %>">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(mtu)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="L2TPMTU" size="10" maxlength="4" value=<% getInfo("L2TPMTU"); %>>
											<font size=2>&nbsp;(512&lt;=MTU Value&lt;=1492)</font>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(connectType)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<select size="1" name="L2TPConnectType" onChange="L2TPTypeSelection();">
										<script>
											<%	write("type = "+getIndex("L2TPConnectType"));%>
											var typeTbl = new Array(showText(continuous),showText(connectDemand),showText(manual));
											for (i=0 ; i<3 ; i++) {
												if ( i == type )
													document.write("<option selected value='" +i+"'>" +typeTbl[i]+ "</option>");
												else
													document.write("<option value='" +i+"'>" +typeTbl[i]+ "</option>");
											}
										</script>
										</select>
										<script>
											document.write('<input type="submit" value="'+showText(connect)+'" name="L2TPConnect" onClick="return saveChang(0)" class="btnsize">')
											document.write('<input type="submit" value="'+showText(disconnect)+'" name="L2TPDisconnect" onClick="return saveChang(1)" class="btnsize">')
										</script>
										<% if ( getIndex("pppConnectStatus") ) write("\n<script>setPPPConnected(); </script>\n"); %>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(idleTime)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="L2TPIdleTime" size="10" maxlength="4" value="<% getInfo("L2TPIdleTime"); %>">&nbsp;(1-1000 minutes)
										</td>
									</tr>
									<input type=hidden value="/wan.asp" name="submit-url" id="submitUrl">
									<input type=hidden value="6" name="wanMode" >
									<input type=hidden value="" name="isApply">
									</table>
									</form>
									</div>
								</td>
							</tr>
						</table>
						<br />
						<table width="700" border="0" cellspacing="1" cellpadding="0" id="wisp_banner" style="display:block;">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666">
									<input onclick="c_fun(5)" type="radio" name="sys">
									<span class="style13" style="cursor:pointer;" onclick="c_fun(5);"><script>dw(WISP)</script></span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<div id="wan_wisp" style="display:none;"><br>
									<form action=/goform/formStaDrvSetup method=POST name="wispSetup">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
										<td colspan="2" align="left" height="25px" class="style14"><script>dw(basicSet)</script> :&nbsp;&nbsp;</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="left" class="style14"><script>dw(WISP)</script>&nbsp:&nbsp
											<input type="radio" value="0" name="stadrv_type" onClick="wisp_sw()"><script>dw(disable)</script>
											<input type="radio" value="1" name="stadrv_type" onClick="wisp_sw()"><script>dw(enable)</script>&nbsp;&nbsp;
										<script>
											statype=<% getInfo("stadrv_type");%>;
											if (statype == 0) document.getElementsByName("stadrv_type")[0].checked = true;
											else document.getElementsByName("stadrv_type")[1].checked = true;
										</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlEssid)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="stadrv_ssid" size="25" maxlength="32" value="">
											<script type="text/javascript">document.wispSetup.stadrv_ssid.value="<% getInfo("stadrv_ssid"); %>"</script>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlChannelNum)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<select size="1" name="stadrv_chan">
										<script type="text/javascript">
											for(i=1;i<=13;i++)
											{
												if ( i== <% getInfo("stadrv_chan"); %>)
													document.write('<option selected value="'+i+'">'+i+'</option>');
												else
													document.write('<option value="'+i+'">'+i+'</option>');
											}
										</script>
										</select>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(SiteSurvey)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											document.write('<input id="setb1" type="button" value="'+showText(selSS)+'" name="selSurvey" onClick="showSiteSurvey()">');
										</script>
										</td>
									</tr>
									<tr>
										<td colspan="2" width="600" height="20" valign="middle" align="left" class="style14"><script>dw(securitySet)</script> :&nbsp;&nbsp;</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(encryption)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<select size="1" name="stadrv_encrypttype" onChange="displayObj();">
										<script>
											var modeTbl = new Array(showText(disable),"WEP","WPA pre-shared key","WPA RADIUS");
											for ( i=0; i<3; i++)
											{
												if ( i == <% getInfo("stadrv_encrypttype"); %>)
													document.write('<option selected value="'+i+'">'+modeTbl[i]+'</option>');
												else
													document.write('<option value="'+i+'">'+modeTbl[i]+'</option>');
											}
										</script>
										</select>
										</td>
									</tr>
									</table>
									<div id="WEP" style="display:none">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(keyLen)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<select size="1" name="stadrv_weplength" ONCHANGE="lengthClick()">
										<script>
											for(i=0;i<2;i++)
											{
												if( i == <% getInfo("stadrv_weplength"); %> )
													document.write('<option selected value="'+i+'">'+(i+1)*64+'-bit</option>');
												else
													document.write('<option value="'+i+'">'+(i+1)*64+'-bit</option>');
											}
										</script>
										</select>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(KeyFormat)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<select size="1" name="stadrv_wepformat" ONCHANGE="setDefaultKeyValue()" class="select">
											<script>
												for(i=0;i<2;i++)
												{
													if( i == <% getInfo("stadrv_wepformat"); %> )
														document.write('<option selected value="'+i+'"></option>');
													else
														document.write('<option value="'+i+'"></option>');
												}
											</script>
										</select>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(keyTx)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<select size="1" name="stadrv_defaultkey" class="select">
										<script>
											for(i=0;i<1;i++)
											{
												if ( i == <% getInfo("stadrv_defaultkey"); %> )
													document.write('<option selected value="'+ i +'">'+showText(Key)+' '+ (i+1) +'</option>');
												else
													document.write('<option value="'+ i +'">'+showText(Key)+' '+ (i+1) +'</option>');
											}
										</script>
										</select>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(keyEncry)</script>:&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="password" name="key1" size="32" maxlength="32">
										</td>
									</tr>
									</table>
									</div>
									<div id="WEPandWPA" style="display:none">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlCtrlWAP)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											cipherTbl = new Array("WPA (TKIP)", "WPA2 (AES)", "WPA2 Mixed");
											for(i=0;i<2;i++)
											{
												if(i==<%getInfo("stadrv_wpacipher");%>)
													document.write('<input type="radio" name="stadrv_wpacipher" value="'+i+'" id="wpaCipher'+i+'" checked>'+cipherTbl[i]+'&nbsp;&nbsp;');
												else
													document.write('<input type="radio" name="stadrv_wpacipher" value="'+i+'" id="wpaCipher'+i+'">'+cipherTbl[i]+'&nbsp;&nbsp;');
											}
										</script>
										</td>
									</tr>
									</table>
									</div>
									<div id="WPA">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(keyFrmPre)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<select size="1" name="stadrv_pskformat" ONCHANGE=setPskKeyFormat()>
										<script>
											pskformatTbl = new Array ( showText(passphrase), "Hex (64"+showText(wlencryptStrCharaolitec)+")");
											for(i=0;i<2;i++)
											{
												if(i==<%getInfo("stadrv_pskformat");%>)
													document.write('<option selected value="'+i+'">'+pskformatTbl[i]+'</option>');
												else
													document.write('<option value="'+i+'">'+pskformatTbl[i]+'</option>');
											}
										</script>
										</select>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(keyPre)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="password" name="stadrv_pskkey" size="32" maxlength="64" value="<% getInfo("stadrv_pskkey");%>">
										</td>
									</tr>
									</table>
									</div>
									<input type=hidden value="/wan.asp" name="submit-url" id="submitUrl">
									<input type=hidden value="" name="isApply">
									</form>
									</div>
								</td>
							</tr>
						</table>
						<br>
						<script>
							if( typeof _IPV6_SUPPORT_ != 'undefined' )
								document.write('<table width="700" border="0" cellspacing="1" cellpadding="0" id="ipv6_banner" style="display:block;">');
							else
								document.write('<table width="700" border="0" cellspacing="1" cellpadding="0" id="ipv6_banner" style="display:none;">');
						</script>
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666">
									<input onclick="c_fun(6)" type="radio" name="sys">
									<span class="style13" style="cursor:pointer;" onclick="c_fun(6);">IPv6</span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<div id="wan_ipv6" style="display:none;"><br>
										<form action="/goform/formIPv6" method="POST" name="formIPv6">
											<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
												<tr>
													<td align="left">
														<p class="style14"><script>dw(wan_ipv6_settings)</script>:&nbsp;&nbsp;<select name="wanipv6" onchange="wanreload(this.value)">
															<option value="0">Link-local only</option>
															<option value="1">Stateless</option>
															<option value="2">DHCPv6</option>
															<option value="3">Static</option>
															<option value="4">6to4</option>
															<option value="5">6in4</option>
															<option value="6">PPPoE (6to4)</option>
															<option value="7">TSP</option>
															<option value="8">DS-Lite</option>
															<option value="9">6rd</option>
															<option value="10">PPPoE (6rd)</option>
															</select>
														</p>
													</td>
												</tr>
											</table>
											<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
												<tr>
													<td width="200" class="style14" align="right"><script>dw(ipv6_link_local_address)</script>&nbsp;:&nbsp;</td>
													<td width="400" class="style1" align="left">&nbsp;<% getInfo("IPv6WANLinkLocal"); %></td>
												</tr>
											</table>
											<span id="WANIPv6Address" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_address)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<% getInfo("IPv6WANAddress"); %></td>
													</tr>
												</table>
											</span>
											<span id="WANIPv6DSLite" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right">CGN IPv6 Address&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6CGNIP" class="text" size="39" maxlength="39" value="<% getInfo("IPv6CGNIP"); %>" /></td>
													</tr>
												</table>
											</span>
											<span id="WANIPv6Static" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_address)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6WANIP" class="text" size="39" maxlength="39" value="<% getInfo("IPv6WANIP"); %>" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_subnet_prefix_length)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6WANPrefix" class="text" size="3" maxlength="3" value="<% getInfo("IPv6WANPrefix"); %>" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_default_gateway)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6Gateway" class="text" size="39" maxlength="39" value="<% getInfo("IPv6Gateway"); %>" /></td>
													</tr>
												</table>
											</span>
											<span id="WANIPv6in4" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_remote_ipv4_address)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6Remote" class="text" size="15" maxlength="15" value="<% getInfo("IPv6Remote"); %>" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_remote_ipv6_address)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6Remote6" class="text" size="39" maxlength="39" value="<% getInfo("IPv6Remote6"); %>" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_local_ipv4_address)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6Local" class="text" size="15" maxlength="15" value="<% getInfo("IPv6Local"); %>" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_local_ipv6_address)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6Local6" class="text" size="39" maxlength="39" value="<% getInfo("IPv6Local6"); %>" /></td>
													</tr>
												</table>
											</span>
											<span id="WANIPv66rd" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right">6rd Server&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv66rdServer" class="text" size="20" maxlength="32" value="<% getInfo("IPv66rdServer"); %>" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right">6rd Prefix&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv66rdPrefix" class="text" size="9" maxlength="9" value="<% getInfo("IPv66rdPrefix"); %>" /></td>
													</tr>
												</table>
											</span>
											<span id="WANIPv6PPPoE" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_username)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="pppUserName" class="text" size="20" maxlength="64" value="<% getInfo("pppUserName"); %>" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_password)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="password" name="pppPassword" class="text" size="20" maxlength="64" value="<% getInfo("pppPassword"); %>" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_service_name)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="pppServName" class="text" size="20" maxlength="30" value="<% getInfo("pppServName"); %>" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_connect_type)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<select name="pppConnectType" onchange="pppreload(this.value)">
															<option value="0"><script>dw(ipv6_continuous)</script></option>
															<option value="1"><script>dw(ipv6_connect_on_demand)</script></option>
															<option value="2"><script>dw(ipv6_manual)</script></option>
															</select>&nbsp;<input type="button" name="pppConnect" value="Connect" class="button" onclick="return pppoe(0)" />&nbsp;<input type="button" name="pppDisconnect" value="Disconnect" class="button" onclick="return pppoe(1)" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_idle_timeout)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="pppIdleTime" class="text" size="4" maxlength="4" value="<% getInfo("wan-ppp-idle"); %>" /> <script>dw(ipv6_minutes)</script></td>
													</tr>
												</table>
												<input type="hidden" name="WANMode" value="2" />
												<input type="hidden" name="WANDHCP" value="3" />
											</span>
											<span id="WANIPv6MTU" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right">MTU&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6MTU" class="text" size="4" maxlength="4" value="<% getInfo("IPv6MTU"); %>" /></td>
													</tr>
												</table>
											</span>
											<span id="WANIPv6TSP" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_server)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6TSPServer" class="text" size="20" maxlength="32" value="<% getInfo("IPv6TSPServer"); %>" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_username)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6TSPID" class="text" size="20" maxlength="32" value="<% getInfo("IPv6TSPID"); %>" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_password)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="password" name="IPv6TSPPW" class="text" size="20" maxlength="32" value="<% getInfo("IPv6TSPPW"); %>" /></td>
													</tr>
												</table>
											</span>
											<span id="WANIPv6TTL" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right">TTL&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6TTL" class="text" size="3" maxlength="3" value="<% getInfo("IPv6TTL"); %>" /></td>
													</tr>
												</table>
											</span>
											<span id="WANIPv6DNSMode" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_dns_mode)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="radio" name="IPv6DNSMode" value="0" onchange="dnsreload(this.value)" /><script>dw(ipv6_auto)</script><input type="radio" name="IPv6DNSMode" value="1" onchange="dnsreload(this.value)" />Manual</td>
													</tr>
												</table>
											</span>
											<span id="WANIPv6DNS" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_primary_dns)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6DNS1" class="text" size="39" maxlength="39" value="<% getInfo("IPv6DNS1"); %>" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_secondary_dns)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6DNS2" class="text" size="39" maxlength="39" value="<% getInfo("IPv6DNS2"); %>" /></td>
													</tr>
												</table>
											</span>
											<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
												<tr>
													<td align="left">
														<p class="style14"><script>dw(lan_ipv6_settings)</script>:&nbsp;&nbsp;<select name="lanipv6" onchange="lanreload(this.value)">
															<option value="0"><script>dw(ipv6_disable)</script></option>
															<option value="1">Stateless</option>
															<option value="2">DHCPv6</option>
															</select>
														</p>
													</td>
												</tr>
											</table>
											<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
												<tr>
													<td width="200" class="style14" align="right"><script>dw(ipv6_link_local_address)</script>&nbsp;:&nbsp;</td>
													<td width="400" class="style1" align="left">&nbsp;<% getInfo("IPv6LANLinkLocal"); %></td>
												</tr>
											</table>
											<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
												<tr>
													<td width="200" class="style14" align="right"><script>dw(ipv6_address)</script>&nbsp;:&nbsp;</td>
													<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6LANIP" class="text" size="39" maxlength="39" value="<% getInfo("IPv6LANIP"); %>" onchange="if (document.formIPv6.lanipv6.selectedIndex == 2) dhcprange()" /></td>
												</tr>
											</table>
											<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
												<tr>
													<td width="200" class="style14" align="right"><script>dw(ipv6_subnet_prefix_length)</script>&nbsp;:&nbsp;</td>
													<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6LANPrefix" class="text" size="3" maxlength="3" value="<% getInfo("IPv6LANPrefix"); %>" /></td>
												</tr>
											</table>
											<span id="LANIPv6Stateless" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_ra_interval)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6Interval" class="text" size="4" maxlength="4" value="<% getInfo("IPv6Interval"); %>" /> <script>dw(ipv6_seconds)</script></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right">MTU&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6LANMTU" class="text" size="4" maxlength="4" value="<% getInfo("IPv6LANMTU"); %>" /></td>
													</tr>
												</table>
											</span>
											<span id="LANIPv6DHCP" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_dhcp_client_start_ipv6)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6DHCPStart" class="text" size="39" maxlength="39" value="<% getInfo("IPv6DHCPStart"); %>" /></td>
													</tr>
												</table>
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_dhcp_client_end_ipv6)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6DHCPEnd" class="text" size="39" maxlength="39" value="<% getInfo("IPv6DHCPEnd"); %>" /></td>
													</tr>
												</table>
											</span>
											<span id="LANIPv6Lifetime" style="display:none">
												<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
													<tr>
														<td width="200" class="style14" align="right"><script>dw(ipv6_lifetime)</script>&nbsp;:&nbsp;</td>
														<td width="400" class="style1" align="left">&nbsp;<input type="text" name="IPv6Lifetime" class="text" size="5" maxlength="5" value="<% getInfo("IPv6Lifetime"); %>" /> <script>dw(ipv6_minutes)</script></td>
													</tr>
												</table>
											</span>
											<table border="0" width="700" style="display:none">
												<tr>
													<td align="right"><input type="submit" value="APPLY" class="button" />&nbsp;&nbsp;<input type="reset" value="CANCEL" class="button" onclick="window.location.reload()" /></td>
												</tr>
											</table>
											<input type="hidden" name="submit-url" value="/wan.asp" />
										</form>
									</div>
								</td>
							</tr>
						</table>
						<form action="/goform/formWanTcpipSetup" method="POST" name="formWanTcpipSetup">
							<% if (getIndex("pppConnectStatus")) write("<input type=\"hidden\" name=\"pppDisconnect\" value=\"1\" />"); else write("<input type=\"hidden\" name=\"pppConnect\" value=\"1\" />"); %>
							<input type="hidden" name="pppUserName" />
							<input type="hidden" name="pppPassword" />
							<input type="hidden" name="pppServName" />
							<input type="hidden" name="pppConnectType" />
							<input type="hidden" name="pppIdleTime" />
							<input type="hidden" name="wanMode" value="2" />
							<input type="hidden" name="submit-url" value="/wan.asp" />
						</form>
						<script type="text/javascript">
						if( typeof _IPV6_SUPPORT_ != 'undefined' )
							init()
						</script>
						<br>
						<table width="700" border="0" cellpadding="2" cellspacing="2">
							<tr>
								<td align="right">
									<script>
										if(wizardEnabled == 1)
										{
											document.write('<input type=button value="'+showText(back1)+'" style ="width:100px" onClick="window.history.back();">');
											document.write('<input type=button value="'+showText(ok1)+'" name="save" style ="width:100px" onClick="return saveChanges()">');
										}
										else
										{
											document.write('<input type=button value="'+showText(apply1)+'" name="save" style ="width:100px" onClick="return saveChanges()">');
										}
									</script>
								</td>
							</tr>
						</table>
					</td>
					<!-- explain -->
					<td width="380" valign="top" bgcolor="#999999">
						<table width="380" border="0" align="right" cellpadding="0" cellspacing="10">
							<tr>
								<td height="400" valign="top">
									<table width="360" height="400 "border="0" cellpadding="5" cellspacing="0" bgcolor="#999999">
										<tr>
											<td height="16" bgcolor="#A5031A"><span class="style2">Help</span></td>
										</tr>
										<tr>
											<td height="384" valign="top" bgcolor="#999999">
											<span class="style1">
											<span class="style13"><script>dw(DynamicIP)</script></span><br>
											<span id="com00" class="style21" style="display:block"><script>dw(wandipContentshort)</script><a class="style71" onclick="com_sw(0,1);" style="cursor:pointer;" > .....more</a></span>
											<span id="com01" class="style21" style="display:none"><script>dw(wandipContent)</script><a class="style71" onclick="com_sw(0,2);" style="cursor:pointer;" > .....close</a></span>
											</span>
											<br>
											<span class="style1">
											<span class="style13"><script>dw(StaticIP)</script></span><br>
											<span id="com10" class="style21" style="display:block"><script>dw(wansipContentshort)</script><a class="style71" onclick="com_sw(1,1);" style="cursor:pointer;" > .....more</a></span>
											<span id="com11" class="style21" style="display:none"><script>dw(wansipContent)</script><a class="style71" onclick="com_sw(1,2);" style="cursor:pointer;" > .....close</a></span>
											</span>
											<br>
											<span class="style1">
											<span class="style13"><script>dw(PPPoE)</script></span><br>
											<span id="com20" class="style21" style="display:block"><script>dw(pppoeContentshort)</script><a class="style71" onclick="com_sw(2,1);" style="cursor:pointer;" > .....more</a></span>
											<span id="com21" class="style21" style="display:none"><script>dw(pppoeContent)</script><a class="style71" onclick="com_sw(2,2);" style="cursor:pointer;" > .....close</a></span>
											</span>
											<br>
											<span class="style1">
											<span class="style13"><script>dw(PPTP)</script></span><br>
											<span id="com30" class="style21" style="display:block"><script>dw(wansetPageContent5short)</script><a class="style71" onclick="com_sw(3,1);" style="cursor:pointer;" > .....more</a></span>
											<span id="com31" class="style21" style="display:none"><script>dw(wansetPageContent5)</script><a class="style71" onclick="com_sw(3,2);" style="cursor:pointer;" > .....close</a></span>
											</span>
											<br>
											<span class="style1">
											<span class="style13"><script>dw(L2TP)</script></span><br>
											<span id="com40" class="style21" style="display:block"><script>dw(wansetPageContent6short)</script><a class="style71" onclick="com_sw(4,1);" style="cursor:pointer;" > .....more</a></span>
											<span id="com41" class="style21" style="display:none"><script>dw(wansetPageContent6)</script><a class="style71" onclick="com_sw(4,2);" style="cursor:pointer;" > .....close</a></span>
											</span>
											<br>
											<span class="style1">
											<span class="style13"><script>dw(WISP)</script></span><br>
											<span id="com50" class="style21" style="display:block"><script>dw(WISPContentshort)</script><a class="style71" onclick="com_sw(5,1);" style="cursor:pointer;" > .....more</a></span>
											<span id="com51" class="style21" style="display:none"><script>dw(WISPContent)</script><a class="style71" onclick="com_sw(5,2);" style="cursor:pointer;" > .....close</a></span>
											</span>
											</td>
										</tr>
									</table>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
			<!-- down menu -->
			<script>Write_tail();</script>
		</td>
	</tr>
</table>
</body>
</html>
<script>
	document.getElementById('wan_dip').style.display = "none"
	document.getElementById('wan_sip').style.display = "none"
	document.getElementById('wan_ppp').style.display = "none"
	document.getElementById('wan_pptp').style.display = "none"
	document.getElementById('wan_l2tp').style.display = "none"
	document.getElementById('wan_wisp').style.display = "none"
	if (wanmod == 0)
	{
		document.getElementById('wan_dip').style.display = "block"
		autoDNSClicked(document.dip)
	}
	else if (wanmod == 1) document.getElementById('wan_sip').style.display = "block"
	else if (wanmod == 2)
	{
		document.getElementById('wan_ppp').style.display = "block"
		autoDNSClicked(document.ppp)
	}
	else if (wanmod == 3)
	{
		document.getElementById('wan_pptp').style.display = "block"
		autoDNSClicked(document.pptp)
	}
	else if (wanmod == 6)
	{
		document.getElementById('wan_l2tp').style.display = "block"
		autoDNSClicked(document.l2tp)
	}
	pppTypeSelection();
	pptpTypeSelection();
	autoIpClicked();
	L2TPTypeSelection();
	updateFormat();
	setPskKeyValue();
	displayObj();
	top.sw_current_sta(internet_sta);
	top.cloud_current_sta(cloud_sta);
	wisp_sw()
	if (duallAccessType==0)
	{
		document.ppp.duallAccessMode[0].checked=true;
		document.ppp.duallAccessMode[1].checked=false;
	}
	else
	{
		document.ppp.duallAccessMode[0].checked=false;
		document.ppp.duallAccessMode[1].checked=true;
	}
	displayUpdate();
</script>
