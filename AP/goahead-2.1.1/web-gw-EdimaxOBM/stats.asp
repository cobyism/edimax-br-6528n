<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<link rel="stylesheet" href="edimax.css">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="file/statustool-n.var"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<script type="text/javascript" src="file/ezview-n.var"></script>
<title></title>
</head>
<script>var cloud_sta = "<% getInfo("cloud-sta"); %>"</script>
<body onLoad="top.startclock()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(4,6)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF">
					<br>
						<table width="600" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td colspan="3" valign="top" align="center">
									<table width="600" border="1" cellpadding="2" cellspacing="2" align="center">
									<script>
									<%write("wlDev = "+getIndex("wlDev")+";");%>
									if (wlDev!=0) {
										document.write('<tr>');
										document.write('<td width="30%" bgcolor="#666666" class="style13" align="center" rowspan="2">' + showText(wirelessLAN) + '</td>');
										document.write('<td width="30%" bgcolor="#FFFFFF" class="style14" align="center"><i>' + showText(sentPackets) + '</i></td>');
										document.write('<td width="20%" bgcolor="#FFFFFF" class="style14" align="center"><% getInfo("wlanTxPacketNum"); %></td>');
										document.write('</tr>');
										document.write('<tr>');
										document.write('<td bgcolor="#FFFFFF" class="style14" align="center"><i>' + showText(receivedPackets) + '</i></td>');
										document.write('<td bgcolor="#FFFFFF" class="style14" align="center"><% getInfo("wlanRxPacketNum"); %></td>');
										document.write('</tr>');
									}
									</script>
									<tr>
									<td bgcolor="#666666" class="style13" align="center" rowspan="2"><script>dw(ethernetLAN)</script></td>
									<td bgcolor="#FFFFFF" class="style14" align="center"><i><script>dw(sentPackets)</script></i></td>
									<td bgcolor="#FFFFFF" class="style14" align="center"><% getInfo("lanTxPacketNum"); %></td>
									</tr>
									<tr>
									<td bgcolor="#FFFFFF" class="style14" align="center"><i><script>dw(receivedPackets)</script></i></td>
									<td bgcolor="#FFFFFF" class="style14" align="center"><% getInfo("lanRxPacketNum"); %></td>
									</tr>
									<script>
									<%  write(" apMode = "+getIndex("apRouterSwitch")+";");%>
									if ( apMode !=1) {
										document.write('<tr><td bgcolor="#666666" class="style13" align="center" rowspan="2">'+showText(ethernetWAN)+'</td>');
										document.write('<td bgcolor="#FFFFFF" class="style14" align="center"><i>'+showText(sentPackets)+'</i></td>');
										document.write('<td bgcolor="#FFFFFF" class="style14" align="center"><% getInfo("wanTxPacketNum"); %></td></tr>');
										document.write('<tr><td bgcolor="#FFFFFF" class="style14" align="center"><i>'+showText(receivedPackets)+'</i></td>');
										document.write('<td bgcolor="#FFFFFF" class="style14" align="center"><% getInfo("wanRxPacketNum"); %></td></tr>');
									}
									</script>
									<tr>
									<td bgcolor="#666666" class="style13" align="center" rowspan="2">Running Time</td>
									<td bgcolor="#FFFFFF" class="style14" align="center"><i><a name="showuptime2"></a></i></td>
									<td bgcolor="#FFFFFF" class="style14" align="center" valign="middle">
									<form action=/goform/formReboot method=POST name="RebootForm">
									<table border="0" cellspacing="0" cellpadding="0" align="center" valign="middle">
									<tr><td align="center" valign="middle">
									<script>buffer='<input class="style1" type=submit value="'+showText(restart)+'">';document.write(buffer);</script>
									</td></tr>
									</table>
									<input type="hidden" value="/main.asp" name="submit-url">
									<input type="hidden" name="re_uptime" value="1">
									</form>
									</td>
									</tr>
									</table>
									<script>document.write('<p align="center"><input type="button" name ="refresh" style="width:80px" value ="'+showText(refresh)+'" onclick="window.location.replace(\'stats.asp\')"></p>');</script>
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
													<span class="style13"><script>dw(statStatistics)</script></span><br>
													<span class="style21"><script>dw(statStatisticsDoc)</script></span>
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