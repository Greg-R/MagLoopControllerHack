/*****
  This software is licensed un the GNU General Public License, Version 3, as presetned in
  https://www.gnu.org/licenses/gpl-3.0.txt. You may use this software for non-commercial use. 
  For commercial use, contact the authors:
  Jack Purdum, W8TEE
  Al Peter, AC8GY

  This header file must be copied into all copies and deriviative works of this software
  and remain the very first thing in the primary source code file.
  
*****/
//                               Compile with STM32F103                         
/*
                                                    LICENSE AGREEMENT

  This program source code and its associated hardware design at subject to the GNU General Public License version 2,
                  https://opensource.org/licenses/GPL-2.0
  with the following additional conditions:
    1. Any commercial use of the hardware or software is prohibited without express, written, permission of the authors.
    2. This entire comment, unaltered, must appear at the top of the primary source file. In the Arduino IDE environemnt, this comment must
       appear at the top of the INO file that contains setup() and loop(). In any other environmentm, it must appear in the file containing
       main().
    3. This notice must appear in any derivative work, regardless of language used.

  ================================================================================================================================================
  Development history:
  Version 20: Aug. 29, 2019, Jack Purdum, W8TEE. Redo the way Calibrate works
  Version 19: Aug. 28, 2019, Jack Purdum, W8TEE. Change the way frequency changes work and autotune.
  Version 18: Aug. 27, 2019, Jack Purdum, W8TEE. Continued frequency changes and modified encoder function.
  Version 17: Aug. 26, 2019, Jack Purdum, W8TEE. Experimented with variable fequency input changes
  Version 16: Aug. 25, 2019, Jack Purdum, W8TEE. Change frequency and stepper motor for min SWR
  Version 15: Aug. 13, 2019, Jack Purdum, W8TEE. Code to calibrate the stepper motor and cap movement. Change EEPROM writes and created default values
  Version 14: Aug.  5, 2019, Jack Purdum, W8TEE. Updated EEPROM organziation
  Version 13: Aug.  1, 2019, Jack Purdum, W8TEE. Change to plot SWR curve after final min SWR found
  Version 12: July 30, 2019, Jack Purdum, W8TEE. Change from auto to manual tune for testing
  Version 11: July 19, 2019, Jack Purdum, W8TEE. Code to calibrate cap for band edge position counts and write to EEPROM
  Version 10: July 17, 2019, Jack Purdum, W8TEE. Code to find and display lowest SWR.
  Version  9: July  2, 2019, Jack Purdum, W8TEE. Frequency processing via the menu.
  Version  8: June 25, 2019, Jack Purdum, W8TEE. Added code to allow Presets to be selected.
  Version  7: June 24, 2019, Jack Purdum, W8TEE. Fixed lock up. The compiler does NOT throw an error if a global is declared in the header file using extern,
                                                EVEN THOUGH that variable is never defined in the program code.
  Version  6: June 19, 2019, Jack Purdum, W8TEE. Tried to find why code locks up. Moved a lot of code around trying to nail it down
  Version  5: June 18, 2019, Jack Purdum, W8TEE. Fixed 60K bloated code size due to threading library being called for local static float
  Version  4: June 18, 2019, Jack Purdum, W8TEE. Worked on stepper movement and SWR display. Position interpolation
  Version  3: June 16, 2019, Jack Purdum, W8TEE. Worked on scaling graph and numerics being displayed
  Version  2: June 14, 2019, Jack Purdum, W8TEE. Started menu system and display
  Version  1: June 13, 2019, Al Peter, AC8GY. Initial code to start testing
*/\

//                                              Compile with STM32F103
#ifndef BEENHERE
#include "MagLoop.h"
#endif

//========================================================= Presets =======================================

/* The lower band edge becomes the intercept value for estimating count values. For example, moving to 14.0MHz would do a fast move
   to a count of 4003L. So, to estimate the stepper count for moving to a frequency of 7.1MHz, it becomes:

    count = intercept + slopeCoefficient * frequencyOffset
          = 60 + .000180 * 100000
          = 60 + 18
    count = 78

    The stepper will move to that frequency using a fast move and then the user fine tunes from there.
*/
long presetFrequencies[MAXBANDS][PRESETSPERBAND] =
{
  { 7030000L,  7040000L,  7010000L,  7197000L,  7250000L,  7285000L},   // 40M
  {10106000L, 10116000L, 10120000L, 10130000L, 10140000L, 10145000L},   // 30M
  {14030000L, 14060000L, 14100000L, 14200000L, 14250000L, 14285000L}    // 20M
};

long bandLimitPositionCounts[3][2] = {
  {  60L,  409L},                         // Sample values from test loop
  {2727L, 2757L},
  {4003L, 4073L}
};

float countPerHertz[] = {
    .000180,                 // (824 - 282) / (7300000 - 7000000)    
    .002600,                 // (3535 - 3405) / (10100000 - 10150000)
    .000582                  // (5075 - 4871) / (14350000 - 14000000)
};

long bandEdges[3][2] = {
  {LOWEND40M, HIGHEND40M},
  {LOWEND30M, HIGHEND30M},
  {LOWEND20M, HIGHEND20M}
};

long hertzPerPixel[]      = {1304,             // 40M units/Hz
                             217,              // 30M  units/Hz
                             1521              // 20M units/Hz
                            };

//========================================================= Objects =======================================
AccelStepper stepper(1, STEPPERPUL, STEPPERDIR);

Adafruit_ILI9341 tft    = Adafruit_ILI9341(TFT_CS, TFT_DC);
Rotary menuEncoder      = Rotary(MENUENCODERPINA, MENUENCODERPINB);
Rotary frequencyEncoder = Rotary(FREQUENCYENCODERPINA, FREQUENCYENCODERPINB);

//Rotary menuEncoder = encoder(MENUENCODERPINA, MENUENCODERPINB, MENUENCODERSWITCH);
//Rotary frequencyEncoder = encoder(FREQUENCYENCODERPINA, FREQUENCYENCODERPINB, FREQUENCYENCODERSWITCH);

//========================================================= Global data ===================================

boolean rotating = false; // debounce
boolean A_set = false;
boolean B_set = false;

unsigned char result1;
unsigned char result2;
char *menuOptions[]     = {"Freq", "Presets", "Calibrate"};
char *menuOptions2[3][3] = { {"Change Band", "Change Freq"},
  {"Change Band", "Activate", "Set"},
  {"Band Edges"}
};

char prompt[22] = {"Set frequency for "};

int currentBand;                                // Should be 40, 30, or 20
int currentPage;
int currentCapPosition;
int encoderPos = 0;       // counter
int menuIndex;

int hertzPerStepperUnitVVC[] = {909,                // 40M 909Hz move per stepper revolution with VVC
                                761,                // 30M
                                614                 // 20M
                               };
// This is for Air variable cap
int hertzPerStepperUnitAir[] = {1235,               // 40M 909Hz move per stepper revolution with VVC
                                761,                // 30M
                                6140                // 20M
                               };



int swFlag     = 0;
int SWRValueOld;
int stepperMove;
int yDotIncrement;
int yDotIncrementOld;

unsigned int lastReportedPos = 1;



long counter2Old  = 0;

long currentFrequency;
long currPosition;
long currPositionOld;
long enDirection;
long freqLoop     = 7000000L;
long jumpPosition = 0;
long oldeCounter1 = 0;


float lastSWR;
float VSWROld;
float SWRValue;
float SWR;

volatile int encoder2;     // + is CW, - is CCW
volatile int en2State;
volatile int menuEncoderState;

volatile long frequencyEncoderMovement;
volatile long menuEncoderMovement;

/*****
  Purpose: Logon Page

  Argument list:
    void

  Return value:
    void
*****/
void Splash()
{
  ErasePage();
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_MAGENTA, ILI9341_BLACK);
  tft.setCursor(10, 20);
  tft.print("Microcontroller Projects");
  tft.setCursor(40, 45);
  tft.print("for Amateur Radio");
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.setCursor(PIXELWIDTH / 2 - 30, PIXELHEIGHT / 4 + 20);
  tft.print("by");
  tft.setCursor(45, PIXELHEIGHT - 100);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.print("Jack Purdum  W8TEE");
  tft.setCursor(65, PIXELHEIGHT - 70);
  tft.println("Al Peter  AC7GY");
  tft.setTextSize(2);

  MyDelay(SPLASHDELAY);
  ErasePage();
}
/*****
  Purpose: to cause a delay in program execution

  Paramter list:
  unsigned long millisWait    // the number of millseconds to wait

  Return value:
  void
*****/
void MyDelay(unsigned long millisWait)
{
  unsigned long now = millis();

  while (millis() - now < millisWait)
    ;     // Twiddle thumbs...
}

void setup()
{
  int status;

#ifdef DEBUG
  Serial.begin(57600);
  MyDelay(1000);
#endif

  tft.begin();
  tft.setRotation(3);
  Splash();

  pinMode(MENUENCODERPINA,   INPUT_PULLUP);
  pinMode(MENUENCODERPINB,   INPUT_PULLUP);
  pinMode(MENUENCODERSWITCH, INPUT_PULLUP);

  pinMode(FREQUENCYENCODERPINA,   INPUT_PULLUP);
  pinMode(FREQUENCYENCODERPINB,   INPUT_PULLUP);
  pinMode(FREQUENCYENCODERSWITCH, INPUT_PULLUP);

  pinMode(ZEROSWITCH, INPUT_PULLUP);
  pinMode(MAXSWITCH, INPUT_PULLUP);

  pinMode(XTEN, INPUT);
  pinMode(XHUNDRED, INPUT);

  pinMode(ANALOGFORWARD, INPUT) ;
  pinMode(ANALOGREFLECTED , INPUT);

  pinMode (W_CLK, OUTPUT);
  pinMode (FQ_UD, OUTPUT);
  pinMode (DATA, OUTPUT);
  pinMode (RESET, OUTPUT);

  attachInterrupt(MENUENCODERPINA,  menuISR,      CHANGE);
  attachInterrupt(MENUENCODERPINB,  menuISR,      CHANGE);
  attachInterrupt(FREQUENCYENCODERPINA, frequencyISR, CHANGE);
  attachInterrupt(FREQUENCYENCODERPINB, frequencyISR, CHANGE);

  ResetStepperToZero();  // Comment this if testing without antenna hardware attached.

//  DefineEEPROMPage();                 // Set pseudo EEPROM page
  ReadEEPROMData();                   // Read the EEPROM data

#ifdef DEBUG                          // So we can see the EEPROM values
  /*
        Values from test run
     bandLimitPositionCounts[0][0] = 60
     bandLimitPositionCounts[0][1] = 409
     bandLimitPositionCounts[1][0] = 2727
     bandLimitPositionCounts[1][1] = 2757
     bandLimitPositionCounts[2][0] = 4003
     bandLimitPositionCounts[2][1] = 4073

        Values from 1st test run
      bandLimitPositionCounts[0][0] = 216;
      bandLimitPositionCounts[0][1] = 1050;
      bandLimitPositionCounts[1][0] = 5600;
      bandLimitPositionCounts[1][1] = 5654;
      bandLimitPositionCounts[2][0] = 8136;
      bandLimitPositionCounts[2][1] = 8272;

      Values from 2nd test run
      bandLimitPositionCounts[0][0] = 644;
      bandLimitPositionCounts[0][1] = 1675;
      bandLimitPositionCounts[1][0] = 2633;
      bandLimitPositionCounts[1][1] = 2643;
      bandLimitPositionCounts[2][0] = 3981;
      bandLimitPositionCounts[2][1] = 4051;

    //    Values from 3rd test run
      bandLimitPositionCounts[0][0] = 0L;
      bandLimitPositionCounts[0][1] = 0L;
      bandLimitPositionCounts[1][0] = 0L;
      bandLimitPositionCounts[1][1] = 0L;
      bandLimitPositionCounts[2][0] = 0L;
      bandLimitPositionCounts[2][1] = 0L;

      bandLimitPositionCounts[0][0] = 288;
      bandLimitPositionCounts[0][1] = 838;
      bandLimitPositionCounts[1][0] = 3419;
      bandLimitPositionCounts[1][1] = 3545;
      bandLimitPositionCounts[2][0] = 4888;
      bandLimitPositionCounts[2][1] = 5099;

    WritePositionCounts();
  */
  ShowEEPROMValues();
  ShowSlopeCoefficients();
#endif

  switch (currentBand) {              // Set the frequency default as 1st preset frequency
    case 40:
      currentFrequency = presetFrequencies[0][0];
      break;
    case 30:
      currentFrequency = presetFrequencies[1][0];
      break;
    case 20:
      currentFrequency = presetFrequencies[2][0];
      break;
    default:
      Serial.print("Shouldn't be here: currentBand = ");
      Serial.println(currentBand);
      currentFrequency = 7050000L;
      break;
  }
  SendFrequency(currentFrequency);    // Set the DDS
  SWR = ReadNewSWRValue();
  UpdateSWR(tft, SWR, "??");
  VSWROld = SWR;

  menuIndex = FREQMENU;
  ShowMainDisplay(tft, 0, SWR);       // Draws top menu line
  ShowSubmenuData(tft, SWR);          // Draws SWR and Freq info
  SendFrequency(currentFrequency);    // Send the frequency
  MyDelay(500L);                      // Let DDS stabilize
        
}

void loop()
{
  char *band[] = {"40M", "30M", "20M"};
  int i, submenuIndex, whichBandOption;
  long minCount;
  float minSWR;

  menuIndex = MakeMenuSelection(tft);           // Select one of the three top menu choices
  MyDelay(200L);                                // Crude debounce
  Serial.print("menuIndex = ");
  Serial.println(menuIndex);

  switch (menuIndex) {
    case FREQMENU:
      if (currPosition != 0L)
        ResetStepperToZero();

      whichBandOption = SelectBand(tft);
      currentFrequency = presetFrequencies[whichBandOption][0]; 
      ChangeFrequency(whichBandOption);
      stepper.setCurrentPosition(currPosition);
Serial.print("Near top currentFreq = ");
Serial.print(currentFrequency);     
Serial.print("    From stepper: currPosition = ");
Serial.print(stepper.currentPosition());     
Serial.print("   currPosition = ");
Serial.println(currPosition);     
      SendFrequency(currentFrequency);
      MyDelay(1000L);                                 // Let DDS catch up
      ShowSubmenuData(tft, SWRValue);                 // Update screen SWR and freq

      minSWR = 10.0;
//      stepper.moveTo(currPosition);
//      stepper.run();
//      ResetStepperToCount();
/*        
  stepper.moveTo(currPosition);
  stepper.run();
  while (stepper.distanceToGo() != 0) {
    stepper.moveTo(currPosition);  
    stepper.run();
  }
*/
Serial.println("After stepper stuff");        

//====================

//      ResetStepperToZero();
      currPosition = 0L;
      stepper.setCurrentPosition(currPosition);
      minCount = currPosition;
Serial.print("Regular   currPosition = ");
Serial.println(currPosition);     
Serial.print("    From stepper: currPosition = ");
Serial.println(stepper.currentPosition());     

      while (true) {      
        if (digitalRead(MAXSWITCH) != HIGH) {           // At the end stop switch??
          ResetStepperToZero();                         // Yep, so leave.
Serial.println("*********** dropped out");         
          return;
        }
        SWRValue = ReadSWRValue();
#ifdef DEBUG
if (currPosition % 50 == 0) {
Serial.print("Mod 50: currPosition = ");
Serial.print(currPosition);
Serial.print("   currentPos = ");
Serial.print(stepper.currentPosition());          
Serial.print("  minCount = ");
Serial.print(minCount);
Serial.print("  currentFrequency = ");
Serial.print(currentFrequency);
Serial.print("  SWRValue = ");
Serial.println(SWRValue);
}
#endif

        if (SWRValue < minSWR) {
          minSWR        = SWRValue;
          minCount      = currPosition;
#ifdef DEBUG         
Serial.print("***** In mins: minSWR = ");
Serial.print(minSWR);
Serial.print("   stepper.currentPos = ");
Serial.print(stepper.currentPosition());          
Serial.print("   minCount = ");
Serial.println(minCount);
#endif          
        }

        //        if (minSWR < TARGETMAXSWR || SWRValue - minSWR > 0.6) {
        if (minSWR < TARGETMAXSWR) {

#ifdef DEBUG
          Serial.print("After setting new start positon  currPosition = ");
          Serial.print(currPosition);
          Serial.print("  minSWR = ");
          Serial.println(minSWR);
#endif
          break;                                // This sends control to next edge
        }
        currPosition++;
        stepper.moveTo(currPosition);         // Move stepper to new position then increment for next pass...
        stepper.run();
//        if (stepper.distanceToGo() == 0) {
//           currPosition++;
//        }

        stepper.moveTo(currPosition);  
        stepper.run();       
      }     
//====================
/*
      stepper.moveTo(currPosition);         // Move stepper to new position then increment for next pass...
      stepper.run();
      if (stepper.distanceToGo() == 0 ) {
        currPosition++;
      }
      SendFrequency(currentFrequency); 
      MyDelay(1000L);
      SWRValue = ReadSWRValue();
      ShowSubmenuData(tft, SWRValue);                 // Update screen SWR and freq
       
      //      submenuIndex = PRESETSPERBAND / 2;                    // Pick one in the middle. This assumes they are in sorted order, lo to hi freq.
//                          // Select the band to be used
//      NewAutoTune(whichBandOption);
      
      currentFrequency = presetFrequencies[whichBandOption][0];

      SendFrequency(currentFrequency);        // Tell the DDS the edge frequency...
      SWR = ReadSWRValue();
      Serial.print("SWR = ");
      Serial.println(SWR);
      ChangeFrequency(whichBandOption);
      currPosition = ConvertFrequencyToStepperCount(currentFrequency);
      Serial.print("currentFrequency = ");
      Serial.println(currentFrequency);
      Serial.print("   currPosition = ");
      Serial.println(currPosition);
      Serial.print("   whichBandOption = ");
      Serial.println(whichBandOption);

      AutoFineTune(whichBandOption);
      stepper.runToNewPosition(currPosition);
      tft.setTextSize(2);
      ShowSubmenuData(tft, SWRValue);                 // Update screen SWR and freq
      tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
      tft.setCursor(40, PIXELHEIGHT - 30);
      tft.print("Press Menu Encoder");
      while (digitalRead(MENUENCODERSWITCH) != LOW) {
        ;
      }
*/      
  EraseBelowMenu();
  ShowSubmenuData(tft, SWRValue);                 // Update screen SWR and freq
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setCursor(40, PIXELHEIGHT - 30);
  tft.print("Press Menu Encoder");
  while (digitalRead(MENUENCODERSWITCH) != LOW) {
    ;
  }


      MyDelay(100L);
      EraseBelowMenu();
      //      ErasePrompts();
      //      menuIndex = 0;
      break;


    case PRESETSMENU:
      whichBandOption = SelectBand(tft);            // Select the band to be used
      submenuIndex = 0;
      ProcessPresets(tft, whichBandOption, submenuIndex);         // Select a preselected frequency
      menuIndex = FREQMENU;                         // When done, start over...
      ShowMainDisplay(tft, menuIndex, SWR);
      ShowSubmenuData(tft, SWR);
      SendFrequency(currentFrequency);
      SWR = ReadNewSWRValue();
      UpdateSWR(tft, SWR, "");
      break;

    case CALIBRATEMENU:
      EraseBelowMenu();                             // Clear work area
      DoNewCalibrate2();
      EraseBelowMenu();
      MyDelay(200L);
      ShowMainDisplay(tft, 0, 0.0);
      ShowSubmenuData(tft, 0.0);
      break;

    default:
#ifdef DEBUG1
      Serial.print("I shouldn't be here. menuIndex = ");
      Serial.println(menuIndex);
#endif
      break;
  }

}
