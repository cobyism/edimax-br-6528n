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
<script>
function refreshLog() {
	document.location.replace("stasylog.asp");
}
var cloud_sta = "<% getInfo("cloud-sta"); %>"
</script>
<title></title>
</head>
<body onLoad="top.startclock()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(4,3)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF">
						<table width="600" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td colspan="3" valign="top" align="center"><br>
									<form method="post" action="goform/formSysLog"  name="sysLog">
										<textarea cols="75" id="logsystem" readonly="readonly" rows="9" wrap="off" edit="off" style="color:Navy"><% getInfo("syslog"); %></textarea>
										<p align="center">
										<script>buffer='<input type ="submit" name ="send" value ="'+showText(save)+'" class="btnsize">';document.write(buffer);</script>
										<script>buffer='<input type ="submit" name ="reset" value ="'+showText(clear)+'" class="btnsize">';document.write(buffer);</script>
										<script>buffer='<input type ="button" name ="refresh" onclick ="refreshLog()" value ="'+showText(refresh)+'" class="btnsize">';document.write(buffer);</script>
										<input type="hidden" value="/stasylog.asp" name="submit-url">
										</p>
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
													<span class="style13"><script>dw(statSystem)</script></span><br />
													<span class="style21"><script>dw(statSystemDoc)</script></span>
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
