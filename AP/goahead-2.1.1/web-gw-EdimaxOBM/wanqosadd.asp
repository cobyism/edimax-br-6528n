<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title>WAN Quality of Service</title>
<script type="text/javascript" src="file/javascript.js"></script>
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
<% QosShow(); %>
function macRange(macAddr) {
	str = macAddr.value;
	for (var i=0; i<str.length; i++) {
		if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) >= 'A' && str.charAt(i) <= 'F') || (str.charAt(i) >= 'a' && str.charAt(i) <= 'f') || (str.charAt(i) == ','))
			continue;
		alert(showText(wanqosaddAlertMac));
		macAddr.focus();
		return 0;
	}
	return 1;
}
function portRange(port) {
	str = port.value;
	for (var i=0; i<str.length; i++) {
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == ',') || (str.charAt(i) == '-'))
			continue;
		alert(showText(wanqosaddAlertPort));
		port.focus();
		return 0;
	}
	return 1;
}
function addClick() {
        strBwVal = document.formQos.bwidthVal;
        strDownVal = document.formQos.bwidthVal;
        strScStIp = document.formQos.sourStIp;
        strScEdIp = document.formQos.sourEdIp;
        strDtStIp = document.formQos.destStIp;
        strDtEdIp = document.formQos.destEdIp;
        strScPort = document.formQos.sourPort;
        strDtPort = document.formQos.destPort;
    if (qosShow[3] != 2) {
        if ((strDownVal.value=="" && strBwVal.value=="") || (strScStIp.value=="" && strScEdIp.value=="" && strDtStIp.value=="" && strDtEdIp.value=="" && strScMac.value=="" && strScPort.value=="" && strDtPort.value=="" && document.formQos.trafType.value==0)) {
            alert(showText(wanqosaddAlertAdd));
            return false;
        }
    }
    if (document.formQos.sourType.value == 0 ) {
        if ( !ipRule(strScStIp, showText(wanqosaddStartIp), "gw", 0))
            return false;
        if ( !ipRule(strScEdIp, showText(wanqosaddEndIp), "gw", 0))
            return false;
    }
    else
        if (strScMac.value != "" )
            if ( !macRange(strScMac))
                return false;
    if ( !ipRule(strDtStIp, showText(wanqosaddStartIpDest), "gw", 0))
        return false;
    if ( !ipRule(strDtEdIp, showText(wanqosaddStartIpDest), "gw", 0))
        return false;
    strScPort.disabled = false;
    strDtPort.disabled = false;
    if (strScPort.value != "" )
    {
        if ( !portRange(strScPort))
            return false;
    }
    else
    {
        strScPort.value = "2-65535"
    }
    if (strDtPort.value != "" )
    {
        if ( !portRange(strDtPort))
            return false;
    }
    else
    {
        strDtPort.value = "2-65535"
    }
    var longVal=0x001;
    document.formQos.configWan.value=longVal;
    document.formQos.configNum.value=qosShow[14];
    if (qosShow[14]) {
        document.formQos.addQos.disabled = true;
        document.formQos.editQos.disabled = false;
    }
    else {
        document.formQos.addQos.disabled = false;
        document.formQos.editQos.disabled = true;
    }
    return true;
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
</script>
</head>
<body bgcolor="#EFEFEF">
<blockquote>
<p align="center"><font class="titlecolor" size="4"><script>dw(qos)</script></font></p>
<p align="center"><font class="textcolor"  size="2"><script>dw(qosAddInfo)</script><br></font></p>
<form action=/goform/formQoS method=POST name="formQos">
<table width="520" border="1" cellspacing="0" cellpadding="2" align="center">
<tr class="table2">
	<td width="30%" align="right" class="style14"><script>dw(qosName);</script> :&nbsp;</td>
	<td width="70%" align="left">&nbsp;<input type="text" name="ruleName" size="15" maxlength="15"></td>
</tr>
<tr class="table2">
	<td width="30%" align="right" class="style14"><script>dw(qosBandwidth);</script> :&nbsp;</td>
	<td width="70%" align="left" nowrap>&nbsp;<select name="downUpType">
	<script>
		var downUpTbl = new Array("Upload","Download","Both");
		if (qosShow[1] != 0) {
			document.write("<option value=0 selected>"+ downUpTbl[0] +"</option>");
			document.write("<option value=1>"+ downUpTbl[1] +"</option>");
		}
		else {
			document.write("<option value=0>"+ downUpTbl[0] +"</option>");
			document.write("<option value=1 selected>"+ downUpTbl[1] +"</option>");
		}
	</script>
	</select>
	<input type="text" name="bwidthVal" size="6" maxlength="6"><font size="2">&nbsp;Kbps</font>
	&nbsp;<select name="bwidthType">
	<script>
		var bwTypeTbl = new Array(showText(guaranteeolitec),"Max");
		if (qosShow[1] != 0) {
			for(i=0; i<2; i++) {
				if (i == qosShow[2])
					document.write('<option value="'+ i +'" selected>'+ bwTypeTbl[i] +'</option>');
				else
					document.write('<option value="'+ i +'">'+ bwTypeTbl[i] +'</option>');
			}
		}
		else {
			for(i=0; i<2; i++) {
				if (i == qosShow[16])
					document.write('<option value="'+ i +'" selected>'+ bwTypeTbl[i] +'</option>');
				else
					document.write('<option value="'+ i +'">'+ bwTypeTbl[i] +'</option>');
			}
		}
	</script>
	</select>
	</td>
</tr>
<input type="hidden" name="sourType" value=0>
<tr class="table2">
	<td width="30%" align="right" class="style14"><script>dw(qosLocalIP);</script> :&nbsp;</td>
	<td width="70%" align="left" nowrap>&nbsp;<input type="text" name="sourStIp" size="15" maxlength="15"><font size="2"> - </font><input type="text" name="sourEdIp" size="15" maxlength="15"></td>
</tr>
<tr class="table2">
	<td width="30%" align="right" class="style14"><script>dw(qosLocalPort);</script> :&nbsp;</td>
	<td width="70%" align="left" nowrap>&nbsp;<input type="text" name="sourPort" size="35" maxlength="55"></td>
</tr>
<tr class="table2">
	<td width="30%" align="right" class="style14"><script>dw(qosRemoteIP);</script> :&nbsp;</td>
	<td width="70%" align="left" nowrap>&nbsp;<input type="text" name="destStIp" size="15" maxlength="15"><font size="2"> - </font><input type="text" name="destEdIp" size="15" maxlength="15"></td></tr>
<tr class="table2">
	<td width="30%" align="right" class="style14"><script>dw(qosRemotePort);</script> :&nbsp;</td>
	<td width="70%" align="left" nowrap>&nbsp;<input type="text" name="destPort" size="35" maxlength="55"></td>
</tr>
<tr class="table2">
	<td width="30%" align="right" class="style14"><script>dw(qosTraffic);</script> :&nbsp;</td>
	<td width="70%" align="left">&nbsp;<select name="trafType" onchange="specified_protocol()"><script>
		var trafTbl = new Array(showText(noneolitec),"SMTP","HTTP","POP3","FTP");
		for(i=0; i<5; i++) {
			if (i == qosShow[9])
				document.write('<option value="'+ i +'" selected>'+ trafTbl[i] +'</option>');
			else
				document.write('<option value="'+ i +'">'+ trafTbl[i] +'</option>');
		}
</script></td></tr>
<tr class="table2">
	<td width="30%" align="right" class="style14"><script>dw(protocal);</script> :&nbsp;</td>
	<td width="70%" align="left">&nbsp;<select name="protocol">
	<script>
		var ptlTbl = new Array("TCP","UDP");
		for(i=0; i<2; i++) {
			if (i == qosShow[10])
				document.write('<option value="'+ i +'" selected>'+ ptlTbl[i] +'</option>');
			else
				document.write('<option value="'+ i +'">'+ ptlTbl[i] +'</option>');
		}
	</script>
	</select></td></tr>
<script>
with(document.formQos) {
    ruleName.value = qosShow[0];
    if( qosShow[1] != 0 ) { // Upload
        bwidthVal.value = qosShow[1];
        sourStIp.value = qosShow[4];
        sourEdIp.value = qosShow[5];
        sourPort.value = qosShow[11];
        destStIp.value = qosShow[7];
        destEdIp.value = qosShow[8];
        destPort.value = qosShow[12];
    }
    else {  // Download
        bwidthVal.value = qosShow[15];
        sourStIp.value = qosShow[7];
        sourEdIp.value = qosShow[8];
        sourPort.value = qosShow[12];
        destStIp.value = qosShow[4];
        destEdIp.value = qosShow[5];
        destPort.value = qosShow[11];
    }
}
</script>
<br>
<table width="520" border="0" cellspacing="0" cellpadding="0" align="center">
<tr>
<td colspan=4 align="right">
<script>document.write('<input type="submit" value="'+showText(save)+'" class="btnsize" name="apply" onClick="return addClick()">');</script>&nbsp;&nbsp;
<script>document.write('<input type="reset" value="'+showText(reset)+'" class="btnsize" name="reset">');</script>
<input type="hidden" value="" name="configWan">
<input type="hidden" value="" name="configNum">
<input type="hidden" value="Add" name="addQos">
<input type="hidden" value="Edit" name="editQos">
<input type="hidden" value="frame_1" name="framename">
</td></tr>
</table>
</form>
</blockquote>
</body>
</html>
