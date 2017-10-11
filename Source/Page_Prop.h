//
//  HTML PAGE
//

const char PAGE_AdminPropSettings[] PROGMEM = R"=====(
<meta name="viewport" content="width=device-width, initial-scale=1" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<a href="admin.html"  class="btn btn--s"><</a>&nbsp;&nbsp;
<strong>Prop Settings</strong>
<title id="Title"></title>
<div id="Header">Unknown node</div>
Uptime: <span id="Uptime"></span><br>

<tr><td align="left" colspan="2"><hr></td></tr>
<form action="" method="post">
<table border="0"  cellspacing="0" cellpadding="2" >

<tr><td align="right">Prop Name          :</td> <td><input type="text" id="PropName" name="PropName" value=""></td></tr>
<tr><td align="right">Prop Argument      :</td><td><input type="text" id="PropArg" name="PropArg" size="6" </td></tr>
<tr><td align="right">Prop Flags[7:0]    :</td><td><input type="checkbox" id="f7" name="f7"> <input type="checkbox" id="f6" name="f6"> <input type="checkbox" id="f5" name="f5"> <input type="checkbox" id="f4" name="f4"> <input type="checkbox" id="f3" name="f3"> <input type="checkbox" id="f2" name="f2"> <input type="checkbox" id="f1" name="f1"> <input type="checkbox" id="f0" name="f0"></td></tr>
<tr><td align="right">UdpPropPort        :</td> <td><input type="text" id="UdpPropPort" name="UdpPropPort" size="6" </td></tr>
<tr><td align="right">Prop Re-Trigger Delay :</td><td><input type="text" id="PropTrigHoldoff" name="PropTrigHoldoff" size="6" </td></tr>
<tr><td align="left" colspan="2"><hr></td></tr>
<tr><td align="right">Prop Enabled       : </td><td><input type="checkbox" id="PropOn" name="PropOn"></td></tr>
<tr><td align="right">Prop Invert        : </td><td><input type="checkbox" id="PropInv" name="PropInv"></td></tr>
<tr><td colspan=2 align="center">Prop Delay[0:3]</td></tr>
<tr><td colspan=2 align="center"><input type="text" id="pd_0" name="pd_0" size="6"> <input type="text" id="pd_1" name="pd_1" size="6"> <input type="text" id="pd_2" name="pd_2" size="6"> <input type="text" id="pd_3" name="pd_3" value="" size="6"></td></tr>
<tr><td colspan=2 align="center">Prop OnTime[0:3]</td></tr>
<tr><td colspan=2 align="center"><input type="text" id="po_0" name="po_0" size="6"> <input type="text" id="po_1" name="po_1" size="6"> <input type="text" id="po_2" name="po_2" size="6"> <input type="text" id="po_3" name="po_3" value="" size="6"></td></tr>
<tr><td colspan=2 align="center">Prop Cycle[0:3]</td></tr>
<tr><td colspan=2 align="center"><input type="text" id="pc_0" name="pc_0" size="6"> <input type="text" id="pc_1" name="pc_1" size="6"> <input type="text" id="pc_2" name="pc_2" size="6"> <input type="text" id="pc_3" name="pc_3" value="" size="6"></td></tr>
<tr><td align="right">Prop Sound         :</td> <td><input type="text" id="PropAudName" name="PropAudName" value=""></td></tr>
<tr><td nowrap align="right"> Prop Sound Delay : </td><td><input type="text" id="PropAudDel" name="PropAudDel" size="6" </td></tr>

<tr><td align="left" colspan="2"><hr></td></tr>

<tr><td align="right">Prop GPIO Trigger  : </td><td><input type="checkbox" id="PropGPIOn" name="PropGPIOn"></td></tr>
<tr><td align="right">Prop GPIO Pin      : </td><td><input type="text" id="PropGPIPin" name="PropGPIPin" size="6" </td></tr>


<tr><td align="left" colspan="2"><hr></td></tr>
<tr><td nowrap align="right"> Sonar Enabled     : </td><td><input type="checkbox" id="SonarOn" name="SonarOn"></td></tr>
<tr><td nowrap align="right"> Sonar Min(in)     : </td><td><input type="text" id="sn" name="sn" size="6" </td></tr>
<tr><td nowrap align="right"> Sonar Max(in)     : </td><td><input type="text" id="sx" name="sx" size="6" </td></tr>
<tr><td nowrap align="right"> Sonar Trigger(in) : </td><td><input type="text" id="st" name="st" size="6" </td></tr>
<tr><td nowrap align="right"> Sonar Sensitivity(%) : </td><td><input type="text" id="ss" name="ss" size="6" </td></tr>
<tr><td nowrap align="right"> Sonar Holdoff(ms) : </td><td><input type="text" id="sh" name="sh" size="6" </td></tr>

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
				setValues("/admin/propvalues");
		});
	});
}
function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}

</script>
)=====";

void send_prop_html()
{
	
	if (server.args() > 0 )  // Save Settings
	{
		v.PropOn = false; // will always get set as below
		v.PropGPIOn = false;
		v.PropInvertSig = false;
		v.SonarOn = false;
		memset(&v.PropFlag, 0, sizeof(v.PropFlag));

		for ( uint8_t i = 0; i < server.args(); i++ ) {
			if (v.Dbug > 77) Serial.print( "(" + (String) i +":" + (String) server.argName(i) + "=" + (String) (server.arg(i)) + ")" );
			if (server.argName(i) == "PropName") strncpy(v.PropName, urldecode(server.arg(i)).c_str(), STRLEN - 1);
			if (server.argName(i) == "PropAudName") strncpy(v.PropAudioName, urldecode(server.arg(i)).c_str(), STRLEN - 1);
			if (server.argName(i) == "PropOn")   v.PropOn = true;
			if (server.argName(i) == "PropGPIOn")   v.PropGPIOn = true;
			if (server.argName(i) == "PropInv")  v.PropInvertSig = true;
			if (server.argName(i) == "SonarOn")  v.SonarOn = true;
			if (server.argName(i) == "PropTrigHoldoff")	v.PropTrigHoldoff	= server.arg(i).toInt();
			if (server.argName(i) == "UdpPropPort")		v.UdpPropPort		= server.arg(i).toInt();
			if (server.argName(i) == "PropGPIPin")		v.PropGPIPin		= server.arg(i).toInt();

			if (server.argName(i) == "f7")  v.PropFlag[7] = true;
			if (server.argName(i) == "f6")  v.PropFlag[6] = true;
			if (server.argName(i) == "f5")  v.PropFlag[5] = true;
			if (server.argName(i) == "f4")  v.PropFlag[4] = true;
			if (server.argName(i) == "f3")  v.PropFlag[3] = true;
			if (server.argName(i) == "f2")  v.PropFlag[2] = true;
			if (server.argName(i) == "f1")  v.PropFlag[1] = true;
			if (server.argName(i) == "f0")  v.PropFlag[0] = true;

			if (server.argName(i) == "pd_0") if (server.arg(i)) 	v.PropDelay[0] = server.arg(i).toInt();
			if (server.argName(i) == "pd_1") if (server.arg(i)) 	v.PropDelay[1] = server.arg(i).toInt();
			if (server.argName(i) == "pd_2") if (server.arg(i)) 	v.PropDelay[2] = server.arg(i).toInt();
			if (server.argName(i) == "pd_3") if (server.arg(i)) 	v.PropDelay[3] = server.arg(i).toInt();

			if (server.argName(i) == "po_0") if (server.arg(i)) 	v.PropOnTime[0] = server.arg(i).toInt();
			if (server.argName(i) == "po_1") if (server.arg(i)) 	v.PropOnTime[1] = server.arg(i).toInt();
			if (server.argName(i) == "po_2") if (server.arg(i)) 	v.PropOnTime[2] = server.arg(i).toInt();
			if (server.argName(i) == "po_3") if (server.arg(i)) 	v.PropOnTime[3] = server.arg(i).toInt();

			if (server.argName(i) == "pc_0") if (server.arg(i)) 	v.PropCycle[0] = server.arg(i).toInt();
			if (server.argName(i) == "pc_1") if (server.arg(i)) 	v.PropCycle[1] = server.arg(i).toInt();
			if (server.argName(i) == "pc_2") if (server.arg(i)) 	v.PropCycle[2] = server.arg(i).toInt();
			if (server.argName(i) == "pc_3") if (server.arg(i)) 	v.PropCycle[3] = server.arg(i).toInt();

			if (server.argName(i) == "PropArg") if (server.arg(i))  v.PropArgument = server.arg(i).toInt();
			if (server.argName(i) == "PropAudDel") if (server.arg(i)) v.PropAudioDelay = server.arg(i).toInt();

			if (server.argName(i) == "sn") if (server.arg(i)) 	    v.SonarMin = server.arg(i).toInt();
			if (server.argName(i) == "sx") if (server.arg(i)) 	    v.SonarMax = server.arg(i).toInt();
			if (server.argName(i) == "st") if (server.arg(i)) 	    v.SonarTrigger = server.arg(i).toInt();
			if (server.argName(i) == "ss") if (server.arg(i)) 	    v.SonarSensitivity = server.arg(i).toInt();
			if (server.argName(i) == "sh") if (server.arg(i)) 	    v.SonarHoldoff = server.arg(i).toInt();
			
		}
		WriteConfig();
	}
	server.send ( 200, "text/html", PAGE_AdminPropSettings ); 
	if (v.Dbug > 58) Serial.println(__FUNCTION__); 
	if (v.Dbug > 57) Serial.println((String) server.args() + " Web server arguments processed");
	for (int i = 0; i < 3; i++) SetRelayOff(i);
}

void send_prop_configuration_values_html()
{
	String values = "";
	values += "Title|" + (String)v.DeviceName + "|div\n";
	values += "Header|" + (String)HeaderString() + "|div\n";
	values += "Uptime|" + FormatTime(millis()) + "|div\n";
	values += "PropName|" + (String)v.PropName + "|input\n";
	values += "PropAudName|" + (String)v.PropAudioName + "|input\n";
	values += "PropOn|" + (String)(v.PropOn ? "checked" : "") + "|chk\n";
	values += "PropGPIOn|" + (String)(v.PropGPIOn ? "checked" : "") + "|chk\n";
	values += "PropInv|" + (String)(v.PropInvertSig ? "checked" : "") + "|chk\n";
    values += "SonarOn|" + (String)(v.SonarOn ? "checked" : "") + "|chk\n";
	values += "PropTrigHoldoff|" + (String) v.PropTrigHoldoff + "|input\n";
	values += "UdpPropPort|" + (String) v.UdpPropPort + "|input\n";
	values += "PropGPIPin|" + (String)v.PropGPIPin + "|input\n";
		
	values += "f7|" + (String)(v.PropFlag[7] ? "checked" : "") + "|chk\n";
	values += "f6|" + (String)(v.PropFlag[6] ? "checked" : "") + "|chk\n";
	values += "f5|" + (String)(v.PropFlag[5] ? "checked" : "") + "|chk\n";
	values += "f4|" + (String)(v.PropFlag[4] ? "checked" : "") + "|chk\n";
	values += "f3|" + (String)(v.PropFlag[3] ? "checked" : "") + "|chk\n";
	values += "f2|" + (String)(v.PropFlag[2] ? "checked" : "") + "|chk\n";
	values += "f1|" + (String)(v.PropFlag[1] ? "checked" : "") + "|chk\n";
	values += "f0|" + (String)(v.PropFlag[0] ? "checked" : "") + "|chk\n";

	values += "pd_0|" + (String)v.PropDelay[0] + "|input\n";
	values += "pd_1|" + (String)v.PropDelay[1] + "|input\n";
	values += "pd_2|" + (String)v.PropDelay[2] + "|input\n";
	values += "pd_3|" + (String)v.PropDelay[3] + "|input\n";

	values += "po_0|" + (String)v.PropOnTime[0] + "|input\n";
	values += "po_1|" + (String)v.PropOnTime[1] + "|input\n";
	values += "po_2|" + (String)v.PropOnTime[2] + "|input\n";
	values += "po_3|" + (String)v.PropOnTime[3] + "|input\n";

	values += "pc_0|" + (String)v.PropCycle[0] + "|input\n";
	values += "pc_1|" + (String)v.PropCycle[1] + "|input\n";
	values += "pc_2|" + (String)v.PropCycle[2] + "|input\n";
	values += "pc_3|" + (String)v.PropCycle[3] + "|input\n";

	values += "PropArg|" + (String)v.PropArgument + "|input\n";
	values += "PropAudDel|" + (String)v.PropAudioDelay + "|input\n";

	values += "sn|" + (String)v.SonarMin + "|input\n";
	values += "sx|" + (String)v.SonarMax + "|input\n";
	values += "st|" + (String)v.SonarTrigger + "|input\n";
	values += "ss|" + (String)v.SonarSensitivity + "|input\n";
	values += "sh|" + (String)v.SonarHoldoff + "|input\n";

	server.send(200, "text/plain", values);
	if (v.Dbug > 17) Serial.println(__FUNCTION__);
	if (v.Dbug > 15) Serial.println("Web Config fill: ");
	if (v.Dbug > 20) Serial.println(values);
}