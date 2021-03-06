#ifndef BEENHERE              // Note the entire file is within #if block is not defined
#define BEENHERE
#include <Arduino.h>

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <EEPROM.h>
#include <Rotary.h>
#include "SPI.h"
#include <AD9850SPI.h>        // https://github.com/F4GOJ/AD9850SPI
#include <AccelStepper.h>     // http://www.airspayce.com/mikem/arduino/AccelStepper/AccelStepper-1.59.zip
#include <Wire.h> 

//=============================== Symbolic Constants ===============================
#define DEBUG                                           // Comment out when not debugging
#define VERSION                     21                  // Software version
#define SPLASHDELAY                 4000L               // Normally, 4000L

#define LOWEND40M                   7000000L            // Define these frequencies for your licensing authority
#define HIGHEND40M                  7300000L            // The 'L' helps document that these are long data types
#define LOWEND30M                  10100000L
#define HIGHEND30M                 10150000L
#define LOWEND20M                  14000000L
#define HIGHEND20M                 14350000L

#define FASTMOVESPEED               900
#define NORMALMOVESPEED             100
#define MAXBUMPCOUNT                2                   // Detent pulses to get "real" bump

#define TARGETMAXSWR                1.05                // Want calibrate values equal to or less than this SWR
#define MAXPOINTSPERSAMPLE          10 
#define MAXMENUES                   3
#define DIGITINDICATOR              220                 // Extended ASCII for a block character

#define LASTBANDUSED                0                   // This number is the byte-offset into the EEPROM memory space
#define OFFSETTOPOSITIONCOUNTS      sizeof(int)         // The start of the stepper positions for the band edges defined above
#define OFFSETTOPRESETS             sizeof(int) * 6     // There are 8 bands edges

#define PIXELWIDTH                  320                 // Display limits
#define PIXELHEIGHT                 240
#define TEXTLINESPACING             20                  // Pixel spacing per line with text size = 2

#define YAXISSTART                  60                  // For graphing purposes
#define YAXISEND                    200
#define XAXISSTART                  50
#define XAXISEND                    280

#define PRESETSPERBAND              6                   // Allow this many preset frequencies on each band
#define MAXBANDS                    3                   // Can only process this many frequency bands

#define INCREMENTPAD                22                  // Used to display increment cursor

#define FREQUENCYTUNINGINCREMENT    1                   // Each encoder detent moves it this many Hz
#define POSITIONDELTA               10                  // The stepper position steps used to alterer the SWR
 
#define FREQMENU                    0                   // Menuing indexes
#define PRESETSMENU                 1
#define CALIBRATEMENU               2
#define BANDCHOICEMENU              3

#define LASTBANDUSED                0                   // This number is the byte-offset into the EEPROM memory space
#define OFFSETTOPOSITIONCOUNTS      sizeof(int)         // The start of the stepper positions for the band edges defined above
#define OFFSETTOPRESETS             sizeof(int) * 6     // There are 8 bands edges

//=============================== Pin Assignments ==================================
#define MENUENCODERSWITCH       PB4     //MENUFREQENCODER switch
#define MENUENCODERPINA         PB6     //MENUFREQENCODER
#define MENUENCODERPINB         PB5     //MENUFREQENCODER

#define FREQUENCYENCODERSWITCH      PB13    //Encoder2 switch
#define FREQUENCYENCODERPINA        PB12    //STEPPERENCODER
#define FREQUENCYENCODERPINB        PA8     //STEPPERENCODER

#define ZEROSWITCH                  PB8
#define MAXSWITCH                   PB7
#define STEPPERDIR                  PB15
#define STEPPERPUL                  PB14

#define XTEN                        PB10
#define XHUNDRED                    PB11

#define ANALOGFORWARD               PA3       // Sampled without transmitter power
#define ANALOGREFLECTED             PA2

#define TFT_DC                      PC15
#define TFT_CS                      PC14
                                          // following three pin definitions are needed by AD9850  
#define W_CLK                       PA1   // connect to AD9850 module word load clock pin (CLK)
#define FQ_UD                       PA0   // connect to freq update pin (FQ)
#define DATA                        PB0   // connect to  data load pin (DATA)
#define RESET                       PB1   // connect to reset pin (RST)


//=============================== Globals ==========================================
extern boolean A_set;
extern boolean B_set;extern unsigned char result1;
extern boolean rotating; // debounce

extern unsigned char result2;
extern char *menuOptions[];
extern char prompt[];

extern int currentBand;
extern int currentCapPosition;
extern int encoderPos;
extern int hertzPerStepperUnitVVC[];
extern int hertzPerStepperUnitAir[];
extern int hertzPerStepperUnitVVC[];
extern int menuIndex;
extern int plotPoints[][MAXPOINTSPERSAMPLE]; 
extern int swFlag;
extern int SWRValueOld;
extern int yDotIncrement;
extern int yDotIncrementOld;

extern unsigned int lastReportedPos;

extern long bandEdges[][2];
extern long bandLimitPositionCounts[][2];
extern long counter2Old;
extern long currentFrequency;
extern long currPosition;
extern long currPositionOld;
extern long enDirection;// change
extern long freqLoop;
extern long jumpPosition;
extern long oldeCounter1;
extern long hertzPerPixel[];
extern long presetFrequencies[][PRESETSPERBAND];

extern float countPerHertz[];
extern float lastSWR;
extern float VSWROld;
extern float SWR;
extern float SWRValue;

extern volatile int en2State;
extern volatile int encoder2;     // + is CW, - is CCW
extern volatile int menuEncoderState;

extern volatile long frequencyEncoderMovement;
extern volatile long menuEncoderMovement;

//=============================== Objects ==========================================
extern AccelStepper stepper;
extern Rotary menuEncoder;
extern Rotary frequencyEncoder;
extern Adafruit_ILI9341 tft;

//=============================== Function Prototypes ==============================
void AutoFineTune(int whichBandOption);
void byte_out(unsigned char byte);
void ChangeFrequency(int whichBandIndex);
void ChangeFrequencyWithTracking(int whichBandOption);
void ClearPositionToZero();
long ConvertFrequencyToStepperCount(long presentFrequency);
void DDSWakeUp();
void DefineEEPROMPage();
void DoCalibrate();
void DoNewCalibrate();
void DoNewCalibrate2();
void DoFastMoveToFrequency(int whichBandOption);
void DoFastStepperMove(int whichBandOption);
void DrawSWRLine(Adafruit_ILI9341 tft, float SWR);
void EraseBelowMenu();
void ErasePage();
void  ErasePrompts();
float FakeSWRValue();
void FineTuneStepper(int whichBandOption);
void frequencyISR();
void GraphAxis(int whichBandOption);
void GraphNoXAxisLables(int whichBandOption);
void HighlightNewPresetChoice(Adafruit_ILI9341 tft, int submenuIndex, int whichBandOption);
void ManualTuneStepper(int whichBandOption);
void menuISR();
int  MakeMenuSelection(Adafruit_ILI9341 tft);
void MyDelay(unsigned long millisWait);
void NewAutoTune(int whichBandOption);
void NewMove(int whichBandOption);
void outOne();
void outZero();
void PlotFinalSWR(float currentSWR);
void PlotGrid(int whichBandOption);
void PlotNewStartingFrequency(int whichBandOption);
void PlotStartingFrequency(int whichBandOption);
void PlotSWRValue(int whichBandOption, float swr);
int  ProcessEncoder();
void ProcessEndPoints(int whichBand, int whichEdge, int x, int y);
void ProcessPresets(Adafruit_ILI9341 tft,int whichBandOption, int submenuIndex);
void ReadBandPresets();
void ReadCurrentBand();
void ReadEEPROMData();
void ReadEncoder();
float ReadNewSWRValue();
void ReadPositionCounts();
float ReadSWRValue();
void ResetStepperToCount();
void ResetStepperToZero();
void RestorePreviousPresetChoice(Adafruit_ILI9341 tft, int submenuIndex, int whichBandOption);
int  SelectBand(Adafruit_ILI9341 tft);
void SendFrequency(long frequency);
void SetFrequency(int whichBandOption);
void ShowCalibrateScreen(Adafruit_ILI9341 tft);
void ShowCurrentPosition(long currentPos, int x, int y);
void ShowEEPROMValues();
int  ShowMainDisplay(Adafruit_ILI9341 tft, int whichMenuPage, float SWR);
void ShowPositionCounts();
void ShowSlopeCoefficients();
void ShowSubmenuData(Adafruit_ILI9341 tft, float SWR);
void ShowSWR(float swr, int x, int y);
void Splash();
void UpdateFrequency(Adafruit_ILI9341 tft);
void UpdatePositionValue(Adafruit_ILI9341 tft, int currPos);
int  WriteBandPresets();
void WriteCurrentBand();
void WriteDefaultEEPROMValues();
void WritePositionCounts();
void UpdateSWR(Adafruit_ILI9341 tft, float SWR, char msg[]);
#endif
