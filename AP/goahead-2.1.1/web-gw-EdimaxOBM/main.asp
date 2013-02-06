<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<link rel="stylesheet" href="edimax.css">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="file/statustool-n.var"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<title></title>
<script>
	var internet_sta = "<% getInfo("sta-current"); %>"
	var cloud_sta = "<% getInfo("cloud-sta"); %>"
</script>
<body onLoad="top.startclock()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(0,0);</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF">
						<br>
						<!-- system -->
						<table width="600" border="1" cellpadding="5" cellspacing="0">
							<tr>
								<td align="center" width="600" height="25px" bgcolor="#666666"><span class="style13"><script>dw(system)</script></span></td>
							</tr>
							<tr>
								<td class="style14" bgcolor="#FFFFFF">
									<table width="590" cellpadding="0" cellspacing="0">
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(model)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff"><% getInfo("name"); %></td>
										</tr>
										<tr>
											<td valign="center" align="right" bgcolor="#ffffff" height="20">&nbsp;<script>dw(upTime)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff"><a id="showuptime"></a></td>
										</tr>
										<tr>
											<td valign="center" align="right" bgcolor="#ffffff" height="20">&nbsp;<script>dw(verHardware)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff">Rev. A</td>
										</tr>
										<tr>
											<td valign="center" align="right" bgcolor="#ffffff" height="20">&nbsp;<script>dw(verBootCode)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff">1.0</td>
										</tr>
										<tr>
											<td valign="center" align="right" bgcolor="#ffffff" height="20">&nbsp;<script>dw(verRuntimeCode)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff"><% getInfo("fwVersion"); %></td>
										</tr>
									</table>
								</td>
							</tr>
						</table>
						<input name="tmp" type="hidden" value="0">
						<br />
						<!-- internet connection -->
						<table width="600" border="1" cellpadding="5" cellspacing="0">
							<form name="internetSta">
								<tr>
									<td align="center" width="600" height="25px" bgcolor="#666666"><span class="style13"><script>dw(statInternet)</script></span></td>
								</tr>
								<tr>
									<td class="style14" bgcolor="#FFFFFF">
										<table width="590" cellpadding="0" cellspacing="0">
											<tr>
												<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(attainIpPtc)</script> :&nbsp;&nbsp;</td>
												<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff" id="sta_current"></td>
												<input type="hidden" name="ip" value="<% getInfo("sta-ip"); %>">
												<input type="hidden" name="dns1" value="<% getInfo("sta-dns1"); %>">
												<input type="hidden" name="dns2" value="<% getInfo("sta-dns2"); %>">
												<input type="hidden" name="dns3" value="<% getInfo("sta-dns3"); %>">
												<input type="hidden" name="gw" value="<% getInfo("sta-gateway"); %>">
											</tr>
											<tr>
												<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(ipAddress)</script> :&nbsp;&nbsp;</td>
												<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff"><% getInfo("sta-ip"); %></td>
											</tr>
											<tr>
												<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(subnetMask)</script> :&nbsp;&nbsp;</td>
												<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff"><% getInfo("sta-mask"); %></td>
											</tr>
											<tr>
												<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(defaultGateway)</script> :&nbsp;&nbsp;</td>
												<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff">
													<script>
														var check = document.internetSta.ip.value;
														if ( check != "")
															document.write('<font size=2>'+document.internetSta.gw.value+'</font>');
														else
															document.write('<font size=2></font>');
													</script>
												</td>
											</tr>
											<tr>
												<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(macAddress)</script> :&nbsp;&nbsp;</td>
												<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff"><% getInfo("sta-mac"); %></td>
											</tr>
											<tr>
												<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(primaryDNS)</script> :&nbsp;&nbsp;</td>
												<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff">
													<script>
														if ( check != "") document.write(document.internetSta.dns1.value);
													</script>
												</td>
											</tr>
											<tr>
												<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(secondaryDNS)</script> :&nbsp;&nbsp;</td>
												<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff">
													<script>
														if ( check != "") document.write(document.internetSta.dns2.value);
													</script>
												</td>
											</tr>
											<tr>
												<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20">Third DNS :&nbsp;&nbsp;</td>
												<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff">
													<script>
														if ( check != "") document.write(document.internetSta.dns3.value);
													</script>
												</td>
											</tr>
										</table>
									</td>
								</tr>
							</form>
						</table>
						<br />
						<!-- wireless Configuration -->
						<form name="DevSta">
						<table width="600" border="1" cellpadding="5" cellspacing="0">
							<tr>
								<td align="center" width="600" height="25px" bgcolor="#666666"><span class="style13"><script>dw(cfgWireless)</script></span></td>
							</tr>
							<tr>
								<td class="style14" bgcolor="#FFFFFF">
									<table width="590" cellpadding="0" cellspacing="0">
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(mode)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff">
												<script>
													<%  write("apMode = "+getIndex("apMode")+";");%>
													var modeTbl = new Array("AP","Station-Ad Hoc","Station-Infrastructure","AP Bridge-Point to Point","AP Bridge-Point to Multi-Point","AP Bridge-WDS","Universal Repeater");
													for (i=0 ; i<7 ; i++)
													{
														if ( i==apMode )
															document.write(modeTbl[i]);
													}
												</script>
											</td>
										</tr>
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(essid)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff" id="mainssid"></td>
											<script>document.getElementById('mainssid').innerHTML = "<% getInfo("ssid_drv"); %>"</script>
										</tr>
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(channelNum)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff"><% getInfo("channel_drv"); %></td>
										</tr>
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(security)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff">
												<script>
													<%  write("encrypt = "+getIndex("encrypt")+";");%>
													<%  write("wep = "+getIndex("wep")+";");%>
													var modeTbl = new Array(showText(disable),"WEP","WPA pre-shared key","WPA RADIUS");
													for (i=0 ; i<6 ; i++)
													{
														if ( i == encrypt )
															document.write(modeTbl[i] +"</td>");
													}
													document.write('</tr>');
												</script>
									</table>
								</td>
							</tr>
						</table>
						<br />
						<!-- LAN Configuration -->
						<table width="600" border="1" cellpadding="5" cellspacing="0">
							<tr>
								<td align="center" width="600" height="25px" bgcolor="#666666"><span class="style13"><script>dw(cfgLAN)</script></span></td>
							</tr>
							<tr>
								<td class="style14" bgcolor="#FFFFFF">
									<table width="590" cellpadding="0" cellspacing="0">
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(ipAddress)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff"><% getInfo("ip"); %></td>
										</tr>
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(subnetMask)</script> :&nbsp;&nbsp;</td>
										<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff"><% getInfo("mask"); %></td>
										</tr>
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(DhcpServer)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff">
												<%
													choice = getIndex("dhcp-current");
													if ( choice == 0 ) write( "<script>dw(disable)</script>" );
													if ( choice == 2 ) write( "<script>dw(enable)</script>" );
												%>
											</td>
										</tr>
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20"><script>dw(macAddress)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff"><% getInfo("hwaddr"); %></td>
										</tr>
									</table>
								</td>
							</tr>
						</table>
						</form>
						<br>
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
													<span class="style13"><script>dw(system)</script></span><br />
													<span class="style21"><script>dw(asphomesyshelp)</script></span>
												</span>
												<br><br>
												<span class="style1">
													<span class="style13"><script>dw(statInternet)</script></span><br />
													<span class="style21"><script>dw(asphomeInternethelp)</script></span>
												</span>
												<br><br>
												<span class="style1">
													<span class="style13"><script>dw(cfgWireless)</script></span><br />
													<span class="style21"><script>dw(asphomeWlConfighelp)</script></span>
												</span>
												<br><br>
												<span class="style1">
													<span class="style13"><script>dw(cfgLAN)</script></span><br />
													<span class="style21"><script>dw(asphomeLANConfighelp)</script></span>
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
	document.getElementById('sta_current').innerHTML = internet_sta;
	top.sw_current_sta(internet_sta);
	top.cloud_current_sta(cloud_sta);
</script>