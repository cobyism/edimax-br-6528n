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
<style type="text/css">
		#in{width:0px; height:20px;background:#0080FF;color:#000000;text-align:center;}
</style>
<script>
Detect_Lang()
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
		window.location.replace("autoDirect.asp")
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
<body class="menu1">
<form action=/goform/formWanAutoDetect method=POST name="tcpip">
<input type="hidden" name="redirect" size="20" value="">
<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr><td width="800" height="94" valign="bottom"><img src="graphics/logo.png" /></td></tr>
</table>
<br>
<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
      <td width="800" align="left" valign="top" bgcolor="#EFEFEF">
	<blockquote>
	<br>
		<span id="setup" style="display:none">
		<table width="700" border="0" cellpadding="0" cellspacing="0">
		<p><a class="titlecolor"><script>dw(CheckingInternet)</script></a></p>
		<br>
		<div id="in" style="width:%"></div>
      	</table>
		</span>
		<br>
		<span id="config" style="display:block">
		<table width="700" border="0" cellpadding="0" cellspacing="0">
		<p><a class="titlecolor"><script>dw(YourConnectType)</script></a></p>
		<p><a class="stringcolor"><script>dw(PleaseEntertheUsername)</script></a></p>
      	</table>
		</span>
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
		document.write('<input type=submit value="'+showText(NEXT)+'" name="B1" onclick="return saveChanges()" class="button">');
		document.write('<input type=hidden value="/autoPPPoE.asp" name="submit-url" id="submitUrl">');
		</script>
		</td>
		</tr>
		</table>
		</span>
		<br>
	</td>
	</tr>
</table>
<input type="hidden" value="2" name="wanMode" >
<input type="hidden" value="ppp" name="ipMode">
<input type="hidden" value="/autoPPPoE.asp" name="submit-url">
<input type="hidden" value=<% getInfo("cloneMac"); %> name="macAddrValue1">
</form>
</body>
</html>
