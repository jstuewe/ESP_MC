//
//  HTML PAGE
//

const char PAGE_AdminMainPage[] PROGMEM = R"=====(
<meta name="viewport" content="width=device-width, initial-scale=1" />
<strong>Administration</strong><br>
<title id="Title"></title>
<div id="Header">Unknown node</div>

<hr>
<a href="\"            style="width:250px" class="btn btn--m btn--blue" >Main Page</a><br>
<a href="general"      style="width:250px" class="btn btn--m btn--blue" >General Configuration</a><br>
<a href="prop"      style="width:250px" class="btn btn--m btn--blue" >Prop Configuration</a><br>
<a href="config"       style="width:250px" class="btn btn--m btn--blue" >Network Configuration</a><br>
<a href="ntp"          style="width:250px" class="btn btn--m btn--blue" >NTP Time Settings</a><br>
<a href="admin\reset"  style="width:250px" class="btn btn--m btn--blue" >Reset</a><br>

<table border="0"  cellspacing="0" cellpadding="2" >
<tr><td align="right">Node Name :</td> <td><span id="nname"></span></td></tr>
<tr><td align="right">PropName  :</td> <td><span id="pname"></span></td></tr>
<tr><td align="right">Device :</td> <td><span id="DevName"></span></td></tr>
<tr><td align="right">Program :</td> <td><span id="PrgVer"></span></td></tr>
<tr><td align="right">Wifi Quality :</td> <td><span id="WifiQual"></span>%</td></tr>
<tr><td align="right">Uptime :</td> <td> <span id="Uptime"></span></td></tr>
<tr><td align="right">Time offline :</td> <td> <span id="DropTime"></span></td></tr>
<tr><td align="right">Wifi Drops :</td> <td><span id="DropCnt"></span></td></tr>
<tr><td align="right">Prop Triggers :</td> <td> <span id="PropCnt"></span></td></tr>
</table>

<script>
window.onload = function ()
{
	load("style.css","css", function() 
	{
		load("microajax.js","js", function() 
		{
				setValues("/admin/adminvalues");
		});
	});
}
function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}

</script>

)=====";

void send_admin_values_html()
{
	String values = "";
	values += "Title|" + (String)v.DeviceName + "|div\n";
	values += "nname|" + (String)v.DeviceName + "|div\n";
	values += "pname|" + (String)v.PropName + "|div\n";
	values += "DevName|" + (String)DefaultHostName + "|div\n";
	values += "PrgVer| " + (String)VersionText + "|div\n";
	values += "Header|" + (String)HeaderString() + "|div\n";
	values += "PropCnt|" + (String)PropCnt + "|div\n";
	values += "WifiQual|" + (String)WifiQuality() + "|div\n";
	values += "Uptime|" + FormatTime(millis()) + "|div\n";
	values += "DropCnt|" + (String)DisconnectCnt + "|div\n";
	values += "DropTime|" + FormatTime(DisconnectMillis) + "|div\n";
	server.send(200, "text/plain", values);
	if (v.Dbug > 7) Serial.println(__FUNCTION__);
}
