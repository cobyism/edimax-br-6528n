<html>
<head>
<meta http-equiv="Content-Type" content="text/html;charset=UTF-8" />
<meta http-equiv="cache-control" content="no-cache,must-revalidate">
<meta http-equiv="cache-control" content="no-cache" />
<meta http-equiv="cache-control" content="no-store" />
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<link rel="stylesheet" href="file/set.css">
<link rel="stylesheet" href="edimax.css">
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="file/multilanguage.var"></script>
<script type="text/javascript" src="FUNCTION_SCRIPT"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<script type="text/javascript" src="file/autowan.var"></script>
<style type="text/css">
		#in{width:0px; height:20px;background:#0080FF;color:#000000;text-align:center;}
</style>
<script type="text/javascript">
var security_query = <%getInfo("security_query");%>
var wanMode = <%getInfo("wanMode");%>
var secMode = <%getInfo("secMode");%>
i=0;secs=120;
function saveChanges()
{
	var str=document.auto.pskValue.value;
	var ssid=document.auto.ssid.value;
	if (ssid == "")
	{
		document.auto.ssid.value = "Edimax"
		document.auto.ssid.focus();
	}
	if (str.length < 8)
	{
		alert("Invalid password. Please enter at least 8 digits.");
		document.auto.pskValue.focus();
		return false;
	}
	document.getElementById('sumitAP').style.display="block";
	document.getElementById('AP').style.display="none";
	ba=setInterval("begin()",secs*10);
	return true;
}
function savereplace()
{
	if ( wanMode == 0 )
		window.location.replace("autoWireless1_2.asp");
	else
		window.location.replace("autoWireless_2.asp");
}
function begin()
{
	i+=1;
	if(i<=100)
		document.getElementById("in").style.width=i+"%";
	else
		window.location.replace("autoDirect.asp")
}
function check(input) {
var x = input
if (x.value != "") {
if (x.value.search("\'") != -1 || x.value.search("\"") != -1 || x.value.search("\<") != -1 || x.value.search("\>") != -1) {
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
					    <td width="700" align="left" valign="top" bgcolor="#EFEFEF">
						<blockquote>
							<br>
							<span id="sumitAP" style="display:none">
							<table width="700" border="0" align="left" cellpadding="0" cellspacing="0">
							<tr><td align="left">
							<p><a class="titlecolor"><script>dw(CheckingInternet)</script></a></p>
							<br>
							<div id="in" style="width:%"></div>
							</td></tr>
							</table>
							</span>
							<br>
							<span id="AP" style="display:block">
							<table width="700" border="0" cellpadding="0" cellspacing="0">
							<tr><td class="titlecolor"><script>dw(ConfigureYourWireless)</script></td></tr>
							</table>
							<br>
						    <table width="700" border="0" cellpadding="0" cellspacing="0">
							<tr>
							<td class="stringcolor"><script>dw(PleaseEnterYourWireless)</script></td>
							<td><input type="text" name="ssid" size="32" maxlength="32" value="<% getInfo("ssid"); %>" class="text" onchange="check(this)">&nbsp;<script>dw(EdimaxAsDefault)</script></td>
							</tr>
							<tr>
							<td class="stringcolor"><script>dw(PleaseEnterYourSecurity)</script></td>
							<td><input type="text" name="pskValue" size="32" maxlength="63" value="<% getInfo("autopskValue");%>" class="text">&nbsp;<script>dw(Morethan8)</script></td>
							</tr>
						    </table>
							<br>
							<table width="700" border="0" cellpadding="0" cellspacing="0">
							<tr>
							<td align="right">
							<script>
							document.write('<input type="submit" value="'+showText(NEXT)+'" name="B1" onclick="return saveChanges()" class="button">&nbsp;&nbsp;');
							document.write('<input type="button" value="'+showText(BACK)+'" onclick="savereplace()">&nbsp;');
							</script>
							<input type="hidden" value="1" name="isApply">
							<input type="hidden" value="/autoAP2.asp" name="submit-url">
							<input type="hidden" value="2" name="selectMode">
							<input type="hidden" value=<% getInfo("cloneMac"); %> name="macAddrValue">
							</td>
							</tr>
					      	</table>
							</span>
						</td>
					    </tr>
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
