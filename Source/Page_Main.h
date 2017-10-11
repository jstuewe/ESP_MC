
const char PAGE_Main[] PROGMEM = R"=====(
<meta name="viewport" content="width=device-width, initial-scale=1" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<h1>Haunt Master Control</h1>     
<meta http-equiv='refresh' content='15'/>
<style> body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; } </style>
<title id="Title"></title>
<div id="Header">Unknown node</div>

<table border="0"  cellspacing="0" cellpadding="2" >
<tr><td align="right">Node Name :</td> <td><span id="nname"></span></td></tr>
<tr><td align="right">PropName  :</td> <td><span id="pname"></span></td></tr>
<tr><td align="right">Uptime    :</td> <td><span id="Uptime"></span></td></tr>
</table>
<hr>
<span class="nowrap" id="buf5"></span><br>
<span class="nowrap" id="buf4"></span><br>
<span class="nowrap" id="buf3"></span><br>
<span class="nowrap" id="buf2"></span><br>
<span class="nowrap" id="buf1"></span><br>
<hr>
<a href=admin   style="width:150px" align="center" class="btn btn--m btn--blue" >Admin Menu</a><br>

<script>                

		window.onload = function ()
		{
			load("style.css","css", function() 
			{
				load("microajax.js","js", function() 
				{
						setValues("/admin/mainvalues");  //-- this function calls the function on the ESP      
				});
			});
		}
		function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}
   </script>
)=====";

void send_main_configuration_values_html()
{
	String values = "";
	values += "Title|" + (String)v.DeviceName + "|div\n";
	values += "Header|" + (String)HeaderString() + "|div\n";
	values += "Uptime|" + FormatTime(millis()) + "|div\n";
	values += "nname|" + (String)v.DeviceName + "|div\n";
	values += "pname|" + (String)v.PropName + "|div\n";
	values += "buf1|" + hBuff[1] + "|div\n";
	values += "buf2|" + hBuff[2] + "|div\n";
	values += "buf3|" + hBuff[3] + "|div\n";
	values += "buf4|" + hBuff[4] + "|div\n";
	values += "buf5|" + hBuff[5] + "|div\n";
	values += "buf6|" + hBuff[6] + "|div\n";
	server.send(60, "text/plain", values);
}

void send_main_html() {        
    if (server.args() > 0 ) { // Are there any POST/GET Fields ? 
		for (uint8_t i = 0; i < server.args(); i++) {
			//No parameters on the main page
			}
//		WriteConfig();
		}
	 server.send(200, "text/html", PAGE_Main);
	 if (v.Dbug > 58) Serial.println(__FUNCTION__);
	 if (v.Dbug > 57) Serial.println((String)server.args() + " Web server arguments processed");
 }
