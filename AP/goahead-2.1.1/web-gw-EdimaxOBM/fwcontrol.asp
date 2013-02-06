<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title>MAC Filtering</title>
<script language ='javascript' src ='file/javascript.js'></script>
<script language ='javascript' src ='file/fwwl-n.var'></script>
<script language ='javascript' src ='file/netsys-n.var'></script>
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
<% write("MFenable = "+getIndex("macFilterEnabled"));%>
function addClick() {
	if (document.formFilterAdd.mac.value=="" && document.formFilterAdd.comment.value=="" )
		return true;
	var str = document.formFilterAdd.mac.value;
	if ( str.length < 12) {
		alert(showText(fwControlAlertNot));
		document.formFilterAdd.mac.focus();
		return false;
	}
	for (var i=0; i<str.length; i++) {
		if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) >= 'a' && str.charAt(i) <= 'f') || (str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
			continue;
		alert(showText(fwControlAlertInv));
		document.formFilterAdd.mac.focus();
		return false;
	}
	return true;
}
function disableDelACPCButton() {
	document.formACPCDel.deleteSelACPC.disabled = true;
	document.formACPCDel.deleteAllACPC.disabled = true;
}
function disableDelButton() {
	document.formFilterDel.deleteSelFilterMac.disabled = true;
	document.formFilterDel.deleteAllFilterMac.disabled = true;
}
function Change() {
	openWindow("/fwaddpc.asp");
}
function goToWeb() {
	if (document.formFilterAdd.enabled.checked==true) {
		document.formFilterEnabled.enabled.value="ON";
		if (macEntryNum == 0)
			alert(showText(fwControlAlertMac));
	}
	document.formFilterEnabled.submit();
}
function goToIpWeb() {
	if (document.formACPCDel.enabled.checked==true) {
		document.formIpFilterEnabled.enabled.value="ON";
		if (ipEntryNum == 0)
			alert(showText(fwControlAlertIp));
	}
	document.formIpFilterEnabled.submit();
}
//Support NetBiosName add by Kyle 2007/12/12
var nameList = new Array(<% getInfo("getnetName");%> new Array("0","0","0"));
function addComputerName(){
		document.forms['formFilterAdd'].elements['mac'].value = document.forms['formFilterAdd'].elements['comList'].value;
}
function loadNetList(){
	for(i=0;i<nameList.length-1;i++){
		document.forms['formFilterAdd'].elements['comList'].options[i+1]=new Option (nameList[i][1], nameList[i][2]);
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
function checkmac(input) {
var x = input
if (!x.value.match(/^[A-Fa-f0-9]{12}$/) || x.value == "" || x.value == "000000000000" || x.value == "ffffffffffff" || x.value == "FFFFFFFFFFFF") {
alert("Invalid MAC Address")
x.value = x.defaultValue
x.focus()
return false
}
else
return true
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
<form action=/goform/formFilter method=POST name="formFilterEnabled">
	<input type="hidden" value="Add" name="addFilterMac">
	<input type="hidden" value="" name="enabled">
	<input type="hidden" value="/fwcontrol.asp" name="submit-url">
</form>
<form action=/goform/formFilter method=POST name="formIpFilterEnabled">
	<input type="hidden" value="Add" name="addACPC">
	<input type="hidden" value="" name="enabled">
	<input type="hidden" value="/fwcontrol.asp" name="submit-url">
</form>
<form action=/goform/formFilter method=POST name="formFilterAdd">
	<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
		<tr>
			<td colspan="3" height="20" valign="middle" align="left" class="style14">
				<script>
					<% write("MDenyenable = "+getIndex("macDenyEnabled")); %>
					if (MFenable) document.write('<input type="checkbox" name="enabled" value="ON" onClick="goToWeb();" checked>&nbsp;&nbsp;'+showText(fwMacEn)+'&nbsp;:')
					else  document.write('<input type="checkbox" name="enabled" value="ON" onClick="goToWeb();">&nbsp;&nbsp;'+showText(fwMacEn)+'&nbsp;:')
					if(MDenyenable)
					{
						document.write('<input type="radio" name="macDenyEnabled" value="yes" onClick="document.formFilterAdd.submit();"checked>'+showText(deny)+'&nbsp;&nbsp;')
						document.write('<input type="radio" name="macDenyEnabled" value="no" onClick="document.formFilterAdd.submit();">'+showText(allow))
					}
					else
					{
						document.write('<input type="radio" name="macDenyEnabled" value="yes" onClick="document.formFilterAdd.submit();">'+showText(deny)+'&nbsp;&nbsp;')
						document.write('<input type="radio" name="macDenyEnabled" value="no" onClick="document.formFilterAdd.submit();" checked>'+showText(allow))
					}
				</script>
			</td>
		</tr>
		<tr>
			<td align="center" height="20" bgcolor="#666666" width="200" class="style13"><script>dw(fwClientMac);</script></td>
			<td align="center" height="20" bgcolor="#666666" width="200" class="style13"><script>dw(computerName);</script></td>
			<td align="center" height="20" bgcolor="#666666" width="200" class="style13"><script>dw(comment);</script></td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="center" class="style1"><input type="text" name="mac" size="15" maxlength="12" onchange="checkmac(this)"></td>
			<td width="200" height="20" valign="middle" align="center" class="style1">
				<input type="button" value="<<" onclick="addComputerName()" style="width: 22; height: 22">
				<select size="1" name= "comList" style="width: 130" onChange="fresh(this.value);">
					<option value="0.0.0.0">--------- Select ---------</option>
					<option value="refresh">--------- Refresh --------</option>
				</select>
			</td>
			<td width="200" height="20" valign="middle" align="center" class="style1"><input type="text" name="comment" size="16" maxlength="16" onchange="check(this)"></td>
		</tr>
	</table>
	<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
		<tr>
			<td colspan="4" align="right">
			<script>
				document.write('<input type="submit" class="btnsize" value="'+showText(add)+'" name="addFilterMac" onClick="return addClick()">');
				document.write('<input type="reset" class="btnsize" value="'+showText(reset)+'" name="reset" onClick="document.formFilterAdd.reset;">');
			</script>
			</td>
		</tr>
		<input type="hidden" value="/fwcontrol.asp" name="submit-url">
	</table>
</form>
<form action=/goform/formFilter method=POST name="formFilterDel">
	<script>
		if(MFenable) document.write('<input type="hidden" name="enabled" value="ON"')
		else  document.write('<input type="hidden" name="enabled" value=""')
	</script>
	<table width="700" border="0" cellpadding="0" cellspacing="1" class="style14">
		<tr>
			<td colspan="5" height="20" valign="middle" align="left"><script>dw(fwMacTable)</script>:&nbsp;</td>
		</tr>
	</table>
	<table width="700" border="1" cellpadding="0" cellspacing="1" align="center" class="style14" bgcolor="#FFFFFF">
		<tr>
			<td align="center" height="20" bgcolor="#666666" width="100" class="style13">NO.</td>
			<td align="center" height="20" bgcolor="#666666" width="200" class="style13"><script>dw(computerName)</script></td>
			<td align="center" height="20" bgcolor="#666666" width="300" class="style13"><script>dw(fwClientMac)</script></td>
			<td align="center" height="20" bgcolor="#666666" width="150" class="style13"><script>dw(comment)</script></td>
			<td align="center" height="20" bgcolor="#666666" width="150" class="style13"><script>dw(select)</script></td>
		</tr>
		<% macFilterList(); %>
	</table>
	<table width="700" border="0" cellpadding="0" cellspacing="1">
		<tr>
			<td colspan="5" align="right">
				<script>
					document.write('<input type="submit" value="'+showText(deleteSelected)+'" name="deleteSelFilterMac" onClick="return deleteClick()" class="btnsize">');
					document.write('<input type="submit" value="'+showText(deleteAll)+'" name="deleteAllFilterMac" onClick="return deleteAllClick()" class="btnsize">');
				</script>
			</td>
		</tr>
		<script>
			<%	write("macEntryNum = "+getIndex("macFilterNum")+";");%>
			if (macEntryNum == 0)
			disableDelButton();
		</script>
	</table>
	<input type="hidden" value="/fwcontrol.asp" name="submit-url">
</form>
<form action=/goform/formFilter method=POST name="formACPCDel">
	<table width="700" border="0" cellpadding="2" cellspacing="2" align="center">
		<tr>
			<td height="20" valign="middle" align="left" class="style14">
				<input type="checkbox" name="enabled" value="ON" <% if (getIndex("ACPCEnabled")) write("checked");%> onClick="goToIpWeb();">&nbsp;&nbsp;<script>dw(fwIPEn);</script>
				<script>
					<% write("IPDenyenable = "+getIndex("ipDenyEnabled")); %>
					if(IPDenyenable)
					{
						document.write('<input type="radio" name="ipDenyEnabled" value="yes" onClick="document.formACPCDel.submit();" checked>'+showText(deny)+'&nbsp;&nbsp;')
						document.write('<input type="radio" name="ipDenyEnabled" value="no" onClick="document.formACPCDel.submit();">'+showText(allow))
					}
					else
					{
						document.write('<input type="radio" name="ipDenyEnabled" value="yes" onClick="document.formACPCDel.submit();">'+showText(deny)+'&nbsp;&nbsp;')
						document.write('<input type="radio" name="ipDenyEnabled" value="no" onClick="document.formACPCDel.submit();" checked>'+showText(allow))
					}
				</script>
			</td>
		</tr>
	</table>
	<br>
	<table width="700" border="0" cellpadding="0" cellspacing="1" class="style14">
		<tr><td colspan="7" height="20" valign="middle" align="left">IP Filtering Table : </td></tr>
	</table>
	<table width="700" border="1" cellpadding="0" cellspacing="1" align="center" class="style14" bgcolor="#FFFFFF">
		<tr>
			<td align="center" height="20" bgcolor="#666666" class="style13">NO.</td>
			<td align="center" height="20" bgcolor="#666666" class="style13"><script>dw(fwClient)</script></td>
			<td align="center" height="20" bgcolor="#666666" class="style13"><script>dw(fwClientIP)</script></td>
			<td align="center" height="20" bgcolor="#666666" class="style13"><script>dw(fwCS)</script></td>
			<td align="center" height="20" bgcolor="#666666" class="style13"><script>dw(protocol)</script></td>
			<td align="center" height="20" bgcolor="#666666" class="style13"><script>dw(portRange)</script></td>
			<td align="center" height="20" bgcolor="#666666" class="style13"><script>dw(select)</script></td>
		</tr>
		<% ACPCList(); %>
	</table>
	<table width="700" border="0" cellpadding="0" cellspacing="1" class="style14">
		<tr>
			<td align="right">
				<script>
					document.write('<input type="button" value="'+showText(addPc)+'" name ="addPc" onClick="Change();" class="btnsize">');
					document.write('<input type="submit" value="'+showText(deleteSelected)+'" name="deleteSelACPC" onClick="return deleteClick()" class="btnsize">');
					document.write('<input type="submit" value="'+showText(deleteAll)+'" name="deleteAllACPC" onClick="return deleteAllClick()" class="btnsize">')
				</script>
			</td>
		</tr>
		<input type="hidden" value="/fwcontrol.asp" name="submit-url">
		<script>
			<%	write("ipEntryNum = "+getIndex("ACPCNum")+";");%>
			if ( ipEntryNum == 0 )
			disableDelACPCButton();
		</script>
	</table>
</form>
<form action="/goform/formrefresh" method="POST" name="name_fresh">
	<input type="hidden" name="submit-url" value="/fwcontrol.asp">
</form>
</body>
</html>
<script>
	loadNetList();
	if(typeof parent.document.getElementById("ACFrame").contentDocument == "undefined")
		parent.document.getElementById("ACFrame").height = parent.document.getElementById("ACFrame").contentWindow.document.body.scrollHeight+40;
	else
		parent.document.getElementById("ACFrame").height = parent.document.getElementById("ACFrame").contentWindow.document.body.scrollHeight;
</script>
