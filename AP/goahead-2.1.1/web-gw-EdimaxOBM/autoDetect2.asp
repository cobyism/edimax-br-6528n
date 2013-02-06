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
<script type="text/javascript">
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
		location.replace("autoDirect2.asp")
	}
}
</script>
</head>
<body onload="start();";>
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
					<form action=/goform/formAutoDetect method=POST name="wanautodetect">
						<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
							<tr>
								<td width="800" align="center" valign="top" bgcolor="#EFEFEF">
									<blockquote>
									<table width="750" border="0" cellpadding="2" cellspacing="0">
										<p><a class="titlecolor"><script>dw(CheckingInternet)</script></a></p>
										<tr>
										<td>
										<div id="in" style="width:%"></div>
										</tr>
										</td>
									</table>
									</blockquote>
								</td>
							</tr>
						</table>
						<input type="hidden" value="/autoDetect2.asp" name="submit-url">
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
<script>
	var internet_sta = "<% getInfo("sta-current"); %>"
	top.sw_current_sta(internet_sta);
</script>
</body>
</html>
