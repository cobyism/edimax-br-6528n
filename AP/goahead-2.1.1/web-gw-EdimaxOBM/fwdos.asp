<html><head><title></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<script language ='javascript' src ='file/javascript.js'></script>
<script language ='javascript' src ='file/fwwl-n.var'></script>
<script language ='javascript' src ='file/netsys-n.var'></script>
<script type="text/javascript" src="file/multilanguage.var"></script>
<style type="text/css">
body {
	margin-left: 0px;
	margin-top: 0px;
	margin-right: 0px;
	margin-bottom: 0px;
}
.style1 {
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
	color: #666666;
}
.style14 {
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
	color: #333333;
}
.style13 {
	font-size: 12px; font-family: Arial, Helvetica, sans-serif; color: #FFCC00; font-weight: bold;
}
</style>
<script>
function Change() {
	openWindow("/fwadv.asp");
}
</script>
</head>
<body bgcolor="#EFEFEF">
<form action=/goform/formPreventionSetup method=POST name="Prevention">
	<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
		<tr>
			<td colspan="5" height="20" valign="middle" align="left" class="style14"><script>dw(fwDosFeature)</script>:&nbsp;</td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="left" class="style14"><script>dw(fwDosDeath)</script>:&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
			<input type="checkbox" name="podEnable" value="ON" <% if (getIndex("podEnable")==1) write("checked"); %>>
		</td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="left" class="style14"><script>dw(fwDosPing)</script>:&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
			<input type="checkbox" name="pingEnable" value="ON" <% if (getIndex("pingEnable")==1) write("checked"); %>>
		</td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="left" class="style14"><script>dw(fwDosScan)</script>:&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
			<input type="checkbox" name="scanEnable" value="ON" <% if (getIndex("scanEnable")==1) write("checked"); %>>
		</td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="left" class="style14"><script>dw(fwDosSync)</script>:&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="checkbox" name="synEnable" value="ON" <% if (getIndex("synEnable")==1) write("checked"); %>>
			</td>
		</tr>
		<tr>
			<td colspan="5" align="right">
				<script>
					document.write('<input type =\"button\" onclick =\"Change();\" value ="'+showText(wlAdvSettings)+'" class="btnsize">');
					document.write('<input type=submit value="'+showText(save)+'" name="B1" style ="width:100px">');
				</script>
				<input type=hidden value="/fwdos.asp" name="submit-url">
				<input type=hidden value="basic" name="basic">
			</td>
		</tr>
	</table>
</form>
</body>
</html>
<script>
	if(typeof parent.document.getElementById("DOSFrame").contentDocument == "undefined")
		parent.document.getElementById("DOSFrame").height = parent.document.getElementById("DOSFrame").contentWindow.document.body.scrollHeight+40;
	else
		parent.document.getElementById("DOSFrame").height = parent.document.getElementById("DOSFrame").contentWindow.document.body.scrollHeight;
</script>
