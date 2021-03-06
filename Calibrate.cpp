#ifndef BEENHERE
#include "MagLoop.h"
#endif

/*****
  Purpose: To display the main menu page

  Argument list:
    Adafruit_ILI9341 tft      the display object

  Return value:
    void
*****/
void ShowCalibrateScreen(Adafruit_ILI9341 tft)
{
  int whichBand, whichEdge;
  unsigned int where, indexedOldSWR, indexedNewSWR;
  int whichHiEdge, whichLowEdge;  // This indexes into the band limits array:

  long counter = 0L;
  long testFrequency;
  float swrOld;

  tft.setCursor(60, YAXISSTART);
  tft.print("Set band endpoints");
  tft.setCursor(60, YAXISSTART + 25);
  tft.print("LOW EDGE");
  tft.setCursor(200, YAXISSTART + 25);
  tft.print("HI EDGE");
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);         // Show bands...
  tft.setCursor(10, YAXISSTART + 50);
  tft.print("40M");
  tft.setCursor(10, YAXISSTART + 75);
  tft.print("30M");
  tft.setCursor(10, YAXISSTART + 100);
  tft.print("20M");
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);         // Show band edges
  tft.setCursor(60, YAXISSTART + 50);
  tft.print(LOWEND40M);
  tft.setCursor(200, YAXISSTART + 50);
  tft.print(HIGHEND40M);
  tft.setCursor(60, YAXISSTART + 75);
  tft.print(LOWEND30M);
  tft.setCursor(200, YAXISSTART + 75);
  tft.print(HIGHEND30M);
  tft.setCursor(60, YAXISSTART + 100);
  tft.print(LOWEND20M);
  tft.setCursor(200, YAXISSTART + 100);
  tft.print(HIGHEND20M);

  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);         // Tell them to adjust SWR
  tft.setCursor(5, YAXISSTART + 140);
  tft.print("Adjust to min SWR:");
  //  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);         // for showi

  for (whichBand = 0; whichBand < MAXBANDS; whichBand++) {
    ProcessEndPoints(whichBand, 0,  60, YAXISSTART + 50 + whichBand * 25);    // Do low edge...
    bandLimitPositionCounts[whichBand][0];                                    // Save position count for low edge
    ProcessEndPoints(whichBand, 1, 200, YAXISSTART + 50 + whichBand * 25);    // ...now high edge
    bandLimitPositionCounts[whichBand][1];                                    // Save position count for high edge
  }
  tft.fillRect(0, YAXISSTART + 140, 310, YAXISSTART + 140, ILI9341_BLACK);
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);         // Tell them to adjust SWR
  tft.setCursor(25, YAXISSTART + 140);
  tft.print("Press Menu Encoder:");
  while (digitalRead(MENUENCODERSWITCH) != LOW)
  {
    counter++;
  }
  WritePositionCounts();
  EraseBelowMenu();
  menuIndex = 0;
  MyDelay(200L);
}


/*****
  Purpose: Function to find minimum SWR at band edges

  Argument list:
    int whichBand        0 = 40M, 1 = 30M, 2 = 20M
    int whichEdge        0 = low, 1 = high
    int x                x coordinate for updating position number
    int y                y                    "

  Return value:
    void
*****/
void ProcessEndPoints(int whichBand, int whichEdge, int x, int y)
{
  unsigned int where, indexedOldSWR, indexedNewSWR;
  long testFrequency;

  testFrequency = bandEdges[whichBand][whichEdge];        // Set the frequency
  currentFrequency = testFrequency;                        // Show on upper menu bar

  tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);         // Highlight first edge
  tft.setCursor(x, y);
  tft.print(testFrequency);
  UpdateFrequency(tft);
  ClearPositionToZero();                            // Reset contact arm to zero
  SendFrequency(testFrequency);                     // Let the DDS know the frequency
  while (digitalRead(FREQUENCYENCODERSWITCH) != LOW) {
    SWR = ReadSWRValue();
    indexedNewSWR = (unsigned int) (SWR * 100.0) ;            // Compensate for floating point fuzzies...
    if (frequencyEncoderMovement) {
      currPosition += frequencyEncoderMovement;        // Works because menuEncoderMovement is either + or -
      frequencyEncoderMovement = 0L;
      ShowSWR(SWR, 225, YAXISSTART + 140);
      ShowCurrentPosition(currPosition, x, y);
      stepper.moveTo(currPosition);
      stepper.run();
    }
  }
  MyDelay(300L);
}
/*****
  Purpose: Display currPosition on display

  Argument list:
    float swr

  Return value:
    void
*****/
void ShowCurrentPosition(long currentPos, int x, int y)
{
  tft.setCursor(x, y);        // Quick erase of old number
  tft.print("         ");

  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(x, y);
  tft.print(currentPos);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
}

/*****
  Purpose: Display SWR on display

  Argument list:
    float swr

  Return value:
    void
*****/
void ShowSWR(float swr, int x, int y)
{
  tft.fillRect(x, y, x + 50, y + 25, ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(x, y);
  tft.print(swr);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);

}
/*****
  Purpose: Position the stepper arm to zero position

  Argument list:
    void

  Return value:
    void
*****/
void ClearPositionToZero()
{
  currPosition = 0L;
  stepper.setMaxSpeed(FASTMOVESPEED);
  stepper.setAcceleration(1100);
  stepper.moveTo(currPosition);
  stepper.run();
  stepper.setCurrentPosition(0);          //set motor position to zero
}

/*****
  Purpose: To set the band end point counts and calculate slope coefficeints

  Argument list:
    void

  Return value:
    void
*****/
void DoCalibrate()
{
  int bandBeingCalicrated;
  int i, j;
  long localPosition, minCount;
  float currentSWR, minSWR;

  stepper.setMaxSpeed(FASTMOVESPEED);
  stepper.setAcceleration(1100);

  localPosition = currPosition;
  bandBeingCalicrated = 0;
  ClearPositionToZero();              // Reset stepper to zero
  localPosition = currPosition = 0L;

  for (i = 0; i < MAXBANDS; i++) {            // For the 3 bands...
    for (j = 0; j < 2; j++) {                 // For each band edge...
      currentFrequency = bandEdges[i][j];     // Select a band edge
      SendFrequency(currentFrequency);        // Tell the DDS the edge frequency...
      minSWR = 999.0;
      while (true) {
        if (digitalRead(MAXSWITCH) != HIGH) {           // At the end stop switch??
          ResetStepperToZero();                         // Yep, so leave.
          return;
        }
        stepper.moveTo(currPosition++);        // Move stepper to new position then increment for next pass...
        stepper.run();
        currentSWR = ReadSWRValue();
#ifdef DEBUG1
        if (currPosition % 50 == 0) {
          Serial.print("Mod 50: , currPosition = ");
          Serial.print(currPosition);
          Serial.print("  currentFrequency = ");
          Serial.print(currentFrequency);
          Serial.print("  currentSWR = ");
          Serial.println(currentSWR);
        }
#endif
        if (currentSWR < minSWR) {
          minSWR        = currentSWR;
          localPosition = currPosition;

          Serial.print("***after REadSWR(),");

          Serial.print(" currPos = ");
          Serial.print(currPosition);
          Serial.print("   minSWR = ");
          Serial.print(minSWR);
          Serial.print("  currentFreq = ");
          Serial.print(currentFrequency);
          Serial.print("  currentSWR = ");
          Serial.println(currentSWR);
        } else {
          //          if (currentSWR > 10.0) {
          //            continue;
          //          }
          if (currentSWR < 10.0 && minSWR - currentSWR > 0.1 || minSWR < 1.02) {
            bandLimitPositionCounts[i][j] = currPosition;
            //  currPosition = (currPosition / 2) - currPosition;   // Move half way back
            Serial.print("After setting new start positon  currPosition = ");
            Serial.print(currPosition);
            Serial.print("  currentSWR = ");
            Serial.println(currentSWR);
            currPosition *= 0.95;
            stepper.moveTo(currPosition);        // Move stepper to new position then increment for next pass...
            stepper.run();
            currPosition++;
            break;
          }
          /*          else {
                      bandLimitPositionCounts[i][j] = currPosition;
                      Serial.print("before break, currPosition = ");
                      Serial.print(currPosition);
                      Serial.print("  currentFrequency = ");
                      Serial.println(currentFrequency);
                      break;                              // Says we fiybd minimum
                    }
          */
        }
      }     // End while (true)
      Serial.print("After while(true), localPosition = ");
      Serial.print(localPosition);
      Serial.print("  minSWR = ");
      Serial.println(minSWR);

    }       // end for (j
  }         // end for (i
  WritePositionCounts();
#ifdef DEBUG
  ShowPositionCounts();
#endif
}



/*****
  Purpose: To set the band end point counts and calculate slope coefficeints automatically. Before testing, values were:

  bandLimitPositionCounts[0][0] = 60
  bandLimitPositionCounts[0][1] = 409
  bandLimitPositionCounts[1][0] = 2727
  bandLimitPositionCounts[1][1] = 2757
  bandLimitPositionCounts[2][0] = 4003
  bandLimitPositionCounts[2][1] = 4073

  Argument list:
    void

  Return value:
    void
*****/
void DoNewCalibrate()
{
  int bandBeingCalicrated;
  int i, j;
  long localPosition, minCount;
  float currentSWR, minSWR, lastSWR;

#ifdef DEBUG
  Serial.println("In DoNewCalibrate");
  ShowPositionCounts();                         // Show the current values
  return;
#endif

  bandBeingCalicrated = 0;
  ClearPositionToZero();                      // Reset stepper to zero
  localPosition = currPosition;
  stepper.setMaxSpeed(NORMALMOVESPEED);


  for (i = 0; i < MAXBANDS; i++) {            // For the 3 bands...
    for (j = 0; j < 2; j++) {                 // For each band edge...
      currentFrequency = bandEdges[i][j];     // Select a band edge
      SendFrequency(currentFrequency);        // Tell the DDS the edge frequency...
      lastSWR = minSWR = 999.0;
      while (true) {
        if (digitalRead(MAXSWITCH) != HIGH) {           // At the end stop switch??
          ResetStepperToZero();                         // Yep, so leave.
          return;
        }

        stepper.moveTo(currPosition);
        while (stepper.distanceToGo() != 0) {
          stepper.run();
        }
        //        stepper.moveTo(currPosition++);        // Move stepper to new position then increment for next pass...
        //        stepper.run();
        lastSWR = currentSWR;
        currentSWR = ReadSWRValue();
#ifdef DEBUG
        if (currPosition % 50 == 0) {
          Serial.print("Mod 50: , currPosition = ");
          Serial.print(currPosition);
          Serial.print("  currentFrequency = ");
          Serial.print(currentFrequency);
          Serial.print("  currentSWR = ");
          Serial.println(currentSWR);
        }
#endif
        if (currentSWR < minSWR) {
          minSWR        = currentSWR;
          localPosition = currPosition;

          Serial.print("*** ");

          Serial.print(" currPos = ");
          Serial.print(currPosition);
          Serial.print("   minSWR = ");
          Serial.print(minSWR);
          Serial.print("  currentFreq = ");
          Serial.print(currentFrequency);
          Serial.print("  currentSWR = ");
          Serial.print(currentSWR);
          Serial.print("  min - curr = ");
          Serial.println(lastSWR - currentSWR);
        } else {
          //          if (currentSWR > 10.0) {
          //            continue;
          //          }
          //          if ((currentSWR < 10.0 && lastSWR - currentSWR > 0.1) || minSWR < 1.02) {
          if ( minSWR < 1.02) {
            bandLimitPositionCounts[i][j] = currPosition;
            //  currPosition = (currPosition / 2) - currPosition;   // Move half way back
            Serial.print("After setting new start positon  currPosition = ");
            Serial.print(currPosition);
            Serial.print("  currentSWR = ");
            Serial.println(currentSWR);
            currPosition *= 1.10;                 // Move past current position
            stepper.moveTo(currPosition);         // Move stepper to new position then increment for next pass...
            stepper.run();
            currPosition++;
            break;                                // This sends control to next edge
          }
          currPosition++;
        }
      }     // End while (true)
      Serial.print("After while(true), localPosition = ");
      Serial.print(localPosition);
      Serial.print("  minSWR = ");
      Serial.println(minSWR);

    }       // end for (j
  }         // end for (i
  WritePositionCounts();
#ifdef DEBUG
  ShowPositionCounts();
#endif

}

/*****
  Purpose: To set the band end point counts and calculate slope coefficeints automatically. Before testing, values were:

  bandLimitPositionCounts[0][0] = 60
  bandLimitPositionCounts[0][1] = 409
  bandLimitPositionCounts[1][0] = 2727
  bandLimitPositionCounts[1][1] = 2757
  bandLimitPositionCounts[2][0] = 4003
  bandLimitPositionCounts[2][1] = 4073

  Argument list:
    void

  Return value:
    void
*****/
void DoNewCalibrate2()
{
  int bandBeingCalicrated;
  int i, j, whichLine;
  long localPosition, minCount;
  float currentSWR, minSWR;

#ifdef DEBUG1
  Serial.println("In DoNewCalibrate2");
  ShowPositionCounts();                         // Show the current values
#endif

  bandBeingCalicrated = 0;
  ClearPositionToZero();                      // Reset stepper to zero
  stepper.setCurrentPosition(0);              // set motor position to zero

  localPosition = currPosition = 0L;
  stepper.setMaxSpeed(NORMALMOVESPEED);

  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setCursor(0, 60);
  tft.print("Frequency    SWR   Count");      // Table header
  tft.setCursor(0, 85);                       // Read to show mins...

  whichLine = 0;                              // X coord for mins
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); // Table data

  for (i = 0; i < MAXBANDS; i++) {            // For the 3 bands...
    for (j = 0; j < 2; j++) {                 // For each band edge...
      currentFrequency = bandEdges[i][j];     // Select a band edge
      SendFrequency(currentFrequency);        // Tell the DDS the edge frequency...
      MyDelay(1000L);
      UpdateFrequency(tft);                   // Change main display data
      UpdateSWR(tft, 0.0, "--  ");

      minSWR = currentSWR = 100.0;            // Need to work with new data
      minCount = 0L;
      while (true) {
        if (digitalRead(MAXSWITCH) != HIGH) {           // At the end stop switch??
          ResetStepperToZero();                         // Yep, so leave.
          return;
        }
        currentSWR = ReadSWRValue();
#ifdef DEBUG
if (currPosition % 50 == 0) {
  Serial.print("Mod 50: currPosition = ");
  Serial.print(currPosition);
  Serial.print("  minCount = ");
  Serial.print(minCount);
  Serial.print("  currentFrequency = ");
  Serial.print(currentFrequency);
  Serial.print("  currentSWR = ");
  Serial.println(currentSWR);
}
#endif
        if (currentSWR < minSWR) {
          minSWR        = currentSWR;
          minCount      = currPosition;
#ifdef DEBUG         
          Serial.print("***** In mins: minSWR = ");
          Serial.print(minSWR);
          Serial.print("   minCount = ");
          Serial.println(minCount);
#endif          
        }
        if (minSWR < TARGETMAXSWR) {
          bandLimitPositionCounts[i][j] = minCount;
Serial.print("    minCount = ");
Serial.println(stepper.currentPosition());          
          tft.setCursor(0, 85 + whichLine * TEXTLINESPACING);
          if (currentFrequency < 10000000) {
            tft.print(" ");
          }
          tft.print(currentFrequency);
          tft.setCursor(150, 85 + whichLine * TEXTLINESPACING);
          tft.print(minSWR);
          tft.setCursor(230, 85 + whichLine * TEXTLINESPACING);
          tft.print(minCount);
          whichLine++;                                                      // Ready for next line of output

#ifdef DEBUG1
          Serial.print("After setting new start positon  currPosition = ");
          Serial.print(currPosition);
          Serial.print("  minSWR = ");
          Serial.println(minSWR);
#endif
          if (i == 1 && j == 1) {                // Move to band end
            currPosition += 5;                   // For 30M
          } else {
            currPosition += 100;
          }
#ifdef DEBUG1
          Serial.print("After setting new start positon  currPosition = ");
          Serial.print(currPosition);
          Serial.print("  minSWR = ");
          Serial.println(minSWR);
#endif
          stepper.moveTo(currPosition);
          stepper.run();

          break;                                // This sends control to next edge
        }
        currPosition++;
        stepper.moveTo(currPosition);         // Move stepper to new position then increment for next pass...
        stepper.run();
        if (stepper.distanceToGo() == 0 ) {
          currPosition++;
        }
      }     // End while (true)
#ifdef DEBUG1
      Serial.print("After while(true), minCount = ");
      Serial.print(minCount);
      Serial.print("  minSWR = ");
      Serial.println(minSWR);
#endif
    }       // end for (j
  }         // end for (i

//  WritePositionCounts();                      // Write values to EEPROM
  MyDelay(100L);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setCursor(40, PIXELHEIGHT - 30);
  tft.print("Press Menu Encoder");
  while (digitalRead(MENUENCODERSWITCH) != LOW) {
    ;
  }
  MyDelay(100L);

#ifdef DEBUG
  ShowPositionCounts();
#endif

}
