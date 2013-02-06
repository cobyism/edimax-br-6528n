<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title></title>
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
function disableDelButton()
{
	document.formPortFwDel.deleteSelPortFw.disabled = true;
	document.formPortFwDel.deleteAllPortFw.disabled = true;
}
function addClick()
{
strIp = document.formPortFwAdd.ip;
strFromPort = document.formPortFwAdd.fromPort;
strToPort = document.formPortFwAdd.toPort;
strComment = document.formPortFwAdd.comment;
	if (strIp.value=="" && strFromPort.value=="" &&	strToPort.value=="" && strComment.value=="" )
		return true;
	if ( !ipRule( strIp, showText(natpfwStrIp), "ip", 0))
		return false;
	if (!portRule(strFromPort, showText(natpfwStrPort), strToPort, showText(natpfwStrPort), 1, 65535, 0))
		return false;
	return true;
}
function goToWeb() {
  document.formPortFwAdd.submit();
}
function goToApply() {
	if(document.formPortFwAdd.enabled.checked == true)
		document.formPortFwApply.enabled.value = "ON"
	document.formPortFwApply.submit();
}
//Support NetBiosName add by Kyle 2007/12/12
var nameList = new Array(<% getInfo("getnetName");%> new Array("0","0","0"));
function addComputerName(){
		document.forms['formPortFwAdd'].elements['ip'].value = document.forms['formPortFwAdd'].elements['comList'].value;
}
function loadNetList(){
	for(i=0;i<nameList.length-1;i++){
		document.forms['formPortFwAdd'].elements['comList'].options[i+1]=new Option (nameList[i][1], nameList[i][0]);
	}
	document.forms['formPortFwAdd'].elements['comList'].options[i+1]=new Option ("--------- Refresh --------", "refresh");
}
function searchComName(ipAdr,type){
	var comName="OFFLINE";
	for (i=0;i<nameList.length-1;i++){
		if(ipAdr==nameList[i][type]){
			comName=nameList[i][1];
			break;
		}
	}
	return comName;
}
function fresh(select) {
	if (select == "refresh")
		document.name_fresh.submit()
}
</script>
</head>
<body bgcolor="#EFEFEF">
<form action=/goform/formPortFw method=POST name="formPortFwAdd">
<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
	<tr>
		<td colspan="2" height="20" valign="middle" align="left" class="style1">
			<input type="checkbox" name="enabled" value="ON" onClick="goToWeb();">&nbsp;<script>dw(enNatPort);</script><br>
		</td>
	</tr>
	<script>
		<% write("portFw = "+getIndex("portFwEnabled")+";"); %>
		if(portFw) document.formPortFwAdd.enabled.checked = true;
	</script>
	<tr>
		<td height="20" bgcolor="#666666" class="style13" width="15%" align="center"><script>dw(natPortPriIP)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="35%" align="center"><script>dw(computerName)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="10%" align="center"><script>dw(type)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="25%" align="center"><script>dw(natPortRange)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="15%" align="center"><script>dw(comment)</script></td>
	</tr>
	<tr>
		<td height="20" valign="middle" align="left" class="style1"><input type="text" name="ip" size="12" maxlength="15"></td>
		<td height="20" valign="middle" align="center" class="style1">
			<input type="button" value="<<" name="addCom" onclick="addComputerName()" style="width: 22; height: 22">
			<select size="1" name= "comList" onchange="fresh(this.value)" style="width: 130">
				<option value="0.0.0.0">--------- Select ---------</option>
				<option value="refresh">--------- Refresh --------</option>
			</select>
		</td>
		<td height="20" valign="middle" align="left" class="style1">
			<select name="protocol">
				<option selected value="0"><script>dw(botholitec)</script></option>
				<option value="1">TCP</option>
				<option value="2">UDP</option>
			</select>
		</td>
		<td height="20" valign="middle" align="center" class="style1">
			<input type="text" name="fromPort" size="5" maxlength="5">-<input type="text" name="toPort" size="5" maxlength="5">
		</td>
		<td height="20" valign="middle" align="left" class="style1">
			<input type="text" name="comment" size="15" maxlength="15">
		</td>
	</tr>
</table>
<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
	<tr>
		<td height="20" valign="middle" align="right" class="style1">
			<script>
				document.write('<input type="submit" value="'+showText(add)+'" name="addPortFw" onClick="return addClick()" class="btnsize">');
				document.write('<input type="reset" value="'+showText(reset)+'" name="reset" onClick="document.formPortFwAdd.reset;" class="btnsize">');
			</script>
			<input type="hidden" value="/natpfw.asp" name="submit-url">
		</td>
	</tr>
</table>
</form>
<form action=/goform/formPortFw method=POST name="formPortFwDel">
<script>
	if(portFw) document.write('<input type="hidden" name="enabled" value="ON">')
	else  document.write('<input type="hidden" name="enabled" value="">')
</script>
<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
	<tr>
		<td width="200" height="20" valign="middle" align="left" class="style14"><script>dw(natPortTable)</script> :&nbsp;&nbsp;</td>
	</tr>
</table>
<table width="700" border="1" cellpadding="0" cellspacing="1" align="center" bgcolor="#FFFFFF" class="style14">
	<tr>
		<td height="20" bgcolor="#666666" class="style13" width="5%" align="center">NO.</td>
		<td height="20" bgcolor="#666666" class="style13" width="25%" align="center"><script>dw(computerName)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="12%" align="center"><script>dw(natPortPriIP)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="12%" align="center"><script>dw(type)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="12%" align="center"><script>dw(natPortRange)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="12%" align="center"><script>dw(comment)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="12%" align="center"><script>dw(select)</script></td>
	</tr>
	<% portFwList(); %>
</table>
<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
	<tr>
		<td height="20" valign="middle" align="right" class="style1">
			<script>
				document.write('<input type="submit" value="'+showText(deleteSelected)+'" name="deleteSelPortFw" onClick="return deleteClick()" class="btnsize">');
				document.write('<input type="submit" value="'+showText(deleteAll)+'" name="deleteAllPortFw" onClick="return deleteAllClick()" class="btnsize">');
			</script>
			<script>
				<% entryNum = getIndex("portFwNum");
				if ( entryNum == 0 ) {
				write( "disableDelButton();" );
				} %>
			</script>
			<input type="hidden" value="/natpfw.asp" name="submit-url">
		</td>
	</tr>
</table>
</form>
<form action="/goform/formrefresh" method="POST" name="name_fresh">
	<input type="hidden" name="submit-url" value="/natpfw.asp">
</form>
</body>
</html>
<script>
	loadNetList();
	if(typeof parent.document.getElementById("NPFrame").contentDocument == "undefined")
		parent.document.getElementById("NPFrame").height = parent.document.getElementById("NPFrame").contentWindow.document.body.scrollHeight+40;
	else
		parent.document.getElementById("NPFrame").height = parent.document.getElementById("NPFrame").contentWindow.document.body.scrollHeight;
</script>
