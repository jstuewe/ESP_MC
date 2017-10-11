
const char PAGE_NTPConfiguration[] PROGMEM = R"=====(
<meta name="viewport" content="width=device-width, initial-scale=1" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<a href="admin.html"  class="btn btn--s"><</a>&nbsp;&nbsp;<strong>NTP Settings</strong>
<hr>
<form action="" method="get">
<table border="0"  cellspacing="0" cellpadding="3" >
<tr><td align="right">NTP Server:</td><td><input type="text" id="ntpserver" name="ntpserver" maxlength="172" value=""></td></tr>
<tr><td align="right">Update:</td><td><input type="text" id="update" name="update" size="3"maxlength="6" value=""> minutes (0=disable)</td></tr>
<tr><td>TimeZone</td><td>
<select  id="tz" name="tz">
	<option value="-12">(GMT-12:00)</option>
	<option value="-11">(GMT-11:00)</option>
	<option value="-10">(GMT-10:00)</option>
	<option value="-9">(GMT-09:00)</option>
	<option value="-8">(GMT-08:00)</option>
	<option value="-7">(GMT-07:00)</option>
	<option value="-6">(GMT-06:00)</option>
	<option value="-5">(GMT-05:00)</option>
	<option value="-4">(GMT-04:00)</option>
	<option value="-3">(GMT-03:00)</option>
	<option value="-2">(GMT-02:00)</option>
	<option value="-1">(GMT-01:00)</option>
	<option value="0">(GMT+00:00)</option>
	<option value="1">(GMT+01:00)</option>
	<option value="2">(GMT+02:00)</option>
	<option value="3">(GMT+03:00)</option>
	<option value="4">(GMT+04:00)</option>
	<option value="5">(GMT+05:00)</option>
	<option value="6">(GMT+06:00)</option>
	<option value="7">(GMT+07:00)</option>
	<option value="8">(GMT+08:00)</option>
	<option value="9">(GMT+09:00)</option>
	<option value="10">(GMT+10:00)</option>
	<option value="11">(GMT+11:00)</option>
	<option value="12">(GMT+12:00)</option>
	<option value="12">(GMT+12:00)</option>
	<option value="13">(GMT+13:00)</option>
</select>
</td></tr>
<tr><td align="right">DstActive saving:</td><td><input type="checkbox" id="dst" name="dst"></td></tr>
<tr><td colspan="2" align="center"><input type="submit" style="width:150px" class="btn btn--m btn--blue" value="Save"></td></tr>
</table>
</form>
<script>
	

window.onload = function ()
{
	load("style.css","css", function() 
	{
		load("microajax.js","js", function() 
		{
				setValues("/admin/ntpvalues");
		});
	});
}
function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}



</script>
)=====";


void send_NTP_configuration_html()
{
	
	 
	if (server.args() > 0 )  // Save Settings
	{
		v.DstActive = false;
		String temp = "";
		for ( uint8_t i = 0; i < server.args(); i++ ) {
			if (server.argName(i) == "ntpserver") strncpy(v.NtpServerName,urldecode(server.arg(i)).c_str(),STRLEN-1); 
			if (server.argName(i) == "update") v.NtpServerUpdateInterval =  server.arg(i).toInt(); 
			if (server.argName(i) == "tz") v.TimeZone =  server.arg(i).toInt(); 
			if (server.argName(i) == "dst") v.DstActive = true; 
		}
		WriteConfig();
		SynchNtp = true;
	}
	server.send ( 200, "text/html", PAGE_NTPConfiguration ); 
	if (v.Dbug > 7) Serial.println(__FUNCTION__); 
	
}






void send_NTP_configuration_values_html()
{
		
	String values ="";
	values += "ntpserver|" + (String) v.NtpServerName + "|input\n";
	values += "update|" +  (String) v.NtpServerUpdateInterval + "|input\n";
	values += "tz|" +  (String) v.TimeZone + "|input\n";
	values += "dst|" +  (String) (v.DstActive ? "checked" : "") + "|chk\n";
	server.send ( 200, "text/plain", values);
	if (v.Dbug > 7) Serial.println(__FUNCTION__); 
	
}
