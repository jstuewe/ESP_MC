// Mirror Room - Haunted Hallway control program. John Stuewe
// Firsut use 2013 & 2014 @ Juans then 2015 at Karl's. 
// Board Fried and was rebuilt without LCD & one shift board in 2016, Converted to ESP_MC in 2017

byte FlickerTable[100] = {
  1,1,1,1,1,1,1,1,1,1,  //Each entry is 100ms of lights on or off.
  0,0,1,1,1,0,0,0,0,0,
  0,0,0,1,1,1,1,1,1,0,
  0,0,0,0,0,0,0,0,0,0,
  1,1,1,0,0,0,1,1,1,1,
  0,0,0,0,0,0,0,0,0,0,
  1,1,1,0,0,0,0,1,1,1,
  0,0,0,0,0,0,0,0,0,0, 
  1,1,1,0,0,0,0,1,1,1,
  0,0,0,0,0,0,0,0,0,0 
};

byte PatternTable[65][3] = {
  { 1,B10000000,B00000000 }  ,
  { 1,B01000000,B00000000 }  ,
  { 1,B00100000,B00000000 }  ,
  { 1,B00010000,B00000000 }  ,
  { 1,B00001000,B00000000 }  ,
  { 1,B00000100,B00000000 }  ,
  { 1,B00000010,B00000000 }  ,
  { 1,B00000001,B00000000 }  ,
  { 1,B00000000,B10000000 }  ,
  { 1,B00000000,B01000000 }  ,
  { 1,B00000000,B00100000 }  ,
  { 1,B00000000,B00010000 }  ,
  { 1,B00000000,B00001000 }  ,
  { 1,B00000000,B00000100 }  ,
  { 1,B00000000,B00000010 }  ,
  { 1,B00000000,B00000001 }  ,
  { 1,B00000000,B00000000 }  ,
  // this is a dummy element for end of table (duration=0)
  { 0,B00000000,B00000000 }
   };

//const char MRversionText[]="Endless Tunnel LED Lights. Halloween 2016 v2.40  OCt29'16";
void Write_SH595();
void Walk_Leds();

//defined in main program Cannot re-define here
//#define MR_SHIFT_DATA_PIN		D2      //Pin connected to DS of 74HC595
//#define MR_SHIFT_LATCH_PIN	D6      // Pin connected to ST_CP of 74HC595
//#define MR_SHIFT_CLOCK_PIN	D7      //Pin connected to SH_CP of 74HC595
//#define MR_GHOUL_PIN			D1
//#define MR_EXIT_PIN			D5


int SH595[2] = { 0,0}; // values for Write_SH595;
int MRState=0;
int MRLastState = 0;
int MRPatternIndex,MRMaxIndex=0;

unsigned long MRLightOnTime=10;     //ms for each light 'flicker'
unsigned long MRLightOffTime=50;    //ms between flickers
unsigned long MRSwitchTimeDelay=3000;   //ms to switch the Ghoul in or out
unsigned long T0MRMillis;               // Time we entered the last state
int MRStateTime;               // Time we entered the last state

//***** Setup *********************************************************************
void MRSetup() { 
	if (v.Dbug>10) {
		snprintf(sBuff, SBUFLEN, "%s Initializing Mirror Room Prop", HeaderString());
		dPrint(sBuff);
	}

  pinMode(MR_GHOUL_PIN, OUTPUT);
  pinMode(MR_EXIT_PIN, OUTPUT);
  pinMode(MR_SHIFT_LATCH_PIN, OUTPUT); //74HC595 shift register
  pinMode(MR_SHIFT_CLOCK_PIN, OUTPUT); //74HC595 shift register
  pinMode(MR_SHIFT_DATA_PIN, OUTPUT);  //74HC595 shift register

  SH595[0]  =B00000000;    
  SH595[1]  =B00000000;    
  Write_SH595();

  if (v.PropDelay[0] == 0) {	// Prop is defaulted, put realistic defaults in
	v.PropDelay[0]  =  5000;	// LightOnCycleTime-2xSwitchTimeDelay = 10000-3000-3000;  //ms for the whole flicker cycle
	v.PropOnTime[0] =   400;	// LightGhoulTime = 400;  //ms to have lights full on when Ghoul is out
	v.PropCycle[0]  =  3000;	// SwitchTimeDelay = 3000;   //ms to switch the Ghoul in or out
	v.PropDelay[1]  =   750;	// ExitDoorDelayToOpen = 0750;  //ms after ghoul goes away until exit door opens
	v.PropOnTime[1] = 10000;	// ExitDoorOpenTime = 10000;  //ms for the exit door to stay open
	v.PropCycle[1]  =  3000;	// ExitCycleTime-ExitDoorOpenTime-ExitDoorDelayToOpen = 13000-10000-750;  //ms for the flicker cycle after the ghoul goes away. Must be larger than ExitDoorTime
  }

  MRMaxIndex=0;
  while(PatternTable[MRMaxIndex][0] != 0 and MRMaxIndex < 100) { //end of table flag
    MRMaxIndex++;  // record how many entries are in the sequence table
  }
  randomSeed(312+ESP.getCycleCount());
  T0MRMillis = millis();
  MRStateTime = 0;
  MRState=0;
}

//*************************************************************************

void PropHandle_MirrorRoom() {
	Write_SH595();
	if (MRState != MRLastState) {
		Write_SH595();
		MRLastState = MRState;
		T0MRMillis = millis(); // time when we entered this state
		MRStateTime = 0;
		MRPatternIndex = 0;
		if (v.Dbug > 25) {
			snprintf(sBuff, SBUFLEN, "%s New MirrorRoom state = %i", HeaderString(), MRState);
			dPrint(sBuff);
		}
	}
	else MRStateTime = millis() - T0MRMillis;

	switch (MRState)
	{
	case 0: //		0	idle, do nothing
		SH595[0] = B00000000;
		SH595[1] = B00000000;
		Write_SH595();
		digitalWrite(MR_GHOUL_PIN, LOW);
		digitalWrite(MR_EXIT_PIN, LOW);
		break;
	case 1: //		1	Blink lights until Trigger Ghoul in & lights out@
		Walk_Leds();    // called every LED blink
//		if (MRStateTime > (unsigned long) v.PropDelay[0]) {
		if (MRStateTime > v.PropDelay[0]) {
			SH595[0] = B00000000;
			SH595[1] = B00000000;
			Write_SH595();  // Lights OFF
			digitalWrite(MR_GHOUL_PIN, HIGH); //Ghoul in
			MRState++;
		}
		break;
	case 2:		//		2	Wait in the dark until Bright Light@
		if (MRStateTime > v.PropCycle[0]) {
			SH595[0] = B11111111;
			SH595[1] = B11111111;
			Write_SH595();  // Lights ON
			MRState++;
		}
		break;
	case 3:		//		3	Bright lights until lights off & Trigger Ghoul exit
		if (MRStateTime > v.PropOnTime[0]) {
			SH595[0] = B00000000; 
			SH595[1] = B00000000;
			Write_SH595();  //Lights off
			digitalWrite(MR_GHOUL_PIN, LOW); //Ghoul out
			MRState++;
		}
		break;
	case 4:		//		4	Wait in the dark for Ghoul to retract
		if (MRStateTime > v.PropCycle[0]) {
			MRState++;
		}
		break;
	case 5:		//		5	blink lights until Trigger Exit door open@
		Walk_Leds();    // called every LED blink
		if (MRStateTime > v.PropDelay[1]) {
			digitalWrite(MR_EXIT_PIN, HIGH); //exit door opening
			MRState++;
		}
		break;
	case 6:		//		6	blink lights & wait with door open until close door @
		Walk_Leds();    // called every LED blink
		if (MRStateTime > v.PropOnTime[1]) {
			digitalWrite(MR_GHOUL_PIN, LOW); // exit door closing
			SH595[0] = B00000000;
			SH595[1] = B00000000;
			Write_SH595();  //Lights off
			MRState++;
		}
		break;
	case 7:		//		7	Wait for exit door to close and go idle
		if (MRStateTime > v.PropCycle[0]) { 
			MRState = 0; // go idle and allow re-trigger
		}
		break;
	} //switch

} // Prop Handle loop




///////////////////////////////////////////////////////////////////////////////////////////////////
// Subroutines     ////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
 

///////////////////////////////////////////////////////////////////////////////////////////////////
void Write_SH595() {
  digitalWrite(MR_SHIFT_LATCH_PIN, LOW);     // take the MR_SHIFT_LATCH_PIN low (Fix outputs)
  shiftOut(MR_SHIFT_DATA_PIN, MR_SHIFT_CLOCK_PIN, MSBFIRST, SH595[0]);      //shift out the bits:
  shiftOut(MR_SHIFT_DATA_PIN, MR_SHIFT_CLOCK_PIN, MSBFIRST, SH595[1]);      // shift out the bits:
  digitalWrite(MR_SHIFT_LATCH_PIN, HIGH);     //take the latch pin high to enable new outputs
  delay(1); //JRS debugging, maybe there isn't enough delay
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Walk_Leds() {
	static unsigned long MRFlickerMillis;
  if (millis() > MRFlickerMillis) {
    if (FlickerTable[(millis()-T0MRMillis)/100 % 100]==0 ) {  // force lights off any 100ms period that FlickerTable[x]=0
      SH595[0] = 0;
      SH595[1] = 0;
      MRFlickerMillis = millis() + random(MRLightOffTime /2, MRLightOffTime);
    }
    else { 
      MRPatternIndex = random(0,MRMaxIndex);
      SH595[0] = PatternTable[MRPatternIndex][1];
      SH595[1] = PatternTable[MRPatternIndex][2];
      MRFlickerMillis = millis() + random(0,MRLightOnTime);       
    }
  } // if delay long enough
} //Walk_Leds