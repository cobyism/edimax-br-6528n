<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title></title>
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<script type="text/javascript" src="file/qosnat-n.var"></script>
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
<% write("entryNum = "+getIndex("sroutNum")); %>
function addClick()
{
	if ( !ipRule( document.formFilterAdd.sroutIp, showText(ipAddress), "ip", 0))
		return false;
	if(!maskRule(document.formFilterAdd.sroutMask, showText(SubnetMask),0))
		return false;
	if ( !ipRule( document.formFilterAdd.sroutGateway, showText(gateway), "ip", 0))
		return false;
	if (!portRule(document.formFilterAdd.sroutCount, showText(natSrHop), 0, "", 0, 255, 0))
		return false;
	return true;
}
function disableDelButton()
{
	document.formFilterDel.delSelSRout.disabled=true;
	document.formFilterDel.delAllSRout.disabled=true;
}
function goToWeb() {
	if (document.formFilterAdd.enabled.checked==true)
		document.formFilterEnabled.enabled.value="ON";
	document.formFilterEnabled.submit();
}
function goToApply() {
	document.formPortFwApply.submit();
}
</script>
</head>
<body bgcolor="#EFEFEF">
<form action=/goform/formFilter method=POST name="formFilterEnabled">
	<input type="hidden" value="Add" name="addSRout">
	<input type="hidden" value="" name="enabled">
	<input type="hidden" value="/natsrout.asp" name="submit-url">
</form>
<form action=/goform/formFilter method=POST name="formFilterAdd">
	<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
		<tr>
			<td colspan="5" align="left" class="style14">
				<input type="checkbox" name="enabled" value="ON" <% if (getIndex("sroutEnabled")) write("checked"); %> ONCLICK="goToWeb();"><script>dw(enNatSr)</script>
			</td>
		</tr>
		<tr align="center">
			<td bgcolor="#666666" class="style13"><script>dw(natSrIP)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(natSrSM)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(natSrGateway)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(natSrHop)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(face)</script></td>
		</tr>
		<tr class="table2" align="center">
			<td bgcolor="#FFFFFF" class="style1"><input type="text" name="sroutIp" size="15" maxlength="15"></td>
			<td bgcolor="#FFFFFF" class="style1"><input type="text" name="sroutMask" size="15" maxlength="15"></td>
			<td bgcolor="#FFFFFF" class="style1"><input type="text" name="sroutGateway" size="15" maxlength="15"></td>
			<td bgcolor="#FFFFFF" class="style1"><input type="text" name="sroutCount" size="3" maxlength="3"></td>
			<td bgcolor="#FFFFFF" class="style1">
			<select name="sroutFace">
				<option selected value="0">LAN</option>
				<option value="1">WAN</option>
			</select>
			</td>
		</tr>
		<tr>
			<td colspan="5">
				<p align="right">
					<script>document.write('<input type="submit" value="'+showText(add)+'" name="addSRout" onClick="return addClick()" class="btnsize">')</script>
					<script>document.write('<input type="reset" value="'+showText(reset)+'" name="reset" onClick="document.formFilterAdd.reset;" class="btnsize">')</script>
					<input type="hidden" value="/natsrout.asp" name="submit-url">
				</p>
			</td>
		</tr>
	</table>
</form>
<form action=/goform/formFilter method=POST name="formFilterDel">
	<table width="700" border="0" cellpadding="0" cellspacing="1" class="style14">
		<tr><td colspan="7" align="left"><script>dw(natSrTable)</script>:</td></tr>
	</table>
	<table width="700" border="1" cellpadding="0" cellspacing="1" bgcolor="#FFFFFF" class="style14">
		<tr align="center">
			<td bgcolor="#666666" class="style13">NO.</td>
			<td bgcolor="#666666" class="style13"><script>dw(natSrIP)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(natSrSM)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(natSrGateway)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(natSrHop)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(face)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(select)</script></td>
		</tr>
		<% StcRoutList(); %>
	</table>
	<table width="700" border="0" cellpadding="0" cellspacing="1" class="style14">
		<tr>
			<td colspan="7" align="right">
				<script>
					document.write('<input type="submit" value="'+showText(deleteSelected)+'" name="delSelSRout" onClick="return deleteClick()" class="btnsize">');
					document.write('<input type="submit" value="'+showText(deleteAll)+'" name="delAllSRout" onClick="return deleteAllClick()" class="btnsize">');
				</script>
				<input type="hidden" value="/natsrout.asp" name="submit-url">
			</td>
		</tr>
	</table>
</form>
</body>
</html>
<script>
	if(!entryNum) disableDelButton()
	if(typeof parent.document.getElementById("SRFrame").contentDocument == "undefined")
		parent.document.getElementById("SRFrame").height = parent.document.getElementById("SRFrame").contentWindow.document.body.scrollHeight+40;
	else
		parent.document.getElementById("SRFrame").height = parent.document.getElementById("SRFrame").contentWindow.document.body.scrollHeight;
</script>
