<html>
  <head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8">
    <meta http-equiv="Pragma" content="no-cache">
    <meta http-equiv="Expires" content="-1">
    <link rel="stylesheet" href="set.css">
    <script type="text/javascript" src="/file/javascript.js"></script>
    <script type="text/javascript" src="/file/multilanguage.var"></script>
	 <script type="text/javascript" src="file/common.var"></script>
    <title></title>
  </head>
<body class="mainbg">
  <br>
  <a class="titlecolor"><script>dw(ActiveWirelessClientTable)</script></a>
  <br>
  <br>
  <a class="textcolor"><script>dw(ActiveWirelessClientTableContent)</script></a>
  <br>
  <br>
  <form action=/goform/formWirelessTbl method=POST name="formWirelessTbl">
	<table border='1' width="650">
	  <tr class="style13">
		<td width="80"><script>dw(MACAddress)</script></td>
		<td width="100"><script>dw(PhyMode)</script></td>
		<td width="70"><script>dw(tpacket)</script></td>
		<td width="70"><script>dw(rpacket)</script></td>
		<td width="90"><script>dw(txrate)</script></td>
		<td width="80"><script>dw(PowerSaving)</script></td>
		<td width="100"><script>dw(ExpiredTime)</script></td>
		</tr>
		<%wirelessClientList(); %>
	</table>
	<input type="hidden" value="/wlstatbl.asp" name="submit-url">
	<br>
	<table border='0' width="650">
		<tr>
			<td align="right">
		  <script>
			document.write('<input type="submit" value="' + showText(Refresh)+'" name="refresh" class="button">&nbsp;&nbsp;');
			document.write('<input type="button" value="' + showText(Close) + '" name="close" onClick="javascript: window.close();" class="button">');
						</script>
		</td>
				</tr>
			</table>
  </form>
  </body>
</html>
