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
i=0;secs=95;
function start()
{
	ba=setInterval("begin()",secs*10);
	document.getElementsByName('wanautodetect')[0].submit();
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
</script>
</head>
<body class="menu1" onload="start();";>
<form action=/goform/formAutoDetect method=POST name="wanautodetect">
<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr><td width="800" height="94" valign="bottom"><img src="graphics/logo.png" /></td></tr>
</table>
<br>
<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
	<td width="800" align="center" valign="top" bgcolor="#EFEFEF"><p><br />
	<blockquote>
	<table width="750" border="0" cellpadding="2" cellspacing="0">
	<p><a class="titlecolor"><script>dw(CheckingInternet)</script></a></p>
	<tr>
	<td>
	<div id="in" style="width:%"></div>
	</td>
	</tr>
	</table>
	</blockquote>
	</td>
	</tr>
</table>
<input type="hidden" value="/autoDetect.asp" name="submit-url">
</form>
</body>
</html>
