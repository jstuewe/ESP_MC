 
//  ESP_MC  Copyright 2017, John Stuewe 
//  ESP8266 Master control.  Mainly for Halloween props
//  With substantial code from ESP_WebConfig at https://www.john-lassen.de/en/projects/esp-8266-arduino-ide-webconfig
// Programmed for Wemos D1 Mini
// Pins on left side are RST, A0, D0, D5, D6, D7, D8, 3.3v_
// Pins on the right side are TX, RX, D1, D2, D3, D4, GND, +5v
// With only 8 pins and three pins used for boot config most pins are overloaded. Be careful what you enable at the same time.

// 6/01/17 JS  WebConfig example working. Now integrating OTA and PostTemp2 code to see if it's compatible
// 7/11/17 JS Full working Sonar trigger over UDP to prop relay control
// 9/07/17 JS v1.55 adding RF receive code. broke code out into RFCtrl.h
// 9/11/17 JS v1.56 added code to act on RF codes from RF1, or RF2 remotes. RF3 codes are in but not active
// 9/15/17 JS 1.57 UDP RF command cleanup, added LED time synch based on "L" UDP packet sent at boot
// 9/22/17 JS 1.58 added more UDP commands. Display version on UDP reboot string
const char VersionText[] = "ESP_MC v1.60 10/10/17";

#define RSVD_D0				D0	//Pin4, GPIO16 (Deepsleep capable, No IRQ) 

#define RELAY_PIN0			D1	//Pin20, GPIO5 WEMOS D1 mini relay shield
#define MR_GHOUL_PIN		D1	//Pin20, GPIO5 v.PropName="Mirror"  Mirror Room Ghoul exit solenoid
//#define SONARPIN			D1  //Pin20, GPIO5 //old. CHANGED 7/28/17:JS

#define PIR_PIN				D2	//Pin19, GPIO4 N\A, debugging.  Signal pin out of PIR module.  PIR has many false positives due to RF interference
#define SONAR_PIN			D2  //Pin19, GPIO4 SonarOn=true for Sonar distance module
#define DHT_PIN				D2	//Pin19, GPIO4 TempOn=true  Temp sensor when TempOn=true. moved off of D4\LED 8/12/17
#define MR_SHIFT_DATA_PIN	D2  //Pin19, GPIO4 v.PropName="Mirror"  Mirror Room Prop
#define RC_RX_PIN	        D2  //Pin19, GPIO4 IrRemoteOn=True  433mhz receive pin


#define RSVD_BOOTSEL		D3	//Pin18, GPIO0 int 10K PU. DTR\CTS tied in. Needs to be HIGH or floating on reset for normal operation
#define DHT_VCC				D3	//Pin18, GPIO0 TempOn=true  3.3V power for PIR module. Must be high on reboot. 10KPU on Wemos D1

#define BUILTIN_LED			D4  //Pin17, GPIO2 Int 10K PU. Built-in LED signal. Alternate Bootloader TX. Do not hard ground
//#define SERIAL1	        D4  //pin17, GPIO2 v.PropName="Fog" to controll RS485 Fog Jet Fury device
//#define DHT_PIN				D4	//Pin17, GPIO2 Wemos DHT11 module pin. //  You can't turn off the lED when reading the temperture when using this pin

#define RELAY_PIN1			D5  //Pin5,  GPIO14 PropOn=true and not a special prop
#define MR_EXIT_PIN			D5  //Pin5,  GPIO14 v.PropName="Mirror"  Miror room prop only
#define RC_TX_PIN           D5  //Pin5,  GPIO14 IrRemoteOn=True 433mhz transmitter pin
#define DFPLAYER_RX			D5	//Pin5,  GPIO14 v.DFPlayerOn=True  SD card audio player

#define RELAY_PIN2			D6  //Pin6,  GPIO12 PropOn=true and not a special prop
#define IR_RX_PIN			D6	//Pin6,  GPIO12 IrRemoteOn=True  IR receiver, receive pin
#define MR_SHIFT_LATCH_PIN	D6  //Pin6,  GPIO12 v.PropName="Mirror"  Miror room prop only
#define DFPLAYER_TX			D6	//Pin6,  GPIO12 v.DFPlayerOn=True  SD card audio player  (Doesn't work @D7 for some reason)

#define RELAY_PIN3			D7	//Pin7,  GPIO13 PropOn=true and not a special prop
#define IR_GND_PIN			D7	//Pin7,  GPIO13 IrRemoteOn=true IR receiver, ground pin
#define MR_SHIFT_CLOCK_PIN	D7	//Pin7,  GPIO13 v.PropName="Mirror"  Miror room prop only
#define BUZZERPIN			D7	//Pin7,  GPIO13 N\A, not used
#define SENSEINPUTPIN		D7	//Pin7,  GPIO13 N\A, still used in thermostat :-(

#define RSVD_BOOTSTRAP		D8	//Pin16, GPIO15 INt 10K PD. multiplex with	U0RTS that must be low to on start for normal bootloader. Can't have anything here that will pull it high on reset
#define SENSEINPUTGND		D8	//Pin16, GPIO15 N\A, still used in thermostat :-(

#define		SERIAL_BAUDRATE     115200
#define		SBUFLEN 150
char		sBuff[SBUFLEN];
#define		NUMMESSAGES 6
String		hBuff[NUMMESSAGES+1];
char		DefaultHostName[20];

#include <ESP8266WiFi.h>
#include "user_interface.h"

#include <WiFiClient.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);		
#include <EEPROM.h>
#include <WiFiUdp.h>
WiFiUDP	 udp;
WiFiUDP	 UdpProp;
#include <Ticker.h>
Ticker   BuzzTicker;

#define DMXBase 0
#include <ESPDMX.h>
DMXESPSerial dmx;	// for RS485 controlled fog machine. uses D4\GPIO2\SERIAL1
boolean SuppressLED = false;  // SERIAL1 uses the same pin as the LED and will fight with it.

//#define _DEBUG  // having issues with DFPlayer going offline after accessing web interface. No deltas when enabling this but it should spit out a lot
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
SoftwareSerial DFPlayerSerial(DFPLAYER_RX, DFPLAYER_TX); // RX, TX @ D6, D7
DFRobotDFPlayerMini myDFPlayer;

unsigned long T0StatusMillis = 0;          // Current time in MS since last loop
unsigned long T0OneSecMillis = 0;         
unsigned long T0PropTriggerMillis = 0;
unsigned long T0WebPostMillis = 0;         
unsigned long T0DisconnectMillis = 0;
unsigned long DisconnectMillis = 0;
unsigned long T0LoopMillis = 0;  // last time the main loop started
unsigned long T0LoopAvgMillis = 0;  // last time the main loop started
unsigned long T0PropMillis = 0;  // last time the prop was activated
unsigned long T0SonarLoop;
volatile unsigned long T0SonarMicros = 0;  // Rising edge of Sonar pulse
volatile unsigned long T1SonarMicros = 0;  // Falling edge of Sonar pulse


//const char* TS_SERVER = "api.thingspeak.com";  // now using v.TsServerName
//now using v.TsApiString
//String apiKey = "ZYDRETMJIAYBMEWO"; //HouseTempLog1 on 1.73 on COM6  & 107
//String apiKey = "75MOSHXNSEE9QXCC"; //HouseTempLog2 on 1.92 on COM8  & 90
//String apiKey = "9Y403WB4SE21PSZZ"; //HouseTempLog3 on 1.91 on COM3  & 108
//String apiKey = "12L34PDZZGUA2TSH"; //temp/test channel using for 1.90 tester 1.108 on Com6
//String apiKey = "OUTOFSERVICEOUTOF";//temp/test channel
int TS_ServerError = -1;  // initial post to indicate MCU restarted
int TS_WifiError = 0;
int TS_PostCnt = 0;
int TS_PostTime = 0;
int MyWifiQuality = 0;
int MyPingTime = 0;
unsigned long  LoopMin = 0;	// Main loop minimum cycle time
unsigned long  LoopMax = 0;	// main loop average cycle time in ms.
unsigned long  LoopAvg = 0;	// Main loop T0MainLoopMillis / LoopCnt
int LoopCnt = 0;	// Main loop itteration count	
int DisconnectCnt = 0; // number or Wifi disconnects
int PropCnt = 0;	// Number of times the prop has been activated
int PropArgument = 0; // Argument passed when prop triggered
int SonarCnt = 0;	// Number of times the Sonar has been activated
int SonarTmpCnt = 0;  // Sonar pings since last status loop
int SonarDistAry[200]; // Sonar array median immages
float SonarDistF = 99; // filtered Sonar reading
float SonarInstF = 99;
int SonarDist = 99;
int LocalTimeOffset = 0;
boolean	SynchNtp = false;	// On SynchNtp = true, NTP will be re-initialized
boolean	ResetNow = false;	// reboot ESP part when true
boolean	PropTriggered = false;


#include <DHT.h>
//#define DHT_PIN		xx		D2		//GPIO0 Signal pin out of 
//#define DHT_PIN				D4		//Wemos DHT11 module pin.
//#define DHT_VCC				D3		//GPIO4 3.3V power for PIR module. Need to short diode to use at 3.3v High or float on reset for a clean reboot. 10KPU on Wemos D1
//#define DHTTYPE				DHT11   // DHT 22  (AM2302), AM2321.  Also supports DHT11 and DHT21
#define DHTTYPE				DHT22   // DHT 22  (AM2302), AM2321.  Also supports DHT11 and DHT21
#define THFILTRATIO       	0.05	// % each new reading affects the running average for temp & humidity reading
float DHThumidity_F;
float DHTtemp_F;
float TempOffset = 0.0;
DHT dht(DHT_PIN, DHTTYPE);


//xxint			udp_Port = 7102;	//now using v.UdpPort  Port to transmit on. 
//xxint			udp_StatNode = 255; //Now using v.UdpStatNode  255 = multicast
//xxint			udp_PropPort = 7777;//Now using v.UdpPropPort
IPAddress	    udp_ip;
int				udp_psize;
char			udp_pbuffer[255];  // buffer for incoming packets.  used for both Status and prop control. Separate if it causes problems


//==================

#define EE_CONFIG_KEY "1001"//key to know we have the right version. Change for each program and each variable structure change to prevent corruption
#define EE_START	0
#define EE_SIZE		1024	// emulated eeprom size (6'17 program is only at 300 bytes, watch the size.
#define STRLEN      25		// maximum string length
struct vStruct { // Global varible structure. Saved to EEprom.  Note: Do NOT use strings. They are variable length and will corrupt everything
	int		Dbug;
	char 	DeviceName[STRLEN];
	char	Ssid[STRLEN];
	char	Password[STRLEN];
	char	PropName[STRLEN];
	char	PropAudioName[STRLEN];
	int		StatusInterval;
	int		UdpPort;
	int		UdpPropPort;
	int		UdpStatNode;
	int		TsPostInt;
	int		PropTrigHoldoff;
	int		PropDelay[4];
	int		PropOnTime[4];
	int		PropCycle[4];
	int		PropArgument;
	int		PropAudioDelay;
	boolean	PropFlag[8];
	boolean PropInvertSig;
	boolean	Rsvd0; //10-1
	boolean Rsvd1;
	boolean Rsvd2;
	boolean Rsvd3;
	int		PropGPIPin; //10-1
	int		RsvdI2;
	int		RsvdI3;
	int		RsvdI4;
	int		SonarMin;
	int		SonarMax;
	int		SonarTrigger;
	int		SonarSensitivity;
	int		SonarHoldoff;
	boolean GlobalEnableOn;
	boolean	SerOn;
	boolean	BeepOn;
	boolean IdentifyOn;
	boolean LedOn;
    boolean IrRemoteOn;
	boolean	TempOn;
	boolean OtaOn;
	boolean	WebOn;
	boolean	UdpOn;
	boolean	TsPostOn;
	boolean SonarOn;
	boolean PropGPIOn; //10/1
	boolean PropOn;
	boolean DFPlayerOn;	//10/1
	boolean DhcpOn;
	boolean ConfigAPOn;
	boolean DstActive;
	byte	StaticIP[4];
	byte	Netmask[4];
	byte	Gateway[4];
	int		WifiPower; // in percent up to 100. need to divide by 20.5 to get actual register setting
	char	NtpServerName[STRLEN];
	long	NtpServerUpdateInterval;
	long	TimeZone;
	char	TSServerName[STRLEN];
	char	TSApiKey[STRLEN];
	int		ConfigTimeout;
	char	Config_Key[4];
}   v;

const char content_favicon_data[] PROGMEM = {
	0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x10, 0x00, 0x01, 0x00, 0x04, 0x00, 0x28, 0x01, 0x00, 0x00, 0x16, 0x00,
	0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00, 0xC0, 0xC0, 0xC0, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x41, 0x00, 0x00, 0x14, 0x44, 0x44, 0x44, 0x41, 0x00, 0x00, 0x00, 0x00,
	0x14, 0x44, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x40, 0x23,
	0x33, 0x20, 0x02, 0x33, 0x32, 0x04, 0x12, 0x30, 0x00, 0x32, 0x23, 0x00, 0x03, 0x21, 0x03, 0x00, 0x00, 0x03, 0x30, 0x03,
	0x00, 0x30, 0x03, 0x03, 0x33, 0x03, 0x30, 0x33, 0x30, 0x30, 0x03, 0x00, 0x00, 0x03, 0x30, 0x03, 0x00, 0x30, 0x02, 0x30,
	0x00, 0x32, 0x23, 0x00, 0x03, 0x20, 0x10, 0x23, 0x33, 0x20, 0x02, 0x33, 0x32, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x04, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x41,
	0x00, 0x00, 0x00, 0x00, 0x14, 0x44, 0x44, 0x44, 0x41, 0x00, 0x00, 0x14, 0x44, 0x44, 0xF8, 0x1F, 0x00, 0x00, 0xE0, 0x07,
	0x00, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01,
	0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0xC0, 0x03, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x00
};

const char *str_status[] = { "WL_IDLE_STATUS","WL_NO_SSID_AVAIL","WL_SCAN_COMPLETED","WL_CONNECTED","WL_CONNECT_FAILED","WL_CONNECTION_LOST","WL_DISCONNECTED" };
const char *str_mode[] = { "WIFI_OFF", "WIFI_STA", "WIFI_AP", "WIFI_AP_STA" };

// Include local header files
#include "TimeNTP.h"
#include "FunctionCode.h"
#include "RFCtrl.h"
#include "PropCtrl_MirrorRoom.h"
#include "PropCtrl.h"
#include "IRCtrl.h"
#include "OTA.h"
// Include the HTML, STYLE and Script "Pages"
#include "Page_Main.h"
#include "Page_Admin.h"
#include "Page_General.h"
#include "Page_Prop.h"
#include "Page_Network.h"
#include "Page_NTPsettings.h"
#include "Page_Script.js.h"
#include "Page_Style.css.h"


//=====================================================================================================
void setup ( void ) { //===============================================================================

	Serial.begin(SERIAL_BAUDRATE);
	Serial.println();
	Serial.println();
	Serial.println(VersionText);

	pinMode(RSVD_BOOTSTRAP, OUTPUT);
	digitalWrite(RSVD_BOOTSTRAP, LOW); // GPIO15 must be low on reset to stay out of SDIO mode
	pinMode(RSVD_BOOTSEL, OUTPUT);
	digitalWrite(RSVD_BOOTSEL, HIGH); // GPIO0 must be high on reset to stay out of bootloader mode

	pinMode(BUILTIN_LED, OUTPUT);
	digitalWrite(BUILTIN_LED, LOW); // turn LED on during config
	pinMode(SENSEINPUTPIN, INPUT_PULLUP);
	pinMode(SENSEINPUTGND, OUTPUT);
	digitalWrite(SENSEINPUTGND, LOW);
	
	sprintf_P(DefaultHostName, PSTR("ESP%06X"), ESP.getChipId());

	EEPROM.begin(EE_SIZE);
	if (!ReadConfig()) // if config not valid, initialize it
	{
		memset(&v, 0, sizeof(v)); // Ensure array is empty and null terminated
		char HostName[20];
		sprintf_P(HostName, PSTR("ESP%06X"), ESP.getChipId());
		strncpy(v.DeviceName, HostName, STRLEN-1);
//		strncpy(v.Ssid, "Stuewe-GN-GR",STRLEN-1);
//		strncpy(v.Password, "cd3f5be032",STRLEN-1);
		strncpy(v.Ssid, "TerrorOnTwoJacks2", STRLEN - 1);
		strncpy(v.Password, "Hallow33n", STRLEN - 1);
		v.Dbug				= 5;		// debug message level. 0=no debug, >10 very verbose
		v.StatusInterval	= 9;		// send status ever x seconds
		v.UdpPort			= 1031;		// channel for status
		v.UdpPropPort		= 6666;		// channel for prop control
		v.UdpStatNode		= 255;		// 255 is multicast to everyone. Change to IP number to target a specific control node
		v.TsPostInt			= 60;		//Post every x seconds. Can't be less than ~30
		v.ConfigTimeout		= 5;		//Config AP point turned off on first web access or after X minutes
		strncpy(v.PropName, "NA", STRLEN - 1);
		strncpy(v.PropAudioName, "", STRLEN - 1);
		v.PropTrigHoldoff	= 10000;	// 4 seconds between prop triggers
		v.PropDelay[0]		= 0;	v.PropDelay[1] = 0;  v.PropDelay[2] = 0;  v.PropDelay[3] = 0;
		v.PropOnTime[0]		= 2000; v.PropOnTime[1] = 0; v.PropOnTime[2] = 0; v.PropOnTime[3] = 0;
		v.PropCycle[0]		= 0;	v.PropCycle[1] = 0;		v.PropCycle[2] = 0;  v.PropCycle[3] = 0;
		v.PropInvertSig = false;
		v.PropArgument		= 0;
		v.PropGPIPin        = 7;		//Default to D7.  Note: use GPILogicToPhysical to get from Wemos Dx to actual ESP8266 GPIO
		v.PropAudioDelay	= 0;
		v.SonarMin			= 5;
		v.SonarMax			= 99;
		v.SonarTrigger		= 24;		// inches. Trigger prop when closer than this
		v.SonarSensitivity	= 100;		// set to 100 for latest reading only. 33% will use 1/3 of new reading using a running average to filter noise
		v.SonarHoldoff		= 50;		// ms to wait between Sonar pulses.
		v.SerOn      = true;
		v.UdpOn      = true;
		v.WebOn      = true;
		v.BeepOn     = false;
		v.LedOn      = true;
		v.IrRemoteOn = false;
		v.IdentifyOn = false;
		v.DFPlayerOn = false;
		memset(&v.PropFlag, 0, sizeof(v.PropFlag));
		v.GlobalEnableOn = true;
		v.TempOn     = false;
		v.SonarOn    = false;
		v.PropOn     = false;
		v.PropGPIOn  = false;	// trigger prop based on GPIO pin.  ie IR break, step pad, PIR, etc.
		v.TsPostOn   = false;	// don't turn on without valid key
		v.DhcpOn     = true;
		v.OtaOn      = true;
		v.ConfigAPOn = true;
		v.DstActive  = true; //Daylight Savings time. True in Winter
		v.StaticIP[0]= 192; v.StaticIP[1]= 168; v.StaticIP[2]= 001; v.StaticIP[3]= 100;
		v.Netmask[0] = 255; v.Netmask[1] = 255; v.Netmask[2] = 255; v.Netmask[3] = 000;
		v.Gateway[0] = 192; v.Gateway[1] = 168; v.Gateway[2] = 001; v.Gateway[3] = 254;
		v.WifiPower = 100; //in percent
		strncpy(v.NtpServerName, "us.pool.ntp.org",STRLEN-1);
		v.NtpServerUpdateInterval = 1440; // Once a day is 1440 minutes
		v.TimeZone     = -6; // -6 us US Central time
		strncpy(v.TSServerName, "api.thingspeak.com",STRLEN-1);
		strncpy(v.TSApiKey, "<TBD>", STRLEN - 1);	// Need valid key to post on Thingspeak
		v.Config_Key[0] = EE_CONFIG_KEY[0]; v.Config_Key[1] = EE_CONFIG_KEY[1]; v.Config_Key[2] = EE_CONFIG_KEY[2]; v.Config_Key[3] = EE_CONFIG_KEY[3]; // must be last variable
		WriteConfig();
		if(v.Dbug>5) Serial.println("Default Config Loaded");
	}
	
//	Serial.printf("Main1:Connectiong WIFI to SSID:%s, PW:%s, key:%s\n", v.Ssid, v.Password, v.Config_Key);

	WiFi.setOutputPower(20.5*v.WifiPower/100); // radio transmit power in DB, from 0 to 20.5
	

	char ApHostName[30] = "Config_";
//	const char ApPassword[] = "12345678";
	const char ApPassword[] = "";
	strncpy(ApHostName + strlen(ApHostName), v.DeviceName, 30);
	if (v.ConfigAPOn) {
		WiFi.disconnect();
		WiFi.mode(WIFI_AP_STA);

		if (WiFi.softAP(ApHostName, ApPassword) == true) { 
			Serial.printf("Config AP enabled at %s on SSID:'%s' PW:'%s' \n", WiFi.softAPIP().toString().c_str(), ApHostName, ApPassword);  //compile isues with string :-(
//			Serial.println("Config AP enabled at " + String(WiFi.softAPIP()) + " on SSID :" + String(HostName) + ", PW : " + String(Password));
		}
		else Serial.println("Error starting Config AP");
	}
	else 	{
		WiFi.mode(WIFI_STA);
	}

	ConfigureWifi();

	SetupTime(v.NtpServerName, v.TimeZone, v.DstActive);

	if (v.OtaOn) { Initialize_OTA(); }
	
	if (v.WebOn) {
		server.on("/",             send_main_html); // main page is blank. go straight to config
//		server.on("/",             []() { server.send(200, "text/html", PAGE_AdminMainPage);   });
		server.on("/admin.html",   []() { server.send(200, "text/html", PAGE_AdminMainPage);   });
		server.on("/admin",        []() { server.send(200, "text/html", PAGE_AdminMainPage);   });
		server.on("/general.html", send_general_html);
		server.on("/general",      send_general_html);
		server.on("/prop.html",    send_prop_html);
		server.on("/prop",         send_prop_html);
		server.on("/config.html",  send_network_configuration_html);
		server.on("/config",       send_network_configuration_html);
		server.on("/ntp.html",     send_NTP_configuration_html);
		server.on("/ntp",          send_NTP_configuration_html);
		server.on("/style.css",    []() { server.send(200, "text/plain", PAGE_Style_css);  });
		server.on("/microajax.js", []() { server.send(200, "text/plain", PAGE_microajax_js);  });
		server.on("/admin/values", send_network_configuration_values_html);
		server.on("/admin/connectionstate", send_connection_state_values_html);
		server.on("/admin/ntpvalues",       send_NTP_configuration_values_html);
		server.on("/admin/generalvalues",   send_general_configuration_values_html);
		server.on("/admin/propvalues",		send_prop_configuration_values_html);
		server.on("/admin/mainvalues",      send_main_configuration_values_html);
		server.on("/admin/adminvalues",     send_admin_values_html);
		server.on("/admin/devicename",      send_devicename_value_html);
		server.on("/admin/reset", []() { ResetNow = true; });
		server.on("/admin/default", ResetEpromValues);
		server.on("/favicon.ico",  []() { server.send_P(200, "image/x-icon", content_favicon_data, sizeof(content_favicon_data));   }); //ref:https://arduino.stackexchange.com/questions/26288/serving-binary-file-with-esp8266webserver
		
		server.onNotFound([]() { if (v.Dbug > 5) Serial.println("Web Page Not Found"); server.send(400, "text/html", "Page not Found");   });
		server.begin();
		Serial.println("HTTP server started");
	}

	if (v.UdpOn || 1) { // always initialize it on so it works if it's turned on later. Otherwise we need to reboot
		udp.begin(v.UdpPort);	//initialize UDP port
		UdpProp.begin(v.UdpPropPort);
		udp_ip = WiFi.localIP();
		udp_ip[3] = v.UdpStatNode;	//Update last byte to status node or 255 for multicast. ie. 192.168.1.255
		if (v.UdpOn && v.Dbug > 25) {
				snprintf(sBuff, SBUFLEN, "%s Status updates sent to %s on UDP port %i", HeaderString(), udp_ip.toString().c_str(), v.UdpPort);
				dPrint(sBuff);
			}
	}

	if (v.TempOn) { 	// DHT temp sensor function is on
		pinMode(DHT_VCC, OUTPUT);
		digitalWrite(DHT_VCC, HIGH); // Provide VCC
		delay(100);
		dht.begin();
		delay(500);
		DHTtemp_F = dht.readTemperature(true); // Read temperature as Fahrenheit (isFahrenheit = true)
		DHThumidity_F = dht.readHumidity();
		if (isnan(DHTtemp_F) || isnan(DHThumidity_F)) {
			snprintf(sBuff, SBUFLEN, "%s Temp Sensor not detected.", HeaderString());
			dPrint(sBuff);
			DHTtemp_F = 0;
			DHThumidity_F = 0;
		}
	}
	
	if (v.IrRemoteOn) {  // IR receiver for control and learning codes
		Serial.println("IR recevier active.");
		pinMode(IR_GND_PIN, OUTPUT);
		digitalWrite(IR_GND_PIN, LOW);
		MyIrRecv.enableIRIn();
	}

	if (v.DFPlayerOn) {  // DFPlayer shield installed https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299
		DFPlayerSerial.begin(9600); // software serial pins defined with variable declaration
		Serial.println(F("Activating DFPlayer Mini SD Player. "));
		if (!myDFPlayer.begin(DFPlayerSerial)) {  //Use softwareSerial to communicate with mp3.
			Serial.println(F("ERROR: Unable to start DFPlayer SD card reader:"));
			Serial.println(F("       1.Please recheck the connection!"));
			Serial.println(F("       2.Please insert the SD card!"));
			snprintf(sBuff, SBUFLEN, "%s ERROR: DFPlayer enabled but not responding", HeaderString());
			dPrint(sBuff);
		}
		myDFPlayer.setTimeOut(100); //Set serial communictaion time out 500ms
		myDFPlayer.volume(0);  //Set volume value (0~30).
		myDFPlayer.EQ(DFPLAYER_EQ_NORMAL); // options: _NORMAL, _POP, _ROCK, _JAZZ, _CLASSIC, _BASS
		myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
		if (v.Dbug >25) {
			Serial.print("DFPlayer State:"   +(String) myDFPlayer.readState()); //read mp3 state
			Serial.print(" Volume:"  + (String) myDFPlayer.readVolume()); //read current volume
			Serial.print(" EQ:"      + (String) myDFPlayer.readEQ()); //read EQ setting
			Serial.print(" FileCnt:" + (String) myDFPlayer.readFileCounts()); //read all file counts in SD card
			Serial.print(" File#:"   + (String) myDFPlayer.readCurrentFileNumber()); //read current play file number
			Serial.println(" FileCnt1:" + (String) myDFPlayer.readFileCountsInFolder(1)); //read fill counts in folder SD:/01
			}
		Serial.println(F("DFPlayer Mini online."));
	}


//	if (v.PropFlag[0]) {
	if (v.IrRemoteOn) {  // enable with IR for now
		MyRfSwitch.enableReceive(RC_RX_PIN);  // 433Mhz recevier data pin for learning and RF receiver
		Serial.println("RF recevier active.");
	}

	if (v.PropGPIOn) {
		if (v.PropInvertSig)
			pinMode(GPILogicToPhysical(v.PropGPIPin), INPUT); //Inverted. Trigger on "1". 
		else
			pinMode(GPILogicToPhysical(v.PropGPIPin), INPUT_PULLUP); //active low. Trigger on "0".
	}

	if (v.BeepOn) {
		pinMode(BUZZERPIN, OUTPUT);
		pinMode(BUZZERPIN, LOW);
		SetBuzzer(400, 100);
	}
	
	if (strcasecmp(v.PropName, "Mirror")==0)	  MRSetup() ; // Props that need special setup
	if (strcasecmp(v.PropName, "AcSwitch")==0)    InitRfTx(); // Props that need special setup
	if (strcasecmp(v.PropName, "AcSwitch2") == 0) InitRfTx(); // Props that need special setup
	if (strcasecmp(v.PropName, "Fog") == 0)	      dmx.init(); // Initialize RS485 on D4\GPIO2\Serial1
	
	T0PropTriggerMillis = millis();
	T0LoopMillis = millis();
	T0LoopAvgMillis = millis();
	T0SonarMicros = 0;
	T1SonarMicros = 0;
	SonarDistF = v.SonarMax;
	SonarDist = v.SonarMax;

	LoopCnt = 0;
	LoopMin = 9999;
	LoopMax = 0;
//	UdpProp.beginPacket(udp_ip, v.UdpPropPort);
//	UdpProp.print("L LocalTimeSynchPulse " + (String)v.DeviceName);
//	UdpProp.endPacket();
	snprintf(sBuff, SBUFLEN, "%s %s ======= Node %s has been restarted =======", HeaderString(), VersionText, DefaultHostName); //  , GetMacAddress().c_str());
	dPrint(sBuff);
	digitalWrite(BUILTIN_LED, HIGH); // turn off LED when configured

} // setup

//=====================================================================================================
//=====================================================================================================
void loop ( void ) { //================================================================================
	int Millis = millis();
	LoopCnt++;
	if (Millis - T0LoopMillis < LoopMin) LoopMin = Millis - T0LoopMillis; //just keep statistics
	if (Millis - T0LoopMillis > LoopMax) LoopMax = Millis - T0LoopMillis; //just keep statistics
	T0LoopMillis = Millis;
	
	if ((WiFi.getMode() == WIFI_AP_STA) && (millis() > (unsigned long) v.ConfigTimeout*1000*60) && WiFi.status() == WL_CONNECTED) {
		 Serial.println("Turning off Config AP");
		 WiFi.mode(WIFI_STA);
	}
	yield();
	if (WiFi.status() != WL_CONNECTED) {
			if (!T0DisconnectMillis) {
			T0DisconnectMillis = millis(); // first loop after disconnetion
			DisconnectCnt += 1;
		} //else it's not the first loop we're disconnected
	}
	else if (T0DisconnectMillis!=0) { // first loop after we've reconnected
		DisconnectMillis += Millis - T0DisconnectMillis;
		if (v.Dbug >8) {
			snprintf(sBuff, SBUFLEN, "%s Wifi drop# %i, restored after %lu ms. %lu seconds total offline. Wifi at %i%%", HeaderString(), DisconnectCnt, Millis - T0DisconnectMillis, DisconnectMillis/1000, WifiQuality());
			dPrint(sBuff);
		}
		T0DisconnectMillis = 0; // were' connected to Wifi again
	}
	yield();
	if (ResetNow)	ResetESPNow();
	if (v.WebOn)	server.handleClient();
	if (v.OtaOn)	ArduinoOTA.handle();
	if (SynchNtp) {
		SetupTime(v.NtpServerName, v.TimeZone, v.DstActive); 
		SynchNtp = false;
	}
	if (v.PropOn && v.GlobalEnableOn)	PropHandle(); // main prop call

	if ((v.LedOn || v.IdentifyOn) && !SuppressLED)	BlinkLed();

	CheckUdp();		// For program status and control check it even if it's off or it will overflow

	CheckUdpProp(); // For Prop triggering. Set PropTriggered=true when it gets a trigger

	if (v.IrRemoteOn && MyRfSwitch.available()) ScanRfSwitch(); // scan or 433mhz codes from RF remote

	if (v.IrRemoteOn && MyIrRecv.decode(&IrResults)) ProcessIrCode(&IrResults); // Receive and process codes\commands from an IR remote

	yield();
//*** Process Status update ****************		
	if ((T0StatusMillis == 0) | (millis() - T0StatusMillis > (unsigned long) v.StatusInterval * 1000)) {

		if (v.Dbug>90) Serial.println("In status loop.");
		T0StatusMillis = millis();
		int MyWifiQuality = WifiQuality();
		if (v.Dbug >4 && v.TempOn) {
			snprintf(sBuff, SBUFLEN, "%s %i%% Temp %d.%0d Humidity %d.%0d%% ACFAN %i", HeaderString(), MyWifiQuality, (int)(DHTtemp_F), int(10 * (DHTtemp_F + .05 - (int)DHTtemp_F)), (int)(DHThumidity_F), int(10 * (DHThumidity_F + .05 - (int)(DHThumidity_F))), digitalRead(SENSEINPUTPIN));
			dPrint(sBuff);
		}
		if (v.Dbug > 6 && v.SonarOn) {
			snprintf(sBuff, SBUFLEN, "%s SonarCnt %i, SonarDist %i", HeaderString(), SonarCnt, SonarDist);
			dPrint(sBuff);
		}
		if (v.Dbug > 65) { // general status line
			LoopAvg = ((millis() - T0LoopAvgMillis) / LoopCnt);
			snprintf(sBuff, SBUFLEN, "%s Wifi:%i%%, heap:%i, LoopCnt %i, Max %lu, Avg %lu ms", HeaderString(), MyWifiQuality, ESP.getFreeHeap(), LoopCnt, LoopMax, LoopAvg);
			dPrint(sBuff);
			LoopCnt = 0;
			LoopMax = 0;
			LoopMin = 9999;
			T0LoopAvgMillis = millis();
		}

		if (0) PropTriggered = true; // debug hook to force prop trigger on every status update
	}
	yield();
//*** Process 1 second loop ****************

	if (millis() - T0OneSecMillis > 1 * 1000) {  // stuff to do every second
		T0OneSecMillis = millis();
		if (v.TempOn) {
			ReadTemp();
		}
		if (v.DFPlayerOn && v.Dbug>10) {
			if (myDFPlayer.available()) {
				if (myDFPlayer.readState() == -1) {
					DFPlayerSerial.begin(9600); // software serial pins defined with variable declaration
//					myDFPlayer.reset();     //Reset the module
					if (!myDFPlayer.begin(DFPlayerSerial)) {  //Use softwareSerial to communicate with mp3.
						snprintf(sBuff, SBUFLEN, "%s ERROR: DFPlayer enabled but not responding", HeaderString());
						dPrint(sBuff);
					}
				}
//JS:20171008 still debugging. Once I access the web interface the DFPlayer stops responding. I've tried to restart it to no avail.

				PrintDfPlayerStatus(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
				if (v.Dbug > 200) {  //returns -1 after each item below times out when there's an error.
					Serial.print("DFPlayer State:" + (String)myDFPlayer.readState()); //read mp3 state
					Serial.print(" Volume:" + (String)myDFPlayer.readVolume()); //read current volume
					Serial.print(" EQ:" + (String)myDFPlayer.readEQ()); //read EQ setting
					Serial.print(" FileCnt:" + (String)myDFPlayer.readFileCounts()); //read all file counts in SD card
					Serial.print(" File#:" + (String)myDFPlayer.readCurrentFileNumber()); //read current play file number
					Serial.println(" FileCnt1:" + (String)myDFPlayer.readFileCountsInFolder(1)); //read fill counts in folder SD:/01
					}
				}
			delay(1); // I'm getting two messags. will delay fix it?
			//todo: convert serial message to UDP
			}
		}
	yield();
//*** Process Sonar Prop trigger ****************
	if (v.SonarOn && (millis() - T0PropTriggerMillis > (unsigned long) v.PropTrigHoldoff) && SonarDist < v.SonarTrigger) {
		unsigned long TimeSinceLastTrigger = (millis() - T0PropTriggerMillis) / 1000; //seconds
		T0PropTriggerMillis = millis();

		if (v.GlobalEnableOn) {
			UdpProp.beginPacket(udp_ip, v.UdpPropPort);
			UdpProp.print("T " + (String)v.PropName + " " + (String) PropArgument);
			UdpProp.endPacket();

			PropCnt += 1;
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s Triggering Prop '%s'. SonarDist %i < %i.  %lu Sec since last trigger", HeaderString(), v.PropName, SonarDist, v.SonarTrigger, TimeSinceLastTrigger);
				dPrint(sBuff);
			}
		}

		//Note: The UDP packet will be received by CheckUdpProp(), which will set PropTriggered=true on the prop MC, which will cause PropHandle() to trigger the prop
	}
	yield();

//*** Process GPIO Prop trigger ****************
	if (v.PropGPIOn && (millis() - T0PropTriggerMillis > (unsigned long)v.PropTrigHoldoff) && (digitalRead(GPILogicToPhysical(v.PropGPIPin)) == (v.PropInvertSig)?1:0) ) {
		unsigned long TimeSinceLastTrigger = (millis() - T0PropTriggerMillis) / 1000; //seconds
		T0PropTriggerMillis = millis();

		if (v.GlobalEnableOn) {
			UdpProp.beginPacket(udp_ip, v.UdpPropPort);
			UdpProp.print("T " + (String)v.PropName + " " + (String)PropArgument);
			UdpProp.endPacket();

			PropCnt += 1;
			if (v.Dbug > 4) {
				snprintf(sBuff, SBUFLEN, "%s Triggering Prop '%s'. GPIO Trigger on GPIO%i (Wemos D%i).  %lu Sec since last trigger", HeaderString(), v.PropName, GPILogicToPhysical(v.PropGPIPin), v.PropGPIPin,TimeSinceLastTrigger);
				dPrint(sBuff);
			}
		}
		//Note: The UDP packet will be received by CheckUdpProp(), which will set PropTriggered=true on the prop MC, which will cause PropHandle() to trigger the prop
	}

	yield();
//*** Process Sonar Ping ****************
	if (v.SonarOn && millis() - T0SonarLoop > (unsigned long)v.SonarHoldoff) SendPing();
			
//*** Process Thingspeak web posting ****************
	if (v.TsPostOn && v.GlobalEnableOn && millis() - T0WebPostMillis > (unsigned long) v.TsPostInt * 1000) { // WEB post interval
		T0WebPostMillis = millis();
		MyWifiQuality = WifiQuality();
		//		MyPingTime = PingTime("8.8.8.8");
		//		MyPingTime = PingTime((char *) TS_SERVER);
		if (v.TsPostOn) { TS_PostNow(); }
	}

	delay(1);
} // main loop

