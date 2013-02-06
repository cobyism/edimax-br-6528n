<html>
<head>
<meta http-equiv="Content-Type" content="text/html;charset=UTF-8" />
<meta http-equiv="Pragma" content="no-cache" />
<meta http-equiv="cache-control" content="no-cache" />
<meta http-equiv="cache-control" content="no-store" />
<meta http-equiv="Expires" content="-1" />
<link rel="stylesheet" type="text/css" href="file/set.css" />
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="file/multilanguage.var"></script>
<script type="text/javascript" src="file/autowan.var"></script>
<script type="text/javascript" src="FUNCTION_SCRIPT"></script>
</head>
<script>
Detect_Lang()
function saveChanges()
{
	if(document.getElementsByName("autoWireless")[0].checked)
	{
		document.auto.enableIQsetup.value=0
		setTimeout("window.location.replace(\"autoDetect.asp\");",1000);
		document.auto.submit();
	}
	if(document.getElementsByName("autoWireless")[1].checked)
	{
		document.auto.enableIQsetup.value=1
		setTimeout("window.location.replace(\"index.asp\");",1000);
		document.auto.submit();
	}
	return true;
}
function Mover()
{
document.getElementsByName("autoWireless")[0].checked=true;
}
function Mout()
{
	document.getElementsByName("autoWireless")[0].checked=false;
}
function Mover1()
{
document.getElementsByName("autoWireless")[1].checked=true;
}
function Mout1()
{
	document.getElementsByName("autoWireless")[1].checked=false;
}
</script>
<body class="menu1">
<form action=/goform/formAutoDetect method=POST name="auto">
<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr><td width="800" height="94" valign="bottom"><img src="graphics/logo.png" /></td></tr>
</table>
<br>
<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="800" align="center" valign="top" bgcolor="#EFEFEF"><br>
			<blockquote>
			<p class="titlecolor"><script>dw(Indexconnect)</script></p>
            	<table border="0" cellspacing="3" cellpadding="0" width="480" align="left">
			<tr valign="middle" align="left" height="40">
			<td width="36"><input type="radio" name="autoWireless" value="yes" onMouseOut="Mout()" onMouseOver="Mover()" onClick="saveChanges()" style="cursor:pointer"><a class></td>
			<td width="400" ><a class="stringcolor" style="cursor:pointer" onMouseOut="Mout()" onMouseOver="Mover()" onClick="saveChanges()"><b><script>dw(PerformIQ)</script></b></a></td>
			</tr>
			<tr valign="middle" align="left" height="40">
			<td width="36"><input type="radio" name="autoWireless" value="no" onMouseOut="Mout1()" onMouseOver="Mover1()" onClick="saveChanges()" style="cursor:pointer"></td>
			<td width="400"><a class="stringcolor" style="cursor:pointer" onMouseOut="Mout1()" onMouseOver="Mover1()" onClick="saveChanges()"><b><script>dw(ConnectManaually)</script></b></a></td>
			</tr>
			</table>
			</blockquote>
		</td>
	</tr>
</table>
<input type="hidden" value="" name="enableIQsetup">
</form>
</body>
</html>
