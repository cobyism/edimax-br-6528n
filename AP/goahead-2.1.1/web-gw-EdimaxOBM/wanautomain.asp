<html>
<head>
<meta http-equiv="Content-Type" content="text/html;charset=UTF-8" />
<link rel="stylesheet" type="text/css" href="file/set.css" />
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="file/multilanguage.var"></script>
<script type="text/javascript" src="FUNCTION_SCRIPT"></script>
<script type="text/javascript" src="/language_pack.txt"></script>
<script type="text/javascript">
function saveChanges() {
  if(document.tcpip.DNSMode[1].checked==true)
  {
    if(document.tcpip.dns1.value=="") document.tcpip.dns1.value="0.0.0.0";
    if(document.tcpip.dns2.value=="") document.tcpip.dns2.value="0.0.0.0";
    if ( !ipRule( document.tcpip.dns1, showText(IPAddr), "ip", 1)) return false;
    if ( !ipRule( document.tcpip.dns2, showText(IPAddr), "ip", 1)) return false;
  }
	if (document.tcpip.dynIPHostName.value=="" && document.tcpip.macAddr.value=="" ) {
		if ( document.tcpip.macAddr.value == "" )
			document.tcpip.macAddr.value="000000000000";
		return true;
	}
	var str = document.tcpip.macAddr.value;
	if ( str.length < 12) {
		alert(showText(wandipAlert));
		document.tcpip.macAddr.focus();
		return false;
	}
	for (var i=0; i<str.length; i++) {
		if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) >= 'a' && str.charAt(i) <= 'f') || (str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
			continue;
		alert(showText(wandipAlert2));
		document.tcpip.macAddr.focus();
		return false;
	}
	return true;
}
function copyto() {
	document.tcpip.macAddr.value=document.tcpip.macAddrValue.value;
}
function autoDNSClicked() {
  if(document.tcpip.DNSMode[0].checked==true){
      document.tcpip.dns1.disabled = true;
      document.tcpip.dns2.disabled = true;
   }
 else{
      document.tcpip.dns1.disabled = false;
      document.tcpip.dns2.disabled = false;
  }
}
</SCRIPT>
</head>
<body class="mainbg">
<blockquote>
<span id="genTlId">
</div>
</span>
<span id="wizTlId" style="display:none">
</span>
<form action=/goform/formWanTcpipSetup method=POST name="tcpip">
<table border="0" cellspacing="2" cellpadding="2" width="520" align="center">
<tr>
<td width="35%" bgcolor="#666666" class="style24"><script>dw(hostName)</script> :&nbsp;</td>
<td width="65%" valign="middle" bgcolor="#FFFFFF" class="style22">
<input type="text" name="dynIPHostName" value="<% getInfo("dynIPHostName"); %>" size="20" maxlength="30"></td>
</tr>
<tr>
<input type=hidden value=<% getInfo("cloneMac"); %> name="macAddrValue">
<td bgcolor="#666666" class="style24"><script>dw(macAddr)</script> :&nbsp;</td>
<td valign="middle" bgcolor="#FFFFFF" class="style22"><input type="text" name="macAddr" value="<% getInfo("wanMac"); %>" size="20" maxlength="12">&nbsp;&nbsp;<script>document.write('<input type="button" value="'+showText(clone)+'" name="Clone" onClick="copyto();" class="btnsize">')</script></td>
</tr>
<tr>
<td width="35%" align="left" bgcolor="#666666" class="style24"><script>dw(dnsAddr)</script> :&nbsp;</td>
<td width="65%" align="left" bgcolor="#FFFFFF" class="style22">&nbsp;<input type="radio" value="0" name="DNSMode" <% if (getIndex("wanDNS")==0) write("checked"); %> onClick="autoDNSClicked();"><script>dw(obtainIP)</script>
  <input type="radio" value="1" name="DNSMode" <% if (getIndex("wanDNS")==1) write("checked"); %> onClick="autoDNSClicked();"><script>dw(useIPAddr)</script>
</td>
</tr>
<tr>
<td width="35%" align="left" bgcolor="#666666" class="style24"><script>dw(dnsAddr1)</script> :&nbsp;</td>
<td width="65%" align="left" bgcolor="#FFFFFF" class="style22">&nbsp;<input type="text" name="dns1" value="<% getInfo("wan-dns1"); %>" size="15" maxlength="30">
  <script>if(document.tcpip.dns1.value=="") document.tcpip.dns1.value="0.0.0.0"</script>
</td>
</tr>
<tr>
<td width="35%" align="left" bgcolor="#666666" class="style24"><script>dw(dnsAddr2)</script> :&nbsp;</td>
<td width="65%" align="left" bgcolor="#FFFFFF" class="style22">&nbsp;<input type="text" name="dns2" value="<% getInfo("wan-dns2"); %>" size="15" maxlength="30">
            <script>if(document.tcpip.dns2.value=="") document.tcpip.dns2.value="0.0.0.0"</script>
          </td>
        </tr>
<tr>
<td bgcolor="#666666" class="style24"><font size=2>TTL :&nbsp;</td>
<td valign="middle" bgcolor="#FFFFFF" class="style22"><font size=2>&nbsp;<input type="radio" value="0" name="pppEnTtl"<% if (getIndex("pppEnTtl")==0) write("checked"); %>><script>dw(disable)</script>&nbsp;&nbsp;
<input type="radio" name="pppEnTtl" value="1"<% if (getIndex("pppEnTtl")==1) write("checked"); %>><script>dw(enable)</script></td></tr>
</table>
<br>
<span id="genBtId">
<table width="520" border="0" cellspacing="0" cellpadding="0" align="center">
<tr>
<td align="right">
	<script>buffer='<input type=submit value="'+showText(apply1)+'" name="save" onclick="return saveChanges();" style ="width:100px">';document.write(buffer);</script>
	<input type=hidden value="/wandip.asp" name="submit-url" id="submitUrl">
	<input type=hidden value="0" name="wanMode">
	<script>buffer='<input type=button value="'+showText(cancel1)+'" style ="width:100px" onClick="document.tcpip.reset();">';document.write(buffer);</script>
</td></tr>
</table>
</span>
<span id="wizBtId" style="display:none">
<table width="520" border="0" cellspacing="0" cellpadding="0" align="center">
<tr>
<td align="right">
	<script>buffer='<input type=button value="'+showText(back1)+'" style ="width:100px" onClick="window.history.back();">';document.write(buffer);</script>
	<script>buffer='<input type=submit value="'+showText(ok1)+'" name="save" style ="width:100px" onClick="return saveChanges()">';document.write(buffer);</script>
</td>
</tr>
</table>
</span>
<script>
autoDNSClicked();
if (wizardEnabled == 0) {
    document.getElementById('genTlId').style.display = "block";
    document.getElementById('genBtId').style.display = "block";
    document.getElementById('wizTlId').style.display = "none";
    document.getElementById('wizBtId').style.display = "none";
}
else {
    document.getElementById('genTlId').style.display = "none";
    document.getElementById('genBtId').style.display = "none";
    document.getElementById('wizTlId').style.display = "block";
    document.getElementById('wizBtId').style.display = "block";
	document.getElementById('submitUrl').value = "/main.htm";
}
</script>
</form>
<blockquote>
</body>
</html>
