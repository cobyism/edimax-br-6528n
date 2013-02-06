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
	<script>Write_Header(4,5)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF">
						<table width="600" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td valign="top" align="center"><br>
									<form action=/goform/formReflashClientTbl method=POST name="formClientTbl">
									<table width="600" border="1" cellpadding="2" cellspacing="2" align="center">
									<tr>
									<td width="17%" bgcolor="#666666" class="style13" align="center"><script>dw(ipAddress)</script></td>
									<script>
									if ( typeof _IP_LIST_TABLE_ != 'undefined' )
										document.write('<td width="23%" bgcolor="#666666" class="style20" align="center">'+showText(statDeviceName)+'</td>');
									</script>
									<td width="20%" bgcolor="#666666" class="style13" align="center"><script>dw(macAddress)</script></td>
									<td width="20%" bgcolor="#666666" class="style13" align="center"><script>dw(timeExpired)</script>(sec)</td>
									<script>
									if ( typeof _IP_LIST_TABLE_ != 'undefined' )
										document.write('<td width="20%" bgcolor="#666666" class="style20" align="center">'+showText(IPStatus)+'</td></tr>');
									</script>
									<% dhcpClientList(); %>
									</table>
									<input type="hidden" value="/stadhcptbl.asp" name="submit-url">
									<p align="center"><script>document.write('<input type="submit" value="'+showText(refresh)+'" name="refresh" class="btnsize">')</script></p>
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
													<span class="style13"><script>dw(statDHCP)</script></span><br>
													<span class="style21"><script>dw(statDHCPDoc)</script></span>
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
