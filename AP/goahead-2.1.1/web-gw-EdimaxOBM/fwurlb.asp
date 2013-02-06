<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title>FireWall URL Blocking</title>
<script language ='javascript' src ='file/javascript.js'></script>
<script language ='javascript' src ='file/fwwl-n.var'></script>
<script language ='javascript' src ='file/netsys-n.var'></script>
<script type="text/javascript" src="file/multilanguage.var"></script>
<style type="text/css">
body {
	margin-left: 0px;
	margin-top: 0px;
	margin-right: 0px;
	margin-bottom: 0px;
}
.style1 {
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
	color: #666666;
}
.style14 {
	font-family: Arial, Helvetica, sans-serif;
	font-size: 12px;
	color: #333333;
}
.style13 {
	font-size: 12px; font-family: Arial, Helvetica, sans-serif; color: #FFCC00; font-weight: bold;
}
</style>
<script>
<% write("entryNum = "+getIndex("URLBNum")+";"); %>
function disableDelButton()
{
	document.formUrlbDel.deleteSelURLB.disabled = true;
	document.formUrlbDel.deleteAllURLB.disabled = true;
}
function addClick() {
	if (entryNum < 20) {
		if (document.formUrlbAdd.URLB.value==""){
			alert(showText(fwurlbAlertInv));
			document.formUrlbAdd.URLB.focus();
			return false;
		}
	}
	else {
		alert(showText(fwurlbAlertCannot));
		return false;
	}
	return true;
}
function updateState()
{
	if (document.formUrlbAdd.enabled.checked)
		document.formUrlbAdd.URLB.disabled=false;
	else
		document.formUrlbAdd.URLB.disabled=true;
}
function goToWeb() {
	if (document.formUrlbAdd.enabled2.checked==true)
		document.formURLBEnabled.enabled.value="ON";
	document.formURLBEnabled.submit();
}
function goToApply() {
	document.formPortFwApply.submit();
}
function check(input) {
var x = input
if (x.value != "") {
if (x.value.search("\'") != -1 || x.value.search("\"") != -1 || x.value.search("\<") != -1 || x.value.search("\>") != -1|| x.value.search("\~") != -1|| x.value.search("\`") != -1) {
alert("Invalid value")
x.value = x.defaultValue
x.focus()
return false
}
else if (x.value.indexOf("?") != -1 || x.value.indexOf("\\") != -1) {
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
<body bgcolor="#EFEFEF">
	<form action=/goform/formUrlb method=POST name="formURLBEnabled">
	<input type="hidden" value="ON" name="isEnabled">
	<input type="hidden" value="" name="enabled">
	<input type="hidden" value="/fwurlb.asp" name="submit-url">
</form>
<form action=/goform/formUrlb method=POST name="formUrlbAdd">
	<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
		<tr>
			<td colspan="2" height="20" valign="middle" align="left" class="style14">
				<input type="checkbox" name="enabled2" value="ON" <% if (getIndex("URLBEnabled")) write("checked");%> onClick="goToWeb();">&nbsp;&nbsp;<script>dw(enfwUrl);</script>
			</td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="left" class="style14"><script>dw(fwUrlKey)</script>:&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="text" name="URLB" size="32" maxlength="32" onchange="check(this)">
			</td>
		</tr>
		<tr>
			<td colspan="5" align="right">
				<script>
					document.write('<input type="submit" value="'+showText(add)+'" name="addURLB" onClick="return addClick()" class="btnsize">');
					document.write('<input type="reset"  value="'+showText(reset)+'" name="reset" onClick="document.formURLBEnabled.reset;" class="btnsize">');
				</script>
				<input type="hidden" value="/fwurlb.asp" name="submit-url">
			</td>
		</tr>
	</table>
</form>
<form action=/goform/formUrlb method=POST name="formUrlbDel">
	<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" bgcolor="#FFFFFF" class="style14">
		<tr>
			<td align="center" height="20" bgcolor="#666666" class="style13">NO.</td>
			<td align="center" height="20" bgcolor="#666666" class="style13"><script>dw(fwUrlKey)</script></td>
			<td align="center" height="20" bgcolor="#666666" class="style13"><script>dw(select)</script></td>
		</tr>
		<% URLBList(); %>
	</table>
	<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
		<tr>
			<td colspan="5" align="right">
				<script>
					document.write('<input type="submit" value="'+showText(deleteSelected)+'"name="deleteSelURLB" class="btnsize">');
					document.write('<input type="submit" value="'+showText(deleteAll)+'" name="deleteAllURLB" class="btnsize">');
					document.write('<input type="reset"  value="'+showText(reset)+'" name="reset" class="btnsize">')
				</script>
				<script>
					<%
					entryNum = getIndex("URLBNum");
					if ( entryNum == 0 ) {
					write( "disableDelButton();" );
					}
					%>
				</script>
				<input type="hidden" value="/fwurlb.asp" name="submit-url">
			</td>
		</tr>
	</table>
</form>
</body>
</html>
<script>
	if(typeof parent.document.getElementById("URBFrame").contentDocument == "undefined")
		parent.document.getElementById("URBFrame").height = parent.document.getElementById("URBFrame").contentWindow.document.body.scrollHeight+40;
	else
		parent.document.getElementById("URBFrame").height = parent.document.getElementById("URBFrame").contentWindow.document.body.scrollHeight;
</script>
