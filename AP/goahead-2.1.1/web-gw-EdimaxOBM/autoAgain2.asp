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
var wanStatus = <%getInfo("wanStatus");%>
<% write("wanmode = "+getIndex("wanMode")); %>
i=0;secs=95;
function TRYover()
{
	document.getElementsByName("selectMode")[0].checked=true;
}
function TRYout()
{
	if(document.getElementsByName("selectMode")[0].checked)
		document.getElementsByName("selectMode")[0].checked=true;
	else
		document.getElementsByName("selectMode")[0].checked=false;
}
function REover()
{
		document.getElementsByName("selectMode")[1].checked=true;
}
function REout()
{
	if(document.getElementsByName("selectMode")[1].checked)
		document.getElementsByName("selectMode")[1].checked=true;
	else
		document.getElementsByName("selectMode")[1].checked=false;
}
function goTry()
{
	document.autoagain.selectMode.value=0
	if (wanStatus != 4)
		document.autoagain.submiturl.value="/autoDetect2.asp"
	else
	{
		document.getElementById('dhcp').style.display="none";
		document.getElementById('setup').style.display="block";
		ba=setInterval("begin()",secs*10);
	}
	document.autoagain.submit();
}
function goRe()
{
	document.autoagain.selectMode.value=1
	if(wanmode == 2)
		document.autoagain.submiturl.value="/autoPPPoE2.asp"
	else
		document.autoagain.submiturl.value="/autoMain2.asp"
	document.autoagain.submit();
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
					<form action=/goform/formAutoAgain method=POST name="autoagain">
					<table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
						<tr>
						<td width="800" align="center" valign="top" bgcolor="#EFEFEF"><p><br />
						<blockquote>
						<script>
							if (wanStatus==4)
							{
								document.write('<span id="setup" style="display:none">')
								document.write('<table width="700" border="0" cellpadding="0" cellspacing="0">')
								document.write('<tr><td align="left"><p><a class="titlecolor">'+showText(CheckingInternet)+'</a></p><br>')
								document.write('<div id="in" style="width:%"></div></td></tr>')
								document.write('</table>')
								document.write('</span>')
								document.write('<span id="dhcp" style="display:block">')
								document.write('<table width="700" border="0" cellpadding="0" cellspacing="0">')
								document.write('<p align="left"><a class="titlecolor">'+showText(PleaseClickok)+'</a></p><br>')
								document.write('<tr align="right"><td><input type="button" value="OK" name="B1" onclick="goTry();" class="button"></td></tr>');
								document.write('</table>')
								document.write('</span>')
								document.write('<input type=hidden value="0" name="wanMode">')
							}
							else
							{
								document.write('<span id="pppoe" style="display:block">')
								document.write('<table width="700" border="0" cellpadding="0" cellspacing="0">')
								document.write('		<tr>')
								document.write('		<p><a class="titlecolor">'+showText(ChekError)+'</a></p><br>')
								document.write('		<td><input  name="selectMode" type="radio" value="0" onclick="goTry();" onmouseover="TRYover();" onmouseout="TRYout();" >'+showText(TryitAgain)+'</td>')
								document.write('		<td><input  name="selectMode" type="radio" value="1" onclick="goRe();" onmouseover="REover();" onmouseout="REout();">'+showText(ReEnter)+'</td>')
								document.write('		</tr>')
								document.write('</table>')
								document.write('</span>')
							}
						</script>
						</td>
						</tr>
						<input type="hidden" value="" name="selectMode">
						<input type="hidden" value="/autoAgain2.asp" name="submiturl">
					</table>
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
