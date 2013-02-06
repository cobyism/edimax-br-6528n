<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title></title>
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="file/multilanguage.var"></script>
<script type="text/javascript" src="file/qosnat-n.var"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
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
var ALGArray = new Array("FTP","H323","IPsec","PPTP","L2TP","SIP");
var ALGComArray = new Array(
natALGComArrayFTP,
natALGComArrayH323,
natALGComArrayIPsec,
natALGComArrayPPTP,
natALGComArrayL2TP,
natALGComArraySIP);
function saveChanges()
{
var longVal=0;
if (document.ALG.Index0.checked==true)	longVal |= 0x001;
if (document.ALG.Index1.checked==true)	longVal |= 0x002;
if (document.ALG.Index2.checked==true)	longVal |= 0x004;
if (document.ALG.Index3.checked==true)	longVal |= 0x008;
if (document.ALG.Index4.checked==true)	longVal |= 0x010;
if (document.ALG.Index5.checked==true)	longVal |= 0x020;
document.ALG.appLyGatewayValue.value=longVal;
return true;
}
</script>
</head>
<body bgcolor="#EFEFEF">
<form action=/goform/formALGSetup method=POST name="ALG">
	<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
		<tr align="center">
			<td width="6%" bgcolor="#666666" class="style13" nowrap><font size=2><script>dw(enable)</script></font><input type="hidden" value="<% getInfo("appLayerGateway"); %>" name="appLyGatewayValue"></td>
			<td width="24%" bgcolor="#666666" class="style13" nowrap><font size=2><script>dw(name)</script></font></td>
			<td width="70%" bgcolor="#666666" class="style13" nowrap><font size=2><script>dw(comment)</script></font></td>
		</tr>
		<script>
			var val=0x001;
			for(i=0;i<6;i++)
			{
				document.write("<tr align=center>");
				if (document.ALG.appLyGatewayValue.value & val)
					document.write('<td bgcolor="#FFFFFF"><input type="checkbox" name="Index'+i+'" checked></td>');
				else
					document.write('<td bgcolor="#FFFFFF" class="style14"><input type="checkbox" name="Index'+i+'"></td>');
				document.write('<td align="left" bgcolor="#FFFFFF" class="style14">' + ALGArray[i] + '</td>');
				document.write('<td align=left bgcolor="#FFFFFF" class="style14">&nbsp;' + showText(ALGComArray[i]) + '</td></tr>');
				val *=2;
			}
		</script>
	</table>
	<p align="right">
		<script>document.write('<input type=submit value="'+showText(save)+'" name="B1" style ="width:100px" onclick="return saveChanges()">');</script>
		<input type=hidden value="/natalg.asp" name="wlan-url">
	</p>
</form>
</body>
</html>
<script>
	if(typeof parent.document.getElementById("ALFrame").contentDocument == "undefined")
		parent.document.getElementById("ALFrame").height = parent.document.getElementById("ALFrame").contentWindow.document.body.scrollHeight+40;
	else
		parent.document.getElementById("ALFrame").height = parent.document.getElementById("ALFrame").contentWindow.document.body.scrollHeight;
</script>
