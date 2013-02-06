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
<script>var internet_sta = "<% getInfo("sta-current"); %>"
var cloud_sta = "<% getInfo("cloud-sta"); %>"</script>
</head>
<body onLoad="top.startclock()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(4,1)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF">
						<table width="600" border="1" cellspacing="1" cellpadding="0">
							<tr>
								<td align="center" width="600" height="25px" bgcolor="#666666"><span class="style13">&nbsp;<script>dw(statInternet)</script></span></td><br>
							</tr>
							<tr>
								<td valign="top" align="center" bgcolor="#FFFFFF">
									<form name="internetSta">
										<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
											<tr>
												<td align="right" width="50%" class="style14"><script>dw(attainIpPtc)</script> :&nbsp;&nbsp;</td>
												<td align="left" class="style14" id="sta_current"></td>
												<input type="hidden" name="ip" value="<% getInfo("sta-ip"); %>">
												<input type="hidden" name="dns1" value="<% getInfo("sta-dns1"); %>">
												<input type="hidden" name="dns2" value="<% getInfo("sta-dns2"); %>">
												<input type="hidden" name="dns3" value="<% getInfo("sta-dns3"); %>">
												<input type="hidden" name="gw" value="<% getInfo("sta-gateway"); %>">
											</tr>
											<tr>
												<td align="right" class="style14"><script>dw(ipAddress)</script> :&nbsp;&nbsp;</td>
												<td align="left" class="style14">&nbsp;<% getInfo("sta-ip"); %></td>
											</tr>
											<tr>
												<td align="right" class="style14"><script>dw(subnetMask)</script> :&nbsp;&nbsp;</td>
												<td align="left" class="style14">&nbsp;<% getInfo("sta-mask"); %></td>
											</tr>
											<tr>
												<td align="right" class="style14"><script>dw(defaultGateway)</script> :&nbsp;&nbsp;</td>
												<td align="left" class="style14">
												<script>
												var check = document.internetSta.ip.value;
												if ( check != "")
													document.write('&nbsp;'+document.internetSta.gw.value);
												else
													document.write('&nbsp;</td>');
												</script>
											</tr>
											<tr>
												<td align="right" class="style14"><script>dw(macAddress)</script> :&nbsp;&nbsp;</td>
												<td align="left" class="style14">&nbsp;<% getInfo("sta-mac"); %></td>
											</tr>
											<tr>
												<td align="right" class="style14"><script>dw(primaryDNS)</script> :&nbsp;&nbsp;</td>
												<td align="left" class="style14">
												<script>
												if ( check != "")
													document.write('&nbsp;'+document.internetSta.dns1.value);
												else
													document.write('&nbsp;');
												</script>
												</td>
											</tr>
											<tr>
												<td align="right" class="style14"><script>dw(secondaryDNS)</script> :&nbsp;&nbsp;</td>
												<td align="left" class="style14">
												<script>
												if ( check != "")
												document.write('&nbsp;'+document.internetSta.dns2.value);
												else
												document.write('&nbsp;');
												</script>
												</td>
											</tr>
											<tr>
												<td align="right" class="style14">Third DNS :&nbsp;&nbsp;</td>
												<td align="left" class="style14">
												<script>
												if ( check != "")
													document.write('&nbsp;'+document.internetSta.dns3.value);
												else
													document.write('&nbsp;');
												</script>
												</td>
											</tr>
										</table>
									</form>
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
													<span class="style13"><script>dw(statInternet)</script></span><br />
													<span class="style21"><script>dw(statInternetDoc)</script></span>
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
