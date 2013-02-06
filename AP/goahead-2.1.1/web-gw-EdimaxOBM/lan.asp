<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" href="edimax.css">
<script type="text/javascript" src="file/multilanguage.var"></script>
<script language ='javascript' src ="file/fwwl-n.var"></script>
<script type="text/javascript" src="file/statustool-n.var"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<script type="text/javascript" src="file/ezview-n.var"></script>
<script type="text/javascript" src="file/javascript.js"></script>
<% language=javascript %>
<script>
var cloud_sta = "<% getInfo("cloud-sta"); %>"
/* function for main */
	var comment = new Array( new Array("com00","com01"))
	function com_sw(cnum,com)
	{
		if (com == 1)
		{
			document.getElementById(comment[cnum][0]).style.display = "none"
			document.getElementById(comment[cnum][1]).style.display = "block"
		}
		else
		{
			document.getElementById(comment[cnum][0]).style.display = "block"
			document.getElementById(comment[cnum][1]).style.display = "none"
		}
	}
/* lan function*/
	<%  write("lanipChanged = "+getIndex("lanipChanged"));%>
	/*------------------------DHCP RELAY---------------------------*/
	<%  write("var show_DHCP ="+getIndex("dhcp"));%>
	/*--------------------------------------------------------------------------*/
	function checkValue(str)
	{
		for (var i=0; i<str.length; i++)
		{
			if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
				(str.charAt(i) >= 'A' && str.charAt(i) <= 'Z') ||
				(str.charAt(i) >= 'a' && str.charAt(i) <= 'z') ||
				(str.charAt(i) == '_')|| (str.charAt(i) == '-')||
				(str.charAt(i) == '.'))
				continue;
			return 0;
		}
		return 1;
	}
	var timeValue = new Array("1800","3600","7200","43200","86400","172800","604800","1209600","315360000");
	var timeName = new Array(showText(halfh),showText(oneh),showText(twohs),showText(halfd),showText(oned),showText(twods),showText(onewk),showText(twowks),showText(forever));
	var initialDhcp;
	function dhcpChange(index)
	{
		if ( index == 1 )
		{
			document.tcpip.dhcpRangeStart.disabled = false;
			document.tcpip.dhcpRangeEnd.disabled = false;
			document.tcpip.DomainName.disabled = false;
			document.tcpip.leaseTime.disabled = false;
//			document.tcpip.dns1.disabled = false;
//			document.tcpip.dns2.disabled = false;
			document.tcpip.dhcpGW.disabled = false;
		}
		else
		{
			document.tcpip.dhcpRangeStart.disabled = true;
			document.tcpip.dhcpRangeEnd.disabled = true;
			document.tcpip.DomainName.disabled = true;
			document.tcpip.leaseTime.disabled = true;
//			document.tcpip.dns1.disabled = true;
//			document.tcpip.dns2.disabled = true;
			document.tcpip.dhcpGW.disabled = true;
		}
	}
	function resetClick()
	{
		dhcpChange( initialDhcp );
		document.tcpip.reset;
	}
	function checkClientRange(start,end)
	{
		start_d = getDigit(start,4);
		start_d += getDigit(start,3)*256;
		start_d += getDigit(start,2)*256;
		start_d += getDigit(start,1)*256;
		end_d = getDigit(end,4);
		end_d += getDigit(end,3)*256;
		end_d += getDigit(end,2)*256;
		end_d += getDigit(end,1)*256;
		if ( start_d < end_d )
			return true;
		return false;
	}
	function saveChanges()
	{
		strIp = document.tcpip.ip;
		strMask = document.tcpip.mask;
		strStartIp = document.tcpip.dhcpRangeStart;
		strEndIp = document.tcpip.dhcpRangeEnd;
		if ( !ipRule( strIp, showText(ipAddr11), "ip", 1)) return false;
		if(!maskRule(strMask, showText(SubnetMask), 1)) return false;
		if ( document.tcpip.dhcp.selectedIndex == 1)
		{
			//********** dhcp start ip **********
			if ( strStartIp.value == strIp.value)
			{
				alert(showText(lanAlert));
				setFocus(strStartIp);
				return false;
			}
			if ( !ipRule( strStartIp, showText(dhcpStart1), "ip", 1)) return false;
			if ( !checkSubnet(strIp.value, strMask.value, strStartIp.value))
			{
				alert(showText(lanAlert2));
				setFocus(strStartIp);
				return false;
			}
			//********** dhcp end ip **********
			if ( !ipRule( strEndIp, showText(dhcpEnd1), "ip", 1)) return false;
			if ( !checkSubnet(strIp.value, strMask.value, strEndIp.value))
			{
				alert(showText(lanAlert3));
				setFocus(strEndIp);
				return false;
			}
			if ( !checkClientRange(strStartIp.value, strEndIp.value) )
			{
				alert(showText(lanAlert4));
				setFocus(strStartIp);
				return false;
			}
//			if (document.tcpip.dns1.value=="") document.tcpip.dns1.value="0.0.0.0";
//			if (document.tcpip.dns2.value=="") document.tcpip.dns2.value="0.0.0.0";
			if ( document.tcpip.dhcpGW.value=="" ) document.tcpip.dhcpGW.value="0.0.0.0";
//			if ( !ipRule( document.tcpip.dns1,showText(PrimaryDNS), "gw", 1)) return false;
//			if ( !ipRule( document.tcpip.dns2,showText(SecondaryDNS), "gw", 1)) return false;
			if ( !ipRule( document.tcpip.dhcpGW, showText(gateAddr)+" ", "ip", 1)) return false;
		}
		if (checkValue(document.tcpip.DomainName.value)==0)
		{
			alert(showText(lanAlert5));
			setFocus(document.tcpip.DomainName);
			return false;
		}
		if (strIp.value!=strIp.defaultValue)
		{
			alert(showText(lanAlert6));
			document.tcpip.ipChanged.value = 1;
		}
		document.tcpip.submit();
		return true;
	}
	function addClick() {
		if ( !macRule(document.formSDHCPAdd.mac,'MAC address', 0))
			return false;
		for(i=1; i<=document.getElementsByName("smacnum")[0].value; i++)
		{
			if( document.formSDHCPAdd.mac.value.toLowerCase() == document.getElementsByName("smac"+i)[0].value.toLowerCase() )
			{
				alert(showText(macthesame));
				return false;
			}
			if( document.formSDHCPAdd.ip.value == document.getElementsByName("sip"+i)[0].value )
			{
				alert("IP address can not be the same");
				return false;
			}
		}
		if( document.formSDHCPAdd.mac.value =="000000000000")
		{
			alert(showText(maccnatbe));
			return false;
		}
		if( document.formSDHCPAdd.mac.value =="ffffffffffff" || document.formSDHCPAdd.mac.value =="FFFFFFFFFFFF")
		{
			alert("MAC can not be FF:FF:FF:FF:FF:FF!");
			return false;
		}
		if( document.formSDHCPAdd.ip.value =="0.0.0.0")
		{
			alert(showText(ipcnatbe));
			return false;
		}
		if( document.formSDHCPAdd.ip.value == document.tcpip.ip.value)
		{
			alert("Static DHCP Leases ip address and lan ip are the same!");
			return false;
		}
		if ( !checkIpAddr(document.formSDHCPAdd.ip, 'IP address'))
			return false;
		return true;
	}
	function checkip()
	{
		if (document.tcpip.ip.value =="0.0.0.0")
		{
			alert(showText(ipcnatbe));
			document.tcpip.ip.value = "";
			return false;
		}
	}
	function disableDelButton()
	{
		document.formSDHCPDel.deleteSelDhcpMac.disabled = true;
		document.formSDHCPDel.deleteAllDhcpMac.disabled = true;
	}
	function goToWeb() {
		if (document.formSDHCPAdd.SDHCPEnabled.checked==true)
			document.formSDHCPEnabled.SDHCPEnabled.value="ON";
		document.formSDHCPEnabled.submit();
	}
	/*
	function sdhcp()
	{
		if (document.formSDHCPAdd.SDHCPEnabled.checked==true)
		{
			document.formSDHCPAdd.mac.disabled=false
			document.formSDHCPAdd.ip.disabled=false
			document.formSDHCPAdd.addSDHCPMac.disabled=false
			document.formSDHCPAdd.reset.disabled=false
		}
		else
		{
			document.formSDHCPAdd.mac.disabled=true
			document.formSDHCPAdd.ip.disabled=true
			document.formSDHCPAdd.addSDHCPMac.disabled=true
			document.formSDHCPAdd.reset.disabled=true
		}
	}
	*/
</script>
<title></title>
</head>
<body onLoad="top.startclock()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(2,2)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF"><br>
						<form action=/goform/formTcpipSetup method=POST name="tcpip">
						<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
						<tr>
						<td colspan="2" align="center" height="25px" bgcolor="#666666" class="style13"><script>dw(lanIP)</script></td>
						</tr>
						<tr>
						<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(IPAddr)</script>:&nbsp;</td>
						<td width="400" height="20" valign="middle" align="left" class="style1">
							<input type="text" name="ip" size="15" maxlength="15" value=<% getInfo("ip-rom"); %> onChange="return checkip()">
						</td>
						</tr>
						<tr>
						<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(subnetMask)</script>:&nbsp;</td>
						<td width="400" height="20" valign="middle" align="left" class="style1">
							<input type="text" name="mask" size="15" maxlength="15" value="<% getInfo("mask-rom"); %>">
						</td>
						</tr>
						<tr>
						<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(spanTree)</script>:&nbsp;</td>
						<td width="400" height="20" valign="middle" align="left" class="style1">
						<select size="1" name="stp">
							<%
								choice = getIndex("stp");
								if ( choice == 0 )
								{
									write( "<option selected value=\"0\"><script>dw(disabled)</script></option>" );
									write( "<option value=\"1\"><script>dw(enabled)</script></option>" );
								}
								else
								{
									write( "<option value=\"0\"><script>dw(disabled)</script></option>" );
									write( "<option selected value=\"1\"><script>dw(enabled)</script></option>" );
								}
							%>
						</select>
						</td>
						</tr>
						<tr>
						<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(DHCPServer)</script>:&nbsp;</td>
						<td width="400" height="20" valign="middle" align="left" class="style1">
						<select size="1" name="dhcp" onChange="dhcpChange(document.tcpip.dhcp.selectedIndex)">
							<%
								choice = getIndex("dhcp");
								if ( choice == 0 )
								{
									write( "<option selected value=\"0\"><script>dw(disabled)</script></option>" );
									write( "<option value=\"2\"><script>dw(enabled)</script></option>" );
									write( "<script>initialDhcp=0</script>");
								}
								if ( choice == 2 )
								{
									write( "<option value=\"0\"><script>dw(disabled)</script></option>" );
									write( "<option selected value=\"2\"><script>dw(enabled)</script></option>" );
									write( "<script>initialDhcp=1</script>");
								}
							%>
						</select>
						</td>
						</tr>
						<tr>
						<input type="hidden" name="leaseTimeGet" value="<% getInfo("leaseTime"); %>">
						<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(leaseTime)</script>:&nbsp;</td>
						<td width="400" height="20" valign="middle" align="left" class="style1">
						<select name="leaseTime">
						<script language="javascript">
							for(i=0;i<9;i++)
							{
								if(timeValue[i]==document.tcpip.leaseTimeGet.value)
									document.write("<option value=\""+timeValue[i]+"\" selected>"+timeName[i]+"</option>");
								else
									document.write("<option value=\""+timeValue[i]+"\">"+timeName[i]+"</option>");
							}
						</script>
						</select>
						</td>
						</tr>
						<tr>
						<td colspan="2" align="center" height="25px" bgcolor="#666666" class="style13"><script>dw(DHCPServer)</script></td>
						</tr>
						<tr>
						<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(startIP)</script>:&nbsp;</td>
						<td width="400" height="20" valign="middle" align="left" class="style1">
							<input type="text" name="dhcpRangeStart" size="15" maxlength="15" value="<% getInfo("dhcpRangeStart"); %>">
						</td>
						</tr>
						<tr>
						<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(endIP)</script>:&nbsp;</td>
						<td width="400" height="20" valign="middle" align="left" class="style1">
							<input type="text" name="dhcpRangeEnd" size="15" maxlength="15" value="<% getInfo("dhcpRangeEnd"); %>">
						</td>
						</tr>
						<tr>
						<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(domainName)</script>:&nbsp;</td>
						<td width="400" height="20" valign="middle" align="left" class="style1">
							<input type="text" name="DomainName" size="20" maxlength="30" value="<% getInfo("DomainName"); %>">
						</td>
						</tr>
						<!--tr>
						<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr1)</script>:&nbsp;</td>
						<td width="400" height="20" valign="middle" align="left" class="style1">
							<input type="text" name="dns1" size="15" maxlength="15" value="< getInfo("wan-dns1"); >" class="text">
						</td>
						</tr>
						<tr>
						<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(dnsAddr2)</script>:&nbsp;</td>
						<td width="400" height="20" valign="middle" align="left" class="style1">
							<input type="text" name="dns2" size="15" maxlength="15" value="< getInfo("wan-dns2");>" class="text">
						</td>
						</tr -->
						<tr>
						<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(gateAddr)</script> :&nbsp;</td>
						<td width="400" height="20" valign="middle" align="left" class="style1">
							<input type="text" name="dhcpGW" size="15" maxlength="15" value="<% getInfo("dhcpGW"); %>">
						</td>
						</tr>
						<script>
							dhcpChange(document.tcpip.dhcp.selectedIndex);
						</script>
						<input type=hidden value="/lan.asp" name="submit-url">
						<input type=hidden value="" name="ipChanged">
						</table>
						</form>
						<form action=/goform/formSDHCP method=POST name="formSDHCPEnabled">
						<input type="hidden" value="Add" name="addSDHCPMac">
						<input type="hidden" value="" name="SDHCPEnabled">
						<input type="hidden" value="/lan.asp" name="submit-url">
						</form>
						<form action=/goform/formSDHCP method=POST name="formSDHCPDel">
						<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
							<tr>
								<td colspan="2" align="center" height="25px" bgcolor="#666666" class="style13"><script>dw(staticdhcp1)</script></td>
							</tr>
							<tr>
								<td colspan="2" height="20" valign="middle" align="left" class="style14"><script>dw(staticdhcp2)</script></td>
							</tr>
						</table>
						<table width="600" border="1" cellpadding="2" cellspacing="0" align="center" bgcolor="#FFFFFF" class="style14">
						<tr class="style13">
						<td align="center" height="20" bgcolor="#666666" width="10%">NO.</td>
						<td align="center" height="20" bgcolor="#666666" width="45%"><script>dw(macAddr)</script></td>
						<td align="center" height="20" bgcolor="#666666" width="35%"><script>dw(ipAddr11)</script></td>
						<td align="center" height="20" bgcolor="#666666" width="15%"><script>dw(select1)</script></td>
						</tr>
						<% SDHCPList(); %>
						</table>
						<br>
						<table width="600" border="0" cellpadding="0" cellspacing="0" align="center">
						<tr>
						<td align="right">
						<script>
							document.write('<input type="submit" value="'+showText(deleteSelected)+'" name="deleteSelDhcpMac" onClick="return deleteClick()" class="btnsize">');
							document.write('<input type="submit" value="'+showText(deleteAll)+'" name="deleteAllDhcpMac" onClick="return deleteAllClick()" class="btnsize">');
							document.write('<input type="reset" value="'+showText(reset)+'" name="reset" class="btnsize">')
						</script>
						</td>
						</tr>
						<input type="hidden" value="/lan.asp" name="submit-url">
						<script>
							<%
								entryNum = getIndex("SDHCPNum");
								if ( entryNum == 0 ) {
									write( "disableDelButton();" );
								}
							%>
						 </script>
						</table>
						</form>
						<form action=/goform/formSDHCP method=POST name="formSDHCPAdd">
						<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
						<tr>
						<td width="600" height="20" valign="middle" align="left" class="style14">
						<input type="checkbox" name="SDHCPEnabled" value="ON" <% if (getIndex("SDHCPEnabled")) write("checked"); %> onclick="goToWeb()">&nbsp;&nbsp;
						<script>dw(enableStaticdhcp)</script>
						</td>
						</tr>
						</table>
						<br>
						<table width="600" border="0" cellpadding="0" cellspacing="0" align="center">
						<tr>
						<td width="5%" align="center" height="20" bgcolor="#666666" class="style13">
						<script>dw(new1)</script>
						<input type="hidden" name="tiny_idx" value="0">
						</td>
						<td width="30%" height="20" valign="middle" align="center" class="style14">
						<script>dw(macAddr)</script>: <input type="text" name="mac" size="15" maxlength="12">
						</td>
						<td width="30%" height="20" valign="middle" align="center" class="style14">
						<script>dw(ipAddr11)</script>: <input type="text" name="ip" size="16" maxlength="16">
						</td>
						<td width="15%" height="20" valign="middle" align="center" class="style14">
						<script>
							document.write('<input type="submit" value="'+showText(add1)+'" name="addSDHCPMac" onClick="return addClick()" class="btnsize">')
							document.write('<input type="reset" value="'+showText(clear1)+'" name="reset" onclick="document.formSDHCPAdd.reset" class="btnsize">')
						</script>
						<input type="hidden" value="/lan.asp" name="submit-url">
						</td>
						</tr>
						</table>
						</form>
						<br>
						<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
						<tr>
						<td colspan="2" height="20" valign="middle" align="right" class="style1">
						<script>
							document.write('<input type=submit value="'+showText(apply1)+'" name="B1" style ="width:100px" onclick="return saveChanges()">');
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
									<table width="360"  height="400 "border="0" cellpadding="5" cellspacing="0" bgcolor="#999999">
										<tr>
											<td height="16" bgcolor="#A5031A"><span class="style2">Help</span></td>
										</tr>
										<tr>
											<td height="384" valign="top" bgcolor="#999999"></td>
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
	var internet_sta = "<% getInfo("sta-current"); %>"
	top.sw_current_sta(internet_sta);
	top.cloud_current_sta(cloud_sta);
//	sdhcp()
</script>
