<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<link rel="stylesheet" href="edimax.css">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="file/qosnat-n.var"></script>
<script type="text/javascript" src="file/multilanguage.var"></script>
<script language ='javascript' src ="file/fwwl-n.var"></script>
<script type="text/javascript" src="file/statustool-n.var"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<script type="text/javascript" src="file/ezview-n.var"></script>
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="FUNCTION_SCRIPT"></script>
<script>
var cloud_sta = "<% getInfo("cloud-sta"); %>"
//========================================= EZQOS function
EMPTY_TD_INNERHTML="<a>&nbsp;</a>"
EZQOS_TD_ID_PREFIX="EZQOS_TD_"
TEMP_TD_ID_PREFIX="TEMP_TD_"
EZQOS_PIC_SIZE="height=\"100%\" width=\"100%\""
TEMP_PIC_SIZE="height=\"35\" width=\"35\""
PIC_ALTNAME=".gif"
function check_enable()
{
	var bandWidth_type0=document.getElementsByName("BandWidth_Type")[0];
	var bandWidth_type1=document.getElementsByName("BandWidth_Type")[1];
	if ( document.formEZQoS.enabled2.checked==true )
	{
		bandWidth_type0.disabled=false;
		bandWidth_type1.disabled=false;
	}
	else
	{
	  bandWidth_type0.disabled=true;
	  bandWidth_type1.disabled=true;
	}
}
function Disable_Bandwidth()
{
	var UpBand=document.formEZQoS.maxup;
	var DownBand=document.formEZQoS.maxdown;
	if (( document.getElementsByName("BandWidth_Type")[1].checked==true ) && ( document.formEZQoS.enabled2.checked==true ))
	{
		UpBand.disabled=false;
		DownBand.disabled=false;
	}
	else
	{
	  UpBand.disabled=true
	  DownBand.disabled=true;
	}
}
function mouseclick_item( obj )
{
	if( obj.innerHTML.toLowerCase() != EMPTY_TD_INNERHTML.toLowerCase())
	{
		//temp block pressed
		if( obj.id.substring(0,TEMP_TD_ID_PREFIX.length).toLowerCase() == TEMP_TD_ID_PREFIX.toLowerCase() )
		{
			REMOVING_PREFIX=TEMP_TD_ID_PREFIX
			ADDING_PREFIX=EZQOS_TD_ID_PREFIX
			OLD_SIZE=TEMP_PIC_SIZE
			NEW_SIZE=EZQOS_PIC_SIZE
		}
		//ezqos block pressed
		else if( obj.id.substring(0,EZQOS_TD_ID_PREFIX.length).toLowerCase() == EZQOS_TD_ID_PREFIX.toLowerCase() )
		{
			REMOVING_PREFIX=EZQOS_TD_ID_PREFIX
			ADDING_PREFIX=TEMP_TD_ID_PREFIX
			OLD_SIZE=EZQOS_PIC_SIZE
			NEW_SIZE=TEMP_PIC_SIZE
		}
		for(i=1; i<=5 && document.getElementById(ADDING_PREFIX+""+parseInt(i)).innerHTML.toLowerCase()!=EMPTY_TD_INNERHTML.toLowerCase() ; i++);
		if(i>5) return;
		temp = obj.innerHTML.substring(0,  obj.innerHTML.search(PIC_ALTNAME)+ PIC_ALTNAME.length+1 )
		document.getElementById(ADDING_PREFIX+""+i).innerHTML = temp +  NEW_SIZE +">"
		for(i=parseInt(obj.id.substring(REMOVING_PREFIX.length, obj.id.length)); i<=5; i++)
		{
			if(i==5)
			{
				document.getElementById(REMOVING_PREFIX + parseInt(i) ).innerHTML=EMPTY_TD_INNERHTML
			}
			else
			{
				obj1=document.getElementById(REMOVING_PREFIX + parseInt(i) )
				obj2=document.getElementById(REMOVING_PREFIX + (parseInt(i)+1) )
				obj1.innerHTML = obj2.innerHTML
			}
		}
		if ( document.getElementById("EZQOS_TD_1").innerHTML == "<a>&nbsp;</a>" || document.getElementById("EZQOS_TD_1").innerHTML == "<A>&nbsp;</A>")
			document.getElementById("apply_button").disabled = false;
		else
		{
			if( document.getElementById("EZQOS_TD_5").innerHTML == "<a>&nbsp;</a>" ||  document.getElementById("EZQOS_TD_5").innerHTML == "<A>&nbsp;</A>" )
				document.getElementById("apply_button").disabled = true;
			else
				document.getElementById("apply_button").disabled = false;
		}
	}
}
function goToWeb() {
		if (document.formEZQoS.enabled2.checked==true)
				document.formEZQoSEnabled.enabled1.value="ON";
			document.formEZQoSEnabled.submit();
	}
function clickapply()
{
	if (!portRule(document.formEZQoS.maxdown, showText(qosdoband), 0,"", 0, 99999, 1))
		return false;
	if (!portRule(document.formEZQoS.maxup, showText(qosupband), 0,"", 0, 99999, 1))
		return false;
	document.EZqosApply.maxdown.value = document.formEZQoS.maxdown.value;
	document.EZqosApply.maxup.value = document.formEZQoS.maxup.value;
	for(i=1; i<=5; i++)
	{
		if(document.getElementById(EZQOS_TD_ID_PREFIX+i).innerHTML.search("APP") != -1)
			document.EZqosApply.ezqos_APP.value = i-1;
		if(document.getElementById(EZQOS_TD_ID_PREFIX+i).innerHTML.search("MEDIA") != -1)
			document.EZqosApply.ezqos_MEDIA.value = i-1;
		if(document.getElementById(EZQOS_TD_ID_PREFIX+i).innerHTML.search("P2P") != -1)
			document.EZqosApply.ezqos_P2P.value = i-1;
		if(document.getElementById(EZQOS_TD_ID_PREFIX+i).innerHTML.search("FTP") != -1)
			document.EZqosApply.ezqos_FTP.value = i-1;
		if(document.getElementById(EZQOS_TD_ID_PREFIX+i).innerHTML.search("GAME") != -1)
			document.EZqosApply.ezqos_GAME.value = i-1;
	}
	return true;
}
//========================================= EZQOS function	end
<% write("entryNum = "+getIndex("wan1QosNum")+";"); %>
/* function for main */
	var comm = new Array( new Array("com00","com01"),
							 new Array("com10","com11"),
							 new Array("com20","com21"),
							 new Array("com30","com31"))
	function com_sw(cnum,com)
	{
		if (com == 1)
		{
			document.getElementById(comm[cnum][0]).style.display = "none"
			document.getElementById(comm[cnum][1]).style.display = "block"
		}
		else
		{
			document.getElementById(comm[cnum][0]).style.display = "block"
			document.getElementById(comm[cnum][1]).style.display = "none"
		}
	}
	function test()
	{
	<% write("NewPage = "+getIndex("newpage"));%>
	<% write("Changed = "+getIndex("Changed"));%>
		if ( Changed == "1" ) {
			alert(showText(wanqosAlertTestCha));
		}
	}
	function set_mod(advmod)
	{
		top.ADV_MODE = advmod;
	}
</script>
<title></title>
</head>
<body onLoad="top.startclock()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(3,0)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF"><br />
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="center" width="700" height="25px" bgcolor="#666666" class="style13"><script>dw(ezqos);</script></td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<form action=/goform/formEZQoS method=POST name="formEZQoSEnabled">
										<input type="hidden" value="ON" name="isEnabled">
										<input type="hidden" value="" name="enabled1">
										<input type="hidden" value="" name="autoDetect1">
										<input type="hidden" value="" name="manual1">
										<input type="hidden" value="EZQoS" name="EZQos_Mode">
										<input type="hidden" value="/ezqos.asp" name="submitUrl">
									</form>
									<form action=/goform/formEZQoS method=POST name="formEZQoS">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
									<td colspan="2" height="20" valign="middle" align="left" class="style1">
									<input type="checkbox" name="enabled2" value="ON" <% if (getIndex("EZQosEnabled")) write("checked");%> onClick="goToWeb();"><script>dw(ezqosEnable);</script>
									</td>
									</tr>
									<!--tr>
									<td colspan="2" height="20" valign="middle" align="left" class="style1">
									<input type="button" name="BandDetect" value="" onClick="top.goEZqos();" class="btnsize">
									<script>document.formEZQoS.BandDetect.value = showText(BandDetect)</script>
									</td>
									</tr-->
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(qosdoband)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="text" name="maxdown" size="5" maxlength="5" value="<% getInfo("maxdownbandwidth"); %>">&nbsp;kbits
									</td>
									</tr>
									<tr>
									<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(qosupband)</script> :&nbsp;&nbsp;</td>
									<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="text" name="maxup" size="5" maxlength="5" value="<% getInfo("maxupbandwidth"); %>">&nbsp;kbits
									</td>
									</tr>
									</table>
									<style type="text/css">
										.with_pic_ezqos   { border:none; width:69px; height:69px; }
										.without_pic_ezqos   { border-style:dashed; border-width:1px; border-color:black; width:69px; height:69px; }
										.without_pic_temp   { border-style:dashed; border-width:1px; border-color:black; width:35px; height:37px; }
									</style>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr><td colspan="2" height="20" valign="middle" align="left" class="style14"><script>dw(ezqosTable)</script> :&nbsp;&nbsp;</td></tr>
									<tr>
									<td align="center">
									<table cellspacing="5" cellpadding="0">
									<tr>
									<td colspan="5" class="style13" bgcolor="#666666">
									<div style="float:left">High</div>
									<div style="float:right">Low</div>
									</td>
									</tr>
									<tr height="71">
									<% EZQosList(); %>
									</tr>
									<tr>
									<td colspan="5" align="center">
									<table cellspacing="5" cellpadding="0">
									<tr height="35">
									<td id="TEMP_TD_1" width="35" class="without_pic_temp" onClick="mouseclick_item(this)"><a>&nbsp;</a></td>
									<td id="TEMP_TD_2" width="35" class="without_pic_temp" onClick="mouseclick_item(this)"><a>&nbsp;</a></td>
									<td id="TEMP_TD_3" width="35" class="without_pic_temp" onClick="mouseclick_item(this)"><a>&nbsp;</a></td>
									<td id="TEMP_TD_4" width="35" class="without_pic_temp" onClick="mouseclick_item(this)"><a>&nbsp;</a></td>
									<td id="TEMP_TD_5" width="35" class="without_pic_temp" onClick="mouseclick_item(this)"><a>&nbsp;</a></td>
									</tr>
									</table>
									</td>
									</tr>
									</table>
									</td>
									</tr>
									</table>
									<div id="moving_img" style="height:0px;position:absolute;display:none;" onMouseDown="mousedown_item(this, event)"></div>
									<input type="hidden" value="1" name="configWan">
									<input type="hidden" value="/ezqos.asp" name="submitUrl">
									</form>
									<form action=/goform/formEZQoS method=POST name="EZqosApply">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
									<td height="20" valign="middle" align="right" class="style1">
									<script>
										document.write('<input type=submit id="apply_button" value="'+showText(apply1)+'" onclick="return clickapply()" style ="width:100px">');
										document.write('<input type=button value="'+showText(cancel1)+'" onClick="set_mod(0);window.location.reload();">');
									</script>
									<input type="hidden" value="/ezqos.asp" name="submitUrl">
									<input type="hidden" value="1" name="isApply">
									<input type="hidden" name="maxup">
									<input type="hidden" name="maxdown">
									<input type="hidden" name="ezqos_GAME">
									<input type="hidden" name="ezqos_P2P">
									<input type="hidden" name="ezqos_FTP">
									<input type="hidden" name="ezqos_MEDIA">
									<input type="hidden" name="ezqos_APP">
									</td>
									</tr>
									</table>
									</form>
								</td>
							</tr>
						</table>
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
													<span class="style13">iQos</span><br>
													<span id="com00" class="style21" style="display:block">
													For optimum networking performance, please enter the correct upload and download bandwidth value of your Internet service.
													<a class="style71" onclick="com_sw(0,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com01" class="style21" style="display:none">
													For optimum networking performance, please enter the correct upload and download bandwidth value of your Internet service. If you are not sure, please contact your Internet service provider.
													<a class="style71" onclick="com_sw(0,2);" style="cursor:pointer;" > .....close</a></span>
												</span>
												<span class="style1">
													<span class="style13">Current iQos Table</span><br>
													<span id="com20" class="style21" style="display:block">
													The icons on the left show the current priority order for various application types (from left to right).
													<a class="style71" onclick="com_sw(2,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com21" class="style21" style="display:none">
													The icons on the left show the current priority order for various application types (from left to right).
													You could re-arrange the priority by clicking the icons in the order that suits your needs.
													After you have arranged your application priorities, click the “Apply” button to enable it.
													The icons will be shown in the order of your preference after the device has restarted.
													<a class="style71" onclick="com_sw(2,2);" style="cursor:pointer;" > .....close</a></span>
												</span>
											<!--	<span class="style1">
													<span class="style13">Bandwidth Detect</span><br>
													<span id="com11" class="style21" style="display:block">
													To ensure the iQos is working properly and optimize the networking performance,
													please click on the "Bandwidth Detect" button to test the real speed of your internet
													connections. The result might be vary from regions to regions.Note: If the test failed or
													the result is far from the real connection speed provided by your ISP, please perform the
													test by using the following bandwidth testing sites and enter the connection speed manually.
													<br><br><a href="http://www.speedtest.net/" target="_blank">http://www.speedtest.net/</a>
													<br><a href="http://www.bandwidthplace.com/" target="_blank">http://www.bandwidthplace.com/</a>
													<br><a href="http://www.bandwidth-test.net/" target="_blank">http://www.bandwidth-test.net/</a>
													<br>*All above testing service are registered of their respective owners.
													</span>
												</span> -->
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
	var internet_sta = "<% getInfo("sta-current"); %>"
	top.sw_current_sta(internet_sta);
	top.cloud_current_sta(cloud_sta);
	function browser() {
        var Sys = {};
        var ua = navigator.userAgent.toLowerCase();
        var s;
        (s = ua.match(/msie ([\d.]+)/)) ? Sys.ie = s[1] :
        (s = ua.match(/firefox\/([\d.]+)/)) ? Sys.firefox = s[1] :
        (s = ua.match(/chrome\/([\d.]+)/)) ? Sys.chrome = s[1] :
        (s = ua.match(/opera.([\d.]+)/)) ? Sys.opera = s[1] :
        (s = ua.match(/version\/([\d.]+).*safari/)) ? Sys.safari = s[1] : 0;
        return Sys;
    }
	clickapply()
	mouseclick_item(document.getElementById("EZQOS_TD_1"));
	mouseclick_item(document.getElementById("EZQOS_TD_1"));
	mouseclick_item(document.getElementById("EZQOS_TD_1"));
	mouseclick_item(document.getElementById("EZQOS_TD_1"));
	mouseclick_item(document.getElementById("EZQOS_TD_1"));
</script>
