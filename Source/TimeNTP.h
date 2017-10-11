// TimeNTP_ESP8266WiFi.h

#ifndef _TIMENTP_ESP8266WIFI_h
#define _TIMENTP_ESP8266WIFI_h

#include <TimeLib.h>		//https://github.com/PaulStoffregen/Time
#include <Time.h>
#include <WiFiUdp.h>		//used by time demo

void digitalClockDisplay(int PrintNewLine);
void TimeTestLoop(); 
void SetupTime(char NtpServerName0[], int TimeZone0, bool DstActive0);
char* TimeString();


#endif