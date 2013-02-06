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
<style type="text/css">
		#in{width:0px; height:20px;background:#0080FF;color:#000000;text-align:center;}
</style>
<script>
function saveChanges(connect)
{
	if (!strRule(document.tcpip.pppUserName,showText(UserName))) return false;
	if (!strRule(document.tcpip.pppPassword,showText(Password))) return false;
	if (!check(document.tcpip.pppUserName)) return false
	if (!check(document.tcpip.pppPassword)) return false
	document.getElementById('config').style.display="none";
	document.getElementById('setup').style.display="block";
	document.getElementById('apply').style.display="none";
	document.getElementById('pppoe').style.display="none";
	start();
	return true;
}
i=0;secs=95;
function start()
{
	ba=setInterval("begin()",secs*10);
}
function begin()
{
	i+=1;
	if(i<=100)
	{
		document.getElementById("in").style.width=i+"%";
	}
	else
	{
		location.replace("autoDirect2.asp")
	}
}
function check(input)
{
	var x = input
	if (x.value != "")
	{
		if (x.value.search("\'") != -1 || x.value.search("\"") != -1 || x.value.search("\<") != -1 || x.value.search("\>") != -1)
		{
			alert("Invalid value")
			x.value = x.defaultValue
			x.focus()
			return false
		}
		else
			return true
	}
	else
		return true
}
function checkmac(input)
{
	var x = input
	if (!x.value.match(/^[A-Fa-f0-9]{12}$/) || x.value == "ffffffffffff" || x.value == "FFFFFFFFFFFF")
		x.value = "000000000000"
	return true
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
					<form action=/goform/formWanAutoDetect method=POST name="tcpip">
					<input type="hidden" name="redirect" size="20" value="">
					<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
						<tr>
					      <td width="800" align="left" valign="top" bgcolor="#EFEFEF">
						<blockquote>
						<br>
							<span id="setup" style="display:none">
							<table width="700" border="0" cellpadding="0" cellspacing="0">
							<tr><td align="left">
							<p><a class="titlecolor"><script>dw(CheckingInternet)</script></a></p>
							<br>
							<div id="in" style="width:%"></div>
							</td></tr>
					      	</table>
							</span>
							<br>
							<span id="config" style="display:block">
							<table width="700" border="0" cellpadding="0" cellspacing="0">
							<p><a class="titlecolor"><script>dw(YourConnectType)</script></a></p>
							<p><a class="stringcolor"><script>dw(PleaseEntertheUsername)</script></a></p>
					      	</table>
							</span>
							<br>
							<span id="pppoe" style="display:block">
							<table border="0" width="700" cellspacing="1" cellpadding="0">
							<tr>
							<td width="35%" class="table1"><script>dw(UserName)</script>&nbsp;:&nbsp;</td>
							<td width="65%" class="table2">&nbsp;<input type="text" name="pppUserName" size="15" maxlength="64" value="<% getInfo("pppUserName"); %>" class="text" onchange="check(this)">	</td>
							</tr>
							<tr>
							<td class="table1"><script>dw(Password)</script>&nbsp;:&nbsp;</td>
							<td class="table2">&nbsp;<input type="text" name="pppPassword" size="15" maxlength="64" value="<% getInfo("pppPassword"); %>" class="text" onchange="check(this)"></td>
							</tr>
							</table>
							</span>
							<br>
							<span id="apply" style="display:block" >
							<table border="0" width="700" cellspacing="0" cellpadding="0">
							<tr>
							<td align="right">
							<script>
							document.write('<input type="submit" value="'+showText(NEXT)+'" name="B1" onclick="return saveChanges()" class="button">');
							</script>
							</td>
							</tr>
							</table>
							</span>
						</td>
						</tr>
					</table>
					<input type="hidden" value="2" name="wanMode" >
					<input type="hidden" value="ppp" name="ipMode">
					<input type="hidden" value="/autoPPPoE2.asp" name="submit-url">
					<input type="hidden" value=<% getInfo("cloneMac"); %> name="macAddrValue1">
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
