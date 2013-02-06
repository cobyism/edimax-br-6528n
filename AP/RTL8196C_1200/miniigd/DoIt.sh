#!/bin/sh
. ../../../define/FUNCTION_SCRIPT
friendlyName="Wireless Router"
if [ "${_SMARTWAYS_}" = "y" ]; then
	manufacturer=""
else
	manufacturer="$_MODE_"
fi
manufacturerURL=""
modelDescription=""
modelName=""
modelNumber=""
modelURL=""
if [ "$_MODE_" = "EdimaxOBML" ]; then
	manufacturer="Edimax Technology Co. Ltd"
	modelName="300Mbps Wireless Router"
	modelNumber="BR-6428nS"
fi
echo "<?xml version=\"1.0\" ?> 
 <root xmlns=\"urn:schemas-upnp-org:device-1-0\">
 <specVersion>
  <major>1</major> 
  <minor>0</minor> 
  </specVersion>
 <device>
  <deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType> 
  <friendlyName>$friendlyName</friendlyName> 
  <manufacturer>$manufacturer</manufacturer> 
  <manufacturerURL>$manufacturerURL</manufacturerURL> 
  <modelDescription>$modelDescription</modelDescription>  
  <modelName>$modelName</modelName> 
  <modelNumber>$modelNumber</modelNumber>
  <modelURL>$modelURL</modelURL> 
  <UDN>uuid:12342409-1234-1234-5678-ee1234cc5678</UDN> 
 <serviceList>
 <service>
  <serviceType>urn:schemas-dummy-com:service:Dummy:1</serviceType> 
  <serviceId>urn:dummy-com:serviceId:dummy1</serviceId> 
  <controlURL>/dummy</controlURL> 
  <eventSubURL>/dummy</eventSubURL> 
  <SCPDURL>/dummy.xml</SCPDURL> 
  </service>
  </serviceList>
 <deviceList>
 <device>
  <deviceType>urn:schemas-upnp-org:device:WANDevice:1</deviceType> 
  <friendlyName>WANDevice</friendlyName> 
  <manufacturer>Wireless Router</manufacturer> 
  <manufacturerURL></manufacturerURL> 
  <modelDescription>WANCommonInterfaceConfig</modelDescription> 
  <modelName>WANCommonInterfaceConfig</modelName> 
  <modelNumber>1</modelNumber> 
  <modelURL></modelURL> 
  <serialNumber>00000000</serialNumber> 
  <UDN>uuid:12342409-1234-1234-5678-ee1234cc5678</UDN> 
  <UPC>MINIUPNPD</UPC> 
 <serviceList>
 <service>
  <serviceType>urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1</serviceType> 
  <serviceId>urn:upnp-org:serviceId:WANCommonIFC1</serviceId> 
  <controlURL>/upnp/control/WANCommonInterfaceConfig</controlURL> 
  <eventSubURL>/upnp/event/WANCommonInterfaceConfig</eventSubURL> 
  <SCPDURL>/wancfg.xml</SCPDURL> 
  </service>
  </serviceList>
 <deviceList>
 <device>
  <deviceType>urn:schemas-upnp-org:device:WANConnectionDevice:1</deviceType> 
  <friendlyName>WANConnectionDevice</friendlyName> 
  <manufacturer>Wireless Router</manufacturer> 
  <manufacturerURL></manufacturerURL> 
  <modelDescription>WANConnectionDevice</modelDescription> 
  <modelName>WANConnectionDevice</modelName> 
  <modelNumber>1</modelNumber> 
  <modelURL></modelURL> 
  <serialNumber>00000000</serialNumber> 
  <UDN>uuid:12342409-1234-1234-5678-ee1234cc5679</UDN> 
  <UPC>MINIUPNPD</UPC> 
 <serviceList>
 <service>
  <serviceType>urn:schemas-upnp-org:service:WANIPConnection:1</serviceType> 
  <serviceId>urn:upnp-org:serviceId:WANIPConn1</serviceId> 
  <controlURL>/upnp/control/WANIPConnection</controlURL> 
  <eventSubURL>/upnp/event/WANIPConnection</eventSubURL> 
  <SCPDURL>/wanipcn.xml</SCPDURL> 
  </service>
  </serviceList>
  </device>
  </deviceList>
  </device>
  </deviceList>
  <presentationURL>http://!ADDR!/</presentationURL> 
  </device>
  </root>" > picsdesc.skl
