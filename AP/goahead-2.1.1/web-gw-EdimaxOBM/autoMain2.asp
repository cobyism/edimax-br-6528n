<html>
<head>
<meta http-equiv="Content-Type" content="text/html;charset=UTF-8" />
<meta http-equiv="cache-control" content="no-cache,must-revalidate">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="cache-control" content="no-cache" />
<meta http-equiv="cache-control" content="no-store" />
<meta http-equiv="Expires" content="-1">
<link rel="stylesheet" href="file/set.css">
<link rel="stylesheet" href="edimax.css">
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="FUNCTION_SCRIPT"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<script type="text/javascript" src="file/multilanguage.var"></script>
<script type="text/javascript" src="file/autowan.var"></script>
<style type="text/css">
		#in{width:0px; height:20px;background:#0080FF;color:#000000;text-align:center;}
</style>
<script>
function Dipover()
{
	document.getElementsByName("wanmode")[0].checked=true;
}
function Dipout()
{
	if(document.getElementsByName("wanmode")[0].checked)
		document.getElementsByName("wanmode")[0].checked=true;
	else
		document.getElementsByName("wanmode")[0].checked=false;
}
function Sipover()
{
	document.getElementsByName("wanmode")[1].checked=true;
}
function Sipout()
{
	if(document.getElementsByName("wanmode")[1].checked)
		document.getElementsByName("wanmode")[1].checked=true;
	else
		document.getElementsByName("wanmode")[1].checked=false;
}
function Pppoeover()
{
	document.getElementsByName("wanmode")[2].checked=true;
}
function Pppoeout()
{
	if(document.getElementsByName("wanmode")[2].checked)
		document.getElementsByName("wanmode")[2].checked=true;
	else
		document.getElementsByName("wanmode")[2].checked=false;
}
function Pptpover()
{
	document.getElementsByName("wanmode")[3].checked=true;
}
function Pptpout()
{
	if(document.getElementsByName("wanmode")[3].checked)
		document.getElementsByName("wanmode")[3].checked=true;
	else
		document.getElementsByName("wanmode")[3].checked=false;
}
function L2tpover()
{
	document.getElementsByName("wanmode")[4].checked=true;
}
function L2tpout()
{
	if(document.getElementsByName("wanmode")[4].checked)
		document.getElementsByName("wanmode")[4].checked=true;
	else
		document.getElementsByName("wanmode")[4].checked=false;
}
function Telover()
{
	document.getElementsByName("wanmode")[5].checked=true;
}
function Telout()
{
	if(document.getElementsByName("wanmode")[5].checked)
		document.getElementsByName("wanmode")[5].checked=true;
	else
		document.getElementsByName("wanmode")[5].checked=false;
}
function saveChanges() {
	if( (!document.getElementsByName("wanmode")[0].checked) && (!document.getElementsByName("wanmode")[1].checked) && (!document.getElementsByName("wanmode")[2].checked) && (!document.getElementsByName("wanmode")[3].checked) && (!document.getElementsByName("wanmode")[4].checked) && (!document.getElementsByName("wanmode")[5].checked))
	{
		alert("Please select one of connection type.")
		return false
	}
	if(document.getElementsByName("wanmode")[0].checked)
	{
		var str1 = document.tcpip.dynIPHostName.value
		if (str1.search('"') != -1) {
			alert(showText(wandipAlert3))
			document.tcpip.dynIPHostName.focus()
			return false
		}
		if (!check(document.tcpip.dynIPHostName)) return false
		if (document.tcpip.dns1.value=="" && document.tcpip.dns2.value=="")
		{
			document.tcpip.dnsMode.value="dnsAuto";
			document.tcpip.dns1.value="0.0.0.0";
			document.tcpip.dns2.value="0.0.0.0";
		}
		else
		{
			document.tcpip.dnsMode.value="dnsManual";
			if (document.tcpip.dns1.value=="") document.tcpip.dns1.value="0.0.0.0";
			if (document.tcpip.dns2.value=="") document.tcpip.dns2.value="0.0.0.0";
			if ( !ipRule( document.tcpip.dns1,showText(PrimaryDNS), "gw", 1)) return false;
			if ( !ipRule( document.tcpip.dns2,showText(SecondaryDNS), "gw", 1)) return false;
		}
		if (document.tcpip.dipmacAddr.value == "")
			document.tcpip.dipmacAddr.value="000000000000"
		document.tcpip.wanMode.value=0
	}
	if(document.getElementsByName("wanmode")[1].checked)
	{
		if ( !ipRule( document.tcpip.ip, showText(IPAddress), "ip", 1)) return false;
		if ( !maskRule( document.tcpip.mask, showText(SubnetMask), 1)) return false;
		if ( !ipRule( document.tcpip.gateway, showText(DefaultGateway), "gw", 1)) return false;
		if ( !subnetRule(document.tcpip.ip, document.tcpip.mask, document.tcpip.gateway, showText(DefaultGateway), showText(IPAddress))) return false;
		if (document.tcpip.dns3.value=="" && document.tcpip.dns4.value=="")
		{
			document.tcpip.dnsMode.value="dnsAuto";
			document.tcpip.dns3.value="0.0.0.0";
			document.tcpip.dns4.value="0.0.0.0";
		}
		else
		{
			document.tcpip.dnsMode.value="dnsManual";
			if (document.tcpip.dns3.value=="") document.tcpip.dns3.value="0.0.0.0";
			if (document.tcpip.dns4.value=="") document.tcpip.dns4.value="0.0.0.0";
			if ( !ipRule( document.tcpip.dns3,showText(PrimaryDNS), "gw", 1)) return false;
			if ( !ipRule( document.tcpip.dns4,showText(SecondaryDNS), "gw", 1)) return false;
		}
		if (document.tcpip.sipmacAddr.value == "")
			document.tcpip.sipmacAddr.value="000000000000"
		document.tcpip.wanMode.value=1
	}
	if(document.getElementsByName("wanmode")[2].checked)
	{
		if (!strRule(document.tcpip.pppUserName,showText(UserName))) return false;
		if (!strRule(document.tcpip.pppPassword,showText(Password))) return false;
		if (!check(document.tcpip.pppUserName)) return false
		if (!check(document.tcpip.pppPassword)) return false
		if (document.tcpip.dns5.value=="" && document.tcpip.dns6.value=="")
		{
			document.tcpip.dnsMode.value="dnsAuto";
			document.tcpip.dns5.value="0.0.0.0";
			document.tcpip.dns6.value="0.0.0.0";
		}
		else
		{
			document.tcpip.dnsMode.value="dnsManual";
			if (document.tcpip.dns5.value=="") document.tcpip.dns1.value="0.0.0.0";
			if (document.tcpip.dns6.value=="") document.tcpip.dns2.value="0.0.0.0";
			if ( !ipRule( document.tcpip.dns5,showText(PrimaryDNS), "gw", 1)) return false;
			if ( !ipRule( document.tcpip.dns6,showText(SecondaryDNS), "gw", 1)) return false;
		}
		if (document.tcpip.pppoemacAddr.value == "")
			document.tcpip.pppoemacAddr.value="000000000000"
		document.tcpip.wanMode.value=2
		document.tcpip.ipMode.value="ppp"
	}
	if(document.getElementsByName("wanmode")[3].checked)
	{
		strIp = document.tcpip.pptpIPAddr;
		strMask = document.tcpip.pptpIPMaskAddr;
		strDefGateway = document.tcpip.pptpDfGateway;
		strPPTPGateway = document.tcpip.pptpGateway;
		if (!strRule(document.tcpip.pptpUserName,showText(UserName))) return false;
		if (!strRule(document.tcpip.pptpPassword,showText(Password))) return false;
		if (document.tcpip.pptpIpMode[1].checked==true)
		{
			if ( !checkSubnet( strIp.value, strMask.value, strPPTPGateway.value) )
			{
				if ( !checkSubnet( strIp.value, strMask.value, strDefGateway.value) )
				{
					alert(showText(InvalidSomething).replace(/#####/,showText(DefaultGateway))+' '+showText(SomethingAndSomethingShouldInSameSubnet).replace(/#####/,showText(DefaultGateway)).replace(/####@/,showText(IPAddress)));
					setFocus(strDefGateway);
					return false;
				}
			}
		}
		document.tcpip.wanMode.value=3
	}
	if(document.getElementsByName("wanmode")[4].checked)
	{
		strIp = document.tcpip.L2TPIPAddr;
		strMask = document.tcpip.L2TPMaskAddr;
		strDefGateway = document.tcpip.L2TPDefGateway;
		strL2TPGateway = document.tcpip.L2TPGateway;
		if (document.tcpip.L2TPIpMode[1].checked==true)
		{
			if (!ipRule( strIp, showText(IPAddress), "ip", 1)) return false
			if (!maskRule(strMask, showText(SubnetMask), 1)) return false
			if (!ipRule( strDefGateway, showText(DefaultGateway), "gw", 1)) return false
		}
		else
		{
			if (document.tcpip.l2tpHostName.value != "" && !document.tcpip.l2tpHostName.value.match(/^[A-Za-z0-9_]{1,30}$/)) {
				alert("Invalid Host Name");
				document.tcpip.l2tpHostName.value = document.tcpip.l2tpHostName.defaultValue;
				document.tcpip.l2tpHostName.focus();
				return false;
			}
		}
		if (!strRule(document.tcpip.L2TPUserName, showText(UserName))) return false;
		if (!strRule(document.tcpip.L2TPPassword, showText(Password))) return false;
		if (!strRule(strL2TPGateway,showText(L2TPGateway))) return false;
		if (document.tcpip.l2tpmacAddr.value == "")
			document.tcpip.l2tpmacAddr.value="000000000000"
		document.tcpip.wanMode.value=6
	}
	if(document.getElementsByName("wanmode")[5].checked)
	{
		if (document.tcpip.telBPEnabled.checked)
		{
			if ( !ipRule( document.tcpip.telBPIPAddr,showText(ServerIPAddress),"ip", 1)) return false;
		}
		if (!strRule(document.tcpip.telBPUserName,showText(UserName))) return false;
		if (!strRule(document.tcpip.telBPPassword,showText(Password))) return false;
		document.tcpip.wanMode.value=4
	}
	document.getElementById('config').style.display="none";
	document.getElementById('setup').style.display="block";
	document.getElementById('apply').style.display="none";
	document.getElementById('dipId').style.display="none";
	document.getElementById('sipId').style.display="none";
	document.getElementById('pppoeId').style.display="none";
	document.getElementById('pptpId').style.display="none";
	document.getElementById('l2tpId').style.display="none";
	document.getElementById('TelBPId').style.display="none";
	document.getElementById('udipId').style.display="none";
	document.getElementById('usipId').style.display="none";
	document.getElementById('upppoeId').style.display="none";
	document.getElementById('upptpId').style.display="none";
	document.getElementById('ul2tpId').style.display="none";
	document.getElementById('uTelBPId').style.display="none";
	start();
	return true
}
i=0;secs=120;
function start()
{
	ba=setInterval("begin()",secs*10);
}
function begin()
{
	i+=1;
	if(i<=100)
	{
		document.getElementById("in").style.width=i+"%";
	}
	else
	{
		window.location.replace("autoDirect2.asp")
	}
}
function copyto() {
	if(document.getElementsByName("wanmode")[0].checked)
		document.tcpip.dipmacAddr.value = document.tcpip.macAddrValue.value
	else if(document.getElementsByName("wanmode")[1].checked)
		document.tcpip.sipmacAddr.value = document.tcpip.macAddrValue4.value
	else if(document.getElementsByName("wanmode")[2].checked)
		document.tcpip.pppoemacAddr.value = document.tcpip.macAddrValue1.value
	else if(document.getElementsByName("wanmode")[3].checked)
		document.tcpip.pptpmacAddr.value = document.tcpip.macAddrValue2.value
	else if(document.getElementsByName("wanmode")[4].checked)
		document.tcpip.l2tpmacAddr.value = document.tcpip.macAddrValue3.value
}
function check(input)
{
	var x = input
	if (x.value != "")
	{
		if (x.value.search("\'") != -1 || x.value.search("\"") != -1 || x.value.search("\<") != -1 || x.value.search("\>") != -1)
		{
			alert("Invalid value")
			x.value = x.defaultValue
			x.focus()
			return false
		}
		else
			return true
	}
	else
		return true
}
function checkip(input)
{
	var x = input
	if (!x.value.match(/\b(?:\d{1,3}\.){3}\d{1,3}\b/) || x.value == "" || x.value == "0.0.0.0" || x.value == "255.255.255.255")
	{
		alert("Invalid IP Address")
		x.value = x.defaultValue
		x.focus()
		return false
	}
	else
		return true
}
function checkmask(input)
{
	var x = input
	if (!x.value.match(/\b(?:\d{1,3}\.){3}\d{1,3}\b/) || x.value == "" || x.value == "0.0.0.0" || x.value == "255.255.255.255")
	{
		alert("Invalid Subnet Mask")
		x.value = x.defaultValue
		x.focus()
		return false
	}
	else
		return true
}
function checkgateway(input)
{
	var x = input
	if (x.value != "" && !x.value.match(/\b(?:\d{1,3}\.){3}\d{1,3}\b/) || x.value == "255.255.255.255")
	{
		alert("Invalid Default Gateway")
		x.value = x.defaultValue
		x.focus()
		return false
	}
	else
		return true
}
function autoDNSClicked()
{
	if(document.tcpip.DNSMode[0].checked==true)
	{
      	document.tcpip.dns1.disabled = true;
      	document.tcpip.dns2.disabled = true;
   	}
	else
	{
      	document.tcpip.dns1.disabled = false;
      	document.tcpip.dns2.disabled = false;
    }
	if(document.tcpip.DNSMode1[0].checked==true)
	{
		document.tcpip.dns5.disabled = true;
		document.tcpip.dns6.disabled = true;
	}
	else
	{
      	document.tcpip.dns5.disabled = false;
      	document.tcpip.dns6.disabled = false;
  	}
}
function autoIpClicked()
{
	if(document.tcpip.pptpIpMode[0].checked==true)
	{
		document.tcpip.pptpIPAddr.disabled=true;
		document.tcpip.pptpIPMaskAddr.disabled=true;
		document.tcpip.pptpDfGateway.disabled = true;
		document.tcpip.pptpHostName.disabled = false;
	}
	else
	{
		document.tcpip.pptpIPAddr.disabled=false;
		document.tcpip.pptpIPMaskAddr.disabled=false;
		document.tcpip.pptpDfGateway.disabled = false;
		document.tcpip.pptpHostName.disabled = true;
	}
	if(document.tcpip.L2TPIpMode[0].checked==true)
	{
		document.tcpip.L2TPIPAddr.disabled = true;
		document.tcpip.L2TPMaskAddr.disabled = true;
		document.tcpip.L2TPDefGateway.disabled = true;
		document.tcpip.l2tpHostName.disabled = false;
	}
	else
	{
		document.tcpip.L2TPIPAddr.disabled = false;
		document.tcpip.L2TPMaskAddr.disabled = false;
		document.tcpip.L2TPDefGateway.disabled = false;
		document.tcpip.l2tpHostName.disabled = true;
	}
}
function display()
{
	if(document.getElementsByName("wanmode")[0].checked)
	{
		document.getElementById('dipId').style.display="block";
		document.getElementById('sipId').style.display="none";
		document.getElementById('pppoeId').style.display="none";
		document.getElementById('pptpId').style.display="none";
		document.getElementById('l2tpId').style.display="none";
		document.getElementById('TelBPId').style.display="none";
	}
	else if (document.getElementsByName("wanmode")[1].checked)
	{
		document.getElementById('dipId').style.display="none";
		document.getElementById('sipId').style.display="block";
		document.getElementById('pppoeId').style.display="none";
		document.getElementById('pptpId').style.display="none";
		document.getElementById('l2tpId').style.display="none";
		document.getElementById('TelBPId').style.display="none";
	}
	else if (document.getElementsByName("wanmode")[2].checked)
	{
		document.getElementById('dipId').style.display="none";
		document.getElementById('sipId').style.display="none";
		document.getElementById('pppoeId').style.display="block";
		document.getElementById('pptpId').style.display="none";
		document.getElementById('l2tpId').style.display="none";
		document.getElementById('TelBPId').style.display="none";
	}
	else if (document.getElementsByName("wanmode")[3].checked)
	{
		document.getElementById('dipId').style.display="none";
		document.getElementById('sipId').style.display="none";
		document.getElementById('pppoeId').style.display="none";
		document.getElementById('pptpId').style.display="block";
		document.getElementById('l2tpId').style.display="none";
		document.getElementById('TelBPId').style.display="none";
	}
	else if (document.getElementsByName("wanmode")[4].checked)
	{
		document.getElementById('dipId').style.display="none";
		document.getElementById('sipId').style.display="none";
		document.getElementById('pppoeId').style.display="none";
		document.getElementById('pptpId').style.display="none";
		document.getElementById('l2tpId').style.display="block";
		document.getElementById('TelBPId').style.display="none";
	}
	else if (document.getElementsByName("wanmode")[5].checked)
	{
		document.getElementById('dipId').style.display="none";
		document.getElementById('sipId').style.display="none";
		document.getElementById('pppoeId').style.display="none";
		document.getElementById('pptpId').style.display="none";
		document.getElementById('l2tpId').style.display="none";
		document.getElementById('TelBPId').style.display="block";
	}
}
function modeSelect(onLoad)
{
	document.getElementById('dipId').style.display="none";
	document.getElementById('sipId').style.display="none";
	document.getElementById('pppoeId').style.display="none";
	document.getElementById('pptpId').style.display="none";
	document.getElementById('l2tpId').style.display="none";
}
function updateState()
{
	if (document.tcpip.telBPEnabled.checked) document.tcpip.telBPIPAddr.disabled=false;
	else document.tcpip.telBPIPAddr.disabled=true;
}
function checkmac(input) {
var x = input
if (!x.value.match(/^[A-Fa-f0-9]{12}$/) || x.value == "ffffffffffff" || x.value == "FFFFFFFFFFFF")
x.value = "000000000000"
return true
}
</script>
</head>
<body onLoad="modeSelect(1)";>
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(1,0)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF">
					<form action=/goform/formWanAutoDetect method=POST name="tcpip">
					<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
						<tr>
					      <td width="700" align="left" valign="top" bgcolor="#EFEFEF">
						<blockquote>
							<table width="700" border="0" cellpadding="0" cellspacing="0">
							<span id="config" style="display:block">
							<p class="titlecolor"><script>dw(PleaseSelect)</script></p>
							</span>
							<span id="setup" style="display:none">
							<p class="titlecolor"><script>dw(CheckingInternet)</script></p>
							<div id="in" style="width:%"></div>
							</span>
							</table>
							<span id="udipId" style="display:block">
							<table border="0" cellspacing="0" cellpadding="0">
							<hr />
							<tr valign="top" align="center" height="40">
							<td width="20"><input type="radio" name="wanmode" value="0" onclick="display();" onmouseover="Dipover();" onmouseout="Dipout();"></td>
							<td width="100"><a class="textcolor"><b><script>dw(DynamicIP)</script></b></a></td>
							<td>:&nbsp;&nbsp;</td>
							<td class="stringcolor" align="left" width="600"><script>dw(detectDynamic)</script></td>
							</tr>
							</table>
							</span>
							<blockquote>
							<span id="dipId" style="display:none">
							<table border="0" cellspacing="1" cellpadding=0 width="520">
							<tr>
							<td width="35%" class="table1"><script>dw(HostName)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="dynIPHostName" value="<% getInfo("dynIPHostName"); %>" size="15" maxlength="30" class="text" onchange="check(this)"></td>
							</tr>
							<tr>
							<input type=hidden value=<% getInfo("cloneMac"); %> name="macAddrValue">
							<td width="35%" class="table1"><script>dw(MACAddress)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="dipmacAddr" value="<% getInfo("wanMac"); %>" size="15" maxlength="12" class="text" onchange="checkmac(this)">
							<script>document.write('<input type="button" value="'+showText(CloneMAC)+'" name="Clone" onClick="copyto();" class="button">');</script>
							</td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(DNSAddess)</script> :&nbsp;</td>
							<td width="65%" class="table2"><input type="radio" value="0" name="DNSMode" <% if (getIndex("wanDNS")==0) write("checked"); %> onClick="autoDNSClicked();"><script>dw(ObtainIPAddressAutomatically)</script>
							<br>
							<input type="radio" value="1" name="DNSMode" <% if (getIndex("wanDNS")==1) write("checked"); %> onClick="autoDNSClicked();"><script>dw(UseFollowingIPAddress)</script>
							</td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(PrimaryDNS)</script> :&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="dns1" value="<% getInfo("wan-dns1"); %>" size="15" maxlength="30">
							<script>if(document.tcpip.dns1.value=="") document.tcpip.dns1.value="0.0.0.0"</script>
							</td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(SecondaryDNS)</script> :&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="dns2" value="<% getInfo("wan-dns2"); %>" size="15" maxlength="30">
							<script>if(document.tcpip.dns2.value=="") document.tcpip.dns2.value="0.0.0.0"</script>
							</td>
							</tr>
							<tr>
							<td width="35%" class="table1"><font size=2>TTL :&nbsp;</td>
							<td width="65%" class="table2"><font size=2>&nbsp;<input type="radio" value="0" name="pppEnTtl"<% if (getIndex("pppEnTtl")==0) write("checked"); %>><script>dw(Disable)</script>&nbsp;&nbsp;
							<input type="radio" name="pppEnTtl" value="1"<% if (getIndex("pppEnTtl")==1) write("checked"); %>><script>dw(Enable)</script></td></tr>
							</table>
							</span>
							</blockquote>
							<span id="usipId" style="display:block">
							<table border="0" cellspacing="0" cellpadding="0">
							<hr />
							<tr valign="top" align="center" height="40">
							<td width="20"><input name="wanmode" type="radio" value="1" onclick="display();" onmouseover="Sipover();" onmouseout="Sipout();"></td>
							<td width="100"><a class="textcolor"><b><script>dw(StaticIP)</script></b></a></td>
							<td>:&nbsp;&nbsp;</td>
							<td class="stringcolor" align="left" width="600"><script>dw(detectStatic)</script></td>
							</tr>
							</table>
							</span>
							<blockquote>
							<span id="sipId" style="display:none" >
							<table border="0" cellspacing="1" cellpadding=0 width="520">
							<tr>
							<td width="35%" class="table1"><script>dw(IPAddress)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="ip" size="15" maxlength="15" value=<% getInfo("wan-ip-rom"); %> class="text" onchange="checkip(this)"></td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(SubnetMask)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="mask" size="15" maxlength="15" value="<% getInfo("wan-mask-rom"); %>" class="text" onchange="checkmask(this)"></td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(DefaultGateway)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="gateway" size="15" maxlength="15" value="<% getInfo("wan-gateway-rom"); %>" class="text" onchange="checkgateway(this)"></td>
							</tr>
							<tr>
							<input type=hidden value=<% getInfo("cloneMac"); %> name="macAddrValue4">
							<td width="35%" class="table1"><script>dw(MACAddress)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="sipmacAddr" value="<% getInfo("wanMac"); %>"size="15" maxlength="12" class="text" onchange="checkmac(this)">
							<script>document.write('<input type="button" value="'+showText(CloneMAC)+'" name="Clone" onClick="copyto();" class="button">');</script>
							</td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(PrimaryDNS)</script> :&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="dns3" value="<% getInfo("wan-dns1"); %>" size="15" maxlength="30">
							<script>if(document.tcpip.dns3.value=="") document.tcpip.dns3.value="0.0.0.0"</script>
							</td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(SecondaryDNS)</script> :&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="dns4" value="<% getInfo("wan-dns2"); %>" size="15" maxlength="30">
							<script>if(document.tcpip.dns4.value=="") document.tcpip.dns4.value="0.0.0.0"</script>
							</td>
							</tr>
							<tr>
							<td width="35%" class="table1"><font size=2>TTL :&nbsp;</td>
							<td width="65%" class="table2"><font size=2>&nbsp;<input type="radio" value="0" name="pppEnTtl1"<% if (getIndex("pppEnTtl")==0) write("checked"); %>><script>dw(Disable)</script>&nbsp;&nbsp;
							<input type="radio" name="pppEnTtl1" value="1"<% if (getIndex("pppEnTtl")==1) write("checked"); %>><script>dw(Enable)</script></td></tr>
							</table>
							</span>
							</blockquote>
							<span id="upppoeId" style="display:block" >
							<table border="0" cellspacing="0" cellpadding="0">
							<hr />
							<tr valign="top" align="center" height="40">
							<td width="20"><input name="wanmode" type="radio" value="2" onclick="display();" onmouseover="Pppoeover();" onmouseout="Pppoeout();"></td>
							<td width="100"><a class="textcolor"><b><script>dw(PPPoE)</script></b></a></td>
							<td>:&nbsp;&nbsp;</td>
							<td class="stringcolor" align="left" width="600"><script>dw(detectPPPoE)</script></td>
							</tr>
							</table>
							</span>
							<blockquote>
							<span id="pppoeId" style="display:none" >
							<table border="0" cellspacing="1" cellpadding=0 width="520">
							<tr>
							<td width="35%" class="table1"><script>dw(UserName)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="pppUserName" size="15" maxlength="64" value="<% getInfo("pppUserName"); %>" class="text" onchange="check(this)"></td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(Password)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="pppPassword" size="15" maxlength="64" value="<% getInfo("pppPassword"); %>" class="text" onchange="check(this)"></td>
							</tr>
							<tr>
							<input type=hidden value=<% getInfo("cloneMac"); %> name="macAddrValue1">
							<td width="35%" class="table1"><script>dw(MACAddress)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="pppoemacAddr" size="15" maxlength="12" value="<% getInfo("wanMac"); %>" onchange="checkmac(this)">
							<script>document.write('<input type="button" value="'+showText(CloneMAC)+'" name="Clone" onClick="copyto();" class="button" >');</script>
							</td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(DNSAddess)</script> :&nbsp;</td>
							<td width="65%" class="table2"><input type="radio" value="0" name="DNSMode1" <% if (getIndex("wanDNS")==0) write("checked"); %> onClick="autoDNSClicked();"><script>dw(ObtainIPAddressAutomatically)</script>
							<br>
							<input type="radio" value="1" name="DNSMode1" <% if (getIndex("wanDNS")==1) write("checked"); %> onClick="autoDNSClicked();"><script>dw(UseFollowingIPAddress)</script>
							</td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(PrimaryDNS)</script> :&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="dns5" value="<% getInfo("wan-dns1"); %>" size="15" maxlength="30">
							<script>if(document.tcpip.dns5.value=="") document.tcpip.dns5.value="0.0.0.0"</script>
							</td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(SecondaryDNS)</script> :&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="dns6" value="<% getInfo("wan-dns2"); %>" size="15" maxlength="30">
							<script>if(document.tcpip.dns6.value=="") document.tcpip.dns6.value="0.0.0.0"</script>
							</td>
							</tr>
							<tr>
							<td width="35%" class="table1"><font size=2>TTL :&nbsp;</td>
							<td width="65%" class="table2"><font size=2>&nbsp;<input type="radio" value="0" name="pppEnTtl2"<% if (getIndex("pppEnTtl")==0) write("checked"); %>><script>dw(Disable)</script>&nbsp;&nbsp;
							<input type="radio" name="pppEnTtl2" value="1"<% if (getIndex("pppEnTtl")==1) write("checked"); %>><script>dw(Enable)</script></td></tr>
							</table>
							</span>
							</blockquote>
							<span id="upptpId" style="display:block" >
							<table border="0" cellspacing="0" cellpadding="0">
							<hr />
							<tr valign="top" align="center" height="40">
							<td width="20"><input name="wanmode" type="radio" value="3" onclick="display();" onmouseover="Pptpover();" onmouseout="Pptpout();"></td>
							<td width="100"><a class="textcolor"><b><script>dw(PPTP)</script></b></a></td>
							<td>:&nbsp;&nbsp;</td>
							<td class="stringcolor" align="left" width="600"><script>dw(detectPPTP)</script></td>
							</tr>
							</table>
							</span>
							<blockquote>
							<span id="pptpId" style="display:none" >
							<table border="0" cellspacing="1" cellpadding=0 width="520">
							<tr>
							<td colspan="2"><input type="radio" value="0" name="pptpIpMode" <% if (getIndex("pptpIpMode")==0) write("checked"); %> onClick="autoIpClicked()"><a class="textcolor">&nbsp;<script>dw(ObtainIPAddressAutomatically)</script></a></td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(HostName)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="pptpHostName" value="<% getInfo("dynIPHostName"); %>" size="15" maxlength="30" class="text" onchange="check(this)"></td>
							</tr>
							<tr>
							<input type=hidden value=<% getInfo("cloneMac"); %> name="macAddrValue2">
							<td width="35%" class="table1"><script>dw(MACAddress)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="pptpmacAddr" size="15" maxlength="12" value="<% getInfo("wanMac"); %>" onchange="checkmac(this)">
							<script>document.write('<input type="button" value="'+showText(CloneMAC)+'" name="Clone" onClick="copyto();" class="button">');</script>
							</td>
							</tr>
							<tr>
							<td colspan="2"><input type="radio" name="pptpIpMode" value="1" <% if (getIndex("pptpIpMode")==1) write("checked"); %> onClick="autoIpClicked()"><a class="textcolor">&nbsp;<script>dw(UseFollowingIPAddress)</script></a></td>
							</tr>
							<tr>
							<td class="table1"><script>dw(IPAddress)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="pptpIPAddr" size="15" maxlength="15" value="<% getInfo("pptpIPAddr"); %>" class="text"></td>
							</tr>
							<tr>
							<td class="table1"><script>dw(SubnetMask)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="pptpIPMaskAddr" size="15" maxlength="15" value="<% getInfo("pptpIPMaskAddr"); %>" class="text"></td>
							</tr>
							<tr>
							<td class="table1"><script>dw(DefaultGateway)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="pptpDfGateway" size="15" maxlength="15" value="<% getInfo("pptpDfGateway"); %>" class="text"></td>
							</tr>
							<tr>
							<td colspan="2">
							<li><a class="textcolor"><b><script>dw(PPTPSettings)</script></b></a></li>
							</td>
							</tr>
							<tr>
							<td class="table1"><script>dw(UserName)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="pptpUserName" size="15" maxlength="64" value="<% getInfo("pptpUserName"); %>" class="text" onchange="check(this)"></td>
							</tr>
							<tr>
							<td class="table1"><script>dw(Password)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="pptpPassword" size="15" maxlength="30" value="<% getInfo("pptpPassword"); %>" class="text" onchange="check(this)"></td>
							</tr>
							<tr>
							<td class="table1"><script>dw(PPTPGateway)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="pptpGateway" size="15" maxlength="30" value="<% getInfo("pptpGateway"); %>" class="text"></td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(ConnectionID)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="pptpConntID" size="15" maxlength="30" value="<% getInfo("pptpConntID"); %>" class="text">&nbsp;(<script>dw(Optional)</script>)</td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(MTU)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="pptpMTU" size="4" maxlength="4" value="<% getInfo("pptpMTU"); %>" class="text"><font size=2>&nbsp;(512&lt;=<script>dw(MTU)</script>&lt;=1492)</td>
							</tr>
							</table>
							</span>
							</blockquote>
							<span id="ul2tpId" style="display:block" >
							<table border="0" cellspacing="0" cellpadding="0">
							<hr />
							<tr valign="top" align="center" height="40">
							<td width="20"><input name="wanmode" type="radio" value="4" onclick="display();" onmouseover="L2tpover();" onmouseout="L2tpout();"></td>
							<td width="100"><a class="textcolor"><b><script>dw(L2TP)</script></b></a></td>
							<td>:&nbsp;&nbsp;</td>
							<td class="stringcolor" align="left" width="600"><script>dw(detectL2TP)</script></td>
							</tr>
							</table>
							</span>
							<blockquote>
							<span id="l2tpId" style="display:none" >
							<table border="0" cellspacing="1" cellpadding=0 width="520">
							<tr>
							<td colspan="2"><input type="radio" value="0" name="L2TPIpMode" <% if (getIndex("L2TPIpMode")==0) write("checked"); %> onClick="autoIpClicked()"><a class="textcolor">&nbsp;<script>dw(ObtainIPAddressAutomatically)</script></a></td>
							</tr>
							<tr>
							<td width="35%" class="table1"><script>dw(HostName)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="l2tpHostName" value="<% getInfo("dynIPHostName"); %>" size="15" maxlength="30" class="text" onchange="check(this)"></td>
							</tr>
							<tr>
							<input type=hidden value=<% getInfo("cloneMac"); %> name="macAddrValue3">
							<td width="35%" class="table1"><script>dw(MACAddress)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="l2tpmacAddr" value="<% getInfo("wanMac"); %>" size="15" maxlength="12" class="text" onchange="checkmac(this)" >
							<script>document.write('<input type="button" value="'+showText(CloneMAC)+'" name="Clone" onClick="copyto();" class="button">');</script>
							</td>
							</tr>
							<tr>
							<td colspan="2"><input type="radio" value="1" name="L2TPIpMode" <% if (getIndex("L2TPIpMode")==1) write("checked"); %> onClick="autoIpClicked()"><a class="textcolor">&nbsp;<script>dw(UseFollowingIPAddress)</script></a></td>
							</tr>
							<tr>
							<td class="table1"><script>dw(IPAddress)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="L2TPIPAddr" size="15" maxlength="15" value="<% getInfo("L2TPIPAddr"); %>" class="text"></td>
							</tr>
							<tr>
							<td class="table1"><script>dw(SubnetMask)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="L2TPMaskAddr" size="15" maxlength="15" value="<% getInfo("L2TPMaskAddr"); %>" class="text"></td>
							</tr>
							<tr>
							<td class="table1"><script>dw(DefaultGateway)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="L2TPDefGateway" size="15" maxlength="15" value="<% getInfo("L2TPDefGateway"); %>" class="text"></td>
							</tr>
							<tr>
							<td colspan="2">
							<li><a class="textcolor"><b><script>dw(L2TPSettings)</script></b></a></li>
							</td>
							</tr>
							<tr>
							<td class="table1"><script>dw(UserName)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="L2TPUserName" size="15" maxlength="64" value="<% getInfo("L2TPUserName"); %>" class="text" onchange="check(this)"></td>
							</tr>
							<tr>
							<td class="table1"><script>dw(Password)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="L2TPPassword" size="15" maxlength="64" value="<% getInfo("L2TPPassword"); %>" class="text" onchange="check(this)"></td>
							</tr>
							<tr>
							<td class="table1"><script>dw(L2TPGateway)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="L2TPGateway" size="15" maxlength="30" value="<% getInfo("L2TPGateway"); %>" class="text"></td>
							</tr>
							<tr>
							<td class="table1"><script>dw(MTU)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="L2TPMTU" size="4" maxlength="4" value="<% getInfo("L2TPMTU"); %>" class="text">&nbsp;(512&lt;=<script>dw(MTU)</script>&lt;=1492)</td>
							</tr>
							</table>
							</span>
							</blockquote>
							<span id="uTelBPId" style="display:block" >
							<table border="0" cellspacing="0" cellpadding="0">
							<hr />
							<tr valign="top" align="center" height="40">
							<td width="20"><input name="wanmode" type="radio" value="5" onclick="display();" onmouseover="Telover();" onmouseout="Telout();"></td>
							<td width="100"><a class="textcolor"><b><script>dw(TelBP)</script></b></a></td>
							<td>:&nbsp;&nbsp;</td>
							<td class="stringcolor" align="left" width="600"><script>dw(TelBPContentMain)</script></td>
							</tr>
							</table>
							</span>
							<blockquote>
							<span id="TelBPId" style="display:none" >
							<table border="0" cellspacing="1" cellpadding=0 width="520">
							<tr>
							<td class="table1" width="40%"><script>dw(UserName)</script>&nbsp;:&nbsp;</td>
							<td class="table2" width="60%">&nbsp;<input type="text" name="telBPUserName" size="15" maxlength="64" value="<% getInfo("telBPUserName"); %>" class="text" onchange="check(this)"></td>
							<tr>
							<td class="table1"><script>dw(Password)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="password" name="telBPPassword" size="15" maxlength="64" value="<% getInfo("telBPPassword"); %>" class="text" onchange="check(this)"></td>
							</tr>
							<tr>
							<td colspan="2"><a class="textcolor"><input type="checkbox" name="telBPEnabled" value="ON" <% if (getIndex("telBPEnabled")) write("checked"); %> onClick="updateState();">&nbsp;<script>dw(AssignLoginServerManually)</script></a></td>
							</tr>
							<tr>
							<td class="table1"><script>dw(ServerIPAddress)</script>&nbsp;:&nbsp;</font></td>
							<td class="table2">&nbsp;<input type="text" name="telBPIPAddr" size="15" maxlength="15" value="<% getInfo("telBPIPAddr"); %>" class="text"></td>
							</tr>
							</tr>
							</table>
							</span>
							</blockquote>
							<span id="apply" style="display:block" >
							<table width="700" border="0" cellpadding="0" cellspacing="0">
							<hr />
							<tr>
							<td align="right">
							<script>
							document.write('<input type=submit value="'+showText(NEXT)+'" name="B1" onclick="return saveChanges();" class="button">');
							document.write('<input type=hidden value="/autoMain2.asp" name="submit-url" id="submitUrl">');
							</script>
							</td>
							</tr>
							</table>
							</span>
							</td>
							</tr>
					</table>
					<script>
					autoDNSClicked();
					autoIpClicked();
					updateState();
					document.getElementsByName("wanmode")[0].checked=false;
					document.getElementsByName("wanmode")[1].checked=false;
					document.getElementsByName("wanmode")[2].checked=false;
					document.getElementsByName("wanmode")[3].checked=false;
					document.getElementsByName("wanmode")[4].checked=false;
					document.getElementsByName("wanmode")[5].checked=false;
					document.write('<input type=hidden value="0" name="wanMode">')
					document.write('<input type=hidden value="fixedIp" name="ipMode">')
					document.write('<input type=hidden value="dnsManual" name="dnsMode">')
					</script>
					</form>
					</td>
					<!-- explain -->
					<td width="380" valign="top" bgcolor="#999999">
						<table width="380" border="0" align="right" cellpadding="0" cellspacing="10">
							<tr>
								<td height="400" valign="top">
									<table width="360"  height="400 "border="0" cellpadding="5" cellspacing="0" bgcolor="#999999">
										<tr><td height="16" bgcolor="#A5031A"><span class="style2">Help</span></td></tr>
										<tr><td height="384" valign="top" bgcolor="#999999"></td></tr>
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
<script>
	var internet_sta = "<% getInfo("sta-current"); %>"
	top.sw_current_sta(internet_sta);
</script>
</html>
