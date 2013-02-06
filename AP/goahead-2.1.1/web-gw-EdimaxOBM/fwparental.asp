<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html;charset=UTF-8" />
<link rel="stylesheet" href="edimax.css">
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="file/multilanguage.var"></script>
<script language ='javascript' src ="file/fwwl-n.var"></script>
<script type="text/javascript" src="file/statustool-n.var"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<script type="text/javascript" src="file/ezview-n.var"></script>
<script type="text/javascript">
var cloud_sta = "<% getInfo("cloud-sta"); %>"
var par_pc_mac = "<% getInfo("cloneMac"); %>"
var par_pc_def_mac = "<% getInfo("ParConMac"); %>"
function ParentalControlEnable() {
	ParentalControlSW(document.formParConEnable)
	document.formParConEnable.submit()
}
function ParentalControlSchEnable()
{
	ParentalControlSW(document.formParConAdd)
	document.formParConAdd.submit()
}
function ParentalControlSW(a) {
	if(document.formParConAdd.schenable.checked == true)
		a.schenable.value = "on"
	if(document.formParConEnable.enable.checked == true)
		a.enable.value = "on"
}
function copymac() {
	document.formParConEnable.mac.value = document.getElementById("yourmac").innerHTML
}
function checkparentmac() {
	var x = document.formParConEnable.mac
	if (!x.value.match(/^[A-Fa-f0-9]{12}$/) || x.value == "" || x.value == "000000000000" || x.value == "ffffffffffff" || x.value == "FFFFFFFFFFFF") {
		alert("Invalid MAC Address")
		x.value = x.defaultValue
		if (x.value == "" || x.value == "000000000000" || x.value == "ffffffffffff" || x.value == "FFFFFFFFFFFF")
			copymac()
		x.focus()
		return false
	}
	else
		return true
}
function checkchildmac() {
	var x = document.formParConAdd.childmac
	if (!x.value.match(/^[A-Fa-f0-9]{12}$/) || x.value == "" || x.value == "000000000000" || x.value == "ffffffffffff" || x.value == "FFFFFFFFFFFF") {
		alert("Invalid MAC Address")
		x.value = x.defaultValue
		x.focus()
		return false
	}
	else if(x.value == document.formParConEnable.mac.value)
	{
		alert("This mac has been parental pc's mac ")
		x.value = x.defaultValue
		x.focus()
		return false
	}
	else
		return true
}
function checktime() {
	var x = document.formParConAdd.timestarthour
	var y = document.formParConAdd.timestophour
	if (x.selectedIndex == y.selectedIndex) {
		x = document.formParConAdd.timestartminute
		y = document.formParConAdd.timestopminute
		if (x.selectedIndex == y.selectedIndex) {
			alert("Invalid Time")
			return false
		}
	}
	return true
}
function gethours() {
	var today = new Date()
	var hours = today.getHours()
	document.formParConAdd.timestarthour.selectedIndex = hours
	if (hours + 1 > 23)
		document.formParConAdd.timestophour.selectedIndex = 0
	else
		document.formParConAdd.timestophour.selectedIndex = hours + 1
}
function fresh(select) {
	if (select == "refresh")
		document.name_fresh.submit()
}
var nameList = new Array(<% getInfo("netRefreshScan"); %> new Array("0","0","0"))
function addComputerName() {
	document.formParConAdd.childmac.value = document.formParConAdd.comList.value
}
function loadNetList() {
	for (i = 0; i < nameList.length - 1; i++) {
		document.formParConAdd.comList.options[i+1] = new Option (nameList[i][1], nameList[i][2])
	}
	document.formParConAdd.comList.options[i+1] = new Option ("--------- Refresh --------", "refresh");
}
function saveChanges() {
	if (!checkchildmac())
		return false
	var weekdays = 0
	if (document.formParConAdd.weekdays0.checked == false && document.formParConAdd.weekdays1.checked == false && document.formParConAdd.weekdays2.checked == false	&& document.formParConAdd.weekdays3.checked == false && document.formParConAdd.weekdays4.checked == false && document.formParConAdd.weekdays5.checked == false && document.formParConAdd.weekdays6.checked == false)
		weekdays = 127
	else {
		if (document.formParConAdd.weekdays0.checked == true)
			weekdays |= 0x001
		if (document.formParConAdd.weekdays1.checked == true)
			weekdays |= 0x002
		if (document.formParConAdd.weekdays2.checked == true)
			weekdays |= 0x004
		if (document.formParConAdd.weekdays3.checked == true)
			weekdays |= 0x008
		if (document.formParConAdd.weekdays4.checked == true)
			weekdays |= 0x010
		if (document.formParConAdd.weekdays5.checked == true)
			weekdays |= 0x020
		if (document.formParConAdd.weekdays6.checked == true)
			weekdays |= 0x040
	}
	document.formParConAdd.weekdays.value = weekdays
	if (!checktime())
		return false
	if (document.formParConEnable.enable.checked == true)
		document.formParConAdd.enable.value = "on"
	document.formParConAdd.mac.value = document.formParConEnable.mac.value
	return true
}
function goToApply() {
	if (document.formParConEnable.enable.checked == true)
		document.formParConApply.enable.value = "on"
	document.formParConApply.mac.value = document.formParConEnable.mac.value
	document.formParConApply.submit()
}
</script>
</head>
<body class="mainbg" onload="loadNetList()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(2,7)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF"><br>
						<form action="/goform/formParCon" method="POST" name="formParConEnable">
						<table width="700" cellpadding="2" cellspacing="2" align="center">
							<tr><td colspan="2" align="left" height="25px" class="style14">
								<p><input type="checkbox" name="enable" onclick="ParentalControlEnable()">Enable Parental Control</p>
							</td></tr>
							<tr align="center" bgcolor="#666666" class="style13">
								<td width="50%">MAC Address of Parental PC</td>
								<td width="50%">MAC Address of Your PC</td>
							</tr>
							<tr border="1" align="center" class="style14">
								<script>
									document.write('<td><input type="text" name="mac" size="12" maxlength="12" value="'+par_pc_def_mac+'" onchange="checkparentmac()"></td>')
									document.write('<td><input type="button" value="<<" onclick="copymac()">&nbsp;&nbsp;')
									if (!par_pc_mac.match(/^[A-Fa-f0-9]{12}$/) || par_pc_mac == "" || par_pc_mac == "000000000000" || par_pc_mac == "ffffffffffff" || par_pc_mac == "FFFFFFFFFFFF")
										document.write('<a id="yourmac">'+par_pc_def_mac+'</a></td>')
									else
										document.write('<a id="yourmac">'+par_pc_mac+'</a></td>')
								</script>
							</tr>
							<input type="hidden" name="schenable">
							<input type="hidden" value="/fwparental.asp" name="submit-url">
						</table>
						</form>
						<br>
						<form action="/goform/formParCon" method="POST" name="formParConAdd">
						<table width="700" cellpadding="2" cellspacing="2" align="center" class="style14">
							<tr>
								<td colspan="2" align="left" height="25px" class="style14">
									<input type="checkbox" name="schenable" onclick="ParentalControlSchEnable()">Enable Parental Control Scheduling Rule
								</td>
							</tr>
						</table>
						<table width="700" border="1" cellpadding="2" cellspacing="2" align="center" class="style14">
							<tr><td colspan="2" align="center" bgcolor="#666666" class="style13">Parental Control Rule</td></tr>
							<tr align="center">
								<td width="50%">MAC:<input type="text" name="childmac" size="12" maxlength="12"></td>
								<td width="50%">
									<input type="button" value="<<" onclick="addComputerName()">&nbsp;&nbsp;
									<select name="comList" onchange="fresh(this.value)">
										<option value="">---- Select ----</option>
										<option value="refresh">---- Refresh ----</option>
									</select>
								</td>
							</tr>
							<tr align="center">
								<td colspan="2">
								<input type="hidden" name="weekdays">
								Weekdays: <a class="textcolor">
								<input type="checkbox" name="weekdays0" value="1" />Sun
								<input type="checkbox" name="weekdays1" value="1" />Mon
								<input type="checkbox" name="weekdays2" value="1" />Tue
								<input type="checkbox" name="weekdays3" value="1" />Wed
								<input type="checkbox" name="weekdays4" value="1" />Thu
								<input type="checkbox" name="weekdays5" value="1" />Fri
								<input type="checkbox" name="weekdays6" value="1" />Sat
								</a>
								</td>
							</tr>
							<tr class="table2" align="center">
								<td>
								Time Start: <a class="textcolor">Hour</a>
								<select name="timestarthour" class="select" onchange="checktime()">
									<option value="0">00</option>
									<option value="1">01</option>
									<option value="2">02</option>
									<option value="3">03</option>
									<option value="4">04</option>
									<option value="5">05</option>
									<option value="6">06</option>
									<option value="7">07</option>
									<option value="8">08</option>
									<option value="9">09</option>
									<option value="10">10</option>
									<option value="11">11</option>
									<option value="12">12</option>
									<option value="13">13</option>
									<option value="14">14</option>
									<option value="15">15</option>
									<option value="16">16</option>
									<option value="17">17</option>
									<option value="18">18</option>
									<option value="19">19</option>
									<option value="20">20</option>
									<option value="21">21</option>
									<option value="22">22</option>
									<option value="23">23</option>
								</select>
								<a class="textcolor">Minute</a>
								<select name="timestartminute" class="select" onchange="checktime()">
									<option value="0">00</option>
									<option value="5">05</option>
									<option value="10">10</option>
									<option value="15">15</option>
									<option value="20">20</option>
									<option value="25">25</option>
									<option value="30">30</option>
									<option value="35">35</option>
									<option value="40">40</option>
									<option value="45">45</option>
									<option value="50">50</option>
									<option value="55">55</option>
								</select>
								</td>
								<td>
								Time Stop: <a class="textcolor">Hour</a>
								<select name="timestophour" class="select" onchange="checktime()">
									<option value="0">00</option>
									<option value="1">01</option>
									<option value="2">02</option>
									<option value="3">03</option>
									<option value="4">04</option>
									<option value="5">05</option>
									<option value="6">06</option>
									<option value="7">07</option>
									<option value="8">08</option>
									<option value="9">09</option>
									<option value="10">10</option>
									<option value="11">11</option>
									<option value="12">12</option>
									<option value="13">13</option>
									<option value="14">14</option>
									<option value="15">15</option>
									<option value="16">16</option>
									<option value="17">17</option>
									<option value="18">18</option>
									<option value="19">19</option>
									<option value="20">20</option>
									<option value="21">21</option>
									<option value="22">22</option>
									<option value="23">23</option>
								</select>
								<a class="textcolor">Minute</a>
								<select name="timestopminute" class="select" onchange="checktime()">
									<option value="0">00</option>
									<option value="5">05</option>
									<option value="10">10</option>
									<option value="15">15</option>
									<option value="20">20</option>
									<option value="25">25</option>
									<option value="30">30</option>
									<option value="35">35</option>
									<option value="40">40</option>
									<option value="45">45</option>
									<option value="50">50</option>
									<option value="55">55</option>
								</select>
								</td>
							</tr>
						</table>
						<script>gethours()</script>
						<table border="0" width="700" cellpadding="2" cellspacing="2" >
							<tr><td align="right">
								<script>document.write('<input type="submit" value="'+showText(Add)+'" name="addSchdule" onclick="return saveChanges()" class="button">')</script>&nbsp;&nbsp;
								<script>document.write('<input type="reset" value="'+showText(Reset)+'" name="reset" class="button">')</script>
								<input type="hidden" name="enable">
								<input type="hidden" name="mac">
								<input type="hidden" value="/fwparental.asp" name="submit-url">
							</td></tr>
						</table>
						</form>
						<script>
							<%  write("parentalcontrol = "+getIndex("ParConEnabled"));%>
							if (parentalcontrol == 0)
							{
								document.formParConEnable.enable.checked = false;
								document.formParConAdd.schenable.checked = false;
							}
							else if (parentalcontrol == 1)
							{
								document.formParConEnable.enable.checked = true;
								document.formParConAdd.schenable.checked = false;
							}
							else if (parentalcontrol == 2)
							{
								document.formParConEnable.enable.checked = false;
								document.formParConAdd.schenable.checked = true;
							}
							else
							{
								document.formParConEnable.enable.checked = true;
								document.formParConAdd.schenable.checked = true;
							}
							if ( document.formParConEnable.mac.value == "" || document.formParConEnable.mac.value == "000000000000" ||
								 document.formParConEnable.mac.value == "ffffffffffff" || document.formParConEnable.mac.value == "FFFFFFFFFFFF" )
								copymac()
						</script>
						<form action="/goform/formParCon" method="POST" name="formParConList">
						<table border="1" width="700" cellpadding="2" cellspacing="2"  bgcolor="#FFFFFF" class="style14">
							<tr>
								<td height="20" bgcolor="#666666" class="style13" align="center">MAC</td>
								<!--<td height="20" bgcolor="#666666" class="style13" align="center">URL</td>-->
								<td height="20" bgcolor="#666666" class="style13" align="center">Weekdays</td>
								<td height="20" bgcolor="#666666" class="style13" align="center">Time Start</td>
								<td height="20" bgcolor="#666666" class="style13" align="center">Time Stop</td>
								<td height="20" bgcolor="#666666" class="style13" align="center">Select</td>
							</tr>
							<% ParConList(); %>
						</table>
						<table border="0" width="700" cellpadding="2" cellspacing="2" >
						<tr><td align="right">
							<script>document.write('<input type="submit" value="'+showText(Delete)+'" name="deleteSelSch" onClick="ParentalControlSW(document.formParConList)" class="button"> ')</script>&nbsp;&nbsp;
							<script>document.write('<input type="submit" value="'+showText(DeleteAll)+'" name="deleteAllSch" onClick="ParentalControlSW(document.formParConList)" class="button" >')</script>&nbsp;&nbsp;
							<script>document.write('<input type="reset" value="'+showText(Reset)+'" name="reset" class="button">')</script>
							<input type="hidden" value="/fwparental.asp" name="submit-url">
							<input type="hidden" name="schenable">
							<input type="hidden" name="enable">
						</td></tr>
						</table>
						</form>
						<form action="/goform/formParCon" method="POST" name="formParConApply">
							<table border="0" width="700">
								<tr><td align="right">
									<script>document.write('<input type=submit value="'+showText(APPLY)+'" onClick="ParentalControlSW(document.formParConApply);goToApply()" class="button">')</script>
									<input type="hidden" name="enable">
									<input type="hidden" name="schenable">
									<input type="hidden" name="mac">
									<input type="hidden" name="submit-url" value="/fwparental.asp">
									<input type="hidden" name="isApply" value="1">
								</td></tr>
							</table>
						</form>
						<form action="/goform/formrefresh" method="POST" name="name_fresh">
							<input type="hidden" name="submit-url" value="/fwparental.asp">
						</form>
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
													<span class="style13"><script>dw(Parental)</script></span><br>
													<span id="com00" class="style21" style="display:block">Non-Parental PCs not listed will not be able to access the Internet.</span>
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
var internet_sta = "<% getInfo("sta-current"); %>"
top.sw_current_sta(internet_sta);
top.cloud_current_sta(cloud_sta);
</script>
