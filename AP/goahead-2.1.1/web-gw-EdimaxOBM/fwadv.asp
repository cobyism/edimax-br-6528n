<html><head><title></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<script language ='javascript' src ='file/javascript.js'></script>
<script language ='javascript' src ='file/fwwl-n.var'></script>
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
<script language="JavaScript">
function validateNum(str)
{
  for (var i=0; i<str.length; i++) {
   	if ( !(str.charAt(i) >='0' && str.charAt(i) <= '9')) {
		alert(showText(fwadvInvValue));
		return false;
  	}
  }
  return true;
}
function saveChanges()
{
	strpodPack = document.DoS.podPack;
	strpodBur = document.DoS.podBur;
	strsynPack = document.DoS.synPack;
	strsynBur = document.DoS.synBur;
if (document.DoS.podEnable.checked) {
	if ( validateNum(strpodPack.value) == 0 ) {
		setFocus(strpodPack);
		return false;
	}
	num = parseInt(strpodPack.value);
	if (strpodPack.value == "" || num < 0 || num > 255) {
		alert(showText(fwadvInvPacket));
		setFocus(strpodPack);
		return false;
	}
	if ( validateNum(strpodBur.value) == 0 ) {
		setFocus(strpodBur);
		return false;
	}
	num = parseInt(strpodBur.value);
	if (strpodBur.value == "" || num < 0 || num > 255) {
		alert(showText(fwadvInvBurst));
		setFocus(strpodBur);
		return false;
	}
}
if (document.DoS.synEnable.checked) {
	if ( validateNum(strsynPack.value) == 0 ) {
		setFocus(strsynPack);
		return false;
	}
	num = parseInt(strsynPack.value);
	if (strsynPack.value == "" || num < 0 || num > 255) {
		alert(showText(fwadvInvPacketSync));
		setFocus(strsynPack);
		return false;
	}
	if ( validateNum(strsynBur.value) == 0 ) {
		setFocus(strsynBur);
		return false;
	}
	num = parseInt(strsynBur.value);
	if (strsynBur.value == "" || num < 0 || num > 255) {
		alert(showText(fwadvInvBurstSync));
		setFocus(strsynBur);
		return false;
	}
}
if (document.DoS.scanEnable.checked) {
	var longVal=0;
	if (document.DoS.Index0.checked==true)	longVal |= 0x001;
	if (document.DoS.Index1.checked==true)	longVal |= 0x002;
	if (document.DoS.Index2.checked==true)	longVal |= 0x004;
	if (document.DoS.Index3.checked==true)	longVal |= 0x008;
	if (document.DoS.Index4.checked==true)	longVal |= 0x010;
	if (document.DoS.Index5.checked==true)	longVal |= 0x020;
	if (document.DoS.Index6.checked==true)	longVal |= 0x040;
	document.DoS.scanNumVal.value=longVal;
}
	return true;
}
function ItemEnable() {
	if (!document.DoS.podEnable.checked) {
		document.DoS.podPack.disabled = true;
		document.DoS.podBur.disabled = true;
		document.DoS.podTime.disabled = true;
	}
	else {
		document.DoS.podPack.disabled = false;
		document.DoS.podBur.disabled = false;
		document.DoS.podTime.disabled = false;
	}
	if (!document.DoS.synEnable.checked) {
		document.DoS.synPack.disabled = true;
		document.DoS.synBur.disabled = true;
		document.DoS.synTime.disabled = true;
	}
	else {
		document.DoS.synPack.disabled = false;
		document.DoS.synBur.disabled = false;
		document.DoS.synTime.disabled = false;
	}
	if (!document.DoS.scanEnable.checked) {
		document.DoS.Index0.disabled = true;
		document.DoS.Index1.disabled = true;
		document.DoS.Index2.disabled = true;
		document.DoS.Index3.disabled = true;
		document.DoS.Index4.disabled = true;
		document.DoS.Index5.disabled = true;
		document.DoS.Index6.disabled = true;
	}
	else {
		document.DoS.Index0.disabled = false;
		document.DoS.Index1.disabled = false;
		document.DoS.Index2.disabled = false;
		document.DoS.Index3.disabled = false;
		document.DoS.Index4.disabled = false;
		document.DoS.Index5.disabled = false;
		document.DoS.Index6.disabled = false;
	}
}
</script>
</head>
<body bgcolor="#EFEFEF">
<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
	<tr><td height="20" valign="middle" align="left" class="style14"><script>dw(fwDosInfo)</script></td></tr>
</table>
<form action=/goform/formPreventionSetup method=POST name="DoS">
	<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
		<tr><td colspan="2" align="center" bgcolor="#666666" class="style13"><script>dw(fwDosFeature)</script></td></tr>
		<input type="hidden" value="<% getInfo("scanNum"); %>" name="scanNumVal">
		<tr>
			<td width="200" height="20" valign="middle" align="left" class="style14">
				<input type="checkbox" name="podEnable" value="ON" <% if (getIndex("podEnable")==1) write("checked"); %> onClick="ItemEnable();"><script>dw(fwDosDeath)</script>:&nbsp;
			</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="text" name="podPack" size="3" maxlength="3" value="<% getInfo("podPack"); %>"><script>dw(fwDosDeath);</script>
				<script>dw(fwadvPacket);</script>
				<script>dw(fwadvPer);</script>
				<select name="podTime">
					<script>
						var TimeTbl = new Array("Second","Minute","Hour");
						<%	write("podTimeVal = "+getIndex("podTime")); %>
						for ( i=0; i<=2; i++) {
							if (i == podTimeVal)
								document.write('<option selected value="'+i+'">'+TimeTbl[i]+'</option>');
							else
								document.write('<option value="'+i+'">'+TimeTbl[i]+'</option>');
						}
					</script>
				</select>
				<script>dw(fwadvBurst);</script>
				<input type="text" name="podBur" size="3" maxlength="3" value="<% getInfo("podBur"); %>">
			</td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="left" class="style14">
				<input type="checkbox" name="pingEnable" value="ON" <% if (getIndex("pingEnable")==1) write("checked"); %>><script>dw(fwDosPing)</script>:&nbsp;
			</td>
			<td width="400" height="20" valign="middle" align="left" class="style1"></td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="left" class="style14">
				<input type="checkbox" name="scanEnable" value="ON" <% if (getIndex("scanEnable")==1) write("checked"); %> onClick="ItemEnable();"><script>dw(fwDosScan)</script>:&nbsp;
			</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<script>
					var scanTbl = new Array("NMAP FIN / URG / PSH","Xmas tree","Another Xmas tree","Null scan","SYN / RST","SYN / FIN","SYN (only unreachable port)");
					var val=0x001;
					for(i=0; i<=6; i++){
						if (document.DoS.scanNumVal.value & val)
							document.write('<input type="checkbox" name="Index'+i+'" checked>');
						else
							document.write('<input type="checkbox" name="Index'+i+'">');
						document.write('<font size=2>' + scanTbl[i] + '</font><br>');
						val *=2;
					}
				</script>
			</td>
		</tr>
		<tr>
			<td width="200" height="20" valign="middle" align="left" class="style14">
				<input type="checkbox" name="synEnable" value="ON" <% if (getIndex("synEnable")==1) write("checked"); %> onClick="ItemEnable();"><script>dw(fwDosSync)</script>:&nbsp;
			</td>
			<td width="400" height="20" valign="middle" align="left" class="style1">
				<input type="text" name="synPack" size="3" maxlength="3" value="<% getInfo("synPack"); %>"><script>dw(fwadvPacket);</script>
				<script>dw(fwadvPer);</script>
					<select name="synTime">
						<script>
						<%	write("synTimeVal = "+getIndex("synTime"));%>
						for ( i=0; i<=2; i++) {
							if (i == synTimeVal)
								document.write('<option selected value="'+i+'">'+TimeTbl[i]+'</option>');
							else
								document.write('<option value="'+i+'">'+TimeTbl[i]+'</option>');
						}
						</script>
					</select>
				<script>dw(fwadvBurst);</script><input type="text" name="synBur" size="3" maxlength="3" value="<% getInfo("synBur"); %>">
				<script>ItemEnable();</script>
			</td>
		</tr>
		<tr>
			<td colspan="2" align="right">
				<script>
					document.write('<input type=submit value="Save" name="B1" style ="width:100px" onClick="return saveChanges()">');
				</script>
				<input type=hidden value="/fwdos.asp" name="submit-url">
				<input type="hidden" value="frame_3" name="framename">
				<input type="hidden" value="ON" name="isApply">
			</td>
		</tr>
	</table>
</form>
</body>
</html>
