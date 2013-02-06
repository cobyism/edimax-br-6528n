<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="file/javascript.js"></script>
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
<title></title>
<script>
var strddns = "<% getInfo("ddnsName"); %>"
function saveChanges()
{
	if (document.ddns.ddnsEnable[1].checked==true)
		return true;
	if (document.ddns.ddnspvidSel.value!="dhis") {
		if (!strRule(document.ddns.ddnsName, showText(domainName)))
			return false;
		if (!strRule(document.ddns.ddnsAccount, showText(accoutEm)))
			return false;
		if (!strRule(document.ddns.ddnsPass,showText(ddnsPass)))
			return false;
	} else {
		if (!strRule(document.ddns.dhisHostID,showText(hostID)))
			return false;
		if ( validateKey( document.ddns.dhisHostID.value ) == 0 ) {
			alert(showText(wanddnsAlert));
			setFocus(document.ddns.dhisHostID);
			return false;
		}
		if (document.ddns.dhisISAddr.value=="") {
    		alert(showText(wanddnsAlert2));
			setFocus(document.ddns.dhisISAddr);
			return false;
		}
		if (document.ddns.dhisSelect.value == 0 ) {
			if (!strRule(document.ddns.dhispass,showText(hostPass)))
				return false;
		}
	}
   return true;
}
function DDNSEnable() {
	with(document.ddns) {
		if (ddnsEnable[0].checked==false) {
			ddnspvidSel.disabled = true;
			ddnsName.disabled = true;
			ddnsAccount.disabled = true;
			ddnsPass.disabled = true;
			dhisHostID.disabled = true;
			dhisISAddr.disabled = true;
			dhisSelect.disabled = true;
			dhispass.disabled = true;
			dhisAuthP1.disabled = true;
			dhisAuthP2.disabled = true;
			dhisAuthQ1.disabled = true;
			dhisAuthQ2.disabled = true;
		}
		else{
			ddnspvidSel.disabled = false;
			if(ddnspvidSel.value == "ns2go") ddnsName.disabled = true;
			else ddnsName.disabled = false;
			ddnsAccount.disabled = false;
			ddnsPass.disabled = false;
			dhisHostID.disabled = false;
			dhisISAddr.disabled = false;
			dhisSelect.disabled = false;
			dhispass.disabled = false;
			dhisAuthP1.disabled = false;
			dhisAuthP2.disabled = false;
			dhisAuthQ1.disabled = false;
			dhisAuthQ2.disabled = false;
		}
	}
}
function displayObj() {
	if (document.ddns.ddnspvidSel.value != "dhis") {
		document.getElementById('genId').style.display = "block";
		document.getElementById('dhisId').style.display = "none";
		document.getElementById('passId').style.display = "none";
		document.getElementById('authId').style.display = "none";
		if(document.ddns.ddnspvidSel.value == "ns2go")
		{
			document.ddns.ddnsName.value="www.ns2go.com";
			document.ddns.ddnsName.disabled=true;
		}
		else
		{
			document.ddns.ddnsName.value=strddns;
			if (document.ddns.ddnsEnable[0].checked==false) document.ddns.ddnsName.disabled=true;
			else document.ddns.ddnsName.disabled=false;
		}
	}
	else {
		document.getElementById('genId').style.display = "none";
		document.getElementById('dhisId').style.display = "block";
		if (document.ddns.dhisSelect.value == 0) {
			document.getElementById('passId').style.display = "block";
			document.getElementById('authId').style.display = "none";
		}
		else {
			document.getElementById('passId').style.display = "none";
			document.getElementById('authId').style.display = "block";
		}
	}
}
</script>
</head>
<body bgcolor="#EFEFEF">
<form action=/goform/formDDNSSetup method=POST name="ddns">
	<input type="hidden" value="<% getInfo("ddnspvidSel"); %>" name="ddnspvidSelGet">
	<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
		<tr>
			<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(DynamicDNS)</script> :&nbsp;&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="radio" name="ddnsEnable"onClick="DDNSEnable();" value="1" ><script>dw(enabled)</script>
				<input type="radio" name="ddnsEnable"onClick="DDNSEnable();" value="0" ><script>dw(disabled)</script>
			</td>
		</tr>
		<script>
			<%
				ddnsNum = getIndex("ddnsEnable");
				if ( ddnsNum == 0 ) write('document.getElementsByName("ddnsEnable")[1].checked = true;');
				if ( ddnsNum == 1 ) write('document.getElementsByName("ddnsEnable")[0].checked = true;');
			%>
		</script>
		<tr>
			<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(provider)</script> :&nbsp;&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<select name="ddnspvidSel" onChange="displayObj();">
					<script>
						var PvidTbl = new Array("qdns","dhs","dyndns","ods","tzo","gnudip","dyns","zoneedit","dhis","cybergate","ns2go");
						var PvidName = new Array("3322(qdns)","DHS","DynDNS","ODS","TZO","GnuDIP","DyNS","ZoneEdit","DHIS","CyberGate","NS2GO");
						for (i=0;i<11;i++) {
							if(PvidTbl[i]==document.ddns.ddnspvidSelGet.value)
								document.write("<option value=\""+PvidTbl[i]+"\" selected>"+PvidName[i]+"</option>");
							else
								document.write("<option value=\""+PvidTbl[i]+"\">"+PvidName[i]+"</option>");
						}
					</script>
				</select>
			</td>
		</tr>
	</table>
	<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="genId">
		<tr>
			<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(domainName)</script> :&nbsp;&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="text" name="ddnsName" size="25" maxlength="30" value="">
			</td>
			<script>document.ddns.ddnsName.value=strddns</script>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(account)</script> :&nbsp;&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="text" name="ddnsAccount" size="25" maxlength="30" value="<% getInfo("ddnsAccount"); %>">
			</td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(passKey)</script> :&nbsp;&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="password" name="ddnsPass" size="25" maxlength="30" value="<% getInfo("ddnsPass"); %>">
			</td>
		</tr>
	</table>
	<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="dhisId">
		<tr>
			<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(hostID)</script> :&nbsp;&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="text" name="dhisHostID" size="25" maxlength="10" value="<% getInfo("dhisHostID"); %>">
			</td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="right" class="style14">ISAddr  :&nbsp;&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="text" name="dhisISAddr" size="25" maxlength="30" value="<% getInfo("dhisISAddr"); %>">
			</td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(authenType)</script> :&nbsp;&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
			<select name="dhisSelect" onClick="displayObj();">
				<script>
					<%	write("dhisSelectGet = "+getIndex("dhisSelect")+";");%>
					var authTypeTbl = new Array("password","QRC");
					for (i=0;i<2;i++) {
						if(dhisSelectGet == i)
							document.write("<option value=\""+i+"\" selected>"+authTypeTbl[i]+"</option>");
						else
							document.write("<option value=\""+i+"\">"+authTypeTbl[i]+"</option>");
					}
				</script>
			</select>
			</td>
		</tr>
	</table>
	<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="passId">
		<tr>
			<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(hostPass)</script> :&nbsp;&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="password" name="dhispass" size="25" maxlength="30" value="<% getInfo("dhispass"); %>">
			</td>
		</tr>
	</table>
	<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="authId">
		<tr>
			<td width="200" height="20" valign="middle" align="right" class="style14">AuthP  :&nbsp;&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="text" name="dhisAuthP1" size="25" maxlength="50" value="<% getInfo("dhisAuthP1"); %>">
			</td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="right" class="style14">AuthP  :&nbsp;&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="text" name="dhisAuthP2" size="25" maxlength="50" value="<% getInfo("dhisAuthP2"); %>">
			</td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="right" class="style14">AuthQ  :&nbsp;&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="text" name="dhisAuthQ1" size="25" maxlength="50" value="<% getInfo("dhisAuthQ1"); %>">
			</td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="right" class="style14">AuthQ  :&nbsp;&nbsp;</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="text" name="dhisAuthQ2" size="25" maxlength="50" value="<% getInfo("dhisAuthQ2"); %>">
			</td>
		</tr>
	</table>
	<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
		<tr>
			<td height="20" valign="middle" align="right" class="style1">
				<script>document.write('<input type=submit value="'+showText(save)+'" name="save" onclick="return saveChanges();" style ="width:100px">');</script>
				<input type="hidden" value="/wanddns.asp" name="submit-url">
			</td>
		</tr>
	</table>
</form>
</body>
</html>
<script>
	DDNSEnable();
	displayObj();
	if(typeof parent.document.getElementById("DDNSFrame").contentDocument == "undefined")
		parent.document.getElementById("DDNSFrame").height = parent.document.getElementById("DDNSFrame").contentWindow.document.body.scrollHeight+40;
	else
		parent.document.getElementById("DDNSFrame").height = parent.document.getElementById("DDNSFrame").contentWindow.document.body.scrollHeight;
</script>
