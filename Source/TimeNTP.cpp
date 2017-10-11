/*
* TimeNTP_ESP8266WiFi.ino
* Example showing time sync to NTP time source
*
* This sketch uses the ESP8266WiFi library
* Modified to allow use as a source file, not main file JS 4/30/17
*/

#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define NTPSTRLEN 25

WiFiUDP NtpUdp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
char NtpServerName[NTPSTRLEN] = "us.pool.ntp.org"; // or "time.nist.gov"; or "time-a.timefreq.bldrdoc.gov";
int  TimeZone = -6; // USA Central Time
bool DstActive = false; // false for summer True for Summer (Manual setting)


time_t getNtpTime();
time_t prevDisplay = 0; // when the digital clock was displayed
void digitalClockDisplay(int PrintNewLine);
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

void SetupTime(char NtpServerName0[], int TimeZone0, bool DstActive0)
{
	Serial.print("Starting UDP time sync on port ");
	if (strlen(NtpServerName0) > 2) strncpy(NtpServerName, NtpServerName0, NTPSTRLEN - 1);
	TimeZone = TimeZone0;
	DstActive = DstActive0;

	NtpUdp.begin(localPort);
	Serial.println(NtpUdp.localPort());
	setSyncProvider(getNtpTime);
	if(timeStatus() != timeSet) {
		delay(500);
		setSyncProvider(getNtpTime); // retry
	}
	if (timeStatus() != timeSet) {
		delay(500);
		setSyncProvider(getNtpTime); // retry a second time
	}
	setSyncInterval(86400); // daily update v ESP8266 keeps time pretty good
}

void TimeTestLoop() //was main loop, just prints the time continuously
{
	if (timeStatus() != timeNotSet) {
		if (now() != prevDisplay) { //update the display only if time has changed
			prevDisplay = now();
			digitalClockDisplay(0);
		}
	}
}

void digitalClockDisplay(int StartNewLine)
{
	// digital clock display of the time
	if (now() > 10000) { //only print year if it has been set
		Serial.print(month());
		Serial.print("/");
		Serial.print(day());
		Serial.print("/");
		Serial.print(year());
		Serial.print(" ");
	}
	Serial.print(hour());
	printDigits(minute());
	printDigits(second());
	Serial.print(" ");
	if (StartNewLine) Serial.println();
}

char* TimeString()
{
	static char MyTimeStr[22];
	if (now() > 10000) //only print year if it has been set
		sprintf(MyTimeStr, "%d/%d/%d %02d:%02d:%02d ", month(), day(), year(), hour(), minute(), second() );
	else
		sprintf(MyTimeStr, "%02d:%02d:%02d ", hour(), minute(), second());
	return(MyTimeStr);
}

void printDigits(int digits)
{
	// utility for digital clock display: prints preceding colon and leading 0
	Serial.print(":");
	if (digits < 10)
		Serial.print('0');
	Serial.print(digits);
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte NTPpacketBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
	IPAddress ntpServerIP; // NTP server's ip address

	while (NtpUdp.parsePacket() > 0); // discard any previously received packets
	// get a random server from the pool
	WiFi.hostByName(NtpServerName, ntpServerIP);
	Serial.print(NtpServerName);
	Serial.print(": ");
	Serial.print(ntpServerIP);
	Serial.print("   ");
	sendNTPpacket(ntpServerIP);
	uint32_t beginWait = millis();
	while (millis() - beginWait < 2000) {
		int size = NtpUdp.parsePacket();
		if (size >= NTP_PACKET_SIZE) {
			Serial.println("NTP Response OK");
			NtpUdp.read(NTPpacketBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 = (unsigned long)NTPpacketBuffer[40] << 24;
			secsSince1900 |= (unsigned long)NTPpacketBuffer[41] << 16;
			secsSince1900 |= (unsigned long)NTPpacketBuffer[42] << 8;
			secsSince1900 |= (unsigned long)NTPpacketBuffer[43];
			return secsSince1900 - 2208988800UL + TimeZone * SECS_PER_HOUR + (DstActive? SECS_PER_HOUR : 0);
		}
	}
	Serial.println("NTP Server not responding");
	return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
	// set all bytes in the buffer to 0
	memset(NTPpacketBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	NTPpacketBuffer[0] = 0b11100011;   // LI, Version, Mode
	NTPpacketBuffer[1] = 0;     // Stratum, or type of clock
	NTPpacketBuffer[2] = 6;     // Polling Interval
	NTPpacketBuffer[3] = 0xEC;  // Peer Clock Precision
							 // 8 bytes of zero for Root Delay & Root Dispersion
	NTPpacketBuffer[12] = 49;
	NTPpacketBuffer[13] = 0x4E;
	NTPpacketBuffer[14] = 49;
	NTPpacketBuffer[15] = 52;
	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	NtpUdp.beginPacket(address, 123); //NTP requests are to port 123
	NtpUdp.write(NTPpacketBuffer, NTP_PACKET_SIZE);
	NtpUdp.endPacket();
}
