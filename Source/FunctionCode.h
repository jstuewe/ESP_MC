//Function code.  Header file vs. C file to be able to utilize globals. (I know, not good coding, but it works -JS)

const char Page_WaitAndReload[] PROGMEM = R"=====(
<meta http-equiv="refresh" content="10; URL=\admin">
 Please Wait....Configuring and Restarting.
)=====";

//Prints to many places
void dPrint(const char* c) {  //=================================================================================
	if (v.SerOn) {
		Serial.print(c);
		Serial.print("\r\n");
	}
	//	if (TelnetServerClient && TelnetServerClient.connected()) {	TelnetServerClient.print(c); } //sample from telnet. not implemented in this program
	if (v.UdpOn) {
		// transmit broadcast package
//		udp.beginPacket(IPAddress(udp_ip[0], udp_ip[1], udp_ip[2], v.UdpStatNode), v.UdpPort);
		udp.beginPacket(udp_ip, v.UdpPort);
		udp.print(c);
		udp.endPacket();
	}

	for (int i = 1; i <= NUMMESSAGES-1 ; i++) {
		hBuff[i - 1].reserve(hBuff[i].length());
		hBuff[i - 1] = hBuff[i];
		}
	hBuff[NUMMESSAGES - 1].reserve(strlen(c) + 1); //Include space for \0
	hBuff[NUMMESSAGES - 1] = c;
}


char* HeaderString() { //=================================================================================
	static char MyHeaderStr[50];
	sprintf(MyHeaderStr, "%s@%i.%i %s(%s)", TimeString(), WiFi.localIP()[2], WiFi.localIP()[3], v.DeviceName,v.PropName);
	//	sprintf(MyHeaderStr, "%s %s", VersionText, " ESP_TS+WebCfg ");
	return(MyHeaderStr);
}


void ResetESPNow() {
	snprintf(sBuff, SBUFLEN, "%s Manual Reset requested. Resetting...", HeaderString());
	dPrint(sBuff);
	if(v.WebOn) server.send(200, "text/html", Page_WaitAndReload);
	delay(500);
	if (digitalRead(0)  != 1)	Serial.println("Error: Pin 0 (D2) is not high. Setting it");
	if (digitalRead(2)  != 1) 	Serial.println("Error: Pin 2 (D4-LED) is not high. Setting it");
	if (digitalRead(15) != 0) 	Serial.println("Error: Pin 15 (D8) is not low. Setting it");
	pinMode(0, OUTPUT);
	digitalWrite(0, HIGH); // must be high on reset to stay out of bootloader mode
	pinMode(2, OUTPUT);
	digitalWrite(2, HIGH); // must be high on reset to boot in run mode
	pinMode(15, OUTPUT);
	digitalWrite(15, LOW); // must be low on reset to stay out of SDIO mode
	delay(100);
	while (digitalRead(0) != 1)  { Serial.println("Error: Pin 0 (D2) is not high"); delay(300); };
	while (digitalRead(2) != 1)  { Serial.println("Error: Pin 2 (D4) is not high"); delay(300); };
	while (digitalRead(15) != 0) { Serial.println("Error: Pin 15 (D8) is not low"); delay(300); };
	Serial.println("Reseting now. Pins 0, 2, & 15 are OK.");
	ESP.reset();
	delay(100);
	Serial.println("Reset failed!!");
	while (1);  //hang here if reset failed, this will cause additional WDT reset
	// note  ets Jan  8 2013,rst cause:2, boot mode:(3,6)			is what we want to see
	// note  ets Jan  8 2013,rst cause:4, boot mode:(1,7)			is a failure
	// note  ets Jan  8 2013, rst cause : 2->4, boot mode : (1, 6)	is a failure
	// testing. if D8 is high I get garbage on reset
}

String GetMacAddress()
{
	uint8_t mac[6];
	char macStr[18] = { 0 };
	WiFi.macAddress(mac);
	sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return  String(macStr);
}

String FormatTime(unsigned long MyMillis)
{
	char TimeStr[12] = { 0 };
	unsigned long Seconds = MyMillis / 1000;
	sprintf(TimeStr, "%02lu:%02lu:%02lu", Seconds / 60 /60, Seconds/60 % 60, Seconds%60 );
	return  String(TimeStr);
}

// convert a single hex digit character to its integer value (from https://code.google.com/p/avr-netino/)
unsigned char h2int(char c) { //=================================================================================
	if (c >= '0' && c <= '9') {	return((unsigned char)c - '0');	}
	if (c >= 'a' && c <= 'f') {	return((unsigned char)c - 'a' + 10); }
	if (c >= 'A' && c <= 'F') {	return((unsigned char)c - 'A' + 10); }
	return(0);
}

// Check the Values is between 0-255
boolean checkRange(String Value) { //=================================================================================
	if (Value.toInt() < 0 || Value.toInt() > 255) { return false; }
	else { return true;	}
}

// (based on https://code.google.com/p/avr-netino/)
String urldecode(String input) { //=================================================================================
	char c;
	String ret = "";
	for (byte t = 0; t<input.length(); t++)
	{
		c = input[t];
		if (c == '+') c = ' ';
		if (c == '%') {
			t++;
			c = input[t];
			t++;
			c = (h2int(c) << 4) | h2int(input[t]);
		}
		ret.concat(c);
	}
	return ret;
}

int WifiQuality() { //=================================================================================
	int myRSSI = WiFi.RSSI();
	if (myRSSI<-100 || myRSSI == 0) myRSSI = -100; else if (myRSSI>-50) myRSSI = -50;
	return(2 * (myRSSI + 100));  // convert to %
}

void ConfigureWifi() //=================================================================================
{
	Serial.printf("Connectiong to WIFI SSID:%s, PW:%s\n", v.Ssid, v.Password);
	WiFi.begin(v.Ssid, v.Password);  //ESP likes begin before configure
	if (!v.DhcpOn) { WiFi.config(IPAddress(v.StaticIP[0], v.StaticIP[1], v.StaticIP[2], v.StaticIP[3]), IPAddress(v.Gateway[0], v.Gateway[1], v.Gateway[2], v.Gateway[3]), IPAddress(v.Netmask[0], v.Netmask[1], v.Netmask[2], v.Netmask[3])); }
	WiFi.hostname(v.DeviceName);
//SDK call not found	wifi_set_phy_mode(PHY_MODE_11N); //    PHY_MODE_11B   = 1, PHY_MODE_11G = 2,	PHY_MODE_11N = 3
//SDK call not found WiFi.setphymode(3);
//SDK call not found Serial.print(WiFi.getmode());

	unsigned long T0MillisCW = millis();
	
	while (WiFi.status() != WL_CONNECTED && (millis() - T0MillisCW < 10000)) delay(100); //wait up to 10 seconds for connection
	
	int MyWifiQuality = WifiQuality();
	Serial.println("Status: " + String(str_status[WiFi.status()]) +", Mode: " + String(str_mode[WiFi.getMode()]) + ", Signal:" + MyWifiQuality + "%");
	if (WiFi.status() == WL_CONNECTED) Serial.printf("IP Address: %s, Host Name:%s\n", WiFi.localIP().toString().c_str(), v.DeviceName);
	if (v.Dbug>5) WiFi.printDiag(Serial);
}



void KillBuzzer() {  //=================================================================================
	analogWrite(BUZZERPIN, 0); //disables PWM in ESP8266
}

void SetBuzzer(int BuzzFreq, int BuzzDelay) {  //=================================================================================
	if (!v.BeepOn) return;
#define BUZZDELAY 500       //ms to buzz
#define BUZZFREQ  500       //hz buzzer frequency

	if (BuzzDelay < 50) BuzzDelay = BUZZDELAY;
	if (BuzzFreq) {
		if (v.Dbug>5) Serial.printf("(B%i)", BuzzFreq); // buzzer always sounds the same frequency. need spkr, not piezo
		analogWrite(BUZZERPIN, BuzzFreq); //enables software PWM on the given pin. PWM may be used on pins 0 to 15
		BuzzTicker.once_ms(BuzzDelay, KillBuzzer);
	}
	else {
		KillBuzzer();
	}
}

void BlinkLed() { //=================================================================================
	int led_state; // note setting output LOW turns the LED on
	unsigned long LedMillis = millis() - LocalTimeOffset; // synchronize all modules LED signal
	if (v.IdentifyOn) {
		led_state = ((LedMillis % 1500) < 750 ) ? LOW : HIGH; // 50% duty cycle once at one hertz
	}
	else if (WiFi.status() == WL_CONNECTED) {
		led_state = ((LedMillis % 2000) < 20 && millis() % 10 == 0) ? LOW : HIGH; // Short blink every second (Second mod kills pulses to controls brightness)
	}
	else {
//		led_state = ((LedMillis % 333) < 111) ? LOW : HIGH;// AP Mode or client failed quick blink 111ms on each 1/3sec
		led_state = ((LedMillis % 200) < 75) ? LOW : HIGH;// AP Mode or client failed quick blink 111ms on each 1/3sec
	}
	digitalWrite(BUILTIN_LED, led_state);
}

int ReadConfig() { //=================================================================================
				   // To make sure there are settings, and they are YOURS!
				   // If nothing is found it will use the default settings.
	if (EEPROM.read(EE_START + sizeof(v) - 1) == EE_CONFIG_KEY[3] &&
		EEPROM.read(EE_START + sizeof(v) - 2) == EE_CONFIG_KEY[2] &&
		EEPROM.read(EE_START + sizeof(v) - 3) == EE_CONFIG_KEY[1] &&
		EEPROM.read(EE_START + sizeof(v) - 4) == EE_CONFIG_KEY[0])
	{ // reads settings from EEPROM
		unsigned int t;
		for (t = 0; t<sizeof(v); t++) {
			*((char*)&v + t) = EEPROM.read(EE_START + t); 
			if (v.Dbug >75) {
				char MyChar = *((char*)&v + t);
				Serial.print(" "); //or Serial.printf(" %i:", t);
				if (isAlphaNumeric(MyChar)) { Serial.write(MyChar); }
				else { Serial.print((int) MyChar); }
				if ((t != 0 && t % 40 == 0) || (t == sizeof(v) - 1)) Serial.println(" ");
			}
		}
		Serial.print(sizeof(v));
		Serial.println(" Bytes of variables retrieved from EEprom, config key "+ (String)v.Config_Key);
		return(1); //success
	}
	else {
		Serial.print("EEprom variable Store invalid. ");
		Serial.write(EEPROM.read(EE_START + sizeof(v) - 4));
		Serial.write(EEPROM.read(EE_START + sizeof(v) - 3));
		Serial.write(EEPROM.read(EE_START + sizeof(v) - 2));
		Serial.write(EEPROM.read(EE_START + sizeof(v) - 1));
		Serial.print(" does not equal ");
		Serial.print(EE_CONFIG_KEY);
		Serial.println(".  Loading defaults");
		return(0); //failure
	}
}

void WriteConfig() { //=================================================================================
	for (unsigned int t = 0; t < sizeof(v); t++)
	{
		char MyChar = *((char*)&v + t);
		EEPROM.write(EE_START + t, MyChar);
		if (v.Dbug >75) {
			Serial.print(" "); //or Serial.printf(" %i:", t);
			if (isAlphaNumeric(MyChar)) { Serial.write(MyChar); }
			else { Serial.print((int)MyChar); }
			if ((t != 0 && t % 40 == 0) || (t == sizeof(v)-1)) Serial.println(" ");
		}
	}
	EEPROM.commit();
	if (EEPROM.read(EE_START + sizeof(v) - 1) == v.Config_Key[3] &&
		EEPROM.read(EE_START + sizeof(v) - 2) == v.Config_Key[2] &&
		EEPROM.read(EE_START + sizeof(v) - 3) == v.Config_Key[1] &&
		EEPROM.read(EE_START + sizeof(v) - 4) == v.Config_Key[0])
	{
		if (v.Dbug > 15) {
			Serial.print("Settings saved to EEprom. ");
			Serial.print(sizeof(v));
			Serial.print(" bytes\r\n");
		}
	}
	else {
		snprintf(sBuff, SBUFLEN, "%s ERROR writing EEprom variable data!!!!!", HeaderString());
		dPrint(sBuff);
	}
}

void ResetEpromValues(void) { //=================================================================================
	EEPROM.write((EE_START + sizeof(v) - 2), 35);  // corrupt key so EEprom defaults will be loaded
	EEPROM.commit();
	ResetNow = true; 
} 

int GPILogicToPhysical(int WemosPin) {
	switch (WemosPin) {
		case 0: return(16);
		case 1: return(5); // Wemos D1 is GPIO5 on wemos pin20
		case 2: return(4);
		case 3: return(0);
		case 4: return(2);
		case 5: return(14);
		case 6: return(12);
		case 7: return(13);
		case 8: return(15);
	}
	return(0); // todo: return error code.  Nothing checks it now so it defaults to Wemos D3 at GPIO0
}

void CheckUdp(void) { //=================================================================================
	if (udp.parsePacket()) // if data in incoming UDP packet
	{
		int udp_len = udp.read(udp_pbuffer, 1024);
		if (v.Dbug>120) Serial.println("(x:" + String(udp_len) + ")");
		if (!v.UdpOn) return; // bail if we're not looking for UDP.  We did clear the buffers though
		if (udp_len > 0) udp_pbuffer[udp_len] = 0;
		// Note: Printing everything results in a UDP storm with more than one node
		//	snprintf(sBuff, SBUFLEN, "%s UDP MSG from %s: %s", HeaderString(), udp.remoteIP().toString().c_str(), udp_pbuffer);
		//	dPrint(sBuff);
		//	udp.beginPacket(udp.remoteIP(), udp.remotePort()); // send a reply packet back
		//	udp.write(udp_preply);
		//	udp.endPacket();
	}
}

void CheckUdpProp(void) { //=================================================================================
	if (UdpProp.parsePacket()) // if data in incoming UDP packet
	{
		int udp_len = UdpProp.read(udp_pbuffer, 1024);
		if (udp_len > 0) udp_pbuffer[udp_len] = 0;
		
		char  uCmd = udp_pbuffer[0]; // first character is the command "T"= Trigger "C"= command, "S"= sound
		char *uAdr = strtok(udp_pbuffer + 2, " ,"); // node or prop name
		char *uArg = strtok(NULL, " ,"); // variable after node or prop name
		if (v.Dbug > 95) {
			snprintf(sBuff, SBUFLEN, "%s UDP MSG from %s: uCmd=%c, uAdr=%s, uArg=%s,%i", HeaderString(), UdpProp.remoteIP().toString().c_str(), uCmd,uAdr,uArg, atoi(uArg));
			dPrint(sBuff);
		}
		else if (uCmd == 'T' && strcasecmp(uAdr,v.PropName)==0 ) {
			PropTriggered = true;
			PropArgument = atoi(uArg);
			if (v.Dbug > 13) {
				snprintf(sBuff, SBUFLEN, "%s UDP MSG from %s: Prop '%s' Trigger accepted. Argument= %i", HeaderString(), UdpProp.remoteIP().toString().c_str(), v.PropName, PropArgument);
				dPrint(sBuff);
			}
		}
		if (uCmd == 'L') {
			Serial.print("LocalTimeOffset being called, watch out");
			if (v.Dbug > 50) {
				snprintf(sBuff, SBUFLEN, "%s LocalTimeOffset = %i", HeaderString(), LocalTimeOffset);
				dPrint(sBuff);
			}
			LocalTimeOffset = millis() % 3000; // Every prop logs what fraction of a second their MCU clock is at when UDP MSG recevied. Used to synchronize LEDs  Identify@1.5 & blink@2 sec LCD=6
		}
		else if (uCmd == 'C'  && strcasecmp(uAdr, "GlobalEnableOn") == 0) {
			v.GlobalEnableOn = atoi(uArg);
			//			WriteConfig();
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s UDP Command received: GlobalEnableOn = %s(%s,%i)", HeaderString(), (v.GlobalEnableOn) ? "True" : "False",uArg,atoi(uArg));
				dPrint(sBuff);
			}
		}
		else if (uCmd == 'C'  && strcasecmp(uAdr, "LedOn") == 0) {
			v.LedOn = atoi(uArg);
//			WriteConfig();
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s UDP Command received: LedOn = %s", HeaderString(), (v.LedOn) ? "True" : "False");
				dPrint(sBuff);
			}
		}
		else if (uCmd == 'C'  && strcasecmp(uAdr, "IdentifyOn") == 0) {
			v.IdentifyOn = atoi(uArg);
			//			WriteConfig();
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s UDP Command received: IdentifyOn = %s", HeaderString(), (v.IdentifyOn) ? "True" : "False");
				dPrint(sBuff);
			}
		}
		else if (uCmd == 'C'  && strcasecmp(uAdr, "Dbug") == 0) {
			v.Dbug = atoi(uArg);
			//			WriteConfig();
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s UDP Command received: Dbug = %i", HeaderString(), v.Dbug);
				dPrint(sBuff);
			}
		}
		
		else if (uCmd == 'C'  && strcasecmp(uAdr, "StatusInterval") == 0) {
			v.StatusInterval = atoi(uArg);
			//			WriteConfig();
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s UDP Command received: StatusInterval = %i", HeaderString(), v.StatusInterval);
				dPrint(sBuff);
			}
		}
		else if (uCmd == 'C'  && strcasecmp(uAdr, "ConfigAPOn") == 0) {
			v.ConfigAPOn = atoi(uArg);
			//			WriteConfig();
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s UDP Command received: ConfigAPOn = %i", HeaderString(), v.ConfigAPOn);
				dPrint(sBuff);
			}
		}
		else if (uCmd == 'C'  && strcasecmp(uAdr, "WifiPower") == 0) {
			v.WifiPower = atoi(uArg);
			//			WriteConfig();
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s UDP Command received: WifiPower = %i", HeaderString(), v.WifiPower);
				dPrint(sBuff);
			}
		}
		else if (uCmd == 'C'  && strcasecmp(uAdr, "SerOn") == 0) {
			v.SerOn = atoi(uArg);
			//			WriteConfig();
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s UDP Command received: SerOn = %i", HeaderString(), v.SerOn);
				dPrint(sBuff);
			}
		}
		else if (uCmd == 'C'  && strcasecmp(uAdr, "UdpOn") == 0) {
			v.UdpOn = atoi(uArg);
			//			WriteConfig();
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s UDP Command received: UdpOn = %i", HeaderString(), v.UdpOn);
				dPrint(sBuff);
			}
		}
		else if (uCmd == 'C'  && strcasecmp(uAdr, "Reboot") == 0) {
			ResetNow = true;;
			//			WriteConfig();
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s UDP Command received: Module rebooting", HeaderString());
				dPrint(sBuff);
			}
		}
		else if (uCmd == 'C'  && strcasecmp(uAdr, "Trigger") == 0) {
			PropTriggered = true;
			//			WriteConfig();
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s UDP Command received: triggering prop manually", HeaderString());
				dPrint(sBuff);
			}
		}
		else if (uCmd == 'C'  && strcasecmp(uAdr, "Defaults") == 0) { //WARNING, This will reset static IP and ALL settings
			ResetEpromValues();
			//			WriteConfig();
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s UDP Command received: Resetting EEprom values", HeaderString());
				dPrint(sBuff);
			}
		}
		else if (uCmd == 'C'  && strcasecmp(uAdr, "Save") == 0) {
			WriteConfig();
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s UDP Command received: Save settings to EEprom", HeaderString());
				dPrint(sBuff);
			}
		}
		else {
			if (v.Dbug > 90) { //JS:Note: Sometimes it will miss the next packet if this is in here.
				snprintf(sBuff, SBUFLEN, "%s UDP MSG from %s: '%s'  It's not for me", HeaderString(), UdpProp.remoteIP().toString().c_str(), udp_pbuffer);
				dPrint(sBuff);
			}
		}
	}
}



void BubbleSort(int a[], int size) {  //=================================================================================
	for (int i = 0; i<(size - 1); i++) {
		for (int o = 0; o<(size - (i + 1)); o++) {
			if (a[o] > a[o + 1]) {
				int t = a[o];
				a[o] = a[o + 1];
				a[o + 1] = t;
			}
		}
	}
}

void ICACHE_RAM_ATTR PingIRQ_OLD1() { //================================================================
	if (digitalRead(SONAR_PIN) == HIGH) T0SonarMicros = micros();
	else T1SonarMicros = micros();
	if (v.Dbug>99) Serial.print("|");
}

void ICACHE_RAM_ATTR PingIRQ() { //================================================================
	if (T0SonarMicros == 0) { // first pulse should be rising edge. Start timer
		T0SonarMicros = micros();
		if (v.Dbug>110) Serial.print("^");
	}
	else {
		if (digitalRead(SONAR_PIN) == LOW) T1SonarMicros = micros(); // End of pulse. If it's not low let it time out without setting it.
		detachInterrupt(SONAR_PIN);
		if (v.Dbug>110) Serial.print("v");
	}
}

void SendPing() {  //================================================================
	T0SonarLoop = millis();
	//		SonarDist = SonarPing(SONARPIN, 75); // old method that hangs main loop. using interrupts now.
	int duration;

	if (T0SonarMicros != 0) { // last ping was valid, process it
		if (T1SonarMicros != 0) {
			duration = T1SonarMicros - T0SonarMicros;
			SonarInstF = (duration / 2.0) / 74.0; // in inches
		}
		else SonarInstF = v.SonarMax; // timed out without a return pulse.  Should only happen when pinging quickly

		if (v.SonarSensitivity == 100) { // use median method, else use averaging method
			SonarDistAry[SonarTmpCnt] = (int)SonarInstF;
			SonarCnt++;
			SonarTmpCnt++;
			if (SonarTmpCnt >= 5) { // should be an even number so int /2 is the center
				BubbleSort(SonarDistAry, SonarTmpCnt);
				if (v.Dbug > 120) Serial.print(" Sorted... ");
				if (v.Dbug > 120) for (int i = 0; i < SonarTmpCnt && i < 20; i++) Serial.print((String)SonarDistAry[i] + " ");
				SonarDistF = SonarDistAry[SonarTmpCnt / 2];
				SonarTmpCnt = 0;
				if (SonarDistF > v.SonarMax) SonarDistF = v.SonarMax;
				if (SonarDistF < v.SonarMin) SonarDistF = v.SonarMin;
				if (v.Dbug > 120) Serial.println(" Median = " + (String)(SonarDistF));
				SonarDist = (int)(SonarDistF + 0.5);
			}
		}
		else { //use averaging method
			   //			SonarDistF = SonarDistF * (1 - SONARFILTRATIO) + SonarInstF * SONARFILTRATIO; // don't allow reading to change more than x% every loop
			SonarDistF = SonarDistF * (1 - v.SonarSensitivity / 100.01) + SonarInstF * v.SonarSensitivity / 100.01; // don't allow reading to change more than x% every loop
			if (SonarDistF > v.SonarMax) SonarDistF = v.SonarMax;
			if (SonarDistF < v.SonarMin) SonarDistF = v.SonarMin;
			SonarDist = (int)(SonarDistF + 0.5);
			if (v.Dbug > 40) Serial.print("(" + (String)((int)(SonarInstF + 0.5)) + "," + (String)SonarDist + ")");
			SonarDistAry[SonarTmpCnt] = (int)SonarInstF;
		}
	}
	else if (v.Dbug>15 && SonarCnt>1) Serial.print("Ping Error!");
	// kick off a new sonar pulse. Don't do this more than once every ~20ms
	detachInterrupt(SONAR_PIN);
	pinMode(SONAR_PIN, OUTPUT);
	digitalWrite(SONAR_PIN, LOW);
	delayMicroseconds(1);
	digitalWrite(SONAR_PIN, HIGH);
	delayMicroseconds(10);
	digitalWrite(SONAR_PIN, LOW);
	delayMicroseconds(1);
	pinMode(SONAR_PIN, INPUT);
	T0SonarMicros = 0;
	T1SonarMicros = 0;
	attachInterrupt(SONAR_PIN, PingIRQ, CHANGE);
}


int SonarPing_POC_ONLY(int SonarPingPin, int SonarMaxDist) { //================================================================
	
	unsigned long duration, distance;
	yield();
	unsigned long T0PingMillis = millis();
	int SonarMaxUs = SonarMaxDist * 2 * 74; //convert inches to microseconds
	pinMode(SonarPingPin, OUTPUT);
	digitalWrite(SonarPingPin, LOW);
	delayMicroseconds(5);
	digitalWrite(SonarPingPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(SonarPingPin, LOW);
	delayMicroseconds(2);
	pinMode(SonarPingPin, INPUT);
	delayMicroseconds(2);
	duration = pulseIn(SonarPingPin, HIGH, SonarMaxUs);
	yield();
//	distance = (duration / 2) / 29.1; // in centimeters
	distance = (duration / 2) / 74.0; // in inches
	if (duration == 0) distance = SonarMaxDist;
	if (v.Dbug>75) {
		snprintf(sBuff, SBUFLEN, "%s: SonarPing routine time %lu, pulse duration %lu distance %lu", HeaderString(), millis()-T0PingMillis, duration, distance );
		dPrint(sBuff);
	}
	return(distance);
}


int WifiPingTime(char *ipAddr) { //=================================================================================
	WiFiClient PingClient;
	int MyPingTime;
	//old	const char* GoogleDnsServer = "8.8.8.8";
	//old	const char* GatewayServer = "192.168.2.1";
	unsigned long T0WifiPingMillis = millis();
	if (PingClient.connect(ipAddr, 80)) {
		MyPingTime = millis() - T0WifiPingMillis;
	}
	else MyPingTime = 0;
	PingClient.stop();
	if (v.Dbug>0) {
		snprintf(sBuff, SBUFLEN, "%s Ping result %i. Routine took %lu ms", HeaderString(), MyPingTime, millis() - T0WifiPingMillis);
		dPrint(sBuff);
	}
	return(MyPingTime);
}




void TS_PostNow() { //================================================================

	WiFiClient TS_client;
	unsigned long T0TSPostMillis = millis();
	String PostStr;
	int ConStat = 0;
	int LoopCnt = 0;
	while (LoopCnt < 5 && !ConStat) {
		ConStat = TS_client.connect(v.TSServerName, 80);
		if (LoopCnt > 0) delay(200);
		LoopCnt++;
	}
	int MyPingTime = millis() - T0TSPostMillis;
	if (v.Dbug>9) {
		snprintf(sBuff, SBUFLEN, "%s TS-connection took result %ims.", HeaderString(), MyPingTime);
		dPrint(sBuff);
	}
	if (ConStat)  // use ip 184.106.153.149 or api.thingspeak.com
	{
		PostStr = v.TSApiKey;
		PostStr += "&field1=";
		PostStr += String(DHTtemp_F);
		PostStr += "&field2=";
		PostStr += String(DHThumidity_F);
		PostStr += "&field3=";
		PostStr += String(MyWifiQuality); // wifi quality in %
		PostStr += "&field4=";
		PostStr += String(MyPingTime);
		PostStr += "&field5=";
		PostStr += String(TS_PostCnt);
		PostStr += "&field6=";
		PostStr += String((digitalRead(SENSEINPUTPIN))?74.5:75.5); // read thermostat fan value

		PostStr += "\r\n\r\n";

		TS_client.print("POST /update HTTP/1.1\n");
		TS_client.print("Host: api.thingspeak.com\n");
		TS_client.print("Connection: close\n");
		TS_client.print("X-THINGSPEAKAPIKEY: " + String(v.TSApiKey) + "\n");
		TS_client.print("Content-Type: application/x-www-form-urlencoded\n");
		TS_client.print("Content-Length: ");
		TS_client.print(PostStr.length());
		TS_client.print("\n\n");
		TS_client.print(PostStr);
		delay(100);
		TS_client.stop();
		TS_PostCnt++;
		if (v.Dbug>0) {
			PostStr[PostStr.length() - 4] = 0;  //take out \r\n\r\n
			snprintf(sBuff, SBUFLEN, "%s %i%% Posted: %s", HeaderString(), MyWifiQuality, PostStr.c_str());
			dPrint(sBuff);
		}
	} //end if client connect
	else {
		TS_client.stop();
		// https://www.arduino.cc/en/Reference/ClientConnect  zero isn't valid, but it's what I'm getting :-(
		snprintf(sBuff, SBUFLEN, "%s ERROR. Unable to connect to Thingspeak wifi:%i%% timeout:%ims Constat:%i errors:%i", HeaderString(), MyWifiQuality, MyPingTime, ConStat, TS_ServerError);
		dPrint(sBuff);
		if (WiFi.status() == WL_CONNECTED) {
			//			IPAddress  ip = WiFi.localIP();
			//			Serial.println("Connected to: " + WiFi.SSID() + " at IP: " + ip[0] + "." + ip[1] + "." + ip[2] + "." + ip[3]);
			TS_ServerError++;
//			if (TS_ServerError>90) ESP.restart(); //reset if there are too many errors
		}
		else {
			Serial.println("WIFI is not connected.");
			WiFi.begin(v.Ssid, v.Password); // try to re-connect, but don't wait for status
			TS_WifiError++;
			delay(1000);
		}
	}

	TS_PostTime = millis() - T0TSPostMillis;

}//end PostNow

void ReadTemp() { //================================================================
	float temp = dht.readTemperature(true)+TempOffset; // Read temperature as Fahrenheit (isFahrenheit = true)
	float humidity = dht.readHumidity();

	if (isnan(temp) || isnan(humidity))
	{
		if (v.Dbug > 35) {
			snprintf(sBuff, SBUFLEN, "%s Temp Sensor error detected.", HeaderString());
			dPrint(sBuff);
		}
	}
	else // temps are valid
	{	//having trouble with rounding function. Need to work on it.
		DHTtemp_F = DHTtemp_F * (1 - THFILTRATIO) + temp * THFILTRATIO; // it doesn't work and I don't know why :-)
		DHThumidity_F = DHThumidity_F * (1 - THFILTRATIO) + humidity * THFILTRATIO; // it doesn't work and I don't know why :-)
//		DHTtemp_F = temp;
//		DHThumidity_F = humidity;
//		if (0) Serial.print(temp); Serial.print(" -> "); Serial.println(DHTtemp_F);
	}
}

