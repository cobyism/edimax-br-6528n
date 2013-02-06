<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title>DMZ Settings</title>
<script language ='javascript' src ='file/javascript.js'></script>
<script language ='javascript' src ='file/fwwl-n.var'></script>
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
function addClick()
{
	if (document.formFilterAdd.dmzType[1].checked == true) {
		if ( !ipRule( document.formFilterAdd.pip, showText(pubIpAddress), "ip", 0))
			return false;
	}
	if ( !ipRule( document.formFilterAdd.cip, showText(clientIpAddress), "ip", 0))
		return false;
  return true;
}
function disableDelButton()
{
	document.formFilterDel.delSelDMZ.disabled = true;
	document.formFilterDel.delAllDMZ.disabled = true;
}
function goToWeb() {
	if (document.formFilterAdd.enabled.checked==true)
		document.formDMZEnabled.enabled.value="ON";
	document.formDMZEnabled.submit();
}
function goToApply() {
	document.filtApply.submit();
}
function disType() {
	if (document.formFilterAdd.dmzType[0].checked == true ) {
		document.formFilterAdd.session.disabled = false;
		document.formFilterAdd.pip.disabled = true;
	}
	else{
		document.formFilterAdd.session.disabled = true;
		document.formFilterAdd.pip.disabled = false;
	}
}
</script>
<script>
//Support NetBiosName add by Kyle 2007/12/12
var nameList = new Array(<% getInfo("getnetName");%> new Array("0","0","0"));
function addComputerName(){
		document.forms['formFilterAdd'].elements['cip'].value = document.forms['formFilterAdd'].elements['comList'].value;
}
function loadNetList(){
	for(i=0;i<nameList.length-1;i++){
		document.forms['formFilterAdd'].elements['comList'].options[i+1]=new Option (nameList[i][1], nameList[i][0]);
	}
	document.forms['formFilterAdd'].elements['comList'].options[i+1]=new Option ("--------- Refresh --------", "refresh");
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
<form action=/goform/formFilter method=POST name="formDMZEnabled">
	<input type="hidden" value="Add" name="addDMZ">
	<input type="hidden" value="" name="enabled">
	<input type="hidden" value="/fwdmz.asp" name="submit-url">
</form>
<form action=/goform/formFilter method=POST name="formFilterAdd">
<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
	<tr>
		<td width="200" height="20" valign="middle" align="left" class="style14">
			<input type="checkbox" name="enabled" value="ON" <% if (getIndex("dmzEnabled")) write("checked");%> onClick="goToWeb();" >&nbsp;&nbsp;
			<script>dw(enableDMZ);</script>
		</td>
	</tr>
</table>
<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
	<tr>
		<td height="20" bgcolor="#666666" class="style13" width="250" align="center"><script>dw(pubIpAddress)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="150" align="center"><script>dw(clientPcIpAddress)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="200" align="center"><script>dw(computerName)</script></td>
	</tr>
	<tr>
		<td height="20" valign="middle" align="left" class="style14">
			<input type="radio" name="dmzType" value="0" checked onClick="disType();"><script>dw(dynamicIp);</script>
			<select name="session">
				<script>
					for (i=1; i<=1; i++) {
					if (i == 1)
					document.write('<option selected value="'+i+'">Session '+i+'</option>');
					else
					document.write('<option value="'+i+'">Session '+i+'</option>');
					}
				</script>
			</select>
		</td>
		<td rowspan="2" height="20" valign="middle" align="center" valign="middle" class="style14">
			<input type="text" name="cip" size="15" maxlength="15">
		</td>
		<td rowspan="2" height="20" valign="middle" align="center" valign="middle" class="style14">
			<input type="button" value="<<" name="addCom" onclick="addComputerName()" style="width: 22; height: 22">
			<select size="1" name= "comList" onchange="fresh(this.value)" style="width: 130">
				<option value="0.0.0.0">--------- Select ---------</option>
				<option value="refresh">--------- Refresh --------</option>
			</select>
		</td>
	</tr>
	<tr>
		<td height="20" valign="middle" align="left" class="style14">
			<input type="radio" name="dmzType" value="1" onClick="disType();"><script>dw(staticsIp);</script>
			<input type="text" name="pip" size="15" maxlength="15">
		</td>
	</tr>
	<tr>
		<td colspan="3" height="20" valign="middle" align="right" class="style1">
			<script>
				document.write('<input type="submit" value="'+showText(add)+'" name="addDMZ" onClick="return addClick()" class="btnsize">');
				document.write('<input type="reset" value="'+showText(reset)+'" name="reset" onClick="document.formFilterAdd.reset;" class="btnsize">');
			</script>
			<input type="hidden" value="/fwdmz.asp" name="submit-url">
		</td>
	</tr>
</table>
</form>
<form action=/goform/formFilter method=POST name="formFilterDel">
<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
	<tr>
		<td width="200" height="20" valign="middle" align="left" class="style14"><script>dw(fwDmzTable)</script> :&nbsp;&nbsp;</td>
	</tr>
</table>
<table width="700" border="1" cellpadding="0" cellspacing="0" align="center" bgcolor="#FFFFFF" class="style14">
	<tr>
		<td height="20" bgcolor="#666666" class="style13" width="5%" align="center">NO.</td>
		<td height="20" bgcolor="#666666" class="style13" width="12%" align="center"><script>dw(computerName)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="12%" align="center"><script>dw(pubIpAddress)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="22%" align="center"><script>dw(clientPcIpAddress)</script></td>
		<td height="20" bgcolor="#666666" class="style13" width="12%" align="center"><script>dw(select)</script></td>
	</tr>
	<% DMZList(); %>
</table>
<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
	<tr>
		<td height="20" valign="middle" align="right" class="style1">
			<script>
				document.write('<input type="submit" value="'+showText(deleteSelected)+'" name="delSelDMZ" onClick="return deleteClick()" class="btnsize">');
				document.write('<input type="submit" value="'+showText(deleteAll)+'" name="delAllDMZ" onClick="return deleteAllClick()" class="btnsize">');
			</script>
			<input type="hidden" value="/fwdmz.asp" name="submit-url">
			<script>
				disType();
				<%
					entryNum = getIndex("dmzNum");
					if ( entryNum == 0 ) {
					write( "disableDelButton();" );
					}
				%>
			</script>
		</td>
	</tr>
</table>
</form>
<form action="/goform/formrefresh" method="POST" name="name_fresh">
	<input type="hidden" name="submit-url" value="/fwdmz.asp">
</form>
</body>
</html>
<script>
	loadNetList();
	if(typeof parent.document.getElementById("DMZFrame").contentDocument == "undefined")
		parent.document.getElementById("DMZFrame").height = parent.document.getElementById("DMZFrame").contentWindow.document.body.scrollHeight+40;
	else
		parent.document.getElementById("DMZFrame").height = parent.document.getElementById("DMZFrame").contentWindow.document.body.scrollHeight;
</script>
