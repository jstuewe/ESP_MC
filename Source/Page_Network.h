//
//  HTML PAGE
//
const char PAGE_NetworkConfiguration[] PROGMEM = R"=====(
<meta name="viewport" content="width=device-width, initial-scale=1" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<a href="admin.html"  class="btn btn--s"><</a>&nbsp;&nbsp;<strong>Network Configuration</strong>
<hr>
<strong>Network Information</strong><br>
<table border="0"  cellspacing="0" cellpadding="3" style="width:310px" >
<tr><td align="right">State :</td><td><span id="connectionstate">Working...</span></td></tr>
<tr><td align="right">Name :</td><td><span id="x_devicename"></span></td></tr>
<tr><td align="right">SSID :</td><td><span id="x_ssid"></span></td></tr>
<tr><td align="right">IP :</td><td><span id="x_ip"></span></td></tr>
<tr><td align="right">Netmask :</td><td><span id="x_netmask"></span></td></tr>
<tr><td align="right">Gateway :</td><td><span id="x_gateway"></span></td></tr>
<tr><td align="right">Mac :</td><td><span id="x_mac"></span></td></tr>
<tr><td colspan="2"><hr></span></td></tr>
</table>

<strong>Wifi connection settings:</strong><br>
<form action="" method="get">
<table border="0"  cellspacing="0" cellpadding="3" style="width:350px" >
<tr><td align="right">SSID:</td><td><input type="text" id="ssid" name="ssid" value=""></td></tr>
<tr><td align="right">Password:</td><td><input type="text" id="password" name="password" value=""></td></tr>
<tr><td align="right">DHCP:</td><td><input type="checkbox" id="dhcp" name="dhcp"></td></tr>
<tr><td align="right">Wifi Power: </td><td><input type="text" id="xmitpwr" name="xmitpwr" size="3" </td></tr>
<tr><td align="right">Static IP:</td><td><input type="text" id="ip_0" name="ip_0" size="3">.<input type="text" id="ip_1" name="ip_1" size="3">.<input type="text" id="ip_2" name="ip_2" size="3">.<input type="text" id="ip_3" name="ip_3" value="" size="3"></td></tr>
<tr><td align="right">Netmask:</td><td><input type="text" id="nm_0" name="nm_0" size="3">.<input type="text" id="nm_1" name="nm_1" size="3">.<input type="text" id="nm_2" name="nm_2" size="3">.<input type="text" id="nm_3" name="nm_3" size="3"></td></tr>
<tr><td align="right">Gateway:</td><td><input type="text" id="gw_0" name="gw_0" size="3">.<input type="text" id="gw_1" name="gw_1" size="3">.<input type="text" id="gw_2" name="gw_2" size="3">.<input type="text" id="gw_3" name="gw_3" size="3"></td></tr>
<tr><td colspan="2" align="center"><input type="submit" style="width:150px" class="btn btn--m btn--blue" value="Save"></td></tr>
</table>
</form>

<hr>
<strong>Networks:</strong><br>
<table border="0"  cellspacing="3" style="width:310px" >
<tr><td><div id="networks">Scanning...</div></td></tr>
<tr><td align="center"><a href="javascript:GetState()" style="width:150px" class="btn btn--m btn--blue">Refresh</a></td></tr>
</table>


<script>

function GetState()
{
	setValues("/admin/connectionstate");
}
function selssid(value)
{
	document.getElementById("ssid").value = value; 
}


window.onload = function ()
{
	load("style.css","css", function() 
	{
		load("microajax.js","js", function() 
		{
					setValues("/admin/values");
					setTimeout(GetState,3000);
		});
	});
}
function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}




</script>


)=====";

//
//  SEND HTML PAGE OR IF A FORM SUMBITTED VALUES, PROCESS THESE VALUES
// 

void send_network_configuration_html()
{
	
	if (server.args() > 0 )  // Save Settings
	{
		String temp = "";
		v.DhcpOn = false;
		for ( uint8_t i = 0; i < server.args(); i++ ) {
			if (server.argName(i) == "ssid") strncpy(v.Ssid, urldecode(server.arg(i)).c_str(),STRLEN-1);
			if (server.argName(i) == "password") strncpy(v.Password, urldecode(server.arg(i)).c_str(),STRLEN-1);
			if (server.argName(i) == "xmitpwr")  v.WifiPower = server.arg(i).toInt();
			if (server.argName(i) == "ip_0") if (checkRange(server.arg(i))) 	v.StaticIP[0] =  server.arg(i).toInt();
			if (server.argName(i) == "ip_1") if (checkRange(server.arg(i))) 	v.StaticIP[1] =  server.arg(i).toInt();
			if (server.argName(i) == "ip_2") if (checkRange(server.arg(i))) 	v.StaticIP[2] =  server.arg(i).toInt();
			if (server.argName(i) == "ip_3") if (checkRange(server.arg(i))) 	v.StaticIP[3] =  server.arg(i).toInt();
			if (server.argName(i) == "nm_0") if (checkRange(server.arg(i))) 	v.Netmask[0] =  server.arg(i).toInt();
			if (server.argName(i) == "nm_1") if (checkRange(server.arg(i))) 	v.Netmask[1] =  server.arg(i).toInt();
			if (server.argName(i) == "nm_2") if (checkRange(server.arg(i))) 	v.Netmask[2] =  server.arg(i).toInt();
			if (server.argName(i) == "nm_3") if (checkRange(server.arg(i))) 	v.Netmask[3] =  server.arg(i).toInt();
			if (server.argName(i) == "gw_0") if (checkRange(server.arg(i))) 	v.Gateway[0] =  server.arg(i).toInt();
			if (server.argName(i) == "gw_1") if (checkRange(server.arg(i))) 	v.Gateway[1] =  server.arg(i).toInt();
			if (server.argName(i) == "gw_2") if (checkRange(server.arg(i))) 	v.Gateway[2] =  server.arg(i).toInt();
			if (server.argName(i) == "gw_3") if (checkRange(server.arg(i))) 	v.Gateway[3] =  server.arg(i).toInt();
			if (server.argName(i) == "dhcp") v.DhcpOn = true;

		}
		WriteConfig();
		ResetNow = true;
	}
	else
	{
		server.send ( 200, "text/html", PAGE_NetworkConfiguration ); 
	}
	if (v.Dbug > 7) Serial.println(__FUNCTION__); 
}

//
//   FILL THE PAGE WITH VALUES
//

void send_network_configuration_values_html()
{

	String values ="";
	values += "x_devicename|" + (String)v.DeviceName + "|div\n";
	values += "x_ssid|" + (String)WiFi.SSID() + "|div\n";
	values += "x_ip|" + (String)WiFi.localIP()[0] + "." + (String)WiFi.localIP()[1] + "." + (String)WiFi.localIP()[2] + "." + (String)WiFi.localIP()[3] + "|div\n";
	values += "x_gateway|" + (String)WiFi.gatewayIP()[0] + "." + (String)WiFi.gatewayIP()[1] + "." + (String)WiFi.gatewayIP()[2] + "." + (String)WiFi.gatewayIP()[3] + "|div\n";
	values += "x_netmask|" + (String)WiFi.subnetMask()[0] + "." + (String)WiFi.subnetMask()[1] + "." + (String)WiFi.subnetMask()[2] + "." + (String)WiFi.subnetMask()[3] + "|div\n";
	values += "x_mac|" + GetMacAddress() + "|div\n";

	values += "ssid|" + (String) v.Ssid + "|input\n";
	values += "password|" +  (String) v.Password + "|input\n";
	values += "xmitpwr|"  + (String) v.WifiPower + "|input\n";
	values += "ip_0|" +  (String) v.StaticIP[0] + "|input\n";
	values += "ip_1|" +  (String) v.StaticIP[1] + "|input\n";
	values += "ip_2|" +  (String) v.StaticIP[2] + "|input\n";
	values += "ip_3|" +  (String) v.StaticIP[3] + "|input\n";
	values += "nm_0|" +  (String) v.Netmask[0] + "|input\n";
	values += "nm_1|" +  (String) v.Netmask[1] + "|input\n";
	values += "nm_2|" +  (String) v.Netmask[2] + "|input\n";
	values += "nm_3|" +  (String) v.Netmask[3] + "|input\n";
	values += "gw_0|" +  (String) v.Gateway[0] + "|input\n";
	values += "gw_1|" +  (String) v.Gateway[1] + "|input\n";
	values += "gw_2|" +  (String) v.Gateway[2] + "|input\n";
	values += "gw_3|" +  (String) v.Gateway[3] + "|input\n";
	values += "dhcp|" +  (String) (v.DhcpOn ? "checked" : "") + "|chk\n";
	server.send ( 200, "text/plain", values);
	if (v.Dbug > 7) Serial.println(__FUNCTION__); 
	
}


//
//   FILL THE PAGE WITH NETWORKSTATE & NETWORKS
//

void send_connection_state_values_html()
{

	String state = "N/A";
	String Networks = "";
	if (WiFi.status() == 0) state = "Idle";
	else if (WiFi.status() == 1) state = "NO SSID AVAILBLE";
	else if (WiFi.status() == 2) state = "SCAN COMPLETED";
	else if (WiFi.status() == 3) state = "CONNECTED";
	else if (WiFi.status() == 4) state = "CONNECT FAILED";
	else if (WiFi.status() == 5) state = "CONNECTION LOST";
	else if (WiFi.status() == 6) state = "DISCONNECTED";



	 int n = WiFi.scanNetworks();
 
	 if (n == 0)
	 {
		 Networks = "<font color='#FF0000'>No networks found!</font>";
	 }
	else
    {
	 
		
		Networks = "Found " +String(n) + " Networks<br>";
		Networks += "<table border='0' cellspacing='0' cellpadding='3'>";
		Networks += "<tr bgcolor='#DDDDDD' ><td><strong>Name</strong></td><td><strong>Quality</strong></td><td><strong>Enc</strong></td><tr>";
		for (int i = 0; i < n; ++i)
		{
			int quality=0;
			if(WiFi.RSSI(i) <= -100)
			{
					quality = 0;
			}
			else if(WiFi.RSSI(i) >= -50)
			{
					quality = 100;
			}
			else
			{
				quality = 2 * (WiFi.RSSI(i) + 100);
			}


			Networks += "<tr><td><a href='javascript:selssid(\""  +  String(WiFi.SSID(i))  + "\")'>"  +  String(WiFi.SSID(i))  + "</a></td><td>" +  String(quality) + "%</td><td>" +  String((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*")  + "</td></tr>";
		}
		Networks += "</table>";
	}
   
	String values ="";
	values += "connectionstate|" +  state + "|div\n";
	values += "networks|" +  Networks + "|div\n";
	server.send ( 200, "text/plain", values);
	if (v.Dbug > 7) Serial.println(__FUNCTION__); 
	
}