<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<link rel="stylesheet" href="edimax.css">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="file/statustool-n.var"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<script type="text/javascript" src="file/multilanguage.var"></script>
<script type="text/javascript" src="file/ezview-n.var"></script>
<script type="text/javascript" src="FUNCTION_SCRIPT"></script>
<% language=javascript %>
<script>
var cloud_sta = "<% getInfo("cloud-sta"); %>"
<%  write("wizardEnabled = "+getIndex("wizardEnabled"));%>
/* function for main */
	var comment = new Array( new Array("com00","com01"),
							 new Array("com10","com11"),
							 new Array("com20","com21"))
	function c_fun(num)
	{
		document.getElementById('TZS').style.display = "none"
		document.getElementById('PWS').style.display = "none"
		document.getElementById('RMS').style.display = "none"
		if (num == 0)
		{
			document.getElementById('TZS').style.display = "block"
			document.getElementsByName("sys")[0].checked = true
		}
		else if (num == 1)
		{
			document.getElementById('PWS').style.display = "block"
			document.getElementsByName("sys")[1].checked = true
		}
		else
		{
			document.getElementById('RMS').style.display = "block"
			document.getElementsByName("sys")[2].checked = true
		}
	}
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
/* function for systimezone */
	var Month = new Array(showText(jan),showText(feb),showText(march),showText(april),showText(may),showText(june),showText(july),showText(august),showText(sep),showText(oct),showText(nov),showText(dec));
	var monthNum = new Array(31,28,31,30,31,30,31,31,30,31,30,31);
	var ntp_zone_array=new Array(
	"(GMT-12:00)Eniwetok, Kwajalein",
	"(GMT-11:00)Midway Island, Samoa",
	"(GMT-10:00)Hawaii",
	"(GMT-09:00)Alaska",
	"(GMT-08:00)Pacific Time (US & Canada); Tijuana",
	"(GMT-07:00)Arizona",
	"(GMT-07:00)Mountain Time (US & Canada)",
	"(GMT-06:00)Central Time (US & Canada)",
	"(GMT-06:00)Mexico City, Tegucigalpa",
	"(GMT-06:00)Saskatchewan",
	"(GMT-05:00)Bogota, Lima, Quito",
	"(GMT-05:00)Eastern Time (US & Canada)",
	"(GMT-05:00)Indiana (East)",
	"(GMT-04:00)Atlantic Time (Canada)",
	"(GMT-04:00)Caracas, La Paz",
	"(GMT-04:00)Santiago",
	"(GMT-03:30)Newfoundland",
	"(GMT-03:00)Brasilia",
	"(GMT-03:00)Buenos Aires, Georgetown",
	"(GMT-02:00)Mid-Atlantic",
	"(GMT-01:00)Azores, Cape Verde Is.",
	"(GMT)Casablanca, Monrovia",
	"(GMT)Greenwich Mean Time: Dublin, Edinburgh, Lisbon, London",
	"(GMT+01:00)Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna",
	"(GMT+01:00)Belgrade, Bratislava, Budapest, Ljubljana, Prague",
	"(GMT+01:00)Barcelona, Madrid",
	"(GMT+01:00)Brussels, Copenhagen, Madrid, Paris, Vilnius",
	"(GMT+01:00)Paris",
	"(GMT+01:00)Sarajevo, Skopje, Sofija, Warsaw, Zagreb",
	"(GMT+02:00)Athens, Istanbul, Minsk",
	"(GMT+02:00)Bucharest",
	"(GMT+02:00)Cairo",
	"(GMT+02:00)Harare, Pretoria",
	"(GMT+02:00)Helsinki, Riga, Tallinn",
	"(GMT+02:00)Jerusalem",
	"(GMT+03:00)Baghdad, Kuwait, Riyadh",
	"(GMT+03:00)Moscow, St. Petersburg, Volgograd",
	"(GMT+03:00)Mairobi",
	"(GMT+03:30)Tehran",
	"(GMT+04:00)Abu Dhabi, Muscat",
	"(GMT+04:00)Baku, Tbilisi",
	"(GMT+04:30)Kabul",
	"(GMT+05:00)Ekaterinburg",
	"(GMT+05:00)Islamabad, Karachi, Tashkent",
	"(GMT+05:30)Bombay, Calcutta, Madras, New Delhi",
	"(GMT+06:00)Astana, Almaty, Dhaka",
	"(GMT+06:00)Colombo",
	"(GMT+07:00)Bangkok, Hanoi, Jakarta",
	"(GMT+08:00)Beijing, Chongqing, Hong Kong, Urumqi",
	"(GMT+08:00)Perth",
	"(GMT+08:00)Singapore",
	"(GMT+08:00)Taipei",
	"(GMT+09:00)Osaka, Sapporo, Tokyo",
	"(GMT+09:00)Seoul",
	"(GMT+09:00)Yakutsk",
	"(GMT+09:30)Adelaide",
	"(GMT+09:30)Darwin",
	"(GMT+10:00)Brisbane",
	"(GMT+10:00)Canberra, Melbourne, Sydney",
	"(GMT+10:00)Guam, Port Moresby",
	"(GMT+10:00)Hobart",
	"(GMT+10:00)Vladivostok",
	"(GMT+11:00)Magadan, Solomon Is., New Caledonia",
	"(GMT+12:00)Auckland, Wllington",
	"(GMT+12:00)Fiji, Kamchatka, Marshall Is.");
	function saveChanges()
	{
		if (document.getElementById('TZS').style.display == "block")
		{
			if ( !ipRule( document.timezone.TimeServerAddr, showText(ipAddr11), "ip", 1))
				return false;
		}
		else if (document.getElementById('PWS').style.display == "block")
		{
			if ( document.password.newpass.value != document.password.confpass.value)
			{
				alert(showText(syspasswdAlert));
				document.password.newpass.focus();
				return false;
			}
			if ( includeSpace(document.password.newpass.value))
			{
				alert(showText(syspasswdAlert2));
				document.password.newpass.focus();
				return false;
			}
			if(document.password.newpass.value == "" )
			{
				alert(showText(syspasswdAlert2));
				document.password.newpass.focus();
				return false;
			}
			var str1 = document.password.newpass.value;
			if (str1.search('"') != -1 || str1.search("'") != -1)
			{
				alert(showText(wandipAlert3));
				document.password.newpass.focus();
				return false;
			}
		}
		else
		{
			if(document.tF.reMangEnable.checked==false)
			return true;
			if ( !ipRule( document.tF.reManHostAddr, showText(IPAddr), "ip", 1))
				return false;
			if (!portRule(document.tF.reManPort, showText(portNum), 0, "", 1, 65535, 1))
				return false;
		}
		return true;
	}
	function TimeEnable()
	{
		if(document.timezone.DayLightEnable.checked==false)
		{
			document.timezone.startMonth.disabled = true;
			document.timezone.startDay.disabled = true;
			document.timezone.endMonth.disabled = true;
			document.timezone.endDay.disabled = true;
		}
		else
		{
			document.timezone.startMonth.disabled = false;
			document.timezone.startDay.disabled = false;
			document.timezone.endMonth.disabled = false;
			document.timezone.endDay.disabled = false;
		}
	}
	function chDayNum(Month,Day) {
		var i, tmp
		for (i = Day.length; i > 0; i--)
			Day.remove(0)
		for (i = 1; i <= monthNum[Month.selectedIndex]; i++) {
			tmp = document.createElement("OPTION")
			tmp.text = i
			tmp.value = i
			try {
				Day.add(tmp,null);
			}
			catch(ex) {
				Day.add(tmp);
			}
		}
	}
	function includeSpace(str)
	{
		for (var i=0; i<str.length; i++)
		{
			if ( str.charAt(i) == ' ' )
			{
				return true;
			}
		}
		return false;
	}
	function Enable()
	{
		if(document.tF.reMangEnable.checked==true) {
			document.tF.reManHostAddr.disabled = false;
			document.tF.reManPort.disabled = false;
		}
		else {
			document.tF.reManHostAddr.disabled = true;
			document.tF.reManPort.disabled = true;
		}
	}
</script>
<title></title>
</head>
<body onLoad="top.startclock()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>
		if(wizardEnabled == 1)
			Write_Header(1,0)
		else
			Write_Header(2,0)
	</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF">
						<!-- systimezone table -->
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
							<script>
								if(wizardEnabled != 1)
								{
									document.write('<td align="left" width="700" height="25px" bgcolor="#666666">')
									document.write('<input onclick="c_fun(0);" type="radio" name="sys" checked>');
									document.write('<span class="style13" style="cursor:pointer;" onclick="c_fun(0);">'+showText(timeZone)+'</span>')
								}
								else
								{
									document.write('<td align="center" width="700" height="25px" bgcolor="#666666">')
									document.write('<span class="style13">'+showText(timeZone)+'</span>')
								}
							</script>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<div id="TZS" style="display:block;">
									<br>
									<form action=/goform/formTimeZoneSetup method=POST name="timezone">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<input type=hidden value=<% getInfo("TimeZoneSel"); %> name="TimeZoneSelget">
									<input type=hidden value=<% getInfo("startMonth"); %> name="startMonthget">
									<input type=hidden value=<% getInfo("startDay"); %> name="startDayget">
									<input type=hidden value=<% getInfo("endMonth"); %> name="endMonthget">
									<input type=hidden value=<% getInfo("endDay"); %> name="endDayget">
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(setTimeZone)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<select name="TimeZoneSel">
											<script language="javascript">
											for(i=0;i<ntp_zone_array.length;i++)
											{
											if(i==document.timezone.TimeZoneSelget.value)
												document.write("<option value=\""+i+"\" selected>"+ntp_zone_array[i]+"</option>");
											else
												document.write("<option value=\""+i+"\">"+ntp_zone_array[i]+"</option>");
											}
											</script>
											</select>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(timeServerAddr)</script> :&nbsp;&nbsp;<br></td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="text" name="TimeServerAddr" size="15" maxlength="15" value=<% getInfo("TimeServerAddr"); %>>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(daylightSave)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
										<input type="checkbox" name="DayLightEnable" value="ON" <% if (getIndex("DayLightEnable")==1) write("checked"); %> onClick="TimeEnable()"><script>dw(enableFunction)</script>
										<select name="startMonth" onChange="chDayNum(this,document.timezone.startDay);">
										<script language="javascript">
										for(i=1;i<13;i++)
										{
											if(i==document.timezone.startMonthget.value)
												document.write("<option value=\""+i+"\" selected>"+Month[(i-1)]+"</option>");
											else
												document.write("<option value=\""+i+"\">"+Month[(i-1)]+"</option>");
										}
										</script>
										</select>
										<select name="startDay">
										<script>
										for(i=1;i<32;i++)
										{
											if(i==document.timezone.startDayget.value)
												document.write("<option value=\""+i+"\" selected>"+i+"</option>");
											else
												document.write("<option value=\""+i+"\">"+i+"</option>");
										}
										</script>
										</select>
										To
										<select class=Wf name="endMonth" onChange="chDayNum(this,document.timezone.endDay);">
										<script language="javascript">
										for(i=1;i<13;i++)
										{
											if(i==document.timezone.endMonthget.value)
												document.write("<option value=\""+i+"\" selected>"+Month[(i-1)]+"</option>");
											else
												document.write("<option value=\""+i+"\">"+Month[(i-1)]+"</option>");
										}
										</script>
										</select>
										<select name="endDay">
										<script>
										for(i=1;i<32;i++)
										{
											if(i==document.timezone.endDayget.value)
												document.write("<option value=\""+i+"\" selected>"+i+"</option>");
											else
												document.write("<option value=\""+i+"\">"+i+"</option>");
										}
										</script>
										</select>
										</td>
									</tr>
									<tr>
										<td width="600" height="20" valign="middle" align="right" colspan="2">
										<script>
											if(wizardEnabled == 1)
											{
												document.write('<input type=button value="'+showText(back1)+'" style ="width:100px" onClick="window.history.back();">&nbsp;&nbsp;');
												document.write('<input type=submit value="'+showText(next1)+'" style ="width:100px" onClick="return saveChanges();">');
												document.write('<input type=hidden value="ON" name="settimezoneEnable">');
												document.write('<input type=hidden value="/wan.asp" name="submit-url" id="submitUrl">');
											}
											else
											{
												document.write('<input type=submit value="'+showText(apply1)+'" style ="width:100px" onClick="return saveChanges();">');
												document.write('<input type=hidden value="/system.asp" name="submit-url" id="submitUrl">');
											}
										</script>
										</td>
									</tr>
									<script type="text/javascript">
										TimeEnable()
										chDayNum(document.timezone.startMonth,document.timezone.startDay)
										chDayNum(document.timezone.endMonth,document.timezone.endDay)
										document.timezone.startMonth.selectedIndex = document.timezone.startMonthget.value - 1
										document.timezone.startDay.selectedIndex = document.timezone.startDayget.value - 1
										document.timezone.endMonth.selectedIndex = document.timezone.endMonthget.value - 1
										document.timezone.endDay.selectedIndex = document.timezone.endDayget.value - 1
									</script>
									</table>
									</form>
									</div>
								</td>
							</tr>
						</table>
						<script>
							if(wizardEnabled == 1)
								document.write('<div  id="wiz" style="display:none;">');
							else
								document.write('<div  id="wiz" style="display:block;">');
						</script>
						<!-- syspass table -->
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666">
									<input onclick="c_fun(1)" type="radio" name="sys" />
									<span class="style13" style="cursor:pointer;" onclick="c_fun(1);">&nbsp;<script>dw(passSet)</script></span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<div id="PWS" style="display:none;"><br>
									<form action=/goform/formPasswordSetup method=POST name="password">
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(currentPass)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="password" name="oldpass" size="20" maxlength="30" value="">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(newPass)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="password" name="newpass" size="20" maxlength="30">
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(confirmPass)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="password" name="confpass" size="20" maxlength="30">
										</td>
									</tr>
									<tr>
										<td width="600" height="20" valign="middle" align="right" colspan="2">
										<script>
											document.write('<input type=submit value="'+showText(apply1)+'" style ="width:100px" onClick="return saveChanges()">');
											document.write('<input type=hidden value="/system.asp" name="submit-url" id="submitUrl">');
										</script>
										</td>
									</tr>
									</table>
									</form>
									</div>
								</td>
							</tr>
						</table>
						<!-- sysremote table -->
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666">
									<input onclick="c_fun(2)" type="radio" name="sys" />
									<span class="style13" style="cursor:pointer;" onclick="c_fun(2);">&nbsp;<script>dw(RemoteM)</script></span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<div id="RMS" style="display:none;"><br>
									<form name="tF" method="post" action=/goform/formReManagementSetup>
									<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(hostAddr)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="reManHostAddr" size="15" maxlength="15" value=<% getInfo("reManHostAddr"); %>>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(port)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="text" name="reManPort" size="5" maxlength="5" value=<% getInfo("reManPort"); %>>
										</td>
									</tr>
									<tr>
										<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(enabled)</script> :&nbsp;&nbsp;</td>
										<td width="400" height="20" valign="middle" align="left" class="style1">
											<input type="checkbox" name="reMangEnable" value="ON" <% if (getIndex("reMangEnable")==1) write("checked"); %> onClick="Enable()">
										</td>
									</tr>
									<tr>
										<td width="600" height="20" valign="middle" align="right" colspan="2">
										<script>
											document.write('<input type=submit value="'+showText(apply1)+'" style ="width:100px" onClick="return saveChanges();">');
											document.write('<input type=hidden value="/system.asp" name="submit-url" id="submitUrl">');
										</script>
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
										<span class="style13"><script>dw(timeZone)</script></span><br>
										<span class="style21"><script>dw(timeZonePageContent)</script></span><br>
									</span>
									<script>
										if(wizardEnabled == 1)
											document.write('<div id="wizcom" style="display:none;">');
										else
											document.write('<div id="wizcom" style="display:block;">');
									</script>
									<br>
									<span class="style1">
										<span class="style13"><script>dw(passSet)</script></span><br>
										<span id="com10" class="style21" style="display:block"><script>dw(passSettingContentshort)</script><a class="style71" onclick="com_sw(1,1);" style="cursor:pointer;" > .....more</a></span>
										<span id="com11" class="style21" style="display:none"><script>dw(passSettingContent)</script><a class="style71" onclick="com_sw(1,2);" style="cursor:pointer;" > .....close</a></span>
									</span>
									<br>
									<span class="style1">
										<span class="style13"><script>dw(RemoteM)</script></span><br>
										<span id="com20" class="style21" style="display:block"><script>dw(sysrmContentshort)</script><a class="style71" onclick="com_sw(2,1);" style="cursor:pointer;" > .....more</a></span>
										<span id="com21" class="style21" style="display:none"><script>dw(sysrmContent)</script><a class="style71" onclick="com_sw(2,2);" style="cursor:pointer;" > .....close</a></span>
									</span>
									</div>
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
	Enable()
	var internet_sta = "<% getInfo("sta-current"); %>"
	top.sw_current_sta(internet_sta);
	top.cloud_current_sta(cloud_sta);
</script>
