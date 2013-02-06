<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<link rel="stylesheet" href="edimax.css">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="FUNCTION_SCRIPT"></script>
<script language="JavaScript">
	var MODEL = _MODEL_
	if ( typeof _WanAutoDetect_ != 'undefined') var DEF_AWD = 1; else DEF_AWD = 0;
	if ( typeof _EDIMAX30_ != 'undefined') var edimae30=1
	else edimae30=0
	var mac = "<% getInfo("nic0mac"); %>"
	var ssiddrv = "<% getInfo("ssid_drv"); %>"
	var iprom = "<%getInfo("ip-rom");%>"
	var fwver = "<%getInfo("fwVersion");%>"
	var scannameTbl;
	var scanipTbl;
	var scanmacTbl;
	var scantypeTbl;
	var nameTbl;
	var ipTbl;
	var macTbl;
	var upTbl;
	var downTbl;
	var typeTbl;
	var blockTbl;
	var blockstTbl;
	var blocketTbl;
	var pfTbl;
	var dmzsetting;
	var WL_MODE;
	function reset_top_item()
	{
		WL_MODE = 0;
	}
	function goEZ(in_mode)
	{
		mode=in_mode;
		cdwindow=window.open('countdown.asp','CountDown','channelmode=0, directories=0,fullscreen=0,height=100,location=0,menubar=0,resizable=1,scrollbars=0,status=0,titlebar=0,toolbar=0,width=450','false');
		cdwindow.focus();
	}
	function startclock()
	{
		stopclock()
		showtime()
	}
	var timerID = null
	var timerRunning = false
	function stopclock()
	{
	   if(timerRunning)
		   clearTimeout(timerID)
	   timerRunning = false
	}
	var now =new Date(<% write(getIndex("systime")); %>);
	var hours = now.getHours()
	var minutes = now.getMinutes()
	var seconds = now.getSeconds()
	function showtime(){
		var timeValue = " " + hours
		timeValue  += ((minutes < 10) ? ":0" : ":") + minutes
		timeValue  += ((seconds < 10) ? ":0" : ":") + seconds
		if( top.mainFrame.document.getElementById("face"))
		{
			top.mainFrame.document.getElementById("face").innerHTML = timeValue
		}
		up_sec += 1;
		if(up_sec==60) { up_sec=0; up_min+=1; }
		if(up_min==60) { up_min=0; up_hr +=1; }
		if(up_hr ==24) { up_hr =0; up_day+=1; }
		var upclock = "" + up_day + "day";
		upclock += ((up_hr  < 10) ? ":0" : ":") + up_hr
		upclock += ((up_min < 10) ? ":0" : ":") + up_min
		upclock += ((up_sec < 10) ? ":0" : ":") + up_sec
		if ( window.parent.mainFrame.location.pathname == "/main.asp" || window.parent.mainFrame.location.pathname == "/stainfo.asp" )
			if( top.mainFrame.document.getElementById("showuptime"))
			{
				top.mainFrame.document.getElementById("showuptime").innerHTML = "Running Time"+" " + upclock
			}
		seconds += 1;
		if(seconds == 60) { seconds=0; minutes+=1; }
		if(minutes == 60) { minutes=0; hours+=1; }
		if(hours == 24)   { hours=0; }
		timerID = setTimeout("showtime()",1000)
		timerRunning = true
	}
	var up_time = "<%getInfo("uptime");%>";
	var up_day  = parseInt(up_time.substring(0,up_time.indexOf("day")));
	var up_hr   = parseInt(up_time.substring(up_time.indexOf("day")+4,up_time.indexOf("h")));
	var up_min  = parseInt(up_time.substring(up_time.indexOf("h")+2,up_time.indexOf("m")));
	var up_sec  = parseInt(up_time.substring(up_time.indexOf("m")+2,up_time.indexOf("s")));
	function sw_current_sta(internet_sta)
	{
		switch(internet_sta)
		{
			case "Dynamic IP disconnect":
			case "PPPoE disconnected":
			case "PPTP disconnected":
			case "L2TP disconnected":
			case "Fixed IP disconnect":
				window.parent.mainFrame.document.getElementsByName("currentsta")[0].src = "images/ball-2.gif";
				break;
		}
	}
	function cloud_current_sta(cloudsta)
	{
		switch(cloudsta)	// jeremy 100715 added
		{
			case "0":
				window.parent.mainFrame.document.getElementsByName("cloudstaimg")[0].src = "images/cloud-logo_up.gif";
				break;
			case "1":
				window.parent.mainFrame.document.getElementsByName("cloudstaimg")[0].src = "images/cloud-logo.gif";
				break;
			default:
				break;
		}
	}
</script>
<title>EDIMAX Technology</title>
</head>
<frameset>
	<frame name="mainFrame" src="main.asp">
</frameset>
</html>
