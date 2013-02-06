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
</head>
<script>
function saveChanges()
{
	if(document.getElementsByName("autoWireless")[0].checked)
	{
		document.auto.enableIQsetup.value=0
		setTimeout("window.location.replace(\"autoDetect2.asp\");",1000);
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
					<form action=/goform/formAutoDetect method=POST name="auto">
					<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
						<tr>
						<td width="800" align="center" valign="top" bgcolor="#EFEFEF">
						<blockquote>
						<p class="titlecolor"><script>dw(Indexconnect)</script></p>
            				<table border="0" cellspacing="3" cellpadding="0" width="480" align="left">
						<tr valign="middle" align="left" height="40">
						<td width="36"><input type="radio" name="autoWireless" value="yes" onMouseOut="Mout()" onMouseOver="Mover()" onClick="saveChanges()" style="cursor:pointer"></td>
						<td width="400" ><a class="stringcolor" onMouseOut="Mout()" onMouseOver="Mover()" onClick="saveChanges()" style="cursor:pointer"><b><script>dw(PerformIQ)</script></b></a></td>
						</tr>
						<tr valign="middle" align="left" height="40">
						<td width="36"><input type="radio" name="autoWireless" value="no" onMouseOut="Mout1()" onMouseOver="Mover1()" onClick="saveChanges()" style="cursor:pointer"></td>
			<td width="400"><a class="stringcolor" onMouseOut="Mout1()" onMouseOver="Mover1()" onClick="saveChanges()" style="cursor:pointer"><b><script>dw(ConnectManaually)</script></b></a></td>
						</tr>
						</table>
						</blockquote>
						</td>
						</tr>
					</table>
					<input type="hidden" value="" name="enableIQsetup">
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
