//Function code.  Header file vs. C file to be able to utilize globals. (I know, not good coding, but it works -JS)

//**RF Routines******************************************************************************************
//**RF Routines******************************************************************************************
//**RF Routines******************************************************************************************

const int Rf1Qty = 11;
const unsigned long Rf1Codes[] = { 10820353,10820356,10820357,   10820358,    10820359,10820360,10820361,10820363,  10820365,   10820367,   10820369 };
const char *Rf1Strings[] =      { "Power", "Light", "BrightUp", "BrightDown", "Full", "Half", "Quarter", "ModeUp", "SpeedDown", "SpeedUp", "ModeDown", "xxERRxx" };

const int Rf2Qty = 11;
const unsigned long Rf2Codes[] = { 15535105,15535108,15535109,  15535110,     15535111,15535112,15535113,15535115,  15535117,   15535119,   15535121 };
const char *Rf2Strings[] =      { "Power", "Light", "BrightUp", "BrightDown", "Full", "Half", "Quarter", "ModeUp", "SpeedDown", "SpeedUp", "ModeDown", "xxERRxx" };

const int Rf3Qty = 10;
const unsigned long Rf3Codes[] = { 626689,626693,626695,626696,626697,626698,626699,626700,626701,626703, };
const char *Rf3Strings[] = { "Power", "ModeUp", "SpeedDown", "Demo", "SpeedUp", "ColorUp", "ModeDown", "BrightUp", "ColorDown", "BrightDown", "xxERRxx" };

const int FogQty = 4;
const int FogPulseLength = 196;
const unsigned long FogCodes[] = { 5570572,5570608,5570752,5570563,0};
const char *FogStrings[] = { "ButtonA", "ButtonB", "ButtonC", "ButtonD", "xxERRxx" };

#include <RCSwitch.h>

//RC 433mhz TX and 433mhz RX modules connected to +5v and GND
//#define RC_RX_PIN	        D2  //Pin19, GPIO4 433mhz receive pin when IrRemoteOn=True
//#define RC_TX_PIN         D5  //Pin5,  GPIO14 433mhz transmitter pin when IrRemoteOn=True.

#define RC_PROTOCOL     1
#define RC_PULSE_LENGTH 184 // 'Delay' between 433mhz pulses. Remote ZAP outlets is 184. remote LED strings are 391
#define RC_BIT_LENGTH   24
unsigned long rc_codes[5][2] = {  // Array of ON/OFF codes. Must be replaced with codes obtained using a sniffer. This matches the 5 outlet remote.
								  // ON     //OFF 
	{ 4543795,4543804 }, /* Outlet 1   remote labeled '0302' */
	{ 4543939,4543948 }, /* Outlet 2 */
	{ 4544259,4544268 }, /* Outlet 3 */
	{ 4545795,4545804 }, /* Outlet 4 */
	{ 4551939,4551948 }, /* Outlet 5 */

};

RCSwitch MyRfSwitch = RCSwitch();
Ticker ClearLastRfCodeTicker;
long int  LastRfCode = 0;

void ClearRfCode() { LastRfCode = 0; }

void InitRfTx(void) {
	MyRfSwitch.enableTransmit(RC_TX_PIN);               // configure physical pin that 433mhz transmitter is on
	MyRfSwitch.setProtocol(RC_PROTOCOL);                // defaults to 1 anyway
	MyRfSwitch.setPulseLength(RC_PULSE_LENGTH);         // this is critical set RF pulse length
//    mySwitch.setRepeatTransmit(2);
	if (v.Dbug > 20) {
		snprintf(sBuff, SBUFLEN, "%s Prop '%s' RC Switch initialized.", HeaderString(), v.PropName);
		dPrint(sBuff);
	}
}

void SetFogState(int ButtonNumber)  //=================================================================================
{
	MyRfSwitch.setPulseLength(FogPulseLength);
	MyRfSwitch.send(FogCodes[ButtonNumber-1], RC_BIT_LENGTH);
}


void SetOutletState(int OutletNumber, bool onOrOff)  //=================================================================================
{
	MyRfSwitch.setPulseLength(RC_PULSE_LENGTH); //JS Need to verify 10/1/17
	MyRfSwitch.send(rc_codes[OutletNumber - 1][onOrOff ? 0 : 1], RC_BIT_LENGTH);
}

void TestOutletFunction(int myDelay) { //=================================================================================
	dPrint("Starting outlet test sequence");
	SetOutletState(1, true);
	delay(myDelay);
	SetOutletState(2, true);
	delay(myDelay);
	SetOutletState(3, true);
	delay(myDelay);
	SetOutletState(4, true);
	delay(myDelay);
	SetOutletState(5, true);
	delay(myDelay);
	SetOutletState(1, false);
	delay(myDelay);
	SetOutletState(2, false);
	delay(myDelay);
	SetOutletState(3, false);
	delay(myDelay);
	SetOutletState(4, false);
	delay(myDelay);
	SetOutletState(5, false);
	delay(myDelay);
	dPrint("Ending outlet test sequence");
}

void ProcessRfCode(unsigned long int value) {
	if (v.Dbug > 10 && v.Dbug <=30 ) { // print out unknown codes so they can be added
		Serial.print("0x");
		Serial.print(value, HEX);
		Serial.print(", ");
	}
	for (int i = 0; i < Rf1Qty; i++) {
		if (value == Rf1Codes[i]) {
			if (v.Dbug > 30) { snprintf(sBuff, SBUFLEN, "%s RF remote 1 Code #%i = %s", HeaderString(), i, Rf1Strings[i]);	dPrint(sBuff); }
				switch (i) { // button numbers go left to right, top to bottom
					case 0: //power
						snprintf(sBuff, SBUFLEN, "%s I need to power on something", HeaderString());
						dPrint(sBuff);
						break;
					case 1:  //light
						if (v.GlobalEnableOn) {
							PropTriggered = true;
							if (v.Dbug > 3) {
								snprintf(sBuff, SBUFLEN, "%s Triggering Local Prop %s from IR", HeaderString(), v.PropName);
								dPrint(sBuff);
							}
						}
						break;
					case 2: //BrightUp
							// v.GlobalEnableOn = !v.GlobalEnableOn;
						v.GlobalEnableOn = true;
						if (v.Dbug > 3) { snprintf(sBuff, SBUFLEN, "%s Setting GlobalEnableOn = %s", HeaderString(), (v.GlobalEnableOn) ? "True" : "False"); dPrint(sBuff); }
						UdpProp.beginPacket(udp_ip, v.UdpPropPort);
						UdpProp.print("C GlobalEnableOn " + (String)v.GlobalEnableOn);
						UdpProp.endPacket();
						break;
					case 3: //Brightdown
						v.GlobalEnableOn = false;
						if (v.Dbug > 3) { snprintf(sBuff, SBUFLEN, "%s Setting GlobalEnableOn = %s", HeaderString(), (v.GlobalEnableOn) ? "True" : "False"); dPrint(sBuff); }
						UdpProp.beginPacket(udp_ip, v.UdpPropPort);
						UdpProp.print("C GlobalEnableOn " + (String)v.GlobalEnableOn);
						UdpProp.endPacket();
						break;
					case 4: //full
						v.LedOn = true;
						if (v.Dbug > 3) { snprintf(sBuff, SBUFLEN, "%s Setting LedOn =  %s", HeaderString(), (v.LedOn) ? "True" : "False"); dPrint(sBuff); }
						UdpProp.beginPacket(udp_ip, v.UdpPropPort);
						UdpProp.print("C LedOn " + (String)v.LedOn);
						UdpProp.endPacket();
						break;
					case 5: //half
						if (v.IdentifyOn) v.IdentifyOn = false;
						else v.IdentifyOn = true;
						UdpProp.beginPacket(udp_ip, v.UdpPropPort);
						UdpProp.print("L LocalTimeSynchPulse " + (String)v.DeviceName);
						UdpProp.endPacket();

						if (v.Dbug > 3) { snprintf(sBuff, SBUFLEN, "%s Setting IdentifyOn =  %s", HeaderString(), (v.IdentifyOn) ? "True" : "False"); dPrint(sBuff); }
						UdpProp.beginPacket(udp_ip, v.UdpPropPort);
						UdpProp.print("C IdentifyOn " + (String)v.IdentifyOn);
						UdpProp.endPacket();
						digitalWrite(BUILTIN_LED, 1);
						break;
					case 6: //quarter
						v.LedOn = false;
						if (v.Dbug > 3) { snprintf(sBuff, SBUFLEN, "%s Setting LedOn =  %s", HeaderString(), (v.LedOn) ? "True" : "False"); dPrint(sBuff); }
						UdpProp.beginPacket(udp_ip, v.UdpPropPort);
						UdpProp.print("C LedOn " + (String)v.LedOn);
						UdpProp.endPacket();
						digitalWrite(BUILTIN_LED, 1);
						break;
					case 7: //modeup
						if (v.GlobalEnableOn) {
							char MySound[] = "Chicken.wav";
							UdpProp.beginPacket(udp_ip, v.UdpPropPort);
							UdpProp.print("S " + (String)MySound + " " + v.PropArgument + " FromRemote");
							UdpProp.endPacket();
							SetFogState(1);
							if (v.Dbug > 3) {
//								snprintf(sBuff, SBUFLEN, "%s Triggering sound %s %i", HeaderString(), MySound, v.PropArgument);
								snprintf(sBuff, SBUFLEN, "%s Triggering Fog machine button %i", HeaderString(), 1);
								dPrint(sBuff);
							}


						}
						break;
					case 8: //speeddown
						if (v.GlobalEnableOn) {
							char MyProp[] = "One";
							UdpProp.beginPacket(udp_ip, v.UdpPropPort);
							UdpProp.print("T " + (String) MyProp + " "+ v.PropArgument + " FromRemote");
							UdpProp.endPacket();
							SetFogState(2);
							if (v.Dbug > 3) {
//								snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), MyProp, v.PropArgument);
								snprintf(sBuff, SBUFLEN, "%s Triggering Fog machine button %i", HeaderString(),2);
								dPrint(sBuff);
							}
						}
						break;
					case 9: //speedup
						if (v.GlobalEnableOn) {
							char MyProp[] = "Two";
							UdpProp.beginPacket(udp_ip, v.UdpPropPort);
							UdpProp.print("T " + (String)MyProp + " " + v.PropArgument + " FromRemote");
							UdpProp.endPacket();
							SetFogState(3);

							if (v.Dbug > 3) {
//								snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), MyProp, v.PropArgument);
								snprintf(sBuff, SBUFLEN, "%s Triggering Fog machine button %i", HeaderString(), 3);
								dPrint(sBuff);
							}
						}
						break;
					case 10: //modeup
						if (v.GlobalEnableOn) {
							char MyProp[] = "Mirror";
							UdpProp.beginPacket(udp_ip, v.UdpPropPort);
							UdpProp.print("T " + (String)MyProp + " " + v.PropArgument + " FromRemote");
							UdpProp.endPacket();
							SetFogState(4);

							if (v.Dbug > 3) {
//								snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), MyProp, v.PropArgument);
								snprintf(sBuff, SBUFLEN, "%s Triggering Fog machine button %i", HeaderString(), 4);
								dPrint(sBuff);
							}
						}

					} //case
				} // value=cide[i]
			} // for i


	for (int i = 0; i < Rf2Qty; i++) {
		if (value == Rf2Codes[i]) {
			if (v.Dbug > 30) { snprintf(sBuff, SBUFLEN, "%s RF remote 2 Code #%i = %s", HeaderString(), i, Rf2Strings[i]);	dPrint(sBuff); }
			switch (i) { // button numbers go left to right, top to bottom
			case 0: //power
				snprintf(sBuff, SBUFLEN, "%s I need to power on something", HeaderString());
				dPrint(sBuff);
				break;
			case 1:  //light
				if (v.GlobalEnableOn) {
					PropTriggered = true;
					if (v.Dbug > 3) {
						snprintf(sBuff, SBUFLEN, "%s Triggering Local Prop %s from IR", HeaderString(), v.PropName);
						dPrint(sBuff);
					}
				}
				break;
			case 2: //BrightUp
					// v.GlobalEnableOn = !v.GlobalEnableOn;
				v.GlobalEnableOn = true;
				if (v.Dbug > 3) { snprintf(sBuff, SBUFLEN, "%s Setting GlobalEnableOn =  %s", HeaderString(), (v.GlobalEnableOn) ? "True" : "False"); dPrint(sBuff); }
				UdpProp.beginPacket(udp_ip, v.UdpPropPort);
				UdpProp.print("C GlobalEnableOn " + (String)v.GlobalEnableOn);
				UdpProp.endPacket();
				break;
			case 3: //Brightdown
				v.GlobalEnableOn = false;
				if (v.Dbug > 3) { snprintf(sBuff, SBUFLEN, "%s Setting GlobalEnableOn =  %s", HeaderString(), (v.GlobalEnableOn) ? "True" : "False"); dPrint(sBuff); }
				UdpProp.beginPacket(udp_ip, v.UdpPropPort);
				UdpProp.print("C GlobalEnableOn " + (String)v.GlobalEnableOn);
				UdpProp.endPacket();
				break;
			case 4: //full
				v.LedOn = true;
				if (v.Dbug > 3) { snprintf(sBuff, SBUFLEN, "%s Setting LedOn =  %s", HeaderString(), (v.LedOn) ? "True" : "False"); dPrint(sBuff); }
				UdpProp.beginPacket(udp_ip, v.UdpPropPort);
				UdpProp.print("C LedOn " + (String)v.LedOn);
				UdpProp.endPacket();
				break;
			case 5: //half
				if (v.IdentifyOn) v.IdentifyOn = false;
				else v.IdentifyOn = true;
				if (v.Dbug > 3) { snprintf(sBuff, SBUFLEN, "%s Setting Idenfity =  %s", HeaderString(), (v.IdentifyOn) ? "True" : "False"); dPrint(sBuff); }
				UdpProp.beginPacket(udp_ip, v.UdpPropPort);
				UdpProp.print("C IdentifyOn " + (String)v.IdentifyOn);
				UdpProp.endPacket();

				break;
			case 6: //quarter
				v.LedOn = false;
				if (v.Dbug > 3) { snprintf(sBuff, SBUFLEN, "%s Setting LedOn =  %s", HeaderString(), (v.LedOn) ? "True" : "False"); dPrint(sBuff); }
				UdpProp.beginPacket(udp_ip, v.UdpPropPort);
				UdpProp.print("C LedOn " + (String)v.LedOn);
				UdpProp.endPacket();
				break;
			case 7: //modeup
				if (v.GlobalEnableOn) {
					char MySound[] = "Chicken.wav";
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("S " + (String)MySound + " " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					SetFogState(1);
					if (v.Dbug > 3) {
						//								snprintf(sBuff, SBUFLEN, "%s Triggering sound %s %i", HeaderString(), MySound, v.PropArgument);
						snprintf(sBuff, SBUFLEN, "%s Triggering Fog machine button %i", HeaderString(), 1);
						dPrint(sBuff);
					}


				}
				break;
			case 8: //speeddown
				if (v.GlobalEnableOn) {
					char MyProp[] = "One";
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("T " + (String)MyProp + " " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					SetFogState(2);
					if (v.Dbug > 3) {
						//								snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), MyProp, v.PropArgument);
						snprintf(sBuff, SBUFLEN, "%s Triggering Fog machine button %i", HeaderString(), 2);
						dPrint(sBuff);
					}
				}
				break;
			case 9: //speedup
				if (v.GlobalEnableOn) {
					char MyProp[] = "Two";
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("T " + (String)MyProp + " " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					SetFogState(3);

					if (v.Dbug > 3) {
						//								snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), MyProp, v.PropArgument);
						snprintf(sBuff, SBUFLEN, "%s Triggering Fog machine button %i", HeaderString(), 3);
						dPrint(sBuff);
					}
				}
				break;
			case 10: //modeup
				if (v.GlobalEnableOn) {
					char MyProp[] = "Mirror";
					UdpProp.beginPacket(udp_ip, v.UdpPropPort);
					UdpProp.print("T " + (String)MyProp + " " + v.PropArgument + " FromRemote");
					UdpProp.endPacket();
					SetFogState(4);

					if (v.Dbug > 3) {
						//								snprintf(sBuff, SBUFLEN, "%s Triggering prop %s %i", HeaderString(), MyProp, v.PropArgument);
						snprintf(sBuff, SBUFLEN, "%s Triggering Fog machine button %i", HeaderString(), 4);
						dPrint(sBuff);
					}
				}

			} //case
		} // value=cide[i]
	} // for i

} //ptocessRFcode


void ScanRfSwitch(void) {  //=================================================================================
	if (MyRfSwitch.available()) {
		//Note: Ensure it's enabled with  mySwitch.enableReceive(RECPIN);  // 433Mhz recevier data pin for learning
		int value = MyRfSwitch.getReceivedValue();
		if (value == 0) {
			Serial.print("Unknown RF encoding");
			MyRfSwitch.resetAvailable();
			return;
		}
		if (value == LastRfCode) {
			MyRfSwitch.resetAvailable();
			return;
		}
		if (v.Dbug > 3) {
			Serial.print("433Mhz Code Received: ");
			Serial.print(MyRfSwitch.getReceivedValue());
			Serial.print(", RC_BIT_LENGTH ");
			Serial.print(MyRfSwitch.getReceivedBitlength());
			Serial.print(", RC_PROTOCOL ");
			Serial.print(MyRfSwitch.getReceivedProtocol());
			Serial.print(", RC_PULSE_LENGTH ");
			Serial.println(MyRfSwitch.getReceivedDelay());
		}
		ProcessRfCode(value);
		MyRfSwitch.resetAvailable();
		LastRfCode = value;
		ClearLastRfCodeTicker.once_ms(1000, ClearRfCode); // clear the Rf code in one second to prevent multiple triggers
	}
}

