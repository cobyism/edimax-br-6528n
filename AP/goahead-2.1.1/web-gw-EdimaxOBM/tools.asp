<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<link rel="stylesheet" href="edimax.css">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="file/statustool-n.var"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<script type="text/javascript" src="file/miscellaneous-n.var"></script>
<script type="text/javascript" src="file/ezview-n.var"></script>
<script type="text/javascript" src="FUNCTION_SCRIPT"></script>
<script>
	var cloud_sta = "<% getInfo("cloud-sta"); %>"
	var comment = new Array( new Array("com00","com01"),
							 new Array("com10","com11"),
							 new Array("com20","com21"))
	function c_fun(num)
	{
		document.getElementById('CT').style.display = "none"
		document.getElementById('FU').style.display = "none"
		document.getElementById('RE').style.display = "none"
		document.getElementById('V6').style.display = "none"
		if (num == 0)
		{
			document.getElementsByName("sys")[0].checked = true
			document.getElementById('CT').style.display = "block"
		}
		else if (num == 1)
		{
			document.getElementsByName("sys")[1].checked = true
			document.getElementById('FU').style.display = "block"
		}
		else if (num == 2)
		{
			document.getElementsByName("sys")[2].checked = true
			document.getElementById('RE').style.display = "block"
		}
		else
		{
			document.getElementsByName("sys")[3].checked = true
			document.getElementById('V6').style.display = "block"
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
	function resetClick()
	{
	   if ( !confirm(showText(tlconAlert)) ) {
		return false;
	  }
	  else
		return true;
	}
	function checktarget()
	{
		var test;
		if( document.upload.target.selectedIndex == 0 )
			test = "upgrade.asp";
		else
			test = "upgradet.asp";
		document.location.replace(test);
	}
	function evaltF()
	{
		if ( messageCheck() )
			return true;
		return false;
	}
	function messageCheck()
	{
		var hid = document.upload;
		if(hid.binary.value.length == 0) {
			alert("Please enter a file.");
			return false;
		}else{
			if (confirm(showText(tlupgradeConfirm)))
			{
				hid.submit();
				alert(showText(tlupgradeAlert));
			}
		}
	}
	function logoutF()
	{
		document.upload.logout.value=1;
		document.upload.submit();
		return true;
	}
	function doConfirm() {
		if (confirm(showText(tlresetConfirm))) {
			alert(showText(tlresetAlert));
			return true;
		}
		document.RebootForm.reset_flag.value=1;
		return false;
	}
</script>
<title></title>
</head>
<body onLoad="top.startclock()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(5,0)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF">
						<!-- Configuration Tools -->
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666">
									<input onclick="c_fun(0);" type="radio" name="sys" />
									<span class="style13"style="cursor:pointer;" onclick="c_fun(0);">&nbsp;<script>dw(cfgtools)</script></span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center" class="style14">
									<div id="CT" style="display:none;">
										<br>
										<table width="600" border="0" cellpadding="2" cellspacing="2" align="center">
											<form action=/goform/formSaveConfig method=POST name="saveConfig">
											<tr>
											<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(backup)</script> :&nbsp;&nbsp;</td>
											<td width="400" height="20" valign="middle" align="left" class="style1">
											<script>document.write('<input type="submit" value="'+showText(save)+'..." name="save" class="btnsize">')</script>
											</tr>
											</form>
											<form method="post" action="goform/formSaveConfig" enctype="multipart/form-data" name="saveConfig">
											<tr>
											<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(restore)</script> :&nbsp;&nbsp;</td>
											<td width="400" height="20" valign="middle" align="left" class="style1" nowrap><input type="file" name="binary" size=24>
											<script>document.write('<input type="submit" value="'+showText(upload)+'" name="load" class="btnsize">')</script></td>
											</tr>
											<tr>
											<td width="200" height="20" valign="middle" align="right" class="style14"><script>dw(rstFactory)</script> :&nbsp;&nbsp;</td>
											<td width="400" height="20" valign="middle" align="left" class="style1"><script>document.write('<input type="submit" value="'+showText(reset)+'" name="reset" onclick="return resetClick()" class="btnsize">')</script></td>
											<input type="hidden" value="/tools.asp" name="submit-url">
											</form>
											</tr>
										</table>
									</div>
								</td>
							</tr>
						</table>
						<!-- Firmware Upgrade -->
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666">
									<input onclick="c_fun(1)" type="radio" name="sys" />
									<span class="style13"style="cursor:pointer;" onclick="c_fun(1);">&nbsp;<script>dw(firewareUpgrade)</script></span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center" class="style14">
									<div id="FU" style="display:none;">
										<br>
										<form method="post" action="goform/formUpload" enctype="multipart/form-data" name="upload">
											<table border="0" cellspacing="0" cellpadding="0"><tr><td>
											<p align="center"><input type="file" size="31" maxlength="31" name="binary" onkeypress="return false"></p>
											</td></tr></table>
											<p align="right">
												<script>buffer='<input type=submit value="'+showText(apply1)+'" onclick="return evaltF();" style ="width:100px">';document.write(buffer);</script>
												<script>buffer='<input type=button value="'+showText(cancel1)+'" style ="width:100px" onClick="document.upload.reset;">';document.write(buffer);</script>
												<input type="hidden" value="/tools.asp" name="submit-url">
											</p>
										</form>
									</div>
								</td>
							</tr>
						</table>
						<!-- Restart -->
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666">
									<input onclick="c_fun(2)" type="radio" name="sys" />
									<span class="style13"style="cursor:pointer;" onclick="c_fun(2);">&nbsp;<script>dw(restart)</script></span>
								</td>
							</tr>
						</table>
						<table width="600" border="0" cellspacing="1" cellpadding="0" class="style14">
							<tr>
								<td valign="top" align="center">
									<div id="RE" style="display:none;">
										<br />
										<p align="left"><script>dw(firmReset)</script></p>
										<form action=/goform/formReboot method=POST name="RebootForm" onsubmit="return doConfirm();">
											<p align="right">
												<script>buffer='<input type=submit value="'+showText(apply1)+'" style ="width:100px">';document.write(buffer);</script>
												<input type="hidden" name="reset_flag" value="0">
												<input type=hidden value="/tools.asp" name="submit-url">
											</p>
										</form>
									</div>
								</td>
							</tr>
						</table>
						<!-- IPv6 -->
						<br>
						<script>
							if( typeof _IPV6_SUPPORT_ != 'undefined' )
								document.write('<table width="700" border="0" cellspacing="1" cellpadding="0" id="ipv6_banner" style="display:block;">');
							else
								document.write('<table width="700" border="0" cellspacing="1" cellpadding="0" id="ipv6_banner" style="display:none;">');
						</script>
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666">
									<input onclick="c_fun(3)" type="radio" name="sys" />
									<span class="style13"style="cursor:pointer;" onclick="c_fun(3);">&nbsp;IPv6</span>
								</td>
							</tr>
						</table>
						<table width="600" border="0" cellspacing="1" cellpadding="0" class="style14">
							<tr>
								<td valign="top" align="center">
									<div id="V6" style="display:none;">
										<br />
										<form action="/goform/formPing6" method="post" name="formPing6">
										<b>Ping6:</b>&nbsp;&nbsp;<input type="text" name="target" />
										<input type="submit" value="Ping6" />
										<input type="hidden" name="submit-url" value="/tools.asp" />
										</form>
										<pre><% getInfo("ping6result"); %></pre>
										<form action="/goform/formIP6Neigh" method="post" name="formIP6Neigh">
										<b>IPv6 Neighbour:</b>&nbsp;&nbsp;<input type="submit" value="Flush" />
										<input type="hidden" name="submit-url" value="/tools.asp" />
										</form>
										<pre><% getInfo("ip6neigh"); %></pre>
										<form action="/goform/formIP6Route" method="post" name="formIP6Route">
										<b>IPv6 Route:</b>&nbsp;&nbsp;<input type="submit" value="Flush" />
										<input type="hidden" name="submit-url" value="/tools.asp" />
										</form>
										<pre><% getInfo("ip6route"); %></pre>
										<pre><% getInfo("ip6routecache"); %></pre>
									</div>
								</td>
							</tr>
						</table>
						<br>
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
													<span class="style13"><script>dw(cfgtools)</script></span><br />
													<span id="com00" class="style21" style="display:block"><script>dw(cfgtoolsDocshort)</script><a class="style71" onclick="com_sw(0,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com01" class="style21" style="display:none"><script>dw(cfgtoolsDoc)</script><a class="style71" onclick="com_sw(0,2);" style="cursor:pointer;" > .....close</a></span>
												</span>
												<br>
												<span class="style1">
													<span class="style13"><script>dw(firewareUpgrade)</script></span><br />
													<span id="com10" class="style21" style="display:block"><script>dw(firmDocshort)</script><a class="style71" onclick="com_sw(1,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com11" class="style21" style="display:none"><script>dw(firmDoc)</script><a class="style71" onclick="com_sw(1,2);" style="cursor:pointer;" > .....close</a></span>
												</span>
												<br>
												<span class="style1">
													<span class="style13"><script>dw(restart)</script></span><br />
													<span id="com20" class="style21" style="display:block"><script>dw(firmResetshort)</script><a class="style71" onclick="com_sw(2,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com21" class="style21" style="display:none"><script>dw(firmReset)</script><a class="style71" onclick="com_sw(2,2);" style="cursor:pointer;" > .....close</a></span>
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
	document.getElementById('CT').style.display = "none"
	document.getElementById('FU').style.display = "none"
	document.getElementById('RE').style.display = "none"
	if (top.tool == 1)
	{
		document.getElementsByName("sys")[1].checked = true
		document.getElementById('FU').style.display = "block"
		top.tool = 0
	}
	else
	{
		document.getElementsByName("sys")[0].checked = true
		document.getElementById('CT').style.display = "block"
	}
	var internet_sta = "<% getInfo("sta-current"); %>"
	top.sw_current_sta(internet_sta);
	top.cloud_current_sta(cloud_sta);
</script>
