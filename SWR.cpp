#ifndef BEENHERE
#include "MagLoop.h"
#endif
#include <Arduino.h>

/*****
  Purpose: To read one bridge measurement

  Parameter list:
  void

  Return value:
  int           the swr * 1000 so it comes back as an int

  CAUTION: Assumes that frequency has already been set
  ALTERNATIVE CALCULATION (Untested ?)
    p = sqrt( ((float)REV) / FWD );   // Calculate reflection coefficient

    VSWR = (1 + p) / (1 - p);         // Calculate VSWR
*****/

float ReadSWRValue()
{

  int i;
  float sum[2] = {0.0, 0.0};

  float FWD = 0.0;
  float REV = 0.0;
  float VSWR;
  
  for (i = 0; i < MAXPOINTSPERSAMPLE; i++) {             // Take multiple samples at each frequency
    MyDelay(5L);
    sum[0] += (float) analogRead(ANALOGFORWARD);
    MyDelay(5L);
    sum[1] += (float) analogRead(ANALOGREFLECTED);
  }
  FWD = sum[0] / (float) MAXPOINTSPERSAMPLE;
  REV = sum[1] / (float) MAXPOINTSPERSAMPLE;

  if (REV >= FWD) {
    VSWR = 999.0;                               // To avoid a divide by zero or negative VSWR then set to max 999
  } else {
    VSWR = ((FWD + REV) / (FWD - REV));         // Calculate VSWR
  }
    
#ifdef DEBUG
static float sNow = 1000.000, sLast = 1000.000;
if (VSWR < 999.0) {
  sNow = VSWR;
  if (sNow < sLast) {
#ifdef DEBUG1 
  Serial.print("In REadSWRValue()   sNow = ");
  Serial.print(sNow);
  Serial.print("  sLast = ");
  Serial.print(sLast);
  Serial.print("   currPosition = ");
  Serial.print(currPosition);
  Serial.print("   sum[1] = ");
  Serial.println(sum[1]);
#endif  
  sLast = sNow;
  }
}
#endif
  return (VSWR);    
}


float ReadNewSWRValue()
{

  int i;
  int sum[2] = {0, 0};

  int FWD = 0;
  int REV = 0;
  float VSWR;
  
  for (i = 0; i < MAXPOINTSPERSAMPLE; i++) {             // Take multiple samples at each frequency
    sum[0] += analogRead(ANALOGFORWARD);
    sum[1] +=analogRead(ANALOGREFLECTED);
  }
  FWD = sum[0] / MAXPOINTSPERSAMPLE;
  REV = sum[1] / MAXPOINTSPERSAMPLE;

  if (REV >= FWD) {
    VSWR = 999.0;                               // To avoid a divide by zero or negative VSWR then set to max 999
  } else {
    VSWR = ((float) (FWD + REV)) /( (float) (FWD - REV));         // Calculate VSWR
  }
    
    
#ifdef DEBUG1
  Serial.print("FWD = ");
  Serial.print(FWD);
  Serial.print("   REV = ");
  Serial.print(REV);
  Serial.print("   SWR = ");
  Serial.print(VSWR);
  Serial.print("   sum[0] = ");
  Serial.print(sum[0]);
  Serial.print("   sum[1] = ");
  Serial.println(sum[1]);
#endif

  return (VSWR);    

}

/*****
  Purpose: To provide a fake swr value for testing

  Parameter list:
  void

  Return value:
  int           the swr * 1000 so it comes back as an int

  CAUTION: Assumes that frequency has already been set
  ALTERNATIVE CALCULATION (Untested ?)
    p = sqrt( ((float)REV) / FWD );   // Calculate reflection coefficient

    VSWR = (1 + p) / (1 - p);         // Calculate VSWR
*****/
float FakeSWRValue()
{
  unsigned int i, increment, pixelsPerPlotPoint;
  int motorBump, x, y;
  long fakeFreq, fakeSWR;

  x = XAXISSTART;
  y = YAXISSTART;
  switch (currentBand) {
    case 40:
      fakeFreq = bandEdges[0][0];        // divide by 1000000
      fakeSWR = 500;              // Divide by 100
      pixelsPerPlotPoint = hertzPerStepperUnitAir[0];
      increment = 10;
      currentCapPosition = 300;
      motorBump = hertzPerStepperUnitAir[0];
      break;

    case 30:
      fakeFreq = bandEdges[1][0];        // divide by 1000000
      fakeSWR = 500;              // Divide by 100
      increment = 5000 / 5;
      pixelsPerPlotPoint = hertzPerStepperUnitAir[1];
      currentCapPosition = 2000;
      motorBump = hertzPerStepperUnitAir[1];
      break;

    case 20:
      fakeFreq = bandEdges[2][0];        // divide by 1000000
      fakeSWR = 500;              // Divide by 100
      increment = 35000U / 5;
      pixelsPerPlotPoint = hertzPerStepperUnitAir[2];
      currentCapPosition = 4900;
      motorBump = hertzPerStepperUnitAir[2];
      break;
  }
#ifdef DEBUG   
  Serial.println("In Fake");
#endif
  fakeFreq = currentFrequency - 750;

  x = XAXISSTART + 120;

  for (i = 0; i < MAXPOINTSPERSAMPLE; i++) {
    if (i < MAXPOINTSPERSAMPLE / 2 && y < YAXISEND) {
      y += 10;
    } else {
      y -= 10;
    }
    x += 2;
    if (y > YAXISEND || y < YAXISSTART)
      break;
    fakeFreq += motorBump;
#ifdef DEBUG     
    Serial.print("x = ");
    Serial.print(x);
    Serial.print("  y = ");
    Serial.print(y);
    Serial.print("  motorBump = ");
    Serial.print(motorBump);
    Serial.print("   fakeFreq = ");
    Serial.println(fakeFreq);
#endif
    tft.drawPixel(x, y, ILI9341_YELLOW);    // point
  }
  return SWR;
}


/*****
  Purpose: Displays the horizontal line for measured SWR

  Paramter list:
    int whichBandOption     // The band being used
    float swr
    
  Return value:
    void
*****/
void PlotSWRValue(int whichBandOption, float swr)
{
  int yAxisPixelPerUnit, totalPixels;
  long moveToStepperIndex;
  float pixelsPerTenth;
  lastSWR = swr;
  
  totalPixels = YAXISEND - YAXISSTART;
  pixelsPerTenth = totalPixels / 50.0;      // Highest SWR is 5, so 50 tenths.


  yDotIncrement = (int) (swr * 10 * pixelsPerTenth);    // Multiply by 10 because it's in tenths
  yDotIncrement = YAXISEND - yDotIncrement + 5;

  tft.fillRect(60, 30, 90, 20, ILI9341_BLACK);
  tft.setCursor(60, 30);        // Update SWR at top
  if (swr > 5.0) {
    tft.print("> 5");           // Shown at top
  } else {
    if (swr < 1.0)
      swr = 1.0;
    tft.print(swr);             // Shown at top

    DrawSWRLine(tft, swr);
    lastSWR = swr;
    SWR = swr;
  }
  PlotGrid(whichBandOption);
#ifdef DEBUG1   
Serial.print("after PlotGrid(), swr = ");
Serial.println(swr);
#endif
}

/*****
  Purpose: To display the SWR horizontal line and the SWR

  Argument list:
    Adafruit_ILI9341 tft      the display object
    float swr                 the current SWR value

  Return value:
    void
*****/
void DrawSWRLine(Adafruit_ILI9341 tft, float swr)
{
  tft.drawLine(XAXISSTART + 2, yDotIncrementOld, XAXISEND, yDotIncrementOld, ILI9341_BLACK);  // Old line removal
  tft.setCursor(XAXISEND - 10, yDotIncrementOld - 7);                                     // Old SWR removal
  tft.print("    ");
   
  tft.drawLine(XAXISSTART + 2, yDotIncrement, XAXISEND, yDotIncrement, ILI9341_RED);  // New Line
  yDotIncrementOld = yDotIncrement;                                               // So we can erase it
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(XAXISEND - 10, yDotIncrement - 7);
  tft.print(swr);             // Shown at end of line
}
