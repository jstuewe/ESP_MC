//Function code.  Header file vs. C file to be able to utilize globals. (I know, not good coding, but it works -JS)

Ticker  PropOnTicker[5];
Ticker  PropOffTicker[5];
Ticker  SoundTicker;

void SetRelayOn(int PropNum) {
	switch (PropNum) {
	case 0:	pinMode(RELAY_PIN0, OUTPUT); digitalWrite(RELAY_PIN0, (v.PropInvertSig ? LOW : HIGH)); break; //high is OFF for new relay boards, High is ON for single relays and old relay board
	case 1:	pinMode(RELAY_PIN1, OUTPUT); digitalWrite(RELAY_PIN1, (v.PropInvertSig ? LOW : HIGH)); break;
	case 2:	pinMode(RELAY_PIN2, OUTPUT); digitalWrite(RELAY_PIN2, (v.PropInvertSig ? LOW : HIGH)); break;
	case 3:	pinMode(RELAY_PIN3, OUTPUT); digitalWrite(RELAY_PIN3, (v.PropInvertSig ? LOW : HIGH)); break;
	}
}
void SetRelayOff_OLD(int PropNum) {
	switch (PropNum) {
	case 0:	pinMode(RELAY_PIN0, INPUT); break;
	case 1:	pinMode(RELAY_PIN1, INPUT); break;
	case 2:	pinMode(RELAY_PIN2, INPUT); break;
	case 3:	pinMode(RELAY_PIN3, INPUT); break;
	}
}

void SetRelayOff(int PropNum) {
	switch (PropNum) {
	case 0:	pinMode(RELAY_PIN0, INPUT); digitalWrite(RELAY_PIN0, LOW); break; // Low is ON for new relay boards, Low is off for the old ones.
	case 1:	pinMode(RELAY_PIN1, INPUT); digitalWrite(RELAY_PIN1, LOW); break;
	case 2:	pinMode(RELAY_PIN2, INPUT); digitalWrite(RELAY_PIN2, LOW); break;
	case 3:	pinMode(RELAY_PIN3, INPUT); digitalWrite(RELAY_PIN3, LOW); break;
	}
}

void SetOutletOn(int PropNum) {
	switch (PropNum) {
	case 0:	SetOutletState(1, true); break;
	case 1:	SetOutletState(2, true); break;
	case 2:	SetOutletState(3, true); break;
	case 3:	SetOutletState(4, true); break;
	case 4:	SetOutletState(5, true); break;
	}
}
void SetOutletOff(int PropNum) {
	switch (PropNum) {
	case 0:	SetOutletState(1, false); break;
	case 1:	SetOutletState(2, false); break;
	case 2:	SetOutletState(3, false); break;
	case 3:	SetOutletState(4, false); break;
	case 4:	SetOutletState(5, false); break;
	}
}


void PropRelayTrigger() {
	for (int i = 0; i <= 3; i++) {
		if (v.PropOnTime[i] > 0) {
			if (v.PropDelay[i] == 0) SetRelayOn(i);
			else PropOnTicker[i].once_ms(v.PropDelay[i], SetRelayOn, i);
			PropOffTicker[i].once_ms(v.PropDelay[i] + v.PropOnTime[i], SetRelayOff, i);
			if (v.Dbug > 5) {
				snprintf(sBuff, SBUFLEN, "%s Prop '%s' Relay%i delaying for %ims and staying on for %ims", HeaderString(), v.PropName, i, v.PropDelay[i], v.PropOnTime[i]);
				dPrint(sBuff);
			}

		}
	}
}

void PropAcOutletTrigger() {
	MyRfSwitch.enableTransmit(RC_TX_PIN);
	for (int i = 0; i <= 3; i++) {
		if (v.PropOnTime[i] > 0) {
			if (v.PropDelay[i] == 0) SetOutletOn(i);
			else PropOnTicker[i].once_ms(v.PropDelay[i], SetOutletOn, i);
			PropOffTicker[i].once_ms(v.PropDelay[i] + v.PropOnTime[i], SetOutletOff, i);
			if (v.Dbug > 5) {
				snprintf(sBuff, SBUFLEN, "%s Prop '%s' AC Outlet %i delaying for %ims and staying on for %ims", HeaderString(), v.PropName, i, v.PropDelay[i], v.PropOnTime[i]);
				dPrint(sBuff);
			}

		}
	}

}


void FogOn(int arg1) { // fog fury jet has five modes D1,D2,D3,D5,& D7.  This program assumes three channel mode D3 and address A.0001
	int FogColor  = (v.PropArgument > 0 ? v.PropArgument : 32);
	int ColorMode = (v.PropCycle[0] > 0 ? v.PropCycle[0] : 100);
	dmx.write(DMXBase + 0, 100);       //ch1 = fog.  >=32 turns fog on
	dmx.write(DMXBase + 1, FogColor);  //ch2 = color. 0=off, 1-255 are different colors\color macros. Only active when fog is active
	dmx.write(DMXBase + 2, ColorMode); //ch3 = LED mode. <32=off, <96=strobe slo2->fast, <159=pulse effect, <160=random strob slow->fast
	dmx.update();                // update the DMX bus
}

void FogOff(int arg1) {
	dmx.write(DMXBase + 0, 0);  // Fog
//	dmx.write(DMXBase + 1, 0);  // Light
//	dmx.write(DMXBase + 2, 0);  // Light mode
	dmx.update();               // update the DMX bus
	SuppressLED = false;
}

void FogTrigger() { // fog fury jet has five modes D1,D2,D3,D5,& D7.  This program assumes three channel mode D3 and address A.0001
	if (v.PropOnTime[0] > 0) {
		SuppressLED = true; // same pin as SERIAL1 on the Wemos D1
		if (v.PropDelay[0] == 0) FogOn(0);
		else PropOnTicker[0].once_ms(v.PropDelay[0], FogOn, 0);
		PropOffTicker[0].once_ms(v.PropDelay[0] + v.PropOnTime[0], FogOff, 0);
		if (v.Dbug > 5) {
			snprintf(sBuff, SBUFLEN, "%s Prop '%s' Fog Machine Triggered for %ims", HeaderString(), v.PropName, v.PropOnTime[0]);
			dPrint(sBuff);
		}
	}
}

void PrintDfPlayerStatus(uint8_t type, int value) {
	switch (type) {
	case TimeOut:
		Serial.println(F("DF SD Player Time Out!"));
		break;
	case WrongStack:
		Serial.println(F("DF SD Player Stack Wrong!"));
		break;
	case DFPlayerCardInserted:
		Serial.println(F("DF SD Player Card Inserted!"));
		break;
	case DFPlayerCardRemoved:
		Serial.println(F("DF SD Player Card Removed!"));
		break;
	case DFPlayerCardOnline:
		Serial.println(F("DF SD Player Card Online!"));
		break;
	case DFPlayerPlayFinished:
		Serial.print(F("DF SD Player Track Number:"));
		Serial.print(value);
		Serial.println(F(" Play Finished!"));
		break;
	case DFPlayerError:
		Serial.print(F("DFPlayerError:"));
		switch (value) {
		case Busy:
			Serial.println(F("Card not found"));
			break;
		case Sleeping:
			Serial.println(F("Sleeping"));
			break;
		case SerialWrongStack:
			Serial.println(F("Get Wrong Stack"));
			break;
		case CheckSumNotMatch:
			Serial.println(F("Check Sum Not Match"));
			break;
		case FileIndexOut:
			Serial.println(F("File Index Out of Bound"));
			break;
		case FileMismatch:
			Serial.println(F("Cannot Find File"));
			break;
		case Advertise:
			Serial.println(F("In Advertise"));
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void TriggerSound() {
	if (v.DFPlayerOn) {
		int DFPFileNum = atoi(v.PropAudioName);
		if (DFPFileNum == 0) DFPFileNum = 1;
		int DFPVolume = (v.PropArgument > 0 ? v.PropArgument : (PropArgument>0) ? PropArgument : 20 ); //use local if non zero, then remote if non-zero, then default
		if (DFPVolume > 30) DFPVolume = 30; // don't go over max volume
		if (v.Dbug > 2) {
			snprintf(sBuff, SBUFLEN, "%s Playing DFPlayer MP3 file# %i at volume %i", HeaderString(), DFPFileNum, DFPVolume);
			dPrint(sBuff);
			}

//		if (myDFPlayer.available()) PrintDfPlayerStatus(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
		myDFPlayer.volume(DFPVolume);  //Set volume value (0~30).
		myDFPlayer.play(DFPFileNum);  //Play the /mp3/00xx.mp3 file on the SD card
//		if (myDFPlayer.available()) PrintDfPlayerStatus(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
		}
	else { // send it to the main audio board
		UdpProp.beginPacket(udp_ip, v.UdpPropPort);
		UdpProp.print("S " + (String)v.PropAudioName + " " + v.PropArgument + " For " + v.PropName);
		UdpProp.endPacket();
		}
}

void PropHandle() {
	if (PropTriggered) {
		if (v.Dbug > 0) { // && v.Dbug < 5) {
			snprintf(sBuff, SBUFLEN, "%s Prop '%s' has been triggered", HeaderString(), v.PropName);
			dPrint(sBuff);
		}

		if (strlen(v.PropAudioName) > 3 || v.DFPlayerOn) { // There is a sound programmed with the prop
			if (v.PropAudioDelay) 
				SoundTicker.once_ms(v.PropAudioDelay, TriggerSound); // trigger the sound after a delay
			else 
				TriggerSound();
		}
		
		if (strcasecmp(v.PropName, "AcSwitch")==0 || strcasecmp(v.PropName,"AcSwitch2") == 0) { 
			PropAcOutletTrigger(); // up to 4 AC outlets controlled
		}
		else if      (strcasecmp(v.PropName,"One")==0) {
//			Serial.print("One");
			PropRelayTrigger();	// up to 4 relays controlled
		} 
		else if (strcasecmp(v.PropName,"Two")==0) {
//			Serial.print("Two");
			PropRelayTrigger();	// up to 4 relays controlled

		}
		else if (strncmp(v.PropName,"Mirror",10)==0) {
			if (!MRState) MRState++; // start Mirrorroom state machine if it's idle
		}

		else if (strcasecmp(v.PropName, "Fog") == 0) {
			FogTrigger();
		}
		
		else { // Standard relay prop
			PropRelayTrigger();	// up to 4 relays controlled
		}
		PropCnt += 1;
		PropTriggered = false;
	}

	if ((MRState != 0) || (MRState != MRLastState)) PropHandle_MirrorRoom();

	//handle any other Prop sequencing after trigger
}

