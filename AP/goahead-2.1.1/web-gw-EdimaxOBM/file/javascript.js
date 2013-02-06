//----------------------------------------------------------------------------------
//                           Disciption
//      This is a JavaScript Document for multiple languages webs.
//      Just to build or modify the cookie when language selected. And show
//  the right text in the selected language!
//      This application must be used with multiple languages doucument
//  which in defined type. 


//----------------------------------------------------------------------------------
//      Define some variables for selecting language. Application select text
//  in right language just by the value of the variable "stype"!

//----------------------------------------------------------------------------------
//      Set cookie and Set language which selected.

function include(filename)
{
	var head = document.getElementsByTagName('head')[0];
	script = document.createElement('script');
	script.src = filename;
	script.type = 'text/javascript';
	head.appendChild(script)
}

function setlanguage()
{
	var vervaldatum = new Date()
	vervaldatum.setDate(vervaldatum.getDate()+365);
	var newValue = document.form.site.options[document.form.site.selectedIndex].value;
  	document.cookie="language="+newValue+"; expires="+vervaldatum+"; path=/";
	parent.window.location.href = "./index.asp";	//reload();	//
}

function getCookie(c_name)
{
	if (document.cookie.length>0)
	{
		c_start=document.cookie.indexOf(c_name + "=")
		if (c_start!=-1)
		{
			c_start=c_start + c_name.length+1;
			c_end=document.cookie.indexOf(";",c_start);
			if (c_end==-1) c_end=document.cookie.length;
			return unescape(document.cookie.substring(c_start,c_end));
		}
	}
	return 0;
}


//----------------------------------------------------------------------------------
//     Show the text in right language.

var stype = getCookie('language');

function showText(text)
{
	if(text[stype]==null || text[stype]=="")
		return text[0];
	else
  		return text[stype];
}

function dw(message,defaultMs)
{
	document.write(showText(message));	
}

function setFocus(field) 
{
	field.value = field.defaultValue;
	field.focus();
	return;
}

function checkFormat(str,msg)
{
	i=1;
	while ( str.length!=0)
	{
		if ( str.charAt(0) == '.' ) i++;
		str = str.substring(1);
	}

	if ( i > 4 )
	{
		alert(msg);
		return false;
	}
	else 
		return true;
}

function validateKey(str)
{
	for (var i=0; i<str.length; i++)
	{
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.' )) continue;
		return 0;
	}
	return 1;
}

function getDigit(str, num)
{
	i=1;
	if ( num != 1 )
	{
		while (i!=num && str.length!=0)
		{
			if ( str.charAt(0) == '.' ) 
				i++;
			str = str.substring(1);
		}
		if ( i!=num )
			return -1;
	}

	for (i=0; i<str.length; i++)
	{
		if ( str.charAt(i) == '.' )
		{
			str = str.substring(0, i);
			break;
		}
	}

	if ( str.length == 0)
		return -1;
	d = parseInt(str, 10);
	return d;
}

function checkDigitRange(str, num, min, max)
{
	d = getDigit(str,num);
	if ( d > max || d < min )
		return false;
	
	return true;
}

function checkIpAddr(field, msg)
{
	if ( validateKey(field.value) == 0)
	{
		alert(msg + ' ' + showText(ItShouldBe4DigitInIpFormat));
		return false;
	}
	
	if ( !checkDigitRange(field.value,1,0,255) )
	{
		alert(msg + ' ' + showText(SomethingShouldBeMinToMax).replace(/#####/, showText(TheFirstNumber)).replace(/####@/,0).replace(/###@#/,255));
		return false;
	}
	
	if ( !checkDigitRange(field.value,2,0,255) )
	{
		alert(msg + ' ' + showText(SomethingShouldBeMinToMax).replace(/#####/, showText(TheSecondNumber)).replace(/####@/,0).replace(/###@#/,255));
		return false;
	}
	
	if ( !checkDigitRange(field.value,3,0,255) )
	{
		alert(msg + ' ' + showText(SomethingShouldBeMinToMax).replace(/#####/, showText(TheThirdNumber)).replace(/####@/,0).replace(/###@#/,255));
		return false;
	}
	
	if ( !checkDigitRange(field.value,4,0,254) )
	{
		alert(msg + ' ' + showText(SomethingShouldBeMinToMax).replace(/#####/, showText(TheFourthNumber)).replace(/####@/,0).replace(/###@#/,254));
		return false;
	}
	return true;
}

function ipRule(ip, str, type, defVal)
{
	if (type == "ip")
	{
		if (ip.value=="")
		{
			alert(showText(SomethingCannotBeEmpty).replace(/#####/, str) + ' ' + showText(ItShouldBe4DigitInIpFormat));
			if (defVal == 1)
				ip.value = ip.defaultValue;
      
			ip.focus();
			return false;
		}
	}
	else
	{
		if (ip.value=="" || ip.value=="0.0.0.0")
		{
			ip.value = "0.0.0.0";
			return true;
		}
	}

	if ( checkFormat(ip.value,showText(InvalidSomething).replace(/#####/, str) + ' ' + showText(ItShouldBe4DigitInIpFormat)) == false)
		return false;

	if ( checkIpAddr(ip, showText(InvalidSomething).replace(/#####/, str) ) == false )
	{
		if (defVal == 1)
			ip.value = ip.defaultValue;
		
		ip.focus();
		return false;
	}
	return true;
}

function portRule( fromPort, fromStr, toPort, toStr, min, max, defVal)
{
//********** from port **********
	if (fromPort.value=="")
	{
		alert(showText(SomethingCannotBeEmpty).replace(/#####/, fromStr) + ' ' + showText(SomethingShouldBeMinToMax).replace(/#####/, fromStr).replace(/####@/, min).replace(/###@#/,max));
		if ( defVal == 1 )
			fromPort.value = fromPort.defaultValue;
		
		fromPort.focus();
		return false;
	}
	
	if ( validateKey( fromPort.value ) == 0 )
	{
		alert(showText(InvalidSomething).replace(/#####/, fromStr) + ' ' + showText(SomethingShouldBeMinToMax).replace(/#####/, fromStr).replace(/####@/, min).replace(/###@#/,max));
		if ( defVal == 1 ) fromPort.value = fromPort.defaultValue;
			fromPort.focus();
		return false;
	}
	
	d1 = getDigit(fromPort.value, 1);
	if (d1 > max || d1 < min)
	{
		alert(showText(InvalidSomething).replace(/#####/, fromStr) + ' ' + showText(SomethingShouldBeMinToMax).replace(/#####/, fromStr).replace(/####@/, min).replace(/###@#/,max));
		if ( defVal == 1 ) fromPort.value = fromPort.defaultValue;
			fromPort.focus();
		return false;
	}
//********** to port **********
	if (toStr != "")
	{
		if (toPort.value!="")
		{
			if ( validateKey( toPort.value ) == 0 )
			{
				alert(showText(InvalidSomething).replace(/#####/, toStr) + ' ' + showText(SomethingShouldBeMinToMax).replace(/#####/, toStr).replace(/####@/, min).replace(/###@#/,max));
				if ( defVal == 1 ) toPort.value = toPort.defaultValue;
					toPort.focus();
				return false;
			}
			d2 = getDigit(toPort.value, 1);
			if (d2 > max || d2 < min)
			{
				alert(showText(InvalidSomething).replace(/#####/, toStr) + ' ' + showText(SomethingShouldBeMinToMax).replace(/#####/, toStr).replace(/####@/, min).replace(/###@#/,max));
				if ( defVal == 1 ) toPort.value = toPort.defaultValue;
					toPort.focus();
				return false;
			}
			if (d1 > d2 )
			{
				alert(showText(InvalidSomething).replace(/#####/, toStr) + ' ' + showText(SomethingShouldLessThanSomething).replace(/#####/,showText(TheFirstNumber)).replace(/####@/,showText(TheSecondNumber)) );
				if ( defVal == 1 ) toPort.value = toPort.defaultValue;
					fromPort.focus();
				return false;
			}
		}
	}
	return true;
}

function checkMask(str, num, msg)
{
	d = getDigit(str,num);
  
	switch(num)
	{
		case 1:
			if( !(d==0 || d==128 || d==192 || d==224 || d==240 || d==248 || d==252 || d==254 || d==255 ))
			{
				alert(msg + ' ' + showText(SomethingShouldBeMACAddressNumbersWith255).replace(/#####/, showText(TheFirstNumber)));
				return false;
			}
			break;
		case 2:
			if( !(d==0 || d==128 || d==192 || d==224 || d==240 || d==248 || d==252 || d==254 || d==255 ))
			{
				alert(msg + ' ' + showText(SomethingShouldBeMACAddressNumbersWith255).replace(/#####/, showText(TheSecondNumber)));
				return false;
			}
			break;
		case 3:
			if( !(d==0 || d==128 || d==192 || d==224 || d==240 || d==248 || d==252 || d==254 || d==255 ))
			{
				alert(msg + ' ' + showText(SomethingShouldBeMACAddressNumbersWith255).replace(/#####/, showText(TheThirdNumber)));
				return false;
			}
			break;
		case 4:
			if( !(d==0 || d==128 || d==192 || d==224 || d==240 || d==248 || d==252 || d==254 ))
			{
				alert(msg + ' ' + showText(SomethingShouldBeMACAddressNumbersWithout255).replace(/#####/, showText(TheFourthNumber)));
				return false;
			}
			break;
		default:
			break;
	}
	return true;
}

function checkmaskRule(mask)
{
	var tmpArray = new Array();
	patt = new RegExp("^[0-9]\{1,3\}[.][0-9]\{1,3\}[.][0-9]\{1,3\}[.][0-9]\{1,3\}$");
	if( ! patt.test(mask) || mask=="0.0.0.0") return false;
	tmpStr = mask;
	j=0;
	while(tmpStr != "")
	{
		stop = (tmpStr.indexOf(".")==-1)?(tmpStr.length):(tmpStr.indexOf("."));
		number = parseInt( tmpStr.substr(0, stop) );

		if(number>=0 && number<=255)
		{
			for(i=0;i<8;i++)
			{
				tmpArray[j*8 + (7-i)] = number % 2;
				number=parseInt(number/2);
			}
		}
		else return false;
		tmpStr = tmpStr.substr(stop+1, tmpStr.length);
		j++;
	}
	
	j=1;
	for(i=0;i<32;i++)
	{
		if(j==1 && tmpArray[i]==0) j=0;
		else if(j==0 && tmpArray[i]==1) return false;
	}
	return true;
}

function maskRule (mask, maskstr, defVal)
{
	if (mask.value == "")
	{
		alert(showText(SomethingCannotBeEmpty).replace(/#####/, maskstr) + ' ' + showText(ItShouldBe4DigitInIpFormat));
		if (defVal == 1) mask.value = mask.defaultValue;
			mask.focus();
		return false;
	}
	if (mask.value == "0.0.0.0")
	{
		alert( showText(InvalidSomething).replace(/#####/, maskstr));
		mask.focus();
		return false;
	}  
	if ( validateKey( mask.value ) == 0 )
	{
		alert( showText(InvalidSomething).replace(/#####/, maskstr) + ' ' + showText(ItShouldBe4DigitInIpFormat));
		if (defVal == 1) mask.value = mask.defaultValue;
			mask.focus();
		return false;
	}
	if ( !checkMask(mask.value,1, showText(InvalidSomething).replace(/#####/, maskstr)) )
	{
		if (defVal == 1) mask.value = mask.defaultValue;
			mask.focus();
		return false;
	}
	if ( !checkMask(mask.value,2, showText(InvalidSomething).replace(/#####/, maskstr)) )
	{
		if (defVal == 1) mask.value = mask.defaultValue;
			mask.focus();
		return false;
	}
	if ( !checkMask(mask.value,3, showText(InvalidSomething).replace(/#####/, maskstr)) )
	{
		if (defVal == 1) mask.value = mask.defaultValue;
			mask.focus();
		return false;
	}
	if ( !checkMask(mask.value,4, showText(InvalidSomething).replace(/#####/, maskstr)) )
	{
		if (defVal == 1) mask.value = mask.defaultValue;
			mask.focus();
		return false;
	}
	
	if( !checkmaskRule(mask.value))
    {
        alert("Subnet mask ip can't use "+mask.value+", try others !");
                
        if (defVal == 1)
			mask.value = mask.defaultValue;
		mask.focus();
		return false;
    }
	return true;
}

function checkSubnet(ip, mask, client)
{
	ip_d = getDigit(ip, 1);
	mask_d = getDigit(mask, 1);
	client_d = getDigit(client, 1);
	if ( (ip_d & mask_d) != (client_d & mask_d ) ) return false;

	ip_d = getDigit(ip, 2);
	mask_d = getDigit(mask, 2);
	client_d = getDigit(client, 2);
	if ( (ip_d & mask_d) != (client_d & mask_d ) ) return false;

	ip_d = getDigit(ip, 3);
	mask_d = getDigit(mask, 3);
	client_d = getDigit(client, 3);
	if ( (ip_d & mask_d) != (client_d & mask_d ) ) return false;

	ip_d = getDigit(ip, 4);
	mask_d = getDigit(mask, 4);
	client_d = getDigit(client, 4);
	if ( (ip_d & mask_d) != (client_d & mask_d ) ) return false;

	return true;
}

function macRule( mac, macstr, defVal)
{
	var str = mac.value;
	if ( str.length < 12)
	{
		alert(showText(SomethingShouldconsist12Hex).replace(/#####/, macstr));
		if ( defVal == 1 ) mac.value = mac.defaultValue;
			mac.focus();
		return false;
	}

	for (var i=0; i<str.length; i++)
	{
		if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) >= 'a' && str.charAt(i) <= 'f') || (str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
			continue;
		alert(showText(InvalidSomething).replace(/#####/, macstr) + ' ' + showText(SomethingConsistOnlyHexadecimal).replace(/#####/, macstr) );
		if ( defVal == 1 ) mac.value = mac.defaultValue;
		mac.focus();
		return false;
	}
	return true;
}

function subnetRule(ip, mask, client, str, str1) 
{
	if (client.value!="" && client.value!="0.0.0.0") 
	{
		if ( !checkSubnet(ip.value, mask.value, client.value)) 
		{
			alert(showText(InvalidSomething).replace(/#####/,str)+' '+showText(SomethingAndSomethingShouldInSameSubnet).replace(/#####/,str).replace(/####@/,str1));
            client.value = client.defaultValue;
			client.focus();
            return false;
        }
    }
	return true;
}

function strRule(name, str) 
{
	if(name.value=="") 
	{
		alert(showText(SomethingCannotBeEmpty).replace(/#####/,str));
		setFocus(name);
		return false;
	}
	
	if(name.value.indexOf("\"") >= 0 || name.value.indexOf("`") >= 0 || name.value.indexOf("$") >= 0 || name.value.indexOf("\\") >= 0 || name.value.indexOf("'") >= 0)
	{
		alert(showText(InvalidSomething).replace(/#####/,str));
		setFocus(name);
		return false;
	}
	return true;
}

function deleteClick()
{
	if ( !confirm(showText(DoYouReallyWantToDeleteSelected)) ) return false;
	else return true;
}

function deleteAllClick()
{
	if ( !confirm(showText(DoYouReallyWantToDeleteAll)) ) return false;
	else return true;
}

function MM_openBrWindow(theURL,winName,features)
{
	window.open(theURL,winName,features);
}

function ShowHelpLink(keyword)
{
	if(keyword!="") document.write('<a href="#"onclick="MM_openBrWindow(\'help.asp#'+keyword+'\',\'help\',\'scrollbars=yes,width=600,height=500\')"><img src="file/info_s.gif" width="20" height="20" border="0" align="middle"></a>');
}

//detect language
function Detect_Lang()
{
	if (navigator.appName == 'Netscape')
		var language = navigator.language;
	else
		var language = navigator.browserLanguage;
		
if (language.toLowerCase().indexOf('de') > -1) stype=1;
else if (language.toLowerCase().indexOf('es') > -1) stype=2;
else if (language.toLowerCase().indexOf('fr') > -1) stype=3;
else if (language.toLowerCase().indexOf('it') > -1) stype=4;
else if (language.toLowerCase().indexOf('nl') > -1) stype=5;
else if (language.toLowerCase().indexOf('pt') > -1) stype=6;
else if (language.toLowerCase().indexOf('cz') > -1) stype=7;
else if (language.toLowerCase().indexOf('pl') > -1) stype=8;
else if (language.toLowerCase().indexOf('ro') > -1) stype=9;
else if (language.toLowerCase().indexOf('ru') > -1) stype=10;
else if (language.toLowerCase().indexOf('sk') > -1) stype=11;
else if (language.toLowerCase().indexOf('tw') > -1) stype=12;
else if (language.toLowerCase().indexOf('cn') > -1) stype=13;
else if (language.toLowerCase().indexOf('da') > -1) stype=14;
else if (language.toLowerCase().indexOf('sv') > -1) stype=15;
else if (language.toLowerCase().indexOf('hu') > -1) stype=16;
else if (language.toLowerCase().indexOf('tu') > -1) stype=17;
else if (language.toLowerCase().indexOf('vn') > -1) stype=18;
else stype=0;
}

function openWindow(url, windowName) {
	var wide=660;
	var high=420;		
	if (document.all)
		var xMax = screen.width, yMax = screen.height;
	else if (document.layers)
		var xMax = window.outerWidth, yMax = window.outerHeight;
	else
		var xMax = 640, yMax=500;		
	var xOffset = (xMax - wide)/2;
	var yOffset = (yMax - high)/3;
	var settings = 'width='+wide+',height='+high+',screenX='+xOffset+',screenY='+yOffset+',top='+yOffset+',left='+xOffset+', resizable=yes, toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=yes';
	window.open( url, windowName, settings );
}
//EdimaxOBM NEW WEB Function

var main_bg_img = new Array("topmenu_1_01.jpg","menu_bg_gray.gif","topmenu_3_03.jpg","menu_bg_gray.gif","topmenu_3_03.jpg","menu_bg_gray.gif","topmenu_3_03.jpg","menu_bg_gray.gif","topmenu_3_03.jpg","menu_bg_gray.gif","topmenu_3_03.jpg","menu_bg_gray.gif","topmenu_2_05.gif");

var main_style = new Array("style5","style5","style5","style5","style5","style5");	
			   
var sub_style = new Array("style10","style10","style10","style10","style10","style10","style10","style10");

var sub_page = new Array( new Array("system.asp","wan.asp","lan.asp","wireless.asp","advance_setting.asp","nat.asp","firewall.asp","fwparental.asp"),
                          new Array("stainfo.asp","stanet.asp","stadevice.asp","stasylog.asp","staslog.asp","stadhcptbl.asp","stats.asp"),
                          new Array("system.asp","wan.asp","lan.asp","wireless.asp","advance_setting.asp","nat.asp","firewall.asp"));

var sub_word = new Array( new Array("dw(system)","dw(wan)","dw(lan)","dw(wireless)","dw(advanceSet)","dw(nat)","dw(firewall)","dw(Parental)"),
                          new Array("dw(system)","dw(statInternet)","dw(statDevice)","dw(statSystem)","dw(statSecurity)","dw(statDHCP)","dw(statStatistics)"),
                          new Array("dw(system)","dw(wan)","dw(lan)","dw(wireless)","dw(advanceSet)","dw(nat)","dw(firewall)"));
function Write_Header(i,j)
{
	for ( k = 0 ; k < 6 ; k++)
	{	
		if(i == k)
		{
			if(i == 0)
			{
				main_bg_img[k*2] = "topmenu_2_01.jpg"
				main_bg_img[k*2+1] = "menu_bg_red.gif"
				main_bg_img[k*2+2] = "topmenu_2_03.jpg"
			}
			else if (i == 5)
			{
				main_bg_img[k*2] = "topmenu_1_03.jpg"
				main_bg_img[k*2+1] = "menu_bg_red.gif"
				main_bg_img[k*2+2] = "topmenu_1_05.gif"
			}
			else if (top.MODEL != "BR6428GN" && top.MODEL != "BR6478GN" && i == 2)
			{
				main_bg_img[k*2] = "topmenu_1_03.jpg"
				main_bg_img[k*2+1] = "menu_bg_red.gif"
				main_bg_img[8] = "topmenu_2_03.jpg"
			}
			else
			{
				main_bg_img[k*2] = "topmenu_1_03.jpg"
				main_bg_img[k*2+1] = "menu_bg_red.gif"
				main_bg_img[k*2+2] = "topmenu_2_03.jpg"
			}
		}
		
		if(i == k) main_style[k] = "style3"
		else main_style[k] = "style5"
	}
	document.write('<tr><td>');
	document.write('<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0" >');
	document.write('<tr><td height="94" colspan="16" valign="bottom">');
	document.write('<img src="images/logo.gif" width="126" height="35"><br>');
	document.write('<table width="1200" border="0" cellspacing="0" cellpadding="0">');
	document.write('<tr><td width="1200" align="right">');
	document.write('<table border="0" align="right" cellpadding="0" cellspacing="0">');
	document.write('<tr>');
	document.write('<td width="25"><img src="images/ball.gif" name="currentsta">&nbsp;</td>');
	document.write('<td width="15"><img src="images/arrow_up.gif"></td>');
	document.write('<td align="left"><span class="style9">&nbsp;<span class="style12" target="mainFrame" size="10" style="cursor:pointer;" onClick="location.replace(\'wireless.asp\')">'+top.ssiddrv+'</span></span></td>');
	document.write('<td width="15"><img src="images/arrow.gif"></td>');
	document.write('<td width="50" class="style9" target="mainFrame" size="10" style="cursor:pointer;" onClick="location.replace(\'system.asp\')"><a id="face"></a></td>');
	document.write('<td width="15"><img src="images/arrow.gif"></td>');
	document.write('<td class="style9" target="mainFrame" size="10" style="cursor:pointer;" onClick="location.replace(\'lan.asp\')">&nbsp;'+top.iprom+'</td>');
	document.write('<td width="15"><img src="images/arrow.gif"></td>');
	document.write('<td class="style9" target="mainFrame" size="10" style="cursor:pointer;" onClick="top.tool = 1;location.replace(\'tools.asp\')">&nbsp;v '+top.fwver+'</td>');
	if(top.edimae30)
		document.write('<td width="200" align="right"><img src="images/cloud-logo.gif" name="cloudstaimg"><br>\
                <font face="Arial, Helvetica, sans-serif" color="#FFCC00" size="1">http://www.edimax.com.tw/'+top.mac+'</td>')
	document.write('<td width="10">');
	document.write('<form name="form">');
	document.write('<td width="90" align="right" valign="middle">');
	document.write('<select name ="site" onchange ="setlanguage()" style ="width:120">');
	document.write('<option name ="SelectXX" value =""><script>dw(setLanguage)</script></option>');
	document.write('<option name ="SelectUK" value ="0">English</option>');
	document.write('<option name ="SelectDE" value ="1">Deutsch</option>');
	document.write('<option name ="SelectES" value ="2">Español</option>');
	document.write('<option name ="SelectFR" value ="3">Français</option>');
	document.write('<option name ="SelectIT" value ="4">Italiano</option>');
	document.write('<option name ="SelectNL" value ="5">Nederlands</option>');
	document.write('<option name ="SelectPT" value ="6">Português</option>');
	document.write('<option name ="SelectCZ" value ="7">Čeština</option>');
	document.write('<option name ="SelectPL" value ="8">Polski</option>');
	document.write('<option name ="SelectRO" value ="9">Română</option>');
	document.write('<option name ="SelectRU" value ="10">Pусский</option>');
	document.write('<option name ="SelectSK" value ="11">Slovenský</option>');
	document.write('<option name ="SelectZH" value ="12">繁體中文</option>');
	document.write('<option name ="SelectCN" value ="13">簡體中文</option>');
	document.write('</select></td></form></td></tr></table></td></tr></table></td></tr>');
	document.write('<tr><td height="30"></td></tr>');
	document.write('<tr>');
	document.write('<td width="12" height="35"><img src="images/'+main_bg_img[0]+'" width="12" height="35"></td>');
	document.write('<td width="98" height="35" background="images/'+main_bg_img[1]+'" class="'+main_style[0]+'"> \
						<div align="center" style="cursor:pointer;" onClick="location.replace(\'main.asp\')">Home</div></td>');
	document.write('<td width="46" height="35"><img src="images/'+main_bg_img[2]+'" width="46" height="35"></td>');
	if( ! top.DEF_AWD )
	{
		document.write('<td width="112" height="35" background="images/'+main_bg_img[3]+'" class="'+main_style[1]+'"> \
						<div align="center" style="cursor:pointer;" onClick="goWiz(1,\'/system.asp\')"><script>dw(quickSetup)</script></div></td>');
	}
	else
	{
		document.write('<td width="112" height="35" background="images/'+main_bg_img[3]+'" class="'+main_style[1]+'"> \
						<div align="center" style="cursor:pointer;" onClick="goWiz(1,\'/autoIndex2.asp\')"><script>dw(quickSetup)</script></div></td>');
	}
	document.write('<td width="46" height="35"><img src="images/'+main_bg_img[4]+'" width="46" height="35"></td>');
	document.write('<td width="112" height="35" background="images/'+main_bg_img[5]+'" class="'+main_style[2]+'"> \
						<div align="center" style="cursor:pointer;" onClick="top.reset_top_item();goWiz(0,\'/system.asp\')"><script>dw(setup)</script></div></td>');
	if(top.MODEL == "BR6428GN" || top.MODEL == "BR6478GN")
	{
		document.write('<td width="46" height="35"><img src="images/'+main_bg_img[6]+'" width="46" height="35"></td>');
		document.write('<td width="112" height="35" background="images/'+main_bg_img[7]+'" class="'+main_style[3]+'"> \
							<div align="center" style="cursor:pointer;" onClick="location.replace(\'ezqos.asp\')">iQoS</div></td>');
	}
	document.write('<td width="46" height="35"><img src="images/'+main_bg_img[8]+'" width="46" height="35"></td>');
	document.write('<td width="112" height="35" background="images/'+main_bg_img[9]+'" class="'+main_style[4]+'"> \
						<div align="center" style="cursor:pointer;" onClick="location.replace(\'stainfo.asp\')"><script>dw(state)</script></div></td>');
	document.write('<td width="46" height="35"><img src="images/'+main_bg_img[10]+'" width="46" height="35"></td>');
	document.write('<td width="112" height="35" background="images/'+main_bg_img[11]+'" class="'+main_style[5]+'"> \
						<div align="center" style="cursor:pointer;" onClick="location.replace(\'tools.asp\')"><script>dw(tool)</script></div></td>');
	document.write('<td width="41" height="35"><img src="images/'+main_bg_img[12]+'" width="41" height="35"></td>');
	if(top.MODEL == "BR6428GN")
		document.write('<td width="313" height="35">');
	else
		document.write('<td width="571" height="35">');
	document.write('<form action=/goform/formWizSetup method=POST name="systemMode">');
	document.write('<input type="hidden" value="" name="setPage">');
	document.write('<input type="hidden" value="" name="wizEnabled">');
	document.write('</form></td></tr></table></td></tr>');
	if ( i == 2 || i == 4)
	{
		var x = 0;
		var z = 8;
		if(top.MODEL == "BR6258GN" || top.MODEL == "BR6458GN")
		{
			var x = 0;
			var z = 7;
		}
		if ( i == 4)
		{
			x = 1;
			z = 7;
		}
		document.write('<tr><td height="27" bgcolor="#A5031A">');
		for (y =0 ; y < z ; y++)
		{
			if ( j == y )
			{
				if (i == 2)
					document.write('<span class="style9">&nbsp;<span class="style10" style="cursor:pointer;" onClick="top.reset_top_item();location.replace(\''+sub_page[x][y]+'\')"> \
									<script>'+sub_word[x][y]+'</script></span>&nbsp;&nbsp;|</span>');
				else
					document.write('<span class="style9">&nbsp;<span class="style10" style="cursor:pointer;" onClick="location.replace(\''+sub_page[x][y]+'\')"> \
									<script>'+sub_word[x][y]+'</script></span>&nbsp;&nbsp;|</span>');
			}
			else
			{
				if (i == 2)
					document.write('<span class="style9">&nbsp;<span class="style11" style="cursor:pointer;" onClick="top.reset_top_item();location.replace(\''+sub_page[x][y]+'\')"> \
									<script>'+sub_word[x][y]+'</script></span>&nbsp;&nbsp;|</span>');
				else
					document.write('<span class="style9">&nbsp;<span class="style11" style="cursor:pointer;" onClick="location.replace(\''+sub_page[x][y]+'\')"> \
									<script>'+sub_word[x][y]+'</script></span>&nbsp;&nbsp;|</span>');
			}
		}
		document.write('</td></tr><tr><td bgcolor="#CCCCCC">&nbsp;</td></tr>');
	}
	else
	{
		document.write('<tr><td height="27" bgcolor="#A5031A"></td></tr>');
		document.write('<tr><td bgcolor="#CCCCCC">&nbsp;</td></tr>');
	}
}

function Write_tail()
{
	document.write('<table width="1200" border="0" align="center" cellpadding="0" cellspacing="0">');
	document.write('<tr><td colspan="2"><img src="images/bar.jpg" width="1200" height="25" /></td></tr></table>');
}

function goWiz(yn,url) {
	document.systemMode.wizEnabled.value=yn;
	document.systemMode.setPage.value=url;
	document.systemMode.submit();
}	

function Write_Wiz_Form(wizurl,wiz)
{
	document.write('<form action=/goform/formWizSetup method=POST name="systemMode">');
	document.write('<input type="hidden" value="'+wizurl+'" name="setPage">');
	document.write('<input type="hidden" value="'+wiz+'" name="wizEnabled">');
	document.write('</form>');
}

var w3c=(document.getElementById)?true:false;
var ie=(document.all)?true:false;
var N=-1;

function createBar(w,h,bgc,brdW,brdC,blkC,speed,blocks,count,action)
{
	if(ie||w3c)
	{
		var t='<div id="_xpbar'+(++N)+'" style="visibility:visible; position:relative; overflow:hidden; width:'+w+'px; height:'+h+'px; background-color:'+bgc+'; border-color:'+brdC+'; border-width:'+brdW+'px; border-style:solid; font-size:1px;">';
		t+='<span id="blocks'+N+'" style="left:-'+(h*2+1)+'px; position:absolute; font-size:1px">';
		for(i=0;i<blocks;i++)
		{
			t+='<span style="background-color:'+blkC+'; left:-'+((h*i)+i)+'px; font-size:1px; position:absolute; width:'+h+'px; height:'+h+'px; '
			t+=(ie)?'filter:alpha(opacity='+(100-i*(100/blocks))+')':'-Moz-opacity:'+((100-i*(100/blocks))/100);
			t+='"></span>';
		}
		t+='</span></div>';
		document.write(t);
		var bA=(ie)?document.all['blocks'+N]:document.getElementById('blocks'+N);
		bA.bar=(ie)?document.all['_xpbar'+N]:document.getElementById('_xpbar'+N);
		bA.blocks=blocks;
		bA.N=N;
		bA.w=w;
		bA.h=h;
		bA.speed=speed;
		bA.ctr=0;
		bA.count=count;
		bA.action=action;
		bA.togglePause=togglePause;
		bA.showBar=function()
		{
			this.bar.style.visibility="visible";
		}
		bA.hideBar=function()
		{
			this.bar.style.visibility="hidden";
		}
		bA.tid=setInterval('startBar('+N+')',speed);
		return bA;
	}
}

function startBar(bn)
{
	var t=(ie)?document.all['blocks'+bn]:document.getElementById('blocks'+bn);
	if(parseInt(t.style.left)+t.h+1-(t.blocks*t.h+t.blocks)>t.w)
	{
		t.style.left=-(t.h*2+1)+'px';
		t.ctr++;
		if(t.ctr>=t.count)
		{
			eval(t.action);
			t.ctr=0;
		}
	}
	else 
		t.style.left=(parseInt(t.style.left)+t.h+1)+'px';
}

function togglePause()
{
	if(this.tid==0)
	{
		this.tid=setInterval('startBar('+this.N+')',this.speed);
	}
	else
	{
		clearInterval(this.tid);
		this.tid=0;
	}
}
//EdimaxOBM NEW WEB Function END
