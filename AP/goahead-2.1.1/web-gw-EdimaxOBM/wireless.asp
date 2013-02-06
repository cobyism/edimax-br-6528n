<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<link rel="stylesheet" href="edimax.css">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<script type="text/javascript" src="/file/multilanguage.var"></script>
<script type="text/javascript" src="/file/fwwl-n.var"></script>
<script type="text/javascript" src="/file/statustool-n.var"></script>
<script type="text/javascript" src="/file/netsys-n.var"></script>
<script type="text/javascript" src="/file/qosnat-n.var"></script>
<script type="text/javascript" src="file/ezview-n.var"></script>
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="FUNCTION_SCRIPT"></script>
<script>
<% getInfo("getDefined"); %>
var cloud_sta = "<% getInfo("cloud-sta"); %>"
var wl_ssid = "<% getInfo("ssid"); %>"
<%getInfo("getDefined");%>
<% write("wpsConType = "+getIndex("wpsConfigType")+";");%>
<% write("apMode = "+getIndex("apMode")+";");%>
<% write("methodVal = "+getIndex("encrypt")+";");%>
<% write("opMode = "+getIndex("opMode")+";");%>
<% write("bandVal = "+getIndex("band")+";");%>
<% write("wlanswitch = "+getIndex("wlanDisabled")+";"); %>
var wlDev = 4
/* function for main */
	var comment = new Array( new Array("com00","com01"),
							 new Array("com10","com11"),
							 new Array("com20","com21"),
							 new Array("com30","com31"),
							 new Array("com40","com41"))
	function com_sw(cnum,com)
	{
		if (com == 1)
		{
			document.getElementById(comment[cnum][0]).style.display = "none"
			document.getElementById(comment[cnum][1]).style.display = "block"
		}
		else
		{
			document.getElementById(comment[cnum][0]).style.display = "block"
			document.getElementById(comment[cnum][1]).style.display = "none"
		}
	}
	function c_fun(num)
	{
		if ( _MODEL_ == "BR6258GN" || _MODEL_ == "BR6228GNS" )
		{
			if(document.getElementsByName("wlanDisabled")[1].checked)
				return true;
		}
		document.getElementById('wl_BS').style.display = "none"
		document.getElementById('wl_ADS').style.display = "none"
		document.getElementById('wl_SW').style.display = "none"
		document.getElementById('wl_SS').style.display = "none"
		document.getElementById('wl_MAF').style.display = "none"
		document.getElementById('wl_WPS').style.display = "none"
		if (num == 0)
		{
			document.getElementsByName("sys")[0].checked = true
			document.getElementById('wl_BS').style.display = "block"
			document.getElementById('wl_SW').style.display = "block"
		}
		else if (num == 1)
		{
			document.getElementsByName("sys")[1].checked = true
			document.getElementById('wl_SS').style.display = "block"
			updateFormat();
			displayObj();
		}
		else if (num == 2)
		{
			document.getElementsByName("sys")[2].checked = true
			document.getElementById('wl_MAF').style.display = "block"
		}
		else
		{
			document.getElementsByName("sys")[3].checked = true
			document.getElementById('wl_WPS').style.display = "block"
		}
	}
	function c_wlb(setmode)
	{
		if(setmode == 0)
		{
			document.getElementById('wl_BS').style.display = "none"
			document.getElementById('wl_ADS').style.display = "block"
		}
		else
		{
			document.getElementById('wl_BS').style.display = "block"
			document.getElementById('wl_ADS').style.display = "none"
		}
	}
	/*wireless function*/
	function showMacClick(url) {
		openWindow(url, 'showWirelessClient' );
	}
	function saveChanges()
	{
		if ( _MODEL_ == "BR6258GN" || _MODEL_ == "BR6228GNS" )
		{
			if(document.getElementsByName("wlanDisabled")[1].checked)
				return true;
		}
		if(document.getElementById('wl_BS').style.display == "block")
		{
			if (apMode == 0 || apMode == 1 || apMode == 2 || apMode == 5 ) {
				if (document.wlanSetup.ssid.value=="") {
					alert(showText(wlbasicAlertEmpty));
					document.wlanSetup.ssid.value = document.wlanSetup.ssid.defaultValue;
					document.wlanSetup.ssid.focus();
					return false;
				}
			}
			if (apMode == 3) {
				if ( !macRule(document.wlanSetup.wlLinkMac1,showText(wlbasicStrMacAddr1), 1))
					return false;
			}
			if (apMode == 4 || apMode == 5 ) {
				if ( !macRule(document.wlanSetup.wlLinkMac1,showText(wlbasicStrMacAddr1), 1))
					return false;
				if ( !macRule(document.wlanSetup.wlLinkMac2,showText(wlbasicStrMacAddr2), 1))
					return false;
				if ( !macRule(document.wlanSetup.wlLinkMac3,showText(wlbasicStrMacAddr3), 1))
					return false;
				if ( !macRule(document.wlanSetup.wlLinkMac4,showText(wlbasicStrMacAddr4), 1))
					return false;
			}
			if(document.wlanSetup.ssid.value != wl_ssid){
				document.wlanSetup.wpsStatus.value="1";
			}
			else{
				document.wlanSetup.wpsStatus.value="0";
			}
		}
		else if(document.getElementById('wl_ADS').style.display == "block")
		{
			if (!portRule(document.advanceSetup.fragThreshold, showText(wladvanceStrFrag), 0,"", 256, 2346, 1))
				return false;
			if (!portRule(document.advanceSetup.rtsThreshold, showText(wladvanceStrRts), 0,"", 0, 2347, 1))
				return false;
			if ( wlDev == 1 || wlDev == 2 ) {
				if (!portRule(document.advanceSetup.beaconInterval, showText(wladvanceStrInterval), 0,"", 20, 1024, 1))
					return false;
			}
			else {
				if (!portRule(document.advanceSetup.beaconInterval, showText(wladvanceStrInterval), 0,"", 20, 1024, 1))
					return false;
				if (!portRule(document.advanceSetup.dtimPeriod, showText(wladvanceStrDtim), 0,"", 1, 10, 1))
					return false;
			}
			if( typeof _MODEL_ != 'undefined' && _MODEL_=="BR6225N" && document.getElementsByName("watchdog_enable")[0].checked==true )
			{
				if ( !portRule(document.getElementsByName("watchdog_interval")[0], "Watch Interval", 0,"", 5, 60, 1)) return false;
				if ( !ipRule( document.getElementsByName("watchdog_host")[0], "Watch Host", "ip", 1)) return false;
			}
		}
		else if(document.getElementById('wl_SS').style.display == "block")
		{
			var keyLen;
			var strMethod = document.wlEncrypt.method.value;
			var str = document.wlEncrypt.pskValue.value;
			if ( strMethod==1)
			{
				if (document.wlEncrypt.length.selectedIndex == 0)
				{
					if ( document.wlEncrypt.format.selectedIndex == 0) keyLen = 5;
					else keyLen = 10;
				}
				else if (document.wlEncrypt.length.selectedIndex == 1)
				{
					if ( document.wlEncrypt.format.selectedIndex == 0) keyLen = 13;
					else keyLen = 26;
				}
				else
				{
					if ( document.wlEncrypt.format.selectedIndex == 0) keyLen = 16;
					else keyLen = 32;
				}
				if (wlValidateKey(0,document.wlEncrypt.key1.value, keyLen)==0)
				{
					//document.wlEncrypt.key1.focus();
					return false;
				}
			}
			//********** radius **********
			if (strMethod == 3 || ((strMethod==0 || strMethod==1) && document.wlEncrypt.enRadius.checked==true))
			{
				if ( !portRule(document.wlEncrypt.radiusPort, showText(wlencryptStrSrvPort), 0, "", 1, 65535, 1))
					return false;
				if( !ipRule( document.wlEncrypt.radiusIP, showText(wlencryptStrSrvAddr), "ip", 1))
					return false;
			}
			//********** psk **********
			if (strMethod==2)
			{
				if (document.wlEncrypt.pskFormat.selectedIndex==1) {
					if (str.length != 64) {
						alert(showText(wlencryptAlertKey64));
						//document.wlEncrypt.pskValue.focus();
						return false;
					}
					takedef = 0;
					if (document.wlEncrypt.pskFormat.value == 1 && document.wlEncrypt.pskValue.value.length == 64) {
						for (var i=0; i<64; i++) {
								if ( str.charAt(i) != '*')
								break;
						}
						if (i == 64 )
							takedef = 1;
					}
					if (takedef == 0) {
						for (var i=0; i<str.length; i++) {
								if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
								(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
								(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') || str.charAt(i) == '*')
								continue;
							alert(showText(wlencryptAlertInvPreVal));
							//document.wlEncrypt.pskValue.focus();
							return false;
						}
					}
				}
				else {
					if (str.length < 8) {
						alert(showText(wlencryptAlertKey8));
						//document.wlEncrypt.pskValue.focus();
						return false;
					}
					if (str.length > 63) {
						alert(showText(wlencryptAlertKeyX64));
						//document.wlEncrypt.pskValue.focus();
						return false;
					}
				}
			}
		}
		else if(document.getElementById('wl_MAF').style.display == "block")
		{
			document.formPortFwApply.submit();
		}
		else if(document.getElementById('wl_WPS').style.display == "block")
		{
			document.Wpsseting.submit();
		}
		return true;
	}
	function save(mode){
		configMode=document.WpsDevice.confMode.selectedIndex;
			if(mode == 1){
			//PIN
				if(!configMode){
					if( checkPin() ==0){
						document.WpsDevice.pinCode.focus();
						return false;
					}
					if( checkValue(document.WpsDevice.pinCode.value) == 0){
						document.WpsDevice.pinCode.focus();
						return false
					}
				}else
				{
					alert("You can configure AP using external registrar .");
				}
				document.WpsDevice.configOption.value="pin";
			}
			else if(mode == 2){//PBC
				document.WpsDevice.configOption.value="pbc";
				alert("You must go to station side to push it button within two minute.");
			}
		document.WpsDevice.startPin.disabled = true;
		document.WpsDevice.startPbc.disabled = true;
		document.getElementsByName("sys").disabled = true;
		countDown(mode);
		document.WpsDevice.submit();
		return true;
	}
	function displayObj()
	{
		if(document.getElementById('wl_BS').style.display == "block")
		{
			var showSsid = new Array("block","block","block","none","none","block","block");
			var showChan = new Array("block","block","none","block","block","block","block");
			var showClit = new Array("block","none","none","none","none","block","block");
			var showSurvey = new Array("none","block","block","none","none","none","none");
			var showMac = new Array("none","none","none","block","block","block","none");
			var showMMac = new Array("none","none","none","none","block","block","none");
			var showRepeater = new Array("none","none","none","none","none","none","block");
			for (i=0; i<=6; i++) {
				if (document.wlanSetup.apMode.value == i) {
					document.getElementById('ssidId').style.display = showSsid[i];
					document.getElementById('chanId').style.display = showChan[i];
					document.getElementById('clitId').style.display = showClit[i];
					document.getElementById('surveyId').style.display = showSurvey[i];
					document.getElementById('macId').style.display = showMac[i];
					document.getElementById('multiMacId').style.display = showMMac[i];
					document.getElementById('repeaterId').style.display = showRepeater[i];
				}
			}
		}
		else if(document.getElementById('wl_SS').style.display == "block")
		{
			var wepTbl = new Array("none","block","none","none");
			var wpaTbl = new Array("none","none","block","block");
			var pskTbl = new Array("none","none","block","none");
			if (apMode==1 || apMode==2 || opMode==1)
				var enRadiusTbl = new Array("none","none","none","none");
			else
				var enRadiusTbl = new Array("block","block","none","none");
			var radiusTbl = new Array("none","none","none","block");
			var inRdsTbl = new Array("none","none","block","block","none","none");
			for (i=0; i<=4; i++) {
				if(typeof(_WIFI_11N_STANDARD_)!='undefined' && bandVal==1 && i==1)
					continue;
				if (document.wlEncrypt.method.value == i) {
					document.getElementById('wepId').style.display = wepTbl[i];
					document.getElementById('wpaId').style.display = wpaTbl[i];
					document.getElementById('pskId').style.display = pskTbl[i];
					document.getElementById('inRdsId').style.display = "none";
					document.getElementById('enRadiusId').style.display = enRadiusTbl[i];
					if (document.getElementById('enRadiusId').style.display=="block" && document.wlEncrypt.enRadius.checked==true) {
						document.getElementById('radiusId').style.display="block";
					}
					else {
						document.getElementById('radiusId').style.display = radiusTbl[i];
					}
				}
			}
			lengthClick();
		}
	}
	function reloadPage() {
		location.reload();
	}
	function resetSSID(){
	   document.wlanSetup.ssid.value = wl_ssid;
	}
	function wlValidateKey(idx, str, len)
	{
		if (str.length ==0) return 1;
		if ( str.length != len) {
			idx++;
			alert(showText(wlencryptStrInvKey) + idx + showText(wlencryptStrShouldBe) + len + showText(wlencryptStrChara));
			return 0;
		}
		if ( str == "*****" ||
			str == "**********" ||
			str == "*************" ||
			str == "****************" ||
			str == "**************************" ||
			str == "********************************" )
			return 1;
		if (document.wlEncrypt.format.selectedIndex==0)
			return 1;
		for (var i=0; i<str.length; i++) {
			if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
				(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
				(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') || str.charAt(i) == '*' )
				continue;
			alert(showText(wlencryptAlertInvVal));
			return 0;
		}
		return 1;
	}
	function updateFormat()
	{
		if (document.wlEncrypt.length.selectedIndex == 0) {
			document.wlEncrypt.format.options[0].text = 'ASCII (5 ' + showText(wlencryptStrCharaolitec) +')';
			document.wlEncrypt.format.options[1].text = 'Hex (10 ' + showText(wlencryptStrCharaolitec) +')';
		}
		else if (document.wlEncrypt.length.selectedIndex == 1){
			document.wlEncrypt.format.options[0].text = 'ASCII (13 ' + showText(wlencryptStrCharaolitec) +')';
			document.wlEncrypt.format.options[1].text = 'Hex (26 ' + showText(wlencryptStrCharaolitec) +')';
		}
		<%
			type = getIndex("keyType");
			write("document.wlEncrypt.format.options[" + type + "].selected = \'true\';");
		%>
		setDefaultKeyValue();
	}
	function setDefaultKeyValue()
	{
		var WEP64Key1 = "<% getInfo("WEP64Key1");%>"
		var WEP128Key1 = "<% getInfo("WEP128Key1");%>"

		if (document.wlEncrypt.length.selectedIndex == 0) {
			if ( document.wlEncrypt.format.selectedIndex == 0) {
				document.wlEncrypt.key1.maxLength = 5;
				document.wlEncrypt.key1_1.maxLength = 5;
				document.wlEncrypt.key1.value = "*****";
				document.wlEncrypt.key1_1.value =WEP64Key1.substring(0,5);
			}
			else {
				document.wlEncrypt.key1.maxLength = 10;
				document.wlEncrypt.key1_1.maxLength = 10;
				document.wlEncrypt.key1.value = "**********";
				document.wlEncrypt.key1_1.value = WEP64Key1;
			}
		}
		else if (document.wlEncrypt.length.selectedIndex == 1) {
			if ( document.wlEncrypt.format.selectedIndex == 0) {
				document.wlEncrypt.key1.maxLength = 13;
				document.wlEncrypt.key1_1.maxLength = 13;
				document.wlEncrypt.key1.value = "*************";
				document.wlEncrypt.key1_1.value = WEP128Key1.substring(0,13);
			}
			else {
				document.wlEncrypt.key1.maxLength = 26;
				document.wlEncrypt.key1_1.maxLength = 26;
				document.wlEncrypt.key1.value = "**************************";
				document.wlEncrypt.key1_1.value = WEP128Key1;
			}
		}
	}
	function lengthClick()
	{
	  updateFormat();
	}
	function addClick() {
		if ( !macRule(document.formFilterAdd.mac,showText(wlMacAddress), 0))
			return false;
		for(i=1; i<=document.getElementsByName("sipnum")[0].value; i++)
		{
			if( document.formFilterAdd.mac.value.toLowerCase() == document.getElementsByName("macn"+i)[0].value.toLowerCase() )
			{
				alert(showText(macthesame));
				return false;
			}
		}
		if( document.formFilterAdd.mac.value =="000000000000" || document.formFilterAdd.mac.value =="ffffffffffff" || document.formFilterAdd.mac.value =="FFFFFFFFFFFF")
		{
			alert("Invalid MAC Address")
			document.formFilterAdd.mac.value = document.formFilterAdd.mac.defaultValue
			document.formFilterAdd.mac.focus()
			return false;
		}
		return true;
	}
	function disableDelButton()
	{
		document.formFilterDel.deleteSelFilterMac.disabled = true;
		document.formFilterDel.deleteAllFilterMac.disabled = true;
	}
	function configChange(index){
		if ( index == 0 ) document.WpsDevice.pinCode.disabled = false;
		else document.WpsDevice.pinCode.disabled = true;
	}
	function checkValue(str)
	{
		for (var i=0; i<str.length; i++)
		{
			if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9'))
				continue;
			alert(showText(wpsAlertInvPin));
			return 0;
		}
		return 1;
	}
	function checkPin()
	{
		if(document.WpsDevice.configOption.value == "pin")
		{
			if (document.WpsDevice.pinCode.value.length ==0)
			{
				alert("Pin Code Value should be set at least 8 characters");
				return 0;
			}
			if ( document.WpsDevice.pinCode.value.length != 8)
			{
				alert("Pin Code Value should be set at least 8 characters");
				return 0;
			}
		}
		return 1;
	}
	function changeMode()
	{
		mode=document.WpsDevice.confMode.selectedIndex;
		if(mode==0)
		{
			document.WpsDevice.pinCode.disabled=false;
			document.WpsDevice.startPbc.disabled = false;
		}
		else
		{
			if (wpsConType)
			{
				alert("The WPS Enrollee only be used at unconfigured!")
				document.WpsDevice.confMode.value = 0
			}
			else
			{
				document.WpsDevice.pinCode.value="";
				document.WpsDevice.pinCode.disabled=true;
				document.WpsDevice.startPbc.disabled = true;
			}
		}
	}
	var secs = 120; var wait = secs * 1000;
	function countDown(mode){
		for(i = 1; i <= secs; i++)
			setTimeout("update(" + i + ","+mode+")", i * 1000);
	}
	function update(num,mode) {
		document.WpsDevice.startPin.disabled = true;
		document.WpsDevice.startPbc.disabled = true;
		if(mode == 1){
		//PIN
			if (num == (wait/1000)) {
				document.WpsDevice.startPin.value = showText(wpsStartpin);
			} else { printnr = (wait / 1000)-num;
				document.WpsDevice.startPin.value =   printnr + " sec.";
			}
		}
		else{
		//PBC
			if (num == (wait/1000)) {
				document.WpsDevice.startPbc.value = showText(wpsStartPbc);
			} else { printnr = (wait / 1000)-num;
				document.WpsDevice.startPbc.value =  printnr + " sec.";
			}
		}
	}
	function goToWeb() {
	if (document.formFilterAdd.wlanAcEnabled.checked==true)
		document.formWlAcEnabled.wlanAcEnabled.value="ON";
	document.formWlAcEnabled.submit();
	}
	function wlCtrl()
	{
		if(document.formFilterAdd.wlanAcEnabled.checked==true)
		{
			document.formFilterAdd.mac.disabled=false
			document.formFilterAdd.comment.disabled=false
			document.formFilterAdd.addFilterMac.disabled=false
			document.formFilterAdd.reset.disabled=false
		}
		else
		{
			document.formFilterAdd.mac.disabled=true
			document.formFilterAdd.comment.disabled=true
			document.formFilterAdd.addFilterMac.disabled=true
			document.formFilterAdd.reset.disabled=true
		}
	}
	function set_mod(wlmod)
	{
		top.WL_MODE = wlmod;
	}
	function wireless_on_off()
	{
		if(document.getElementsByName("wlanDisabled")[1].checked)
		{
			document.getElementsByName("sys")[0].disabled=true
			document.getElementsByName("sys")[1].disabled=true
			document.getElementsByName("sys")[2].disabled=true
			document.getElementsByName("sys")[3].disabled=true
			document.getElementById('wl_BS').style.display = "none"
			document.getElementById('wl_ADS').style.display = "none"
			document.getElementById('wl_SWApply').style.display = "block"
		}
		else
		{
			document.getElementsByName("sys")[0].disabled=false
			document.getElementsByName("sys")[1].disabled=false
			document.getElementsByName("sys")[2].disabled=false
			document.getElementsByName("sys")[3].disabled=false
			document.getElementById('wl_BS').style.display = "block"
			document.getElementById('wl_SWApply').style.display = "none"
			displayObj()
		}
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
function hideWepvalue(x)
{
if (x == 1)
{
	document.getElementById('keyhidewep').checked=true;
	document.getElementById('wephide').style.display="block";
	document.getElementById('wepdisplay').style.display="none";
}	
else
{
	document.getElementById('keyhidewep1').checked=false;
	document.getElementById('wephide').style.display="none";
	document.getElementById('wepdisplay').style.display="block";
}
}
function setWepvalue(x)
{
if (x == 1)
document.wlEncrypt.key1.value=document.wlEncrypt.key1_1.value;
else
document.wlEncrypt.key1_1.value=document.wlEncrypt.key1.value;
}
function hidePskvalue(x)
{
if (x == 1)
{
	document.getElementById('keyhide').checked=true;
	document.getElementById('hidepsk').style.display="block";
	document.getElementById('displaypsk').style.display="none";
}	
else
{
	document.getElementById('keyhide1').checked=false;
	document.getElementById('hidepsk').style.display="none";
	document.getElementById('displaypsk').style.display="block";
}
}
function setpskvalue(x)
{
if (x == 1)
document.wlEncrypt.pskValue.value=document.wlEncrypt.pskValue_1.value;
else
document.wlEncrypt.pskValue_1.value=document.wlEncrypt.pskValue.value;
}

</script>
</head>
<body onLoad="top.startclock()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(2,3)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF"><br />
						<script>
							<% write("wl_sw = "+getIndex("wlanDisabled")+";"); %>
							if (wl_sw &&  _MODEL_ == "BR6428GN")
							{
								document.write("The wireless module has been disabled !  Please turn on the hardware switch to start the wireless module .");
								document.write('<div  style="display:none">')
							}
							else
								document.write('<div  style="display:block">')
						</script>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666" class="style13">
									<input onclick="c_fun(0)" type="radio" name="sys"/>
									<span class="style13" style="cursor:pointer;" onclick="c_fun(0);" name="WS">&nbsp;Wireless Settings</span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<form action=/goform/formWlanSetup method=POST name="wlanSetup">
									<div id="wl_SW" style="display:block"><br>
										<script>
											if ( _MODEL_ == "BR6258GN" || _MODEL_ == "BR6228GNS" )
											{
												document.write('<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" >')
												document.write('<tr>')
												document.write('<td width="200" height="20" valign="middle" align="right" class="style14">'+showText(WirelessModule)+' :&nbsp;&nbsp;</td>')
												document.write('<td width="400" height="20" valign="middle" align="left" class="style1">')
												if (wlanswitch == 0)
												{
													document.write('<input type="radio" name="wlanDisabled" value="OFF" onclick="wireless_on_off();" checked/>'+showText(Enable)+'&nbsp;&nbsp;')
													document.write('<input type="radio" name="wlanDisabled" value="ON" onclick="wireless_on_off();" />'+showText(Disable))
												}
												else
												{
													document.write('<input type="radio" name="wlanDisabled" value="OFF" onclick="wireless_on_off();" />'+showText(Enable)+'&nbsp;&nbsp;')
													document.write('<input type="radio" name="wlanDisabled" value="ON" onclick="wireless_on_off();" checked />'+showText(Disable))
												}
												document.write('</td></tr></table>')
											}
										</script>
									</div>
									<div id="wl_BS" style="display:none">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" >
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(mode)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<select name="apMode" onChange="displayObj();">
									<script>
										var modeTbl = new Array("AP","Station-Ad Hoc","Station-Infrastructure","AP Bridge-Point to Point","AP Bridge-Point to Multi-Point","AP Bridge-WDS","Universal Repeater");
										for ( i=0; i<=6; i++)
										{
											if ( (wlDev!=3 && wlDev!=4) || (i!=1 ))
											{
												if ( i==apMode) document.write('<option selected value="'+i+'">'+modeTbl[i]+'</option>');
												else document.write('<option value="'+i+'">'+modeTbl[i]+'</option>');
											}
										}
									</script>
									</select>
									<script>document.write('&nbsp;&nbsp;<input type="button" value="'+showText(advancedSet)+'" onClick="c_wlb(0)">');</script>
									</td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(band)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<script>
										document.write('<select name="band">');
										bbandVal = bandVal-1;
										var bandTbl = new Array("2.4 GHz (B)","2.4 GHz (N)","2.4 GHz (B+G)", "2.4 GHz (G)", "2.4 GHz (B+G+N)");
										for (i=0; i<5; i++)
										{
											if (i==bbandVal) document.write('<option selected value="'+i+'">'+bandTbl[i]+'</option>');
											else document.write('<option value="'+i+'">'+bandTbl[i]+'</option>');
										}
									</script>
									</select>
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="ssidId" style="display:none">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlEssid)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="text" name="ssid" size="25" maxlength="32" value="<% getInfo("ssid"); %>" onchange="check(this)">
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="chanId" style="display:none">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlChannelNum)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<select size="1" name="chan">
									<script>
										<% write("var regDomain = "+getIndex("regDomain"));%>
										<% write("var defaultChan = "+getIndex("channel"));%>
										var Reg_to_Domain = new Array("0", "0", "1", "2", "3", "5");
										var region = Reg_to_Domain[regDomain-1];
										var domainMin = new Array("1","1","10","10","14","1","3","5");
										var domainMax = new Array("11","13","11","13","14","14","9","13");
										var Reg_to_Domain_A = new Array("7", "0", "7", "2", "3", "1");
										var region_A = Reg_to_Domain_A[regDomain-1];
										var achnn = new Array(
													  new Array("36", "40", "44", "48", "52", "56", "60", "64", "149", "153", "157", "161", "165"),
													  new Array("36", "40", "44", "48", "52", "56", "60", "64", "100"),
													  new Array("36", "40", "44", "48", "52", "56", "60", "64"),
													  new Array("52", "56", "60", "64", "149", "153", "157", "161"),
													  new Array("149", "153", "157", "161", "165"),
													  new Array("149", "153", "157", "161"),
													  new Array("36", "40", "44", "48"),
													  new Array("36", "40", "44", "48", "52", "56", "60", "64", "100", "104", "108", "112", "116", "120", "124", "128", "132", "136", "140", "149", "153", "157", "161", "165"),
													  new Array("52", "56", "60", "64"),
													  new Array("100", "104", "108", "112", "116", "120", "124", "128", "132", "136", "140")
													);
										for (j=domainMin[region]; j<=domainMax[region]; j++)
										{
											if ( j==defaultChan) document.write('<option selected value="'+j+'">'+j+'</option>');
											else document.write('<option value="'+j+'">'+j+'</option>');
										}
									</script>
									</select>
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="clitId" style="display:none">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlAssClients)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>document.write('<input type=button value="'+showText(swActCli)+'" name="showMac" onClick=showMacClick("/wlstatbl.asp")>')</script>
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="surveyId" style="display:none">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlSiteSurvey)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>document.write('<input type="button" value="'+showText(selSS)+'" name="selSurvey" onClick="showMacClick(\'/wlsurvey2.asp\')">')</script>
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="repeaterId" style="display:none">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlRootApSsid)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="text" name="repeaterSSID" size="25" maxlength="32" value="">
									</td>
									<script>document.wlanSetup.repeaterSSID.value="<% getInfo("repeaterSSID"); %>";</script>
									</tr>
									<script>
										document.write('<tr><td width="200" height="20" valign="middle" align="right" class="style14">' + showText(wlSiteSurvey)+' :&nbsp;&nbsp;</td>');
										document.write('<td width="400" height="20" valign="middle" align="left" class="style1">&nbsp;<input type=button value="'+showText(selSS)+'" name="selSurvey" onClick=showMacClick("/wlsurvey2.asp")></td></tr>');
									</script>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="macId" style="display:none">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlMacAddress)</script> 1 :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="text" size="15" maxlength="12" value="<% getInfo("wlLinkMac1"); %>" name="wlLinkMac1">
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="multiMacId" style="display:none">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlMacAddress)</script> 2 :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="text" size="15" maxlength="12" value="<% getInfo("wlLinkMac2"); %>" name="wlLinkMac2">
									</td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlMacAddress)</script> 3 :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="text" size="15" maxlength="12" value="<% getInfo("wlLinkMac3"); %>" name="wlLinkMac3">
									</td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlMacAddress)</script> 4 :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="text" size="15" maxlength="12" value="<% getInfo("wlLinkMac4"); %>" name="wlLinkMac4">
									</td>
									</tr>
									</table>
									<script>
										if (opMode==1) document.wlanSetup.chan.disabled=true;
									</script>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
									<td colspan="2" height="20" valign="middle" align="right">
									<script>
										document.write('<input type="hidden" value="0" name="wpsStatus">');
										document.write('<input type="hidden" value="'+opMode+'" name="wisp">');
										document.write('<input type=submit value="'+showText(apply1)+'" name="B1" onclick="set_mod(0);return saveChanges()" style ="width:100px">');
									</script>
									</td>
									</tr>
									</table>
									</div>
									<div id="wl_SWApply" style="display:none">
										<script>
											document.write('<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">')
											document.write('<tr><td colspan="2" height="20" valign="middle" align="right">')
											document.write('<input type=submit value="'+showText(apply1)+'" name="B1" onclick="set_mod(0);return saveChanges()" style ="width:100px">')
											document.write('</td></tr></table>')
										</script>
									</div>
									<input type="hidden" value="/wireless.asp" name="wlan-url">
									</form>
									<div id="wl_ADS" style="display:none">
									<form action=/goform/formAdvanceSetup method=POST name="advanceSetup">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" >
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlFragThreshold)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="text" name="fragThreshold" size="10" maxlength="4" value=<% getInfo("fragThreshold"); %>>&nbsp;(256-2346)
									</td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlRtsThreshold)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="text" name="rtsThreshold" size="10" maxlength="4" value=<% getInfo("rtsThreshold"); %>>&nbsp;(0-2347)
									</td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlAdIntvl)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="text" name="beaconInterval" size="10" maxlength="4" value="<% getInfo("beaconInterval"); %>">&nbsp;(20-1024 ms)
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="dtimId"style="display:none">
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlDtimPeriod)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="dtimPeriod" size="10" maxlength="2" value="<% getInfo("dtimPeriod"); %>">&nbsp;(1-10)
										</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="rtlRateId" style="display:none">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlDataRate)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<select size="1" name="txRate">
									<script>
										<%  write("auto = "+getIndex("rateAdaptiveEnabled")+";");%>
										<%  write("txrate = "+getIndex("fixTxRate")+";");%>
										rate_mask = [7,1,1,1,1,2,2,2,2,2,2,2,2];
										rate_name =["Auto","1M","2M","5.5M","11M","6M","9M","12M","18M","24M","36M","48M","54M"];
										band2mask = [0, 1, 4, 7, 2, 7];
										mask=0;
										mask = band2mask[bandVal];
										defidx=0;
										for (idx=0, i=0; i<=28; i++) {
											if (rate_mask[i] & mask) {
												if (i == 0)
													rate = 0;
												else
													if(wlDev!=4)
														rate = (1 << (i-1));
													else
														rate = i;
												if (txrate == rate)
													defidx = idx;
												document.write('<option value="' + i + '">' + rate_name[i] + '\n');
												idx++;
											}
										}
										document.advanceSetup.txRate.selectedIndex=defidx;
									</script>
									</select>
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="11NRateId">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14">N&nbsp;<script>dw(wlDataRate)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<select size="1" name="NtxRate">
									<script>
										<%  write("txrate = "+getIndex("NfixTxRate")+";");%>
										<%  write("txStream = "+getIndex("txStream")+";");%>
										rate_mask = [7,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4];
										rate_name =["Auto", "MCS 0", "MCS 1", "MCS 2", "MCS 3", "MCS 4", "MCS 5", "MCS 6", "MCS 7", "MCS 8", "MCS 9", "MCS 10", "MCS 11", "MCS 12", "MCS 13", "MCS 14", "MCS 15"];
										band2mask = [0, 1, 4, 3, 1, 7];
										mask=0;
										mask = band2mask[bandVal];
										defidx=0;
										if(txStream == 1)
											maxMCS=8;
										else
											maxMCS=16;
										for (idx=0, i=0; i<=maxMCS; i++)
										{
											if (rate_mask[i] & mask)
											{
												if (i == 0)
													rate = 0;
												else
												{
													rate = (1 << (i+12));
												}
												if (txrate == rate)
													defidx = idx;
												document.write('<option value="' + i + '">' + rate_name[i] + '\n');
												idx++;
											}
											else
											{
												if(band != 5 && band != 2)
													document.write('<option value="' + i + '">' + 'None' + '\n'); break;
											}
										}
										document.advanceSetup.NtxRate.selectedIndex=defidx;
									</script>
									</select>
									</td>
									</tr>
									</table>
									<input type="hidden" name="getRate" value="<% getInfo("wlanTranRate"); %>">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="ipnRateId" style="display:none">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(TransmitRate)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<select size="1" name="txRate">
									<script>
										var rateTbl=new Array("auto","1M","2M","5.5M","6M","9M","11M","12M","18M","24M","36M","48M","54M");
										var rValTbl=new Array("auto","10","20","55","60","90","110","120","180","240","360","480","540");
										if (wlDev==1 || wlDev==3)
										{
											for (i=0; i<=12; i++)
											{
												if (document.advanceSetup.getRate.value == rValTbl[i]) document.write('<option selected value="'+rValTbl[i]+'">'+rateTbl[i]+'</option>');
												else document.write('<option value="'+rValTbl[i]+'">'+rateTbl[i]+'</option>');
											}
										}
									</script>
									</select>
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="NChanWidthId">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlChanWidth)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<script>
										<%  write("NChanW = "+getIndex("NChanWidth")+";");%>
										if(!NChanW)
										{
											document.write('<input type="radio" name="wlanNChanWidth" value="0" checked>Auto 20/40 MHZ&nbsp;&nbsp;');
											document.write('<input type="radio" name="wlanNChanWidth" value="1" >20 MHZ');
										}
										else
										{
											document.write('<input type="radio" name="wlanNChanWidth" value="0" >Auto 20/40 MHZ&nbsp;&nbsp;');
											document.write('<input type="radio" name="wlanNChanWidth" value="1" checked>20 MHZ');
										}
									</script>
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="rtlPrmbId">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlPreambleType)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<script>
										<%  write("pream = "+getIndex("preamble")+";");%>
										if(!pream)
										{
											document.write('<input type="radio" name="preamble" value="long" checked>'+showText(wlPreambleShort)+'&nbsp;&nbsp;');
											document.write('<input type="radio" name="preamble" value="short">'+showText(wlPreambleLong)+'&nbsp;&nbsp;');
										}
										else
										{
											document.write('<input type="radio" name="preamble" value="long">'+showText(wlPreambleShort)+'&nbsp;&nbsp;');
											document.write('<input type="radio" name="preamble" value="short" checked>'+showText(wlPreambleLong)+'&nbsp;&nbsp;');
										}
									</script>
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="bcId">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlBroadcastEssid)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<script>
										<%  write("BSSID = "+getIndex("hiddenSSID")+";");%>
										if(!BSSID)
										{
											document.write('<input type="radio" name="hiddenSSID" value="no" checked>'+showText(enable)+'&nbsp;&nbsp;');
											document.write('<input type="radio" name="hiddenSSID" value="yes">'+showText(disable)+'&nbsp;&nbsp;');
										}
										else
										{
											document.write('<input type="radio" name="hiddenSSID" value="no">'+showText(enable)+'&nbsp;&nbsp;');
											document.write('<input type="radio" name="hiddenSSID" value="yes" checked>'+showText(disable)+'&nbsp;&nbsp;');
										}
									</script>
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="ctsId">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlCtsProtect)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<script>
										<%  write("cts = "+getIndex("wlanCts")+";");%>
										if(cts == 0)
										{
											document.write('<input type="radio" name="wlanCts" value="auto" checked>'+showText(autos)+'&nbsp;&nbsp;');
											document.write('<input type="radio" name="wlanCts" value="always">'+showText(always)+'&nbsp;&nbsp;');
											document.write('<input type="radio" name="wlanCts" value="none">'+showText(none)+'&nbsp;&nbsp;');
										}
										else if(cts==1)
										{
											document.write('<input type="radio" name="wlanCts" value="auto">'+showText(autos)+'&nbsp;&nbsp;');
											document.write('<input type="radio" name="wlanCts" value="always" checked>'+showText(always)+'&nbsp;&nbsp;');
											document.write('<input type="radio" name="wlanCts" value="none">'+showText(none)+'&nbsp;&nbsp;');
										}
										else
										{
											document.write('<input type="radio" name="wlanCts" value="auto">'+showText(autos)+'&nbsp;&nbsp;');
											document.write('<input type="radio" name="wlanCts" value="always">'+showText(always)+'&nbsp;&nbsp;');
											document.write('<input type="radio" name="wlanCts" value="none" checked>'+showText(none)+'&nbsp;&nbsp;');
										}
									</script>
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="wmmId">
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlWMM)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<script>
										<%  write("wmm = "+getIndex("wlanWmm")+";");%>
										if(wmm)
										{
											document.write('<input type="radio" name="wlanWmm" value="no" checked>'+showText(enable)+'&nbsp;&nbsp;');
											document.write('<input type="radio" name="wlanWmm" value="yes">'+showText(disable)+'&nbsp;&nbsp;');
										}
										else
										{
											document.write('<input type="radio" name="wlanWmm" value="no">'+showText(enable)+'&nbsp;&nbsp;');
											document.write('<input type="radio" name="wlanWmm" value="yes" checked>'+showText(disable)+'&nbsp;&nbsp;');
										}
									</script>
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
									<td colspan="2" height="20" valign="middle" align="right">
									<script>
										document.write('<input type=button value="Basic Settings" style ="width:100px" onClick="c_wlb(1);">');
										document.write('<input type=submit value="'+showText(apply1)+'" name="B1" onclick="set_mod(0);return saveChanges()" style ="width:100px">');
									</script>
									<input type="hidden" value="/wireless.asp" name="submit-url">
									</td>
									</tr>
									</table>
									</form>
									</div>
								</td>
							</tr>
						</table>
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666" class="style13">
									<input onclick="c_fun(1)" type="radio" name="sys"/>
									<span class="style13" style="cursor:pointer;" onclick="c_fun(1);" name="SS">&nbsp;<script>dw(securitySet)</script></span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<div id="wl_SS" style="display:none"><br>
									<form action=/goform/formWlEncrypt method=POST name="wlEncrypt">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
										<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(encryption)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<select size="1" name="method" onChange="displayObj();">
										<script>
											var modeTbl = new Array(showText(disable),"WEP","WPA pre-shared key","WPA RADIUS");
											for ( i=0; i<4; i++)
											{
												if ( i == methodVal)
												{
													if (apMode==1 || apMode==2 || opMode==1)
													{
														if (i!=3) document.write('<option selected value="'+i+'">'+modeTbl[i]+'</option>');
													}
													else document.write('<option selected value="'+i+'">'+modeTbl[i]+'</option>');
												}
												else
												{
													if (apMode==1 || apMode==2 || opMode==1)
													{
														if (i!=3) document.write('<option value="'+i+'">'+modeTbl[i]+'</option>');
													}
													else document.write('<option value="'+i+'">'+modeTbl[i]+'</option>');
												}
											}
											if (bandVal == 2) {
												if (document.wlEncrypt.method.length == 4)
													document.wlEncrypt.method.remove(3);
												document.wlEncrypt.method.remove(1);
											}
										</script>
										</select>
										</td>
										</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="wepId">
										<input type="hidden" name="wepEnabled" value="ON">
										<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(keyLen)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<select size="1" name="length" ONCHANGE=lengthClick()>
											<script>
												<%  write("wepmod = "+getIndex("wep")+";");%>
												if(wepmod != 2)
												{
													document.write('<option value=1 selected>64-bit</option>')
													document.write('<option value=2>128-bit</option>')
												}
												else if(wepmod == 2)
												{
													document.write('<option value=1>64-bit</option>')
													document.write('<option value=2 selected>128-bit</option>')
												}
											</script>
											</select>
										</td>
										</tr>
										<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(keyFrm)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<select size="1" name="format" ONCHANGE=setDefaultKeyValue()>
											<option value=1>ASCII</option>
											<option value=2>Hex</option>
										</select>
										</td>
										</tr>
										<input type="hidden" value="1" name="defaultTxKeyId">
										<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(keyEncry)</script>&nbsp;:&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<div id="wephide" style="display:block">
											<input type="password" name="key1" size="32" maxlength="32" onchange="setWepvalue(0)">
											<input type="checkbox" name="hidewep" value="ON" onclick="hideWepvalue(0);" checked="true" id="keyhidewep"><script>dw(keyHide)</script>
										</div>
										<div id="wepdisplay" style="display:none">
											<input type="text" name="key1_1" size="32" maxlength="32" onchange="setWepvalue(1)">
											<input type="checkbox" name="hidewep" value="OFF" onclick="hideWepvalue(1);" id="keyhidewep1"><script>dw(keyHide)</script>
										</div>
										</td>
										</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="wpaId" style="display:none">
										<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlCtrlWAP)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<script>
											<%  write("wpamod = "+getIndex("wpaCipher")+";");%>
											if ( typeof(_WIFI_11N_STANDARD_)!='undefined' && bandVal != 2 )
											{
												if(wpamod == 1)
												{
													document.write('<input type="radio" name="wpaCipher" value="1" id="wpaCipher1" checked>WPA(TKIP)&nbsp;&nbsp;');
													document.write('<input type="radio" name="wpaCipher" value="2" id="wpaCipher2">WPA2(AES)&nbsp;&nbsp;');
												}
												else if (wpamod == 2)
												{
													document.write('<input type="radio" name="wpaCipher" value="1" id="wpaCipher1">WPA(TKIP)&nbsp;&nbsp;');
													document.write('<input type="radio" name="wpaCipher" value="2" id="wpaCipher2" checked>WPA2(AES)&nbsp;&nbsp;');
												}
												else
												{
													document.write('<input type="radio" name="wpaCipher" value="1" id="wpaCipher1">WPA(TKIP)&nbsp;&nbsp;');
													document.write('<input type="radio" name="wpaCipher" value="2" id="wpaCipher2">WPA2(AES)&nbsp;&nbsp;');
												}
											}
											else
											{
												document.write('<input type="radio" name="wpaCipher" value="2" id="wpaCipher2" checked="checked" >WPA2(AES)&nbsp;&nbsp;');
											}
											if ((apMode==0 || apMode==5) && bandVal != 2) {
												if(wpamod == 3) document.write('<input type="radio" name="wpaCipher" value="3" checked> WPA2 Mixed');
												else document.write('<input type="radio" name="wpaCipher" value="3"> WPA2 Mixed');
											}
										</script>
										</td>
										</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="pskId">
										<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(keyFrmPre)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<select size="1" name="pskFormat">
											<script>
												<%  write("pskmod = "+getIndex("pskFormat")+";");%>
												if(!pskmod)
												{
													document.write('<option value=0 selected>'+showText(passphrase)+'</option>')
													document.write('<option value=1>Hex (64 '+showText(wlencryptStrCharaolitec)+')</option>')
												}
												else
												{
													document.write('<option value=0>'+showText(passphrase)+'</option>')
													document.write('<option value=1 selected>Hex (64 '+showText(wlencryptStrCharaolitec)+')</option>')
												}
											</script>
											</select>
										</td>
										</tr>
										<script>var pskval = "<%getInfo("pskValue");%>"</script>
										<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(keyPre)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<div id="hidepsk" style="display:block">
											<script>document.write('<input type="password" name="pskValue" size="32" maxlength="64" value="'+pskval+'" onchange="setpskvalue(0)">')</script>
											<input type="checkbox" name="hidePsk" value="ON" onclick="hidePskvalue(0);" checked="true" id="keyhide"><script>dw(keyHide)</script>
										</div>
										<div id="displaypsk" style="display:none">
											<script>document.write('<input type="text" name="pskValue_1" size="32" maxlength="64" value="'+pskval+'" onchange="setpskvalue(1)">')</script><input type="checkbox" name="hidePsk" value="OFF" onclick="hidePskvalue(1);" id="keyhide1"><script>dw(keyHide)</script>
										</div>
										</td>
										</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="inRdsId">
										<tr>
										<td colspan="2" height="20" valign="middle" align="left" class="style1">
											<input type="checkbox" name="sel1xMode" value="ON" <% if (getIndex("1xMode")==0) write("checked"); %>  onClick="disRADIUS();">&nbsp;&nbsp;<script>dw(wlCtrlSer)</script>
										</td>
										</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="enRadiusId" style="display:none">
										<tr>
										<td colspan="2" height="20" valign="middle" align="left" class="style1">
											<input type="checkbox" name="enRadius" value="ON" <% if (getIndex("enable1X")==1) write("checked"); %>  onClick="displayObj();">&nbsp;&nbsp;<script>dw(wlCtrlEn802)</script>
										</td>
										</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center" id="radiusId">
										<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlCtrlSerIP)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="radiusIP" size="15"maxlength="15" value=<% getInfo("rsIp"); %>>
										</td>
										</tr>
										<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlCtrlSerPort)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="radiusPort" size="5" maxlength="5" value=<% getInfo("rsPort"); %>>
										</td>
										</tr>
										<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wlCtrlSerPass)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="password" name="radiusPass" size="20" maxlength="64" value=<% getInfo("rsPassword"); %>>
										</td>
										</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
										<tr>
										<td colspan="2" height="20" valign="middle" align="right" class="style1">
											<script>buffer='<input type=submit value="'+showText(apply1)+'" onclick="set_mod(0);return saveChanges()" style ="width:100px">';document.write(buffer);</script>
											<input type="hidden" value="/wireless.asp" name="submit-url">
										</td>
										</tr>
									</table>
									</form>
									</div>
								</td>
							</tr>
						</table>
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666" class="style13">
									<input onclick="c_fun(2)" type="radio" name="sys"/>
									<span class="style13" style="cursor:pointer;" onclick="c_fun(2);" name="WMF">&nbsp;<script>dw(wlMacAddrFiltering)</script></span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<div id="wl_MAF" style="display:none"><br>
									<form action=/goform/formWlAc method=POST name="formWlAcEnabled">
									<input type="hidden" value="Add" name="addFilterMac">
									<input type="hidden" value="" name="wlanAcEnabled">
									<input type="hidden" value="/wireless.asp" name="submit-url">
									</form>
									<form action=/goform/formWlAc method=POST name="formFilterAdd">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
									<td height="20" valign="middle" align="left" class="style14">
										<input type="checkbox" name="wlanAcEnabled" value="ON" <% if (getIndex("wlanAcEnabled")) write("checked"); %> onclick="set_mod(1);goToWeb();wlCtrl()">&nbsp;&nbsp;<script>dw(wlCtrlAccEn)</script>
									</td>
									</tr>
									</table>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
									<td width="5%" align="center" height="20" bgcolor="#666666" class="style13">
										<script>dw(news)</script>
										<input type="hidden" name="tiny_idx" value="0">
									</td>
									<td width="30%" height="20" valign="middle" align="center" class="style14">
										<script>dw(wlMacAddress)</script>: <input type="text" name="mac" size="15" maxlength="12">
									</td>
									<td width="30%" height="20" valign="middle" align="center" class="style14">
										Comment: <input type="text" name="comment" size="16" maxlength="16" class="text" onchange="check(this)">
									</td>
									<td width="15%" height="20" valign="middle" align="center" class="style14">
									<script>
										document.write('<input type="submit" value="'+showText(add)+'" name="addFilterMac" onClick="set_mod(1);return addClick()" class="btnsize">')
										document.write('<input type="reset" value="'+showText(clear)+'" name="reset" class="btnsize">')
									</script>
									<input type="hidden" value="/wireless.asp" name="submit-url">
									</td>
									</tr>
									</table>
									</form>
									<br>
									<form action=/goform/formWlAc method=POST name="formFilterDel">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
										<td height="20" valign="middle" align="left" class="style14"><script>dw(wlCtrlTableInfo)</script> :&nbsp;&nbsp;</td>
									</tr>
									</table>
									<table width="600" border="1" cellpadding="0" cellspacing="0" align="center" bgcolor="#FFFFFF" class="style14">
									<tr class="style13">
										<td align="center" height="20" bgcolor="#666666" width="10%">NO.</td>
										<td align="center" height="20" bgcolor="#666666" width="45%"><script>dw(wlMacAddress)</script></td>
										<td align="center" height="20" bgcolor="#666666" width="35%">Comment</td>
										<td align="center" height="20" bgcolor="#666666" width="15%"><script>dw(select)</script></td>
									</tr>
									<% wlAcList(); %>
									</table>
									<br>
									<table width="600" border="0" cellpadding="0" cellspacing="0" align="center">
									<tr>
									<td align="right">
									<script>
										document.write('<input type="submit" value="'+showText(deleteSelected)+'" name="deleteSelFilterMac" onClick="set_mod(1);return deleteClick()" class="btnsize">');
										document.write('<input type="submit" value="'+showText(deleteAll)+'" name="deleteAllFilterMac" onClick="set_mod(1);return deleteAllClick()" class="btnsize">');
									</script>
									<input type="hidden" value="/wireless.asp" name="submit-url">
									<script>
									<%
										entryNum = getIndex("wlanAcNum");
										if ( entryNum == 0 ) {
											write( "disableDelButton();" );
										}
									%>
									</script>
									</td>
									</tr>
									</table>
									</form>
									<br>
									<form action=/goform/formWlAc method=POST name="formPortFwApply">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
									<td colspan="2" height="20" valign="middle" align="right" class="style1">
									<script>buffer='<input type=submit value="'+showText(apply1)+'" onclick="set_mod(0);saveChanges();" style ="width:100px">';document.write(buffer);</script>
									<input type="hidden" value="/wireless.asp" name="submit-url">
									<input type="hidden" value="1" name="isApply">
									</td>
									</tr>
									</table>
									</form>
									</div>
								</td>
							</tr>
						</table>
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0" align="center">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666" class="style13">
									<input onclick="c_fun(3)" type="radio" name="sys">
									<span class="style13" style="cursor:pointer;" onclick="c_fun(3);" name="WPS">&nbsp;<script>dw(wpsSetting)</script></span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<div id="wl_WPS" style="display:none"><br>
									<form action=/goform/formWpsEnable method=POST name="Wpsseting">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
									<td height="20" valign="middle" align="left" class="style1">
									<input type="checkbox" name="wpsEnable"  value="ON" onClick="set_mod(2);saveChanges();" <% if (getIndex("wpsEnable")==1)  write("checked ");%> >
									<script>
										if(stype==7) document.write('WPS '+showText(enable))
										else document.write(showText(enable)+' WPS')
									 </script>
									</td>
									<input type=hidden value="/wireless.asp" name="wlan-url">
									</tr>
									</table>
									</form>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
									<td colspan="2" align="left" height="25px" class="style14"><script>dw(wpsSetupInfo)</script> :&nbsp;&nbsp;</td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wspStatus)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<script>
										if (!wpsConType) document.write(showText(WPSunconfigured));
										else document.write(showText(WPSconfigured))
									</script>
									</td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wpsPinCode)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1"><%getInfo("pinCode");%></td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(ssid)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1"><script>document.write(wl_ssid);</script></td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(authMode)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<script>
										var modeTbl = new Array(showText(disable),"WEP","WPA pre-shared key","WPA RADIUS");
										document.write(modeTbl[methodVal]);
									</script>
									</td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14">Authentication Key :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1"><% getInfo("wps_key_val"); %></td>
									</tr>
									</table>
									<form action=/goform/formWpsStart method=POST name="WpsDevice">
									<input type=hidden value="/wireless.asp" name="wlan-url">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
									<td colspan="2" align="left" height="25px" class="style14"><script>dw(deviceConfigure)</script> &nbsp;&nbsp;</td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(configMode)</script></td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<select size="1" name="confMode" onchange="changeMode()">
									<script>
										<% write("choice = "+getIndex("wpsConfigMode")+";");%>
										if (!choice) document.write('<option value="0" selected>Registrar</option><option value="1">Enrollee</option>')
										else document.write('<option value="0">Registrar</option><option value="1" selected>Enrollee</option>')
									</script>
									</select>
									</td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wpsPubMode)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
									<input type=hidden value="pin" name="configOption">
									<script>document.write('<input type="button" name="startPbc" value="'+showText(wpsStartPbc)+'" onclick="set_mod(2);return save(2);"  style ="width:80px">');</script>
									</td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(wpsPinMode)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="text" name="pinCode" size="15" maxlength="8">
										<script>document.write('<input type="button" name="startPin" value="'+showText(wpsStartpin)+'" onclick="set_mod(2);return save(1);"  style ="width:80px">');</script>
									</td>
									</tr>
									</table>
									</form>
									</div>
								</td>
							</tr>
						</table>
						</div>
					</td>
					<!-- explain -->
					<td width="380" valign="top" bgcolor="#999999">
						<table width="380" border="0" align="right" cellpadding="0" cellspacing="10">
							<tr>
								<td height="400" valign="top">
									<table width="360"  height="400 "border="0" cellpadding="5" cellspacing="0" bgcolor="#999999">
										<tr>
											<td height="16" bgcolor="#A5031A"><span class="style2">Help</span></td>
										</tr>
										<tr>
											<td height="384" valign="top" bgcolor="#999999">
												<span class="style1">
													<span class="style13">Wireless <script>dw(basicSet)</script></span><br>
													<span id="com00" class="style21" style="display:block"><script>dw(wlBasicInfoshort)</script><a class="style71" onclick="com_sw(0,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com01" class="style21" style="display:none"><script>dw(wlBasicInfo)</script><a class="style71" onclick="com_sw(0,2);" style="cursor:pointer;" > .....close</a></span>
													<br>
													<span class="style13">Wireless <script>dw(advancedSet)</script></span><br>
													<span id="com10" class="style21" style="display:block"><script>dw(wlAdInfoshort)</script><a class="style71" onclick="com_sw(1,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com11" class="style21" style="display:none"><script>dw(wlAdInfo)</script><a class="style71" onclick="com_sw(1,2);" style="cursor:pointer;" > .....close</a></span>
												</span>
												<br>
												<span class="style1">
													<span class="style13"><script>dw(securitySet)</script></span><br>
													<span id="com20" class="style21" style="display:block"><script>dw(wlScrInfoshort)</script><a class="style71" onclick="com_sw(2,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com21" class="style21" style="display:none"><script>dw(wlScrInfo)</script><a class="style71" onclick="com_sw(2,2);" style="cursor:pointer;" > .....close</a></span>
												</span>
												<br>
												<span class="style1">
													<span class="style13"><script>dw(wlMacAddrFiltering)</script></span><br>
													<span id="com30" class="style21" style="display:block"><script>dw(wlCtrlInfoshort)</script><a class="style71" onclick="com_sw(3,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com31" class="style21" style="display:none"><script>dw(wlCtrlInfo)</script><a class="style71" onclick="com_sw(3,2);" style="cursor:pointer;" > .....close</a></span>
												</span>
												<br>
												<span class="style1">
													<span class="style13"><script>dw(wpsSetting)</script></span><br>
													<span id="com40" class="style21" style="display:block"><script>dw(wpsInfoshort)</script><a class="style71" onclick="com_sw(4,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com41" class="style21" style="display:none"><script>dw(wpsInfo)</script><a class="style71" onclick="com_sw(4,2);" style="cursor:pointer;" > .....close</a></span>
												</span>
											</td>
										</tr>
									</table>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
			<!-- down menu -->
			<script>Write_tail();</script>
		</td>
	</tr>
</table>
</body>
</html>
<script>
	document.getElementById('wl_BS').style.display = "none"
	document.getElementById('wl_SW').style.display = "none"
	document.getElementById('wl_SS').style.display = "none"
	document.getElementById('wl_MAF').style.display = "none"
	document.getElementById('wl_WPS').style.display = "none"
	if (top.WL_MODE == 1)
	{
		document.getElementsByName("sys")[2].checked=true;
		document.getElementById('wl_MAF').style.display = "block";
	}
	else if (top.WL_MODE == 2)
	{
		document.getElementsByName("sys")[3].checked=true;
		document.getElementById('wl_WPS').style.display = "block";
	}
	else
	{
		document.getElementsByName("sys")[0].checked=true;
		if( (_MODEL_ == "BR6258GN" || _MODEL_ == "BR6228GNS") && document.getElementsByName("wlanDisabled")[1].checked)
		{
			document.getElementsByName("sys")[0].disabled=true
			document.getElementsByName("sys")[1].disabled=true
			document.getElementsByName("sys")[2].disabled=true
			document.getElementsByName("sys")[3].disabled=true
			document.getElementById('wl_SW').style.display = "block"
			document.getElementById('wl_SWApply').style.display = "block"
		}
		else
		{
			document.getElementById('wl_SW').style.display = "block"
			document.getElementById('wl_BS').style.display = "block"
		}
	}
	if (document.Wpsseting.wpsEnable.checked != true )
	{
		document.WpsDevice.confMode.disabled=true
		document.WpsDevice.startPbc.disabled=true
		document.WpsDevice.pinCode.disabled=true
		document.WpsDevice.startPin.disabled=true
	}
	updateFormat();
	displayObj();
	var internet_sta = "<% getInfo("sta-current"); %>"
	top.sw_current_sta(internet_sta);
	top.cloud_current_sta(cloud_sta);
	wlCtrl()
</script>
