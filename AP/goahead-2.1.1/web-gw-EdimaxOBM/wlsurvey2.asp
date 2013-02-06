<html>
  <head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8">
    <meta http-equiv="Pragma" content="no-cache">
    <meta http-equiv="Expires" content="-1">
    <link rel="stylesheet" href="set.css">
    <script type="text/javascript" src="file/javascript.js"></script>
    <script type="text/javascript" src="/file/multilanguage.var"></script>
	 <script type="text/javascript" src="file/common.var"></script>
    <title></title>
<script>
var connectEnabled=0, autoconf=0;
var sel_value;
function verifyBrowser() {
  var ms = navigator.appVersion.indexOf("MSIE");
  ie4 = (ms>0) && (parseInt(navigator.appVersion.substring(ms+5, ms+6)) >= 4);
  var ns = navigator.appName.indexOf("Netscape");
  ns= (ns>=0) && (parseInt(navigator.appVersion.substring(0,1))>=4);
  if (ie4)
	return "ie4";
  else
	if(ns)
		return "ns";
	else
		return false;
}
function disableButton (button) {
  if (verifyBrowser() == "ns")
  	return;
  if (document.all || document.getElementById)
    button.disabled = true;
  else if (button) {
    button.oldOnClick = button.onclick;
    button.onclick = null;
    button.oldValue = button.value;
    button.value = 'DISABLED';
  }
}
function enableButton (button) {
  if (verifyBrowser() == "ns")
  	return;
  if (document.all || document.getElementById)
    button.disabled = false;
  else if (button) {
    button.onclick = button.oldOnClick;
    button.value = button.oldValue;
  }
}
function enableConnect(val)
{
  document.forms['formWlSiteSurvey'].elements['done'].disabled = false;
  connectEnabled=1;
  sel_value=val;
}
function connectClick()
{
  var i;
  var apNum=document.forms['formWlSiteSurvey'].elements['apCount'].value;
  if (connectEnabled==1) {
	for ( i=0; i<apNum; i++) {
		if (sel_value == "sel"+i) {
			apmode=window.opener.document.forms['wlanSetup'].elements['apMode'].selectedIndex;
			if(apmode==1)
				window.opener.document.forms['wlanSetup'].ssid.value = document.forms['formWlSiteSurvey'].elements['ssid'+i].value;
			else
				window.opener.document.forms['wlanSetup'].repeaterSSID.value = document.forms['formWlSiteSurvey'].elements['ssid'+i].value;
			window.opener.document.forms['wlanSetup'].chan.value = parseInt(document.forms['formWlSiteSurvey'].elements['chan'+i].value);
		}
	}
		window.close();
  }
  	return false;
}
function loadonstart() {
	<%  write("sitesurveyed = "+getIndex("sitesurveyed"));%>
	if (sitesurveyed == 0)
	{
		disableButton(document.formWlSiteSurvey.refresh);
		document.searching.submit();
		//window.opener.reloadPage();
	}
}
</script>
</head>
  <body class="mainbg" onload="loadonstart();">
    <blockquote>
      <br>
      <a class="titlecolor"><script>dw(WirelessSiteSurvey)</script></a>
      <br>
      <br>
		<table width="700" border="0" cellspacing="0" cellpadding="0">
		<tr><td><font size=2>
 		<script>dw(WirelessSiteSurveyContent);</script>
		</font></td></tr>
		</table>
		<form action=/goform/formWlSiteSurvey method=POST name="searching">
			<input type="hidden" value="Refresh" name="refresh">
			<input type="hidden" value="/wlsurvey2.asp" name="submit-url">
		</form>
		<form action=/goform/formWlSiteSurvey method=POST name="formWlSiteSurvey">
		  <table width="700" border="1" cellspacing="0" cellpadding="0">
          <tr class="style13" >
            <td align="center"><script>dw(Select)</script></td>
            <td align="center"><script>dw(SSID)</script></td>
            <td align="center"><script>dw(BSSID)</script></td>
            <td align="center"><script>dw(Band)</script></td>
            <td align="center"><script>dw(Channel)</script></td>
            <td align="center"><script>dw(Type)</script></td>
            <td align="center"><script>dw(Encryption)</script></td>
            <td align="center"><script>dw(Signal)</script></td>
          </tr>
		  <% wlSiteSurveyTbl(); %>
		  </table>
		  <br>
		 <script>document.write('<input type="submit" value="'+showText(Refresh)+'" name="refresh" class="button">');</script>
       <script>document.write('<input type="button" value="'+showText(Done)+'" name="done" onClick="connectClick()" class="button">');</script>
       <script>document.write('<input type="button" value="'+showText(Close)+'" name="close" onClick="window.close();" class="button">');</script>
		 <input type="hidden" value="/wlsurvey2.asp" name="submit-url">
		 <script>
				document.forms['formWlSiteSurvey'].elements['done'].disabled = true;
		 </script>
		</form>
    </blockquote>
  </body>
</html>
