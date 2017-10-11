#ifndef OTA_H
#define OTH_H

#include <ArduinoOTA.h>		//https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA

void Initialize_OTA() { //=================================================================================
						// ArduinoOTA.setPort(8266);						// Port defaults to 8266
						// ArduinoOTA.setPassword((const char *)"123");	// No authentication by default
	ArduinoOTA.setHostname(v.DeviceName);
	ArduinoOTA.onStart([]() {
		Serial.println("OTA update starting");
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("OTA updated complete");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("OTA Progress: %u%%\n", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("OTA update Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("OTA Update Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("OTA Update Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("OTA Update Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA Update Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("OTA Update End Failed");
	});
	ArduinoOTA.begin();
	Serial.println("OTA firmware update service initialized");
}

#endif
