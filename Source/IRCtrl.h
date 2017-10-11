//Function code.  Header file vs. C file to be able to utilize globals. (I know, not good coding, but it works -JS)

//**IR Routines******************************************************************************************
//**IR Routines******************************************************************************************
//**IR Routines******************************************************************************************

//IR power pin connected to 3.3v
//#define IR_RX_PIN			D6	//Pin6,  GPIO12 IR receiver, receive pin when IrRemoteOn=True
//#define IR_GND_PIN			D7	//Pin7,  GPIO13 IR receiver, ground pin when IrRemoteOn=True

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
IRrecv MyIrRecv(IR_RX_PIN);
decode_results IrResults;
unsigned int LastIrCode = 0;
uint16_t CAPTURE_BUFFER_SIZE = 1024; //1024 is big enough for air conditioners, TVs should be much smaller

Ticker ClearIrCodeTicker;

const int NorcentCodeQty = 37;
const unsigned long NorcentCodes[] = { 0xEA7D1A2D,0x7EE63009,0x6E4EDAA5,0xD7EE0469,0xB5F783B1,0x689BBDC9,0xA5602E4D,0x30F289C9,0x17532CC5,0x65FE34ED,0x20CBB3C9,0x93BB41ED,0xC65EAD4D,0x812C2C29,0x8FA59C29,0xC2490789,0x7D168665,0x4FC4645,0x53F67069,0x197C5BED,0x86DEDAC9,0xA9F0ADE9,0xC166DBC5,0x5EA99E49,0xE46AC265,0x5CBD31C9,0x22431D4D,0x8390DD8D,0x7CCD4D09,0x5C4A2BAD,0xC9830D29,0x1DE03405,0xB7476185,0xBE0AF209,0x685228C9,0x1B9C68A9,0x272A3565 };
const char *NorcentStrings[] = { "Standby", "Prog", "Display", "OpenClose", "Reset", "Subtitle", "Language", "Angle", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Plus10", "Time", "Enter", "Left", "Right", "Up", "Down", "Setup", "Repeat", "VolDown", "VolUp", "Zoom", "Prev", "Next", "Rew", "FastFwd", "Pause", "Stop", "Play","XXerrXX" };

const int LedRemoteCodeQty = 21;
const unsigned long LedRemoteCodes[] = { 0x65FE34ED, 0x353CE3C9,0xD7EE0469,0xC50DCE9,0xF8C67AC5,0x3FD21A5,0x1D4AD685,0x86DEDAC9,0x8390DD8D,0xEA7D1A2D,0x54D5D969,0xC9830D29,0xE46AC265,0x59F75F25,0x7CCD4D09,0x947173A1,0xA9F0ADE9,0x22431D4D,0xC166DBC5,0x778D8C09,0x272A3565 };
const char *LedRemoteStrings[] = { "Power", "Source", "Mute", "Record", "ChUp", "TimeShift", "VolDown", "FullScreen", "VolUp", "0", "ChDown", "Recall", "1", "2", "3", "4", "5", "6", "7", "8", "9" , "XXerrXX"};

const int Sony0VideoQty = 37;
const unsigned long Sony0VideoCodes[] = { 0x17FF4E7, 0xD19B2B68, 0x4A767F5C, 0xA06EB70E, 0x6C8D26E2, 0x266EB071, 0xEAD278E2, 0x6F99D03C, 0x624A4781, 0x2A13B600, 0x2F5AA57F, 0x6DED89E4, 0xBDADC113, 0xBA30D01A, 0x94FDD53B, 0x84A5A28, 0xF8421DF, 0x9676CBA6, 0xDA509F8A, 0x5B528CCD, 0xCB17D8C5, 0x4ADFEC33, 0xDE39F8DA, 0xD230569E, 0x8E5682BA, 0x86D7155C, 0x2A2FE0BF, 0xCA478504, 0xA34D9AEC, 0xF9FA9477, 0x5085E1C8, 0x8E5C75CC, 0x9C4AD69C, 0xA878234, 0xA6A850A9, 0x1A035278, 0xB6CA49D3 };
const char *Sony0VideoStrings[] = { "Eject", "Power", "TV-Video", "Counter", "AudioMonitor", "Display", "Clear", "VcrPlus", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Enter", "SP/EP", "VolPlus", "ChPlus", "Input-Select", "VolDown", "ChDown", "Record", "x2Speed", "Slow", "IndexDown", "IndexUp", "Menu", "ComPass", "Play", "Left", "Right", "Up", "Down", "xxERRxx" };

const int SonyVideoQty = 37;
const unsigned long SonyVideoCodes[] = { 0x69A,0xA90,0xA50,0x83A,0xE90,0x5D0,0xC7A,0x35D,0x10,0x810,0x410,0xC10,0x210,0xA10,0x610,0xE10,0x110,0x910,0xD10,0x1BA,0x490,0x90,0xF3A,0xC90,0x890,0xB9A,0x29A,0xC5A,0xEBA,0x6BA,0xB3A,0xBBA,0xC5D,0x45D,0x445D,0x385D, 0x785D };
const char *SonyVideoStrings[] = { "Eject", "Power", "TV-Video", "Counter", "AudioMonitor", "Display", "Clear", "VcrPlus", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Enter", "SP/EP", "VolPlus", "ChPlus", "Input-Select", "VolDown", "ChDown", "Record", "x2Speed", "Slow", "IndexDown", "IndexUp", "Menu", "ComPass", "Play", "Left", "Right", "Up", "Down", "xxERRxx" };





void ClearIrCode() { LastIrCode = 0; }

void MyIrDecode(decode_results *results) {
	if (v.Dbug > 4 && v.Dbug <20) { // print out unknown codes so they can be added
		Serial.print("0x");
		Serial.print((unsigned long) results->value, HEX);
//		serialPrintUint64(results->value, HEX); //// print() & println() can't handle printing long longs. (uint64_t)
		Serial.print(", ");
	}

	for (int i = 0; i < SonyVideoQty; i++) {
		if (results->value == SonyVideoCodes[i]) {
			if (v.Dbug > 30) { snprintf(sBuff, SBUFLEN, "%s SonyVideo IR Code #%i = %s", HeaderString(), i, SonyVideoStrings[i]);	dPrint(sBuff); }
			switch (i) { // button numbers go left to right, top to bottom
			case 0:
				v.GlobalEnableOn = !v.GlobalEnableOn;
				if (v.Dbug > 3) { snprintf(sBuff, SBUFLEN, "%s Setting GlobalEnableOn =  %s", HeaderString(), (v.GlobalEnableOn) ? "True" : "False"); dPrint(sBuff); }
				UdpProp.beginPacket(udp_ip, v.UdpPropPort);
				UdpProp.print("C GlobalEnableOn " + (String)v.GlobalEnableOn);
				UdpProp.endPacket();
			case 1:
				snprintf(sBuff, SBUFLEN, "%s I need to power on something", HeaderString());
				dPrint(sBuff);
				break;
			case 2:
				if (v.IdentifyOn) v.IdentifyOn = false;
				else v.IdentifyOn = true;
				break;

			case 5: //display
				if (v.GlobalEnableOn) {
					char MySound[] = "HootHoot.wav";
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("S " + (String) MySound + " " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering sound %s %i", HeaderString(), MySound, v.PropArgument);
						dPrint(sBuff);
					}
				}
				break;
			case 6: //CLEAR
				if (v.GlobalEnableOn) {
					char MySound[] = "Chicken.wav";
						UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("S " + (String) MySound + " " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering sound %s %i", HeaderString(), MySound, v.PropArgument);
						dPrint(sBuff);
					}
				}
				break;
			case 7: //VCR+
				if (v.GlobalEnableOn) {
					char MySound[] = "CookPotTrauma.wav";
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("S " + (String) MySound + " " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering sound %s %i", HeaderString(), MySound, v.PropArgument);
						dPrint(sBuff);
					}
				}
				break;

					
			case 8:
				if (v.GlobalEnableOn) {
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("T " + (String) "One " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), "One", v.PropArgument); 
						dPrint(sBuff);
					}

				}
				break;
			case 9:
				if (v.GlobalEnableOn) {
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("T " + (String) "Two " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), "Two", v.PropArgument);
						dPrint(sBuff);
					}
				}
				break;
			case 10:
				if (v.GlobalEnableOn) {
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("T " + (String) "Three " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), "Three", v.PropArgument);
						dPrint(sBuff);
					}
				}
				break;
			case 11:
				if (v.GlobalEnableOn) {
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
//					UdpProp.print("T " + (String) "Four " + v.PropArgument + " FromRemote");
					UdpProp.print("S " + (String) "MIB_Angry.wav " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), "Four", v.PropArgument);
						dPrint(sBuff);
					}
				}
				break; 
			case 12:
				if (v.GlobalEnableOn) {
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
//					UdpProp.print("T " + (String) "Five " + v.PropArgument + " FromRemote");
					UdpProp.print("S " + (String) "MIB_Startle.wav " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), "Five", v.PropArgument);
						dPrint(sBuff);
					}
				}
				break; 
			case 13:
				if (v.GlobalEnableOn) {
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
//					UdpProp.print("T " + (String) "Six " + v.PropArgument + " FromRemote");
					UdpProp.print("S " + (String) "PopGoestheWeasel.wav " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), "Six", v.PropArgument);
						dPrint(sBuff);
					}
				}
				break; 
			case 14:
				if (v.GlobalEnableOn) {
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("S " + (String) "ScoobyDooGhost.wav " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), "Seven", v.PropArgument);
						dPrint(sBuff);
					}
				}
				break; 
			case 15:
				if (v.GlobalEnableOn) {
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("S " + (String) "Scream.wav " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("T " + (String) "Eight " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), "Eight", v.PropArgument);
						dPrint(sBuff);
					}
				}
				break; 
			case 16:
				if (v.GlobalEnableOn) {
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("S " + (String) "Snakes.wav " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					delay(3);
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("T " + (String) "Nine " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), "Nine", v.PropArgument);
						dPrint(sBuff);
					}
				}
				break; 
			case 17:  //"0" key
				if (v.GlobalEnableOn) {
					PropTriggered = true;
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering Local Prop %s from IR", HeaderString(), v.PropName);
						dPrint(sBuff);
					}

				}
				break; 
			case 32: // Play key
				if (v.GlobalEnableOn) {
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("S " + (String) "E/Bradlyfire.wav " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					delay(1);
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("T " + (String) "Mirror " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i from IR", HeaderString(), "Mirror ", v.PropArgument);
						dPrint(sBuff);
					}
				}
				break;
			case 99:
				if (v.GlobalEnableOn) {
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("T " + (String) "tbd" + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					snprintf(sBuff, SBUFLEN, "%s Triggering prop 'tbd'", HeaderString());
					dPrint(sBuff);
				}
				break; 
			}
		}
	}

	for (int i = 0; i < LedRemoteCodeQty; i++) {
		if (results->value == LedRemoteCodes[i]) {
			if (v.Dbug > 3) { snprintf(sBuff, SBUFLEN, "%s Chicklet IR Code #%i = %s", HeaderString(), i, LedRemoteStrings[i]);	dPrint(sBuff); }
			switch (i) {
			case 1: 
				snprintf(sBuff, SBUFLEN, "%s I need to power on something. The little remote said so", HeaderString());
				dPrint(sBuff);
				break;
			case 16: 
				Serial.println("Switch 34");
				break;
			}
		}
	}

	//note: Norcent code removed. too many conflicts with chicklet remote

}


void  ircode(decode_results *results)
{
	// Print Code
	Serial.print("0x");
	Serial.print((unsigned long)results->value, HEX);
//	serialPrintUint64(results->value, HEX); //// print() & println() can't handle printing long longs. (uint64_t)
	Serial.print(", ");


}

void encoding(decode_results *results) {
	switch (results->decode_type) {
	default:
	case UNKNOWN:      Serial.print("UNKNOWN");       break;
	case NEC:          Serial.print("NEC");           break;
	case NEC_LIKE:     Serial.print("NEC (non-strict)");  break;
	case SONY:         Serial.print("SONY");          break;
	case RC5:          Serial.print("RC5");           break;
	case RC5X:         Serial.print("RC5X");          break;
	case RC6:          Serial.print("RC6");           break;
	case RCMM:         Serial.print("RCMM");          break;
	case DISH:         Serial.print("DISH");          break;
	case SHARP:        Serial.print("SHARP");         break;
	case JVC:          Serial.print("JVC");           break;
	case SANYO:        Serial.print("SANYO");         break;
	case SANYO_LC7461: Serial.print("SANYO_LC7461");  break;
	case MITSUBISHI:   Serial.print("MITSUBISHI");    break;
	case SAMSUNG:      Serial.print("SAMSUNG");       break;
	case LG:           Serial.print("LG");            break;
	case WHYNTER:      Serial.print("WHYNTER");       break;
	case AIWA_RC_T501: Serial.print("AIWA_RC_T501");  break;
	case PANASONIC:    Serial.print("PANASONIC");     break;
	case DENON:        Serial.print("DENON");         break;
	case COOLIX:       Serial.print("COOLIX");        break;
	}
	if (results->repeat) Serial.print(" (Repeat)");
}

// Dump out the decode_results structure.
//
void dumpInfo(decode_results *results) {
	if (results->overflow)
		Serial.printf("WARNING: IR code too big for buffer (>= %d). "
			"These results shouldn't be trusted until this is resolved. "
			"Edit & increase CAPTURE_BUFFER_SIZE.\n",
			CAPTURE_BUFFER_SIZE);

	// Show Encoding standard
	Serial.print("Encoding  : ");
	encoding(results);
	Serial.println("");

	// Show Code & length
	Serial.print("Code      : ");
	serialPrintUint64(results->value, 16);
	Serial.print(" (");
	Serial.print(results->bits, DEC);
	Serial.println(" bits)");
}

uint16_t getCookedLength(decode_results *results) {
	uint16_t length = results->rawlen - 1;
	for (uint16_t i = 0; i < results->rawlen - 1; i++) {
		uint32_t usecs = results->rawbuf[i] * RAWTICK;
		// Add two extra entries for multiple larger than UINT16_MAX it is.
		length += (usecs / UINT16_MAX) * 2;
	}
	return length;
}

// Dump out the decode_results structure.
//
void dumpRaw(decode_results *results) {
	// Print Raw data
	Serial.print("Timing[");
	Serial.print(results->rawlen - 1, DEC);
	Serial.println("]: ");

	for (uint16_t i = 1; i < results->rawlen; i++) {
		if (i % 100 == 0)
			yield();  // Preemptive yield every 100th entry to feed the WDT.
		if (i % 2 == 0) {  // even
			Serial.print("-");
		}
		else {  // odd
			Serial.print("   +");
		}
		Serial.printf("%6d", results->rawbuf[i] * RAWTICK);
		if (i < results->rawlen - 1)
			Serial.print(", ");  // ',' not needed for last one
		if (!(i % 8)) Serial.println("");
	}
	Serial.println("");  // Newline
}

// Dump out the decode_results structure.
//
void dumpCode(decode_results *results) {
	// Start declaration
	Serial.print("uint16_t ");               // variable type
	Serial.print("rawData[");                // array name
	Serial.print(getCookedLength(results), DEC);  // array size
	Serial.print("] = {");                   // Start declaration

											 // Dump data
	for (uint16_t i = 1; i < results->rawlen; i++) {
		uint32_t usecs;
		for (usecs = results->rawbuf[i] * RAWTICK;
			usecs > UINT16_MAX;
			usecs -= UINT16_MAX)
			Serial.printf("%d, 0", UINT16_MAX);
		Serial.print(usecs, DEC);
		if (i < results->rawlen - 1)
			Serial.print(", ");  // ',' not needed on last one
		if (i % 2 == 0) Serial.print(" ");  // Extra if it was even.
	}

	// End declaration
	Serial.print("};");  //

						 // Comment
	Serial.print("  // ");
	encoding(results);
	Serial.print(" ");
	serialPrintUint64(results->value, HEX);

	// Newline
	Serial.println("");

	// Now dump "known" codes
	if (results->decode_type != UNKNOWN) {
		// Some protocols have an address &/or command.
		// NOTE: It will ignore the atypical case when a message has been decoded
		// but the address & the command are both 0.
		if (results->address > 0 || results->command > 0) {
			Serial.print("uint32_t address = 0x");
			Serial.print(results->address, HEX);
			Serial.println(";");
			Serial.print("uint32_t command = 0x");
			Serial.print(results->command, HEX);
			Serial.println(";");
		}

		// All protocols have data
		Serial.print("uint64_t data = 0x");
		serialPrintUint64(results->value, 16);
		Serial.println(";");
	}
}

void ProcessIrCode(decode_results *results) {
	
	if (results->value == LastIrCode) {
		MyIrRecv.resume();
		return;
	}
	
	MyIrDecode(results);
	//			dumpInfo(results);           // Output the results
	//			dumpRaw(results);            // Output the results in RAW format
	//			dumpCode(results);           // Output the results as source code
				Serial.println("");           // Blank line between entries
	MyIrRecv.resume();              // Prepare for the next value
	LastIrCode = results->value;
	ClearIrCodeTicker.once_ms(1000, ClearIrCode); // clear the IR code in one second to prevent multiple triggers
}

