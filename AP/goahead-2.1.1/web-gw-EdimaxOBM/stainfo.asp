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
	var cloud_sta = "<% getInfo("cloud-sta"); %>"
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
</script>
<title></title>
</head>
<body onLoad="top.startclock()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(4,0)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF">
						<br>
						<table width="600" border="1" cellpadding="5" cellspacing="0">
							<tr>
								<td align="center" width="600" height="25px" bgcolor="#666666"><span class="style13"><script>dw(system)</script></span></td>
							</tr>
							<tr>
								<td class="style14" bgcolor="#FFFFFF">
									<table width="590" cellpadding="0" cellspacing="0">
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20">&nbsp;<script>dw(model)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff">&nbsp;<% getInfo("name"); %></td>
										</tr>
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20">&nbsp;<script>dw(upTime)</script> :&nbsp;&nbsp;</td>
										<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff"><a id="showuptime"></a></td>
										</tr>
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20">&nbsp;<script>dw(verHardware)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff">Rev. A</td>
										</tr>
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20">&nbsp;<script>dw(verBootCode)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff">1.0</td>
										</tr>
										<tr>
											<td valign="center" align="right" width="50%" bgcolor="#ffffff" height="20">&nbsp;<script>dw(verRuntimeCode)</script> :&nbsp;&nbsp;</td>
											<td width="50%" height="20" align="left" valign="center" bgcolor="#ffffff"><% getInfo("fwVersion"); %></td>
										</tr>
									</table>
								</td>
							</tr>
						</table>
						<input name="tmp" type="hidden" value="0">
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
											<td height="384" valign="top" bgcolor="#999999"></td></tr>
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
