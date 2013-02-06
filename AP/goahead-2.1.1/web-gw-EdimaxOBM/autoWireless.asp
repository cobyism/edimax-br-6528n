<html>
<head>
<meta http-equiv="Content-Type" content="text/html;charset=UTF-8" />
<meta http-equiv="cache-control" content="no-cache,must-revalidate">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="cache-control" content="no-cache" />
<meta http-equiv="cache-control" content="no-store" />
<meta http-equiv="Expires" content="-1">
<link rel="stylesheet" type="text/css" href="file/set.css" />
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="FUNCTION_SCRIPT"></script>
<script type="text/javascript" src="file/multilanguage.var"></script>
<script type="text/javascript" src="file/autowan.var"></script>
<script type="text/javascript">
Detect_Lang()
var secModeTbl = <%getInfo("secModeAll");%>;
function mouseover()
{
	document.getElementsByName("autoWireless")[0].checked=true;
}
function mouseout()
{
	document.getElementsByName("autoWireless")[0].checked=false;
}
function mouseover1()
{
	document.getElementsByName("autoWireless")[1].checked=true;
}
function mouseout1()
{
	document.getElementsByName("autoWireless")[1].checked=false;
}
function saveChanges()
{
	if(document.getElementsByName("autoWireless")[0].checked)
	{
		window.location.replace("autoAP.asp");
	}
	if(document.getElementsByName("autoWireless")[1].checked)
	{
		document.auto.selectMode.value=2
		document.auto.submit();
		setTimeout("window.open('','_self');top.window.close();", 1000);
	}
	return true;
}
</script>
</head>
<body class="menu1">
<form action=/goform/formAutoWireless method=POST name="auto">
<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr><td width="800" height="94" valign="bottom"><img src="graphics/logo.png" /></td></tr>
</table>
<br>
<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
      <tr>
      <td width="750" align="center" valign="top" bgcolor="#EFEFEF">
	<br>
	<p class="titlecolor"><script>dw(Congratulations)</script></p>
		<blockquote>
            <table border="0" cellspacing="3" cellpadding="0" width="480" align="left">
			<tr valign="middle" align="left" height="40">
			<td width="36"><input type="radio" name="autoWireless" value="0" onClick="saveChanges()" onmouseover="mouseover()" onmouseout="mouseout()" style="cursor:pointer"></td>
			<script>
			if (secModeTbl == 0)
				document.write('<td width="400"><a class="stringcolor" onmouseover="mouseover()" onmouseout="mouseout()" style="cursor:pointer" onClick="saveChanges()"><b>'+showText(SetupWirelessSec)+'</b></a></td>')
			else
				document.write('<td width="400"><a class="stringcolor" onmouseover="mouseover()" onmouseout="mouseout()" style="cursor:pointer" onClick="saveChanges()"><b>'+showText(SetupWirelessSec1)+'</b>&nbsp;<img src="graphics/img_check.gif"></a></td>')
			</script>
			</tr>
			<tr valign="middle" align="left" height="40">
			<td width="36"><input type="radio" name="autoWireless" value="3" onClick="saveChanges()" onmouseover="mouseover1()" onmouseout="mouseout1()" style="cursor:pointer"></td>
			<td width="400"><a class="stringcolor" onmouseover="mouseover1()" onmouseout="mouseout1()" style="cursor:pointer" onClick="saveChanges()"><b><script>dw(Exit)</script></b></a></td>
			</tr>
		</blockquote>
		</table>
	</td>
	</tr>
</table>
<input type="hidden" value="" name="selectMode">
<input type=hidden value=<% getInfo("cloneMac"); %> name="macAddrValue">
</form>
</body>
</html>
