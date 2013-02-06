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
function addClick()
{
	if (document.formPortFwAdd.ip.value=="" && document.formPortFwAdd.fromPort.value=="" &&
		document.formPortFwAdd.toPort.value=="" && document.formPortFwAdd.comment.value=="" )
		return true;
	if ( !ipRule( document.formPortFwAdd.ip, showText(ipAddress), "ip", 0))
		return false;
	if (!portRule(document.formPortFwAdd.fromPort, showText(natPortPri), 0, "", 1, 65535, 0))
		return false;
	if (!portRule(document.formPortFwAdd.toPort, showText(natPortPub), 0, "", 1, 65535, 0))
		return false;
	//Fixed remote management port the same with Virtual Server. Kyle 20081027
	var remoteManagementPort = <% getInfo("reManPort");%>
	var remoteManagementEnable=<%write(getIndex("reMangEnable"));%>
	if(remoteManagementEnable == 1 && document.formPortFwAdd.toPort.value == remoteManagementPort ){
		alert("Port "+remoteManagementPort +" is the same with remote management port");
		return false;
	}
	//end Fixed
   return true;
}
function disableDelButton()
{
	document.formPortFwDel.deleteSelPortFw.disabled = true;
	document.formPortFwDel.deleteAllPortFw.disabled = true;
}
function goToWeb() {
	document.formPortFwAdd.submit();
}
function fresh(select) {
	if (select == "refresh")
		document.name_fresh.submit()
}
</script>
<script>
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
<form action=/goform/formVirtualSv method=POST name="formPortFwAdd">
	<table width="700" border="0" cellpadding="0" cellspacing="0" align="center">
		<tr>
			<td colspan="6" height="20" valign="middle" align="left" class="style14">
				<input type="checkbox" name="enabled" value="ON" <% if (getIndex("virtSVEnabled")) write("checked");%> onClick="goToWeb();">
				<script>dw(enNatVrServer)</script>
			</td>
		</tr>
	</table>
	<table width="700" border="0" cellpadding="0" cellspacing="1" align="center">
		<tr align="center">
			<td bgcolor="#666666" class="style13"><script>dw(natPortPriIP)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(computerName)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(natPortPri)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(type)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(natPortPub)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(comment)</script></td>
		</tr>
		<tr align="center">
			<td bgcolor="#FFFFFF" class="style1"><input type="text" name="ip" size="13" maxlength="15"></td>
			<td bgcolor="#FFFFFF" class="style1">
				<input type="button" value="<<" onclick="addComputerName()" style="width: 22; height: 22">
				<select size="1" name= "comList" onchange="fresh(this.value)" style="width: 130">
					<option value="0.0.0.0">--------- Select ---------</option>
					<option value="refresh">--------- Refresh --------</option>
				</select>
			</td>
			<td bgcolor="#FFFFFF" class="style1"><input type="text" name="fromPort" size="3" maxlength="5"></td>
			<td bgcolor="#FFFFFF" class="style1">
				<select name="protocol">
					<option selected value="0"><script>dw(botholitec)</script></option>
					<option value="1">TCP</option>
					<option value="2">UDP</option>
				</select>
			</td>
			<td bgcolor="#FFFFFF" class="style1"><input type="text" name="toPort" size="3" maxlength="5"></td>
			<td bgcolor="#FFFFFF" class="style1"><input type="text" name="comment" size="10" maxlength="10" onchange="check(this)"></td>
		</tr>
	<tr align="right">
		<td colspan="6">
			<input type="submit" value="" name="addPortFw" onClick="return addClick()" class="btnsize">
			<input type="reset" value="" name="reset" onClick="document.formPortFwAdd.reset;" class="btnsize">
			<input type="hidden" value="/natvser.asp" name="submit-url">
			<script language ="javascript">
				document.formPortFwAdd.addPortFw.value =showText(add);
				document.formPortFwAdd.reset.value =showText(reset);
			</script>
		</td>
	</tr>
	</table>
</form>
<br>
<form action=/goform/formVirtualSv method=POST name="formPortFwDel">
	<input type="hidden" name="enabled" value="<% if (getIndex("virtSVEnabled")) write("ON");%>">
	<table width="700" border="0" cellpadding="0" cellspacing="1" align="center">
		<tr><td colspan="8" align="left" class="style14"><script>dw(natVrTable)</script>:</td></tr>
	</table>
	<table width="700" border="1" cellpadding="0" cellspacing="1" align="center" bgcolor="#FFFFFF" class="style14">
		<tr align="center">
			<td bgcolor="#666666" class="style13">NO.</td>
			<td bgcolor="#666666" class="style13"><script>dw(computerName)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(natPortPriIP)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(natPortPri)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(type)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(natPortPub)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(comment)</script></td>
			<td bgcolor="#666666" class="style13"><script>dw(select)</script></td>
		</tr>
		<% virtualSvList(); %>
	</table>
	<p align="right">
		<script>
			document.write('<input type="submit" value="'+showText(deleteSelected)+'" name="deleteSelPortFw" onClick="return deleteClick()" class="btnsize">');
			document.write('<input type="submit" value="'+showText(deleteAll)+'" name="deleteAllPortFw" onClick="return deleteAllClick()" class="btnsize">');
		</script>
	</p>
	<script>
		<% entryNum = getIndex("vserNum");
		if ( entryNum == 0 ) {
		write( "disableDelButton();" );
		} %>
	</script>
	<input type="hidden" value="/natvser.asp" name="submit-url">
</form>
<form action="/goform/formrefresh" method="POST" name="name_fresh">
	<input type="hidden" name="submit-url" value="/natvser.asp">
</form>
</body>
</html>
<script>
	loadNetList();
	if(typeof parent.document.getElementById("VSFrame").contentDocument == "undefined")
		parent.document.getElementById("VSFrame").height = parent.document.getElementById("VSFrame").contentWindow.document.body.scrollHeight+40;
	else
		parent.document.getElementById("VSFrame").height = parent.document.getElementById("VSFrame").contentWindow.document.body.scrollHeight+40;
</script>
