<html>
<head>
<title></title>
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="file/qosnat-n.var"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
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
</head>
<body bgcolor="#EFEFEF">
<form action=/goform/formUPNPSetup method=POST name="UPNPSetup">
	<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
		<tr>
			<td align="right" class="style14"><script>dw(natUpnpFeature)</script> :</td>
			<td align="left" class="style14">
				<input type=radio value="yes" name="upnpEnable" onClick="document.UPNPSetup.submit();" <% if (getIndex("upnpEnable")==1) write("checked"); %>><font size="2"><script>dw(enable)</script>
				<input type=radio value="no" name="upnpEnable" onClick="document.UPNPSetup.submit();" <% if (getIndex("upnpEnable")==0) write("checked"); %>><font size="2"><script>dw(disable)</script>
			</td>
		</tr>
	</table>
	<input type="hidden" value="/natupnp.asp" name="submit-url">
</form>
</body>
</html>
