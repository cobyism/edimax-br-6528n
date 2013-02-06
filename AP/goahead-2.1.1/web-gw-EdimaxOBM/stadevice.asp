<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<link rel="stylesheet" href="edimax.css">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="file/statustool-n.var"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<script type="text/javascript" src="file/ezview-n.var"></script>
<script type="text/javascript" src="file/multilanguage.var"></script>
<title></title>
</head>
<script>var cloud_sta = "<% getInfo("cloud-sta"); %>"</script>
<body onLoad="top.startclock()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(4,2)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF">
						<br>
						<table width="600" border="1" cellspacing="1" cellpadding="0">
							<tr>
								<td valign="top" align="center" bgcolor="#FFFFFF">
									<form name="DevSta">
										<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
											<tr>
											<td colspan="2" align="center" width="600" height="25px" bgcolor="#666666" class="style13"><script>dw(cfgWireless)</script></td>
											</tr>
											<tr><td height="10"></td></tr>
											<tr>
											<td align="right" class="style14">&nbsp;<script>dw(mode)</script> :&nbsp;&nbsp;</td>
											<td align="left" width="50%" class="style14">&nbsp;
											<script>
											<%  write("apMode = "+getIndex("apMode")+";");%>
											var modeTbl = new Array("AP","Station-Ad Hoc","Station-Infrastructure","AP Bridge-Point to Point","AP Bridge-Point to Multi-Point","AP Bridge-WDS","Universal Repeater");
											for (i=0 ; i<7 ; i++) {
											if ( i==apMode )
											document.write(modeTbl[i]);
											}
											</script>
											</td>
											</tr>
											<tr>
											<td align="right" class="style14">&nbsp;<script>dw(essid)</script> :&nbsp;&nbsp;</td>
											<script>document.write("<td align=\"left\" class=style14>&nbsp;<% getInfo("ssid_drv"); %></td></tr>");</script>
											</tr>
											<tr>
											<td align="right" class="style14">&nbsp;<script>dw(channelNum)</script> :&nbsp;&nbsp;</td>
											<td align="left" class="style14">&nbsp;<% getInfo("channel_drv"); %></td></tr>
											<tr>
											<td align="right" class="style14">&nbsp;<script>dw(security)</script> :&nbsp;&nbsp;</td>
											<td align="left" class="style14">&nbsp;
											<script>
											<%  write("encrypt = "+getIndex("encrypt")+";");%>
											<%  write("wep = "+getIndex("wep")+";");%>
											var modeTbl = new Array(showText(disable),"WEP","WPA pre-shared key","WPA RADIUS");
											for (i=0 ; i<6 ; i++) {
												if ( i == encrypt )
													document.write(modeTbl[i] +"</td>");
											}
											document.write('</tr>');
											</script>
											<tr><td height="10"></td></tr>
											<td colspan="2" align="center" width="600" height="25px" bgcolor="#666666" class="style13"><script>dw(cfgLAN)</script></td>
											<tr><td height="10"></td></tr>
											</tr>
											<tr>
											<td align="right" class="style14">&nbsp;<script>dw(ipAddress)</script> :&nbsp;&nbsp;</td>
											<td align="left" class="style14">&nbsp;<% getInfo("ip"); %></td>
											</tr>
											<tr>
											<td align="right" class="style14">&nbsp;<script>dw(subnetMask)</script> :&nbsp;&nbsp;</td>
											<td align="left" class="style14">&nbsp;<% getInfo("mask"); %></td>
											</tr>
											<tr>
											<td align="right" class="style14">&nbsp;<script>dw(DhcpServer)</script> :&nbsp;&nbsp;</td>
											<td align="left" class="style14">&nbsp;<%  choice = getIndex("dhcp-current");
											if ( choice == 0 ) write( "<script>dw(disable)</script>" );
											if ( choice == 2 ) write( "<script>dw(enable)</script>" );
											%></td>
											</tr>
											<tr>
											<td align="right" class="style14">&nbsp;<script>dw(macAddress)</script> :&nbsp;&nbsp;</td>
											<td align="left" class="style14">&nbsp;<% getInfo("hwaddr"); %></td>
											</tr>
										</table>
									</form>
									</div>
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
											<td height="384" valign="top" bgcolor="#999999">
												<span class="style1">
													<span class="style13"><script>dw(statDevice)</script></span><br />
													<span class="style21"><script>dw(statDeviceDoc)</script></span>
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
	var internet_sta = "<% getInfo("sta-current"); %>"
	top.sw_current_sta(internet_sta);
	top.cloud_current_sta(cloud_sta);
</script>
