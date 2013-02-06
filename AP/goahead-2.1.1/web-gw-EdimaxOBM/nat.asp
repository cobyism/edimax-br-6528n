<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<link rel="stylesheet" href="edimax.css">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<script type="text/javascript" src="file/multilanguage.var"></script>
<script language ='javascript' src ="file/fwwl-n.var"></script>
<script type="text/javascript" src="file/statustool-n.var"></script>
<script type="text/javascript" src="file/netsys-n.var"></script>
<script type="text/javascript" src="/file/qosnat-n.var"></script>
<script type="text/javascript" src="file/ezview-n.var"></script>
<script type="text/javascript" src="file/javascript.js"></script>
<script type="text/javascript" src="FUNCTION_SCRIPT"></script>
<script>
var cloud_sta = "<% getInfo("cloud-sta"); %>"
/* function for main */
	var comm = new Array( new Array("com00","com01"),
							 new Array("com10","com11"),
							 new Array("com20","com21"),
							 new Array("com30","com31"),
							 new Array("com40","com41"))
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
	function c_fun(num)
	{
		document.getElementById('VS').style.display = "none"
		document.getElementById('SA').style.display = "none"
		document.getElementById('UP').style.display = "none"
		document.getElementById('AL').style.display = "none"
		document.getElementById('SR').style.display = "none"
		if (num == 0)
		{
			document.getElementById("VSFrame").src = "/natvser.asp"
			document.getElementsByName("sys")[0].checked = true
			document.getElementById('VS').style.display = "block"
		}
		else if (num == 1)
		{
			document.getElementById("SAFrame").src = "/natsapp.asp"
			document.getElementsByName("sys")[1].checked = true
			document.getElementById('SA').style.display = "block"
		}
		else if (num == 2)
		{
			document.getElementById("UPFrame").src = "/natupnp.asp"
			document.getElementsByName("sys")[2].checked = true
			document.getElementById('UP').style.display = "block"
		}
		else if (num == 3)
		{
			document.getElementById("ALFrame").src = "/natalg.asp"
			document.getElementsByName("sys")[3].checked = true
			document.getElementById('AL').style.display = "block"
		}
		else
		{
			document.getElementById("SRFrame").src = "/natsrout.asp"
			document.getElementsByName("sys")[4].checked = true
			document.getElementById('SR').style.display = "block"
		}
	}
</script>
<title></title>
</head>
<body onLoad="top.startclock()">
<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
	<!-- top menu -->
	<script>Write_Header(2,5)</script>
	<!-- middle menu -->
	<tr>
		<td>
			<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<!-- main -->
					<td width="820" align="center" valign="top" bgcolor="#EFEFEF"><br />
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666" class="style13" id="VS_banner" style="display:bloock;">
									<input onclick="c_fun(0);" type="radio" name="sys">
									<span class="style13"style="cursor:pointer;" onclick="c_fun(0);">&nbsp;<script>dw(virtualServer)</script></span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<div id="VS" style="display:none">
										<iframe id="VSFrame" name="frame_1" src="" frameborder="0" width="700" scrolling="no"></iframe>
									</div>
								</td>
							</tr>
						</table>
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666" class="style13" id="SA_banner" style="display:block;">
									<input onclick="c_fun(1)" type="radio" name="sys">
									<span class="style13"style="cursor:pointer;" onclick="c_fun(1);">&nbsp;<script>dw(specialApp)</script></span>
								</td>
							</tr>
						</table>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td valign="top" align="center">
									<div id="SA" style="display:none">
										<iframe id="SAFrame" name="frame_2" src="" frameborder="0" width="700" scrolling="no"></iframe>
									</div>
								</td>
							</tr>
						</table>
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666" class="style13" id="UP_banner" style="display:block;">
									<input onclick="c_fun(2)" type="radio" name="sys" />
									<span class="style13"style="cursor:pointer;" onclick="c_fun(2);">&nbsp;<script>dw(upnp)</script></span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<div id="UP" style="display:none">
										<iframe id="UPFrame" name="frame_3" src="" frameborder="0" width="700" height="50"></iframe>
									</div>
								</td>
							</tr>
						</table>
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666" class="style13" id="AL_banner" style="display:block;">
									<input onclick="c_fun(3)" type="radio" name="sys" />
									<span class="style13"style="cursor:pointer;" onclick="c_fun(3);">&nbsp;<script>dw(alg)</script></span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<div id="AL" style="display:none">
										<iframe id="ALFrame" name="frame_4" src="" frameborder="0" width="700"></iframe>
									</div>
								</td>
							</tr>
						</table>
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="left" width="700" height="25px" bgcolor="#666666" class="style13">
									<input onclick="c_fun(4)" type="radio" name="sys">
									<span class="style13"style="cursor:pointer;" onclick="c_fun(4);">&nbsp;<script>dw(natSr)</script></span>
								</td>
							</tr>
							<tr>
								<td valign="top" align="center">
									<div id="SR" style="display:none">
										<iframe id="SRFrame" name="frame_5" src="" frameborder="0" width="700" scrolling="no"></iframe>
									</div>
								</td>
							</tr>
						</table>
						<br>
						<table width="700" border="0" cellspacing="1" cellpadding="0">
							<tr>
								<td align="right">
									<form action=/goform/ApplyToReboot method=POST name="AdvApplyToReboot">
										<input type="hidden" value="/nat.asp" name="submit-url">
										<input type=submit value="Apply" style ="width:100px">
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
													<span class="style13"><script>dw(virtualServer)</script></span><br>
													<span id="com00" class="style21" style="display:block"><script>dw(natVrInfoshort)</script><a class="style71" onclick="com_sw(0,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com01" class="style21" style="display:none"><script>dw(natVrInfo)</script><a class="style71" onclick="com_sw(0,2);" style="cursor:pointer;" > .....close</a></span>
												</span>
												<br>
												<span class="style1">
													<span class="style13"><script>dw(specialApp)</script></span><br>
													<span id="com10" class="style21" style="display:block"><script>dw(natSappInfoshort)</script><a class="style71" onclick="com_sw(1,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com11" class="style21" style="display:none"><script>dw(natSappInfo)</script><a class="style71" onclick="com_sw(1,2);" style="cursor:pointer;" > .....close</a></span>
												</span>
												<br>
												<span class="style1">
													<span class="style13"><script>dw(upnp)</script></span><br>
													<span id="com20" class="style21" style="display:block"><script>dw(natUPnPInfoshort)</script><a class="style71" onclick="com_sw(2,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com21" class="style21" style="display:none"><script>dw(natUPnPInfo)</script><a class="style71" onclick="com_sw(2,2);" style="cursor:pointer;" > .....close</a></span>
												</span>
												<span class="style1">
													<span class="style13"><script>dw(alg)</script></span><br>
													<span id="com30" class="style21" style="display:block"><script>dw(natAlgInfoshort)</script><a class="style71" onclick="com_sw(3,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com31" class="style21" style="display:none"><script>dw(natAlgInfo)</script><a class="style71" onclick="com_sw(3,2);" style="cursor:pointer;" > .....close</a></span>
												</span>
												<br>
												<span class="style1">
													<span class="style13"><script>dw(natSr)</script></span><br>
													<span id="com40" class="style21" style="display:block"><script>dw(natSrInfoshort)</script><a class="style71" onclick="com_sw(4,1);" style="cursor:pointer;" > .....more</a></span>
													<span id="com41" class="style21" style="display:none"><script>dw(natSrInfo)</script><a class="style71" onclick="com_sw(4,2);" style="cursor:pointer;" > .....close</a></span>
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
	document.getElementsByName("sys")[0].checked=true;
	document.getElementById('SA').style.display = "none"
	document.getElementById('UP').style.display = "none"
	document.getElementById('AL').style.display = "none"
	document.getElementById('SR').style.display = "none"
	document.getElementById("VSFrame").src = "/natvser.asp"
	document.getElementById('VS').style.display = "block"
	var internet_sta = "<% getInfo("sta-current"); %>"
	top.sw_current_sta(internet_sta);
	top.cloud_current_sta(cloud_sta);
</script>
