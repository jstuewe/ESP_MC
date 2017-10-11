//
//  General Settings HTML PAGE
//


const char PAGE_AdminGeneralSettings[] PROGMEM =  R"=====(
<meta name="viewport" content="width=device-width, initial-scale=1" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<a href="admin.html"  class="btn btn--s"><</a>&nbsp;&nbsp;
<strong>General Settings</strong>
<title id="Title"></title>
<div id="Header">Unknown node</div>
Uptime: <span id="Uptime"></span><br>

<hr>
<form action="" method="post">
<table border="0"  cellspacing="0" cellpadding="3" >
<tr><td align="right">Name of Node</td>	<td><input type="text" id="Nodename" name="Nodename" value=""></td></tr>
<tr><td align="left" colspan="2"><hr></td></tr> 
<tr><td align="right"> Debug Level:</td> <td><input type="text" id="Dbug" name="Dbug" size="4" </td></tr>
<tr><td align="right"> Status Interval:</td><td><input type="text" id="StatInt" name="StatInt" size="4" </td> </tr>
<tr><td align="right"> Global Enable:</td><td><input type="checkbox" id="GblEnOn" name="GblEnOn"></td></tr>
<tr><td align="right"> Led Enabled:</td><td><input type="checkbox" id="LedOn" name="LedOn"></td></tr>
<tr><td align="right"> Identify Enabled:</td><td><input type="checkbox" id="IdOn" name="IdOn"></td></tr>
<tr><td align="right"> Beep Enabled:</td><td><input type="checkbox" id="BeepOn" name="BeepOn"></td></tr>
<tr><td align="right"> IR Remote Enabled:</td><td><input type="checkbox" id="IrOn" name="IrOn"></td></tr>
<tr><td align="right"> DFPlayer Enabled:</td><td><input type="checkbox" id="AudOn" name="AudOn"></td></tr>
<tr><td align="right"> Temp Enabled:</td><td><input type="checkbox" id="TempOn" name="TempOn"></td></tr>
<tr><td align="right"> Serial Enabled:</td><td><input type="checkbox" id="SerOn" name="SerOn"></td></tr>
<tr><td align="right"> UDP Messaging Enabled:</td><td><input type="checkbox" id="UdpOn" name="UdpOn"></td></tr>
<tr><td align="right"> Udp Port:</td><td><input type="text" id="UdpPort" name="UdpPort" size="7" </td></tr>
<tr><td align="right"> Udp Status Node:</td><td><input type="text" id="UdpStatNode" name="UdpStatNode" size="7" </td></tr>
<tr><td align="right"> OTA Updates Enabled:</td><td><input type="checkbox" id="OtaOn" name="OtaOn"></td></tr>
<tr><td align="right"> Web Access Enabled:</td><td><input type="checkbox" id="WebOn" name="WebOn"></td></tr>
<tr><td align="right"> Config AP Enabled:</td><td><input type="checkbox" id="ConfigAPOn" name="ConfigAPOn"></td></tr>
<tr><td align="right"> Config AP Timeout (min):</td><td><input type="text" id="ConfigTimeout" name="ConfigTimeout" size="4" ></td></tr>
<tr><td align="right"> Thingspeak Posting Enabled:</td><td><input type="checkbox" id="TsPostOn" name="TsPostOn"></td></tr>
<tr><td align="right"> Thingspeak Post Interval:</td><td><input type="text" id="TsPostInt" name="TsPostInt" size="4" </td></tr>
<tr><td align="right"> Thingspeak Server:</td><td><input type="text" id="TSServerName" name="TSServerName" value=""></td></tr>
<tr><td align="right"> Thingspeak API Key:</td><td><input type="text" id="TSApiKey" name="TSApiKey" value=""></td></tr>

<tr><td colspan="2" align="center"><input type="submit" style="width:150px" class="btn btn--m btn--blue" value="Save"></td></tr>
<tr><td colspan="2" align="center"><a href="admin\reset"  style="width:130px"  class="btn btn--m btn--blue" >Reset Node</a><br></tr>
<tr><td colspan="2" align="center"><a href="admin\default"  style="width:130px"  class="btn btn--m btn--red" >Reset Defaults</a><br></tr>
</table>
</form>
<script>
 

window.onload = function ()
{
	load("style.css","css", function() 
	{
		load("microajax.js","js", function() 
		{
				setValues("/admin/generalvalues");
		});
	});
}
function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}

</script>
)=====";


// Functions for this Page
void send_devicename_value_html()
{
		
	String values ="";
	values += "Nodename|" + (String) v.DeviceName + "|div\n";
	server.send (60, "text/plain", values);
	if (v.Dbug > 24) Serial.println(__FUNCTION__); 
	
}

void send_general_html()
{
	
	if (server.args() > 0 )  // Save Settings
	{
		v.GlobalEnableOn = false;
		v.SerOn = false;
		v.UdpOn = false;
		v.LedOn = false;
		v.BeepOn = false;
		v.IdentifyOn = false;
		v.IrRemoteOn = false;
		v.DFPlayerOn = false;
		v.TempOn = false;
		v.TsPostOn = false;
		v.OtaOn = false;
		v.WebOn = false;
		v.ConfigAPOn = false;


		for ( uint8_t i = 0; i < server.args(); i++ ) {
			if (v.Dbug > 25) Serial.print( "(" + (String) i +":" + (String) server.argName(i) + "=" + (String) (server.arg(i)) + ")" );
			if (server.argName(i) == "Dbug")        v.Dbug = server.arg(i).toInt();
			if (server.argName(i) == "StatInt")     v.StatusInterval = server.arg(i).toInt();
			if (server.argName(i) == "UdpPort")     v.UdpPort = server.arg(i).toInt();
			if (server.argName(i) == "UdpStatNode") v.UdpStatNode = server.arg(i).toInt();
			if (server.argName(i) == "TsPostInt")   v.TsPostInt = server.arg(i).toInt();
			if (server.argName(i) == "GblEnOn")       v.GlobalEnableOn = true;
			if (server.argName(i) == "SerOn")         v.SerOn = true;
			if (server.argName(i) == "UdpOn")         v.UdpOn = true;
			if (server.argName(i) == "BeepOn")        v.BeepOn = true;
			if (server.argName(i) == "IdOn")          v.IdentifyOn = true;
			if (server.argName(i) == "LedOn")         v.LedOn = true;
			if (server.argName(i) == "IrOn")		  v.IrRemoteOn = true; // needs a reset to work properly
			if (server.argName(i) == "AudOn")		  v.DFPlayerOn = true; // needs a reset to work properly
			if (server.argName(i) == "TempOn")        v.TempOn = true;
			if (server.argName(i) == "TsPostOn")      v.TsPostOn = true;
			if (server.argName(i) == "OtaOn")         v.OtaOn = true;
			if (server.argName(i) == "WebOn")         v.WebOn = true;
			if (server.argName(i) == "ConfigAPOn")    v.ConfigAPOn = true;
			if (server.argName(i) == "ConfigTimeout") v.ConfigTimeout = server.arg(i).toInt();
			if (server.argName(i) == "TSServerName") strncpy(v.TSServerName, urldecode(server.arg(i)).c_str(), STRLEN - 1);
			if (server.argName(i) == "TSApiKey")     strncpy(v.TSApiKey,     urldecode(server.arg(i)).c_str(), STRLEN - 1);
			if (server.argName(i) == "Nodename")     strncpy(v.DeviceName,   urldecode(server.arg(i)).c_str(), STRLEN - 1);
		}
		WriteConfig();
		if (!v.LedOn) digitalWrite(BUILTIN_LED, HIGH); // ensure LED is off when disabled
	}
	server.send ( 200, "text/html", PAGE_AdminGeneralSettings ); 
	if (v.Dbug > 58) Serial.println(__FUNCTION__); 
	if (v.Dbug > 57) Serial.println((String) server.args() + " Web server arguments processed");
	
	
}

void send_general_configuration_values_html()
{
	String values = "";
	values += "Title|" + (String)v.DeviceName + "|div\n";
	values += "Header|" + (String)HeaderString() + "|div\n";
	values += "Uptime|" + FormatTime(millis()) + "|div\n";
	values += "Nodename|" + (String)v.DeviceName + "|input\n";
	values += "Dbug|" + (String)v.Dbug + "|input\n";
	values += "StatInt|" + (String)v.StatusInterval + "|input\n";
	values += "GblEnOn|" + (String)(v.GlobalEnableOn ? "checked" : "") + "|chk\n";
	values += "UdpPort|" + (String)v.UdpPort + "|input\n";
	values += "UdpStatNode|" + (String)v.UdpStatNode + "|input\n";
	values += "TsPostInt|" + (String)v.TsPostInt + "|input\n";
	values += "SerOn|" + (String)(v.SerOn ? "checked" : "") + "|chk\n";
	values += "UdpOn|" + (String)(v.UdpOn ? "checked" : "") + "|chk\n";
	values += "BeepOn|" + (String)(v.BeepOn ? "checked" : "") + "|chk\n";
	values += "LedOn|" + (String)(v.LedOn ? "checked" : "") + "|chk\n";
	values += "IdOn|" + (String)(v.IdentifyOn ? "checked" : "") + "|chk\n";
	values += "TempOn|" + (String)(v.TempOn ? "checked" : "") + "|chk\n";
	values += "IrOn|" + (String)(v.IrRemoteOn ? "checked" : "") + "|chk\n";
	values += "AudOn|" + (String)(v.DFPlayerOn ? "checked" : "") + "|chk\n"; 
	values += "TsPostOn|" + (String)(v.TsPostOn? "checked" : "") + "|chk\n";
	values += "OtaOn|" + (String)(v.OtaOn? "checked" : "") + "|chk\n";
	values += "WebOn|" + (String)(v.WebOn ? "checked" : "") + "|chk\n";
	values += "ConfigAPOn|" + (String)(v.ConfigAPOn ? "checked" : "") + "|chk\n";
	values += "ConfigTimeout|" +   (String) v.ConfigTimeout +  "|input\n";
	values += "TSServerName|" + (String) v.TSServerName + "|input\n";
	values += "TSApiKey|" + (String) v.TSApiKey + "|input\n";
	server.send(200, "text/plain", values);
	if (v.Dbug > 17) Serial.println(__FUNCTION__);
	if (v.Dbug > 15) Serial.println("Web Config fill: ");
	if (v.Dbug > 20) Serial.println(values);
}