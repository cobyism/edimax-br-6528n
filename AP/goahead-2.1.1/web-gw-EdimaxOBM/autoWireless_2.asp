<html>
<head>
<meta http-equiv="Content-Type" content="text/html;charset=UTF-8" />
<meta http-equiv="cache-control" content="no-cache,must-revalidate">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="cache-control" content="no-cache" />
<meta http-equiv="cache-control" content="no-store" />
<meta http-equiv="Expires" content="-1">
<link rel="stylesheet" href="file/set.css">
<link rel="stylesheet" href="edimax.css">
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="FUNCTION_SCRIPT"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<script type="text/javascript" src="file/multilanguage.var"></script>
<script type="text/javascript" src="file/autowan.var"></script>
<script type="text/javascript">
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
		location.replace("autoAP2.asp");
	}
	if(document.getElementsByName("autoWireless")[1].checked)
	{
		document.auto.selectMode.value=2
		document.auto.submit();
		setTimeout("location.replace('main.asp');", 300);
	}
	return true;
}
</script>
</head>
<body>
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(1,0)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF">
					<form action=/goform/formAutoWireless method=POST name="auto">
					<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
					      <tr>
					      <td width="750" align="center" valign="top" bgcolor="#EFEFEF">
						<blockquote>
						<p class="titlecolor"><script>dw(Congratulations)</script></p>
						    <table border="0" cellspacing="3" cellpadding="0" width="480" align="left">
								<tr valign="middle" align="left" height="40">
								<td width="36"><input type="radio" name="autoWireless" value="0" onClick="saveChanges()" onmouseover="mouseover()" onmouseout="mouseout()" style="cursor:pointer"></td>
								<script>
								if (secModeTbl == 0)
									document.write('<td width="400"><a class="stringcolor" onClick="saveChanges()" onmouseover="mouseover()" onmouseout="mouseout()" style="cursor:pointer"><b>'+showText(SetupWirelessSec)+'</b></a></td>')
								else
									document.write('<td width="400"><a class="stringcolor" onClick="saveChanges()" onmouseover="mouseover()" onmouseout="mouseout()" style="cursor:pointer"><b>'+showText(SetupWirelessSec1)+'</b>&nbsp;<img src="graphics/img_check.gif"</td>')
								</script>
								</tr>
								<tr valign="middle" align="left" height="40">
								<td width="36"><input type="radio" name="autoWireless" value="3" onClick="saveChanges()" onmouseover="mouseover1()" onmouseout="mouseout1()" style="cursor:pointer"></td>
								<td width="400"><a class="stringcolor" onClick="saveChanges()" onmouseover="mouseover1()" onmouseout="mouseout1()" style="cursor:pointer"><b><script>dw(Exit)</script></b></td>
								</tr>
							</blockquote>
							</table>
						</td>
						</tr>
					</table>
					<input type="hidden" value="" name="selectMode">
					<input type=hidden value=<% getInfo("cloneMac"); %> name="macAddrValue">
					</form>
					</td>
					<!-- explain -->
					<td width="380" valign="top" bgcolor="#999999">
						<table width="380" border="0" align="right" cellpadding="0" cellspacing="10">
							<tr>
								<td height="400" valign="top">
									<table width="360"  height="400 "border="0" cellpadding="5" cellspacing="0" bgcolor="#999999">
										<tr><td height="16" bgcolor="#A5031A"><span class="style2">Help</span></td></tr>
										<tr><td height="384" valign="top" bgcolor="#999999"></td></tr>
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
<script>
	var internet_sta = "<% getInfo("sta-current"); %>"
	top.sw_current_sta(internet_sta);
</script>
</html>
