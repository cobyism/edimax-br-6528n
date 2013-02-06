<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="file/qosnat-n.var"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
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
<% write("entryNum = "+getIndex("wan1QosNum")+";"); %>
function editClick(form)
{
	if (!checkNum(form))
		return false;
	return true;
}
function disableDelButton(form)
{
	form.delSelQos.disabled = true;
	form.delAllQos.disabled = true;
	form.showQos.disabled = true;
}
function replWeb() {
	if (entryNum < 16) {
		openWindow("/wanqosadd.asp");
	}
	else {
		alert(showText(wanqosAlertRepWeb));
		return false;
	}
}
function checkNum(form) {
	var checkFlag = 0;
	for (var i=1; i<form.elements.length; i++) {
		if (form.elements[i].type == 'checkbox' ) {
			if (form.elements[i].checked == true)
				checkFlag++;
		}
	}
	if (checkFlag >= 2) {
		alert(showText(wanqosAlertChkNum));
		return false;
	}
	if (checkFlag < 1) {
		alert("Please select one to edit");
		return false;
	}
	return true;
}
function clickapply()
{
	if (!portRule(document.formWan1Qos.maxdown, showText(qosdoband), 0,"", 0, 99999, 1))
		return false;
	if (!portRule(document.formWan1Qos.maxup, showText(qosupband), 0,"", 0, 99999, 1))
		return false;
	document.qosApply.maxdown.value = document.formWan1Qos.maxdown.value;
	document.qosApply.maxup.value = document.formWan1Qos.maxup.value;
	return true;
}
function Clickdelete()
{
	return deleteClick();
}
function ClickdeleteAll()
{
	return deleteAllClick();
}
function goToWeb() {
	if (document.formWan1Qos.enabled2.checked==true)
		document.formQoSEnabled.enabled1.value="ON";
	document.formQoSEnabled.submit();
}
function enmvButton(form)
{
	form.moveUpQos.disabled = true;
	form.moveDownQos.disabled = true;
}
function specified_protocol()
{
	switch ( document.formQos.trafType.value )
	{
	case "0":
		document.formQos.sourPort.disabled=false;
		document.formQos.destPort.disabled=false;
		break;
	default:
		document.formQos.sourPort.value="";
		document.formQos.destPort.value="";
		document.formQos.sourPort.disabled=true;
		document.formQos.destPort.disabled=true;
		break;
	}
}
function displayObj()
{
	if (document.formQos.sourType.value == 0) {
		document.getElementById('ipId').style.display = "block";
		document.getElementById('macId').style.display = "none";
	}
	else {
		document.getElementById('ipId').style.display = "none";
		document.getElementById('macId').style.display = "block";
	}
	specified_protocol();
}
function edit_apply()
{
	parent.editapply = 1
}
</script>
</head>
<body bgcolor="#EFEFEF">
	<form action=/goform/formQoS method=POST name="formQoSEnabled">
		<input type="hidden" value="ON" name="isEnabled">
		<input type="hidden" value="" name="enabled1">
		<input type="hidden" value="QoS" name="EZQos_Mode">
		<input type="hidden" value="/wanqos.asp" name="submitUrl">
	</form>
	<form action=/goform/formQoS method=POST name="formWan1Qos">
		<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
			<tr>
				<td colspan="2" height="20" valign="middle" align="left" class="style1">
					<input type="checkbox" name="enabled2" value="ON" <% if (getIndex("wan1QosEnabled")) write("checked");%> onClick="goToWeb();">
					<script>dw(qosEnable);</script>
				</td>
			</tr>
			<tr>
				<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(qosdoband)</script> :&nbsp;&nbsp;</td>
				<td width="400" height="20" valign="middle" align="left" class="style1">
					<input type="text" name="maxdown" size="5" maxlength="5" value="<% getInfo("maxdownbandwidth"); %>">&nbsp;kbits
				</td>
			</tr>
			<tr>
				<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(qosupband)</script> :&nbsp;&nbsp;</td>
				<td width="400" height="20" valign="middle" align="left" class="style1">
					<input type="text" name="maxup" size="5" maxlength="5" value="<% getInfo("maxupbandwidth"); %>">&nbsp;kbits
				</td>
			</tr>
			<tr>
				<td colspan="2" height="20" valign="middle" align="left" class="style14"><script>dw(qosTable)</script> :&nbsp;&nbsp;</td>
			</tr>
		</table>
		<table width="600" border="1" cellpadding="0" cellspacing="0" align="center" bgcolor="#FFFFFF" class="style14">
			<tr class="style13">
				<td height="20" bgcolor="#666666" width="10%" align="center"><script>dw(qosPrior)</script></td>
				<td height="20" bgcolor="#666666" width="30%" align="center"><script>dw(qosName)</script></td>
				<td height="20" bgcolor="#666666" width="25%" align="center"><script>dw(qosUpload)</script></td>
				<td height="20" bgcolor="#666666" width="25%" align="center"><script>dw(qosDnload)</script></td>
				<td height="20" bgcolor="#666666" width="10%" align="center"><script>dw(qosSelect)</script></td>
			</tr>
			<% Wan1QosList(); %>
		</table>
		<br>
		<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
			<tr>
				<td height="20" valign="middle" align="right">
					<script>
						document.write('<input type="button" name="addQos" value="'+showText(add)+'" onClick="replWeb();"  class="btnsize">');
						document.write('<input type="submit" name="showQos" value="'+showText(edit)+'" onClick="edit_apply();return editClick(document.formWan1Qos);"  class="btnsize">');
						document.write('<input type="submit" name="delSelQos" value="'+showText(deleteSelected)+'" onClick="return Clickdelete()" class="btnsize">');
						document.write('<input type="submit" name="delAllQos" value="'+showText(deleteAll)+'" onClick="return ClickdeleteAll()" class="btnsize">');
						document.write('<input type="submit" name="moveUpQos" value="'+showText(moveUp)+'" onClick="return checkNum(document.formWan1Qos);" class="btnsize">');
						document.write('<input type="submit" name="moveDownQos" value="'+showText(moveDown)+'" onClick="return checkNum(document.formWan1Qos);" class="btnsize">');
					</script>
					<input type="hidden" value="/wanqos.asp" name="submitUrl">
					<input type="hidden" value="1" name="configWan">
				</td>
			</tr>
		</table>
	</form>
	<form action=/goform/formQoS method=POST name="qosApply">
		<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
			<tr>
				<td height="20" valign="middle" align="right" class="style1">
					<input type=submit value="Save" onclick="return clickapply()" style ="width:100px">
					<input type="hidden" value="/wanqos.asp" name="submitUrl">
					<input type="hidden" name="maxup">
					<input type="hidden" name="maxdown">
				</td>
			</tr>
		</table>
	</form>
</body>
</html>
<script>
	if(typeof parent.document.getElementById("AQOSFrame").contentDocument == "undefined")
		parent.document.getElementById("AQOSFrame").height = parent.document.getElementById("AQOSFrame").contentWindow.document.body.scrollHeight+40;
	else
		parent.document.getElementById("AQOSFrame").height = parent.document.getElementById("AQOSFrame").contentWindow.document.body.scrollHeight;
	if(parent.editapply)
	{
		openWindow("/wanqosadd.asp");
		parent.editapply = 0
	}
	if ( entryNum == 0 ) disableDelButton(document.formWan1Qos)
	if ( entryNum < 2 ) enmvButton(document.formWan1Qos)
</script>
