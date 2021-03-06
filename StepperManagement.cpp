#ifndef BEENHERE
#include "MagLoop.h"
#endif


/*****
  Purpose: Resets stepper motor to 0 position

  Parameter list:
    void
  Return value:
    void

  CAUTION:
*****/
void ResetStepperToZero()
{

#ifdef DEBUG
  Serial.println("In ResetStepperToZero()");
#endif
  digitalWrite(ZEROSWITCH, HIGH);               // Force a read
  //  while (digitalRead(ZEROSWITCH) != LOW) {      // move to zero position

  stepper.setMaxSpeed(FASTMOVESPEED);
  stepper.setAcceleration(1100);

  while (true) {      // move to zero position
#ifdef DEBUG1
    Serial.print("In while... currPosition = ");
    Serial.print(currPosition);
    Serial.print("  STEPPERDIR = ");
    Serial.print(analogRead(STEPPERDIR));
    Serial.print("   STEPPERPUL = ");
    Serial.println(analogRead(STEPPERPUL));
#endif
    currPosition--;
    stepper.moveTo(currPosition);
    stepper.run();
    if (digitalRead(ZEROSWITCH) == LOW)
      break;
  }
  stepper.setMaxSpeed(NORMALMOVESPEED);
  currPosition = 0L;
}



/*****
  Purpose: To move the cap[acitor to the approximate location via the stepper motor

  Parameter list:
  int whichBandOption           the band selected (probably don't need since currentFrequency tells us

  Return value:
    void

  CAUTION:

  int hertzPerStepperUnitVVC[] = {909,              // 40M 909Hz move per stepper revolution with VVC
                                761,                // 30M
                                614                 // 20M
                          };

*****/
void DoFastStepperMove(int whichBandOption)
{
  int yAxisPixelPerUnit, totalPixels, yDotIncrement;
  long moveToStepperIndex;
  float pixelsPerTenth, swr;

  // Find the number
  moveToStepperIndex = (currentFrequency - bandEdges[whichBandOption][0]) / hertzPerStepperUnitAir[whichBandOption];
#ifdef DEBUG1
  Serial.print("currentFrequency = ");
  Serial.print(currentFrequency);
  Serial.print("   moveToStepperIndex = ");
  Serial.println(moveToStepperIndex);
#endif
  stepper.move(moveToStepperIndex);
  swr = ReadSWRValue();
#ifdef DEBUG1
  Serial.print("swr = ");
  Serial.println(swr);
#endif

  totalPixels = YAXISEND - YAXISSTART;
  pixelsPerTenth = totalPixels / 50.0;      // HIghest SWR is 5, so 50 tenths.
}


/*****
  Purpose: To fine tune the capacitor to the approximate location via the stepper motor

  Parameter list:
    int whichBandOption           the band selected (probably don't need since currentFrequency tells us

  Return value:
    void

  CAUTION:

*****/

void FineTuneStepper(int whichBandOption)
{

  digitalWrite(FREQUENCYENCODERSWITCH, HIGH);
  while (digitalRead(FREQUENCYENCODERSWITCH) != LOW) {
    if (frequencyEncoderMovement != 0) {
      tft.drawLine(XAXISSTART, yDotIncrement, XAXISEND, yDotIncrement, ILI9341_BLACK);   // Erase old line
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
      tft.setCursor(XAXISEND - 10, yDotIncrement - 7);
      tft.print("    ");

      //Serial.print("SWR = ");
      // Serial.println(SWR);
      if (frequencyEncoderMovement == 1) {                                              // Clockwise

        currentFrequency += hertzPerStepperUnitAir[whichBandOption];
        if (currentFrequency >= bandEdges[whichBandOption][1]) {       // Frequency is too high
          currentFrequency -= hertzPerStepperUnitAir[whichBandOption];  // Put back where it was
        }
      }
      if (frequencyEncoderMovement == -1) {

        currentFrequency -= hertzPerStepperUnitAir[whichBandOption];
        if (currentFrequency >= bandEdges[whichBandOption][0]) {       // Frequency is too low
          currentFrequency += hertzPerStepperUnitAir[whichBandOption];  // Put back where it was
        }
      }
      PlotSWRValue(whichBandOption, SWR);
      frequencyEncoderMovement = 0;
    }
  }
}


/*****
  Purpose: To adjust stepper motor to minimum SWR

  Parameter list:
    int whichBandOption           the band selected (probably don't need since currentFrequency tells us

  Return value:
    void

  CAUTION:

*****/
void NewMove(int whichBandOption)
{
  long lowestSWRPosition;
  float lowestSWR, oldSWR, currentSWR;

  oldSWR = lowestSWR = 10.0;
  currPosition = stepper.currentPosition();
  lastSWR = currentSWR = ReadSWRValue();
#ifdef DEBUG
  Serial.print("\nIn NewMove() before while... currPosition = ");
  Serial.print(currPosition);
  Serial.print("   currentSWR  = ");
  Serial.println(currentSWR);
#endif

  digitalWrite(ZEROSWITCH, HIGH);
  currPosition = 0;
  digitalWrite(FREQUENCYENCODERSWITCH, HIGH);
  while (digitalRead(FREQUENCYENCODERSWITCH) != LOW && digitalRead(MAXSWITCH) != LOW) {
    currPosition += 10;
    stepper.moveTo(currPosition);
    stepper.run();

    currentSWR = ReadSWRValue();            // Measure new SWR
#ifdef DEBUG
    Serial.print("currentSWR = ");
    Serial.print(currentSWR);
    Serial.print("     lowestSWR = ");
    Serial.println(lowestSWR);
#endif
    if (currentSWR < 5.0) {
      if (currentSWR <= lowestSWR) {
        lowestSWR = currentSWR;
        lowestSWRPosition = currPosition;

        /*  Serial.print("In while currPosition = ");
          Serial.print(currPosition);
          Serial.print("   currentSWR = ");
          Serial.print(currentSWR);
          Serial.print("   oldSWR = ");
          Serial.println(oldSWR);*/

        //        PlotSWRValue(whichBandOption, currentSWR);
        //        MyDelay(1000L);
      }
    }
    if (abs(currentSWR - oldSWR) < 0.01 && currentSWR < 2.0) {
      break;
    }
    oldSWR = currentSWR;
  }
  SWR = currentSWR;
#ifdef DEBUG
  Serial.print("Before PlotSWRValue, currPosition = ");
  Serial.print(currPosition);
  Serial.print("   currentSWR = ");
  Serial.println(currentSWR);
#endif
  PlotSWRValue(whichBandOption, SWR);
#ifdef DEBUG
  Serial.print("After PlotSWRValue, before ManualTuneStepper, currPosition = ");
  Serial.print(currPosition);
  Serial.print("   currentSWR = ");
  Serial.println(currentSWR);
#endif

  ManualTuneStepper(whichBandOption);         // Can we improve it?

}


/*****
  Purpose: To adjust stepper motor to minimum SWR

  Parameter list:
    int whichBandOption           the band selected (probably don't need since currentFrequency tells us

  Return value:
    void

  CAUTION:

*****/
void ManualTuneStepper(int whichBandOption)
{
  long lowestSWRPosition;
  float lowestSWR, oldSWR, currentSWR;

  currentSWR = lowestSWR = oldSWR = SWR;               //  Lowest SWR automatically found in NewMove()

#ifdef DEBUG
  Serial.print("In ManualTuneStepper(), currPosition = ");
  Serial.print(currPosition);
  Serial.print("   currentSWR = ");
  Serial.println(currentSWR);
#endif

  while (digitalRead(FREQUENCYENCODERSWITCH) != LOW && digitalRead(MAXSWITCH) != LOW) {

    currPosition += frequencyEncoderMovement * FREQUENCYTUNINGINCREMENT;                                        // Change position according to encoder direction
    stepper.moveTo(currPosition);
    stepper.run();

    currentSWR = ReadSWRValue();            // Measure new SWR

#ifdef DEBUG
    Serial.print("currentSWR = ");
    Serial.print(currentSWR);
    Serial.print("     lowestSWR = ");
    Serial.println(lowestSWR);
#endif
    if (currentSWR < lowestSWR) {          // If move produces a lower SWR...
      lowestSWR = currentSWR;
      lowestSWRPosition = currPosition;
#ifdef DEBUG
      Serial.print("In Manual while currPosition = ");
      Serial.print(currPosition);
      Serial.print("   currentSWR = ");
      Serial.print(currentSWR);
      Serial.print("   oldSWR = ");
      Serial.println(oldSWR);
#endif
      SWR = currentSWR;
      //      PlotSWRValue(whichBandOption, SWR);
      MyDelay(1000L);
    } else {                                // Move did NOT lower SWR
      tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
      tft.setCursor(150, 220);
      tft.print("WORSE");
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
      if (frequencyEncoderMovement < 0)                       // They tuned CCW, so...
        currPosition += FREQUENCYTUNINGINCREMENT;                                   // ...restore to previous position
      else
        currPosition -= FREQUENCYTUNINGINCREMENT;
      //      PlotSWRValue(whichBandOption, SWR);
      MyDelay(5000L);                                         // Pause long enough to see it...
      tft.fillRect(150, 220, 90, 20, ILI9341_BLACK);        // ...then erase it
      break;
    }
    oldSWR = currentSWR;
  }
  PlotSWRValue(whichBandOption, currentSWR);
}

/*****
  Purpose: To use the stepper motor to move to a currentFrequency as fast as possible

  Parameter list:
    int whichBandOption   index for current band being used

  Return value:
    void

  CAUTION:

*****/
void DoFastMoveToFrequency(int whichBandOption)
{
  /*
    // This switch performs a linear transform on the desired frequency: c = b + mF
    switch (currentBand) {
    case 40:       //   intercept                  + slopeCoefficient * newFrequency
      currPosition = bandLimitPositionCounts[0][0] + countPerHertz[0] * (currentFrequency - LOWEND40M);
      break;

    case 30:
      currPosition = bandLimitPositionCounts[1][0] + countPerHertz[1] * (currentFrequency - LOWEND30M);
      break;

    case 20:
      currPosition = bandLimitPositionCounts[2][0] + countPerHertz[2] * (currentFrequency - LOWEND20M);
      break;

    default:
    #ifdef DEBU
      Serial.print("Shouldn't be here: currentBand = ");
      Serial.println(currentBand);
    #endif
      break;
    }
  */
  currPosition = ConvertFrequencyToStepperCount(currentFrequency);
  stepper.setMaxSpeed(FASTMOVESPEED);       // Get ready for a fast move
  stepper.setAcceleration(1100);
  stepper.moveTo(currPosition);
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  stepper.setMaxSpeed(NORMALMOVESPEED);     // Go back to slow move
}


/*****
  Purpose: Allow the user to change frequency and have the stepper automatically follow frequency change

  Parameter list:
    int whichBandOption       index into the current band array, 0 - 2

  Return value:
    void

  CAUTION:

*****/
void ChangeFrequencyWithTracking(int whichBandOption)
{
  long presentFrequency, lastFrequency;
  long presentCount, lastCount, temporaryIncrement;
  float currentSWR;

  temporaryIncrement = 100L;
  presentFrequency = lastFrequency = currentFrequency;
  lastCount = presentCount = currPosition;
  frequencyEncoderMovement = 1;
  while (digitalRead(FREQUENCYENCODERSWITCH) != LOW && digitalRead(MAXSWITCH) != LOW) {
    if (frequencyEncoderMovement != 0) {
      //      presentFrequency += frequencyEncoderMovement * FREQUENCYTUNINGINCREMENT;   // Get new frequency
      presentFrequency += frequencyEncoderMovement * temporaryIncrement;   // Get new frequency
      currentFrequency = presentFrequency;
      presentCount = ConvertFrequencyToStepperCount(presentFrequency);

      stepper.moveTo(presentCount);
      stepper.run();

      currentSWR = ReadSWRValue();            // Measure new SWR
      PlotSWRValue(whichBandOption, currentSWR);
      UpdateFrequency(tft);
      frequencyEncoderMovement = 0;
#ifdef DEBUG1
      Serial.print("In Change: currentSWR = ");
      Serial.print(currentSWR);
      Serial.print("  presentCount = ");
      Serial.print(presentCount);
      Serial.print("   currentFrequency = ");
      Serial.println(currentFrequency);
#endif
    }
  }
}


/*****
  Purpose: Allow the user to change frequency and have the stepper automatically follow frequency change

  Parameter list:
    long presentFrequency     the present frequency of the DDS

  Return value:
    void

  CAUTION:

*****/
long ConvertFrequencyToStepperCount(long presentFrequency)
{
  long count;

  /*
    Serial.print("Top oc convert: currentBand = ");
    Serial.print(currentBand);
    Serial.print("      presentFrequency = ");
    Serial.println(presentFrequency);
  */

  switch (currentBand) {
    case 40:       //   intercept                  + slopeCoefficient * newFrequency
      count = bandLimitPositionCounts[0][0] + (long) (countPerHertz[0] * ((float) (presentFrequency - LOWEND40M)));
      break;

    case 30:
      count = bandLimitPositionCounts[1][0] + (long) (countPerHertz[1] * ((float) (presentFrequency - LOWEND30M)));
      break;

    case 20:
      count = bandLimitPositionCounts[2][0] + (long) (countPerHertz[2] * ((float) (presentFrequency - LOWEND20M)));
      Serial.print("       count = ");
      Serial.println(count);
      break;

    default:
#ifdef DEBUG
      Serial.print("Shouldn't be here: currentBand = ");
      Serial.println(currentBand);
#endif
      break;
  }
  currPosition = count;
#ifdef DEBUG
  Serial.print("In Convert: bandLimitPositionCounts = ");
  Serial.println(bandLimitPositionCounts[2][0]);
  Serial.print("                   countPerHertz[2] = ");
  Serial.println(countPerHertz[2], 6);
  Serial.print("      presentFrequency - LOWEND20M) = ");
  Serial.println(presentFrequency - LOWEND20M);
  Serial.print("                        count = ");
  Serial.print(count);
  Serial.print("     count1 = ");
  Serial.print(bandLimitPositionCounts[2][0] + (long) (countPerHertz[2] * ((float) (presentFrequency - LOWEND20M))));
  Serial.print(" currPosition = ");
  Serial.println(currPosition);
#endif
  return count;
}

/*****
  Purpose: To adjust stepper motor to minimum SWR

  Parameter list:
    int whichBandOption           the band selected (probably don't need since currentFrequency tells us

  Return value:
    void

  CAUTION:

*****/
void AutoFineTune(int whichBandOption)
{
  int movesMade;
  long oldStepperPos, currentStepperPos, minStepperPos;
  float minSWR, oldSWR, currentSWR;

  //currentStepperPos = currPosition;
  //stepper.moveTo(currentStepperPos);
  //stepper.run();

  stepper.moveTo(currPosition);         // Move stepper to new position then increment for next pass...
  stepper.run();
  if (stepper.distanceToGo() == 0 ) {
    currPosition++;
  }

  SendFrequency(currentFrequency);        // Tell the DDS the edge frequency...
  oldSWR = minSWR = SWR;
  minStepperPos = currPosition;
  currentSWR = ReadSWRValue();          // Get the current value
#ifdef DEBUG
  Serial.print("In top AutoFineTune() currentStepperPos = ");
  Serial.print(currentStepperPos);
  Serial.print(" currentSWR = ");
  Serial.print(currentSWR);
  Serial.print(" oldSWR = ");
  Serial.println(oldSWR);
#endif
  // SWR from lower band edge
  currentStepperPos = currPosition;
  // We're adding to the current stepper position
  currentStepperPos += POSITIONDELTA;                   // Add a few steps and see what changes
  movesMade         = 0;
  //  minSWR            = currentSWR;
  //  minStepperPos     = currentStepperPos;
  while (true) {
    currentSWR = ReadSWRValue();          // Get the current value
    if (currentSWR < minSWR)
      minSWR = currentSWR;

    oldStepperPos = currentStepperPos;
    if (currentSWR < oldSWR) {
      currentStepperPos += POSITIONDELTA;                   // Add a few steps and see what changes
      oldSWR = currentSWR;
    } else {
      if (currentSWR > oldSWR) {
        currentStepperPos -= POSITIONDELTA;                   // Add a few steps and see what changes
        oldSWR = currentSWR;
      } else {
        if (currentSWR > oldSWR) {
          currentStepperPos += POSITIONDELTA;                   // Add a few steps and see what changes
          oldSWR = currentSWR;
        } else {
          if (currentSWR < oldSWR) {
            currentStepperPos -= POSITIONDELTA;                   // Add a few steps and see what changes
            oldSWR = currentSWR;
          }
        }
      }
#ifdef DEBUG
      Serial.print("In while currentStepperPos = ");
      Serial.print(currentStepperPos);
      Serial.print("   oldStepperPos = ");
      Serial.print(oldStepperPos);
      Serial.print("   minSWR = ");
      Serial.print(minSWR);
      Serial.print("   currentSWR = ");
      Serial.print(currentSWR);
      Serial.print("   oldSWR = ");
      Serial.println(oldSWR);
#endif
      if (currentSWR < minSWR) {
        minStepperPos = currentStepperPos;
        minSWR        = currentSWR;
      }
      stepper.moveTo(currentStepperPos);
      stepper.run();
      movesMade++;
      if (movesMade > 25) {
        break;
      }
      if (currentSWR < 1.03) {                // Good enough?
        break;
      }
    }
  }
  currPosition = minStepperPos;
  SWRValue     = minSWR;

#ifdef DEBUG
  Serial.print("Bottom*** currPosition = ");
  Serial.print(currPosition);
  Serial.print("  movesMade = ");
  Serial.print(movesMade);
  Serial.print("  SWRValue = ");
  Serial.print(SWRValue);
  Serial.print("   minStepperPos = ");
  Serial.print(minStepperPos);
  Serial.print("   minSWR = ");
  Serial.println(minSWR);
#endif
}

/*****
  Purpose: To select new frequency and automatically adjust the mag loop to that frequency

  Parameter list:
    int whichBandOption           the band selected (probably don't need since currentFrequency tells us

  Return value:
    void

  CAUTION:

*****/
void NewAutoTune(int whichBandOption)
{
  int movesMade;
  long oldStepperPos, currentStepperPos, minStepperPos;
  float minSWR, oldSWR, currentSWR;
  int i, changeDigit, digitSpacing, halfScreen, incrementPad, insetMargin, insetPad;
  long defaultIncrement;

  insetPad         = 0;                         // Used to align digit indicator
  incrementPad     = 0;
  digitSpacing     = 20;
  insetMargin      = 20;
  defaultIncrement = 100L;
  halfScreen   = PIXELHEIGHT / 2 - 25;

  if (whichBandOption == 0) {                 // 40M??
    insetPad -= digitSpacing;            // smaller number, so less spacing to a given digit
  }
  //currentStepperPos = currPosition;
  //stepper.moveTo(currentStepperPos);
  //stepper.run();
  /*
    DoFastMoveToFrequency(whichBandOption);                           // Move cap to lower band edge
    SendFrequency(currentFrequency);                                  // Set DDS the edge frequency...
  */
  currentFrequency = presetFrequencies[whichBandOption][0];         // This is for the band they selected
  while (digitalRead(MAXSWITCH) != LOW || digitalRead(FREQUENCYENCODERSWITCH) != LOW) {

    //
    if (menuEncoderMovement) {          // Change increment
      tft.fillRect(insetMargin + (insetPad + incrementPad) + digitSpacing * 6, halfScreen - 35, PIXELWIDTH  * .80, 35, ILI9341_BLACK);
      MyDelay(100L);

      Serial.print("In ChangeFreq: menuEncoderMovement = ");
      Serial.println(menuEncoderMovement);
      if (menuEncoderMovement == 1) {
        incrementPad += INCREMENTPAD;
        if (incrementPad > INCREMENTPAD * 3) {              // Don't overshoot or...
          incrementPad -= INCREMENTPAD;                     // ...back off one
        } else {
          defaultIncrement /= 10;                           // Increment is powers of 10
          if (defaultIncrement < 1) {                       // Don't go too far right
            defaultIncrement = 1L;
          }
        }
      }
      if (menuEncoderMovement == -1) {
        incrementPad -= INCREMENTPAD;
        if (incrementPad < -INCREMENTPAD * 4)               // Don't undershoot either
          incrementPad += INCREMENTPAD;
        defaultIncrement *= 10;
        if (defaultIncrement > 1000000L) {
          defaultIncrement = 1000000L;
        }
      }
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(insetMargin + (insetPad + incrementPad) + digitSpacing * 6, halfScreen - 35);     // Assume 1KHz increment
      tft.print((char) DIGITINDICATOR);
      tft.setTextColor(ILI9341_GREEN);
      menuEncoderMovement = 0;
#ifdef DEBUG
      Serial.print("In ChangeFreq: defaultIncrement = ");
      Serial.print(defaultIncrement);
      Serial.print("   incrementPad = ");
      Serial.println(incrementPad);
#endif
    }
    //    frequencyEncoderMovement = ProcessEncoder();
    if (frequencyEncoderMovement) {
      currentFrequency += (long) (frequencyEncoderMovement * defaultIncrement);
      currPosition = ConvertFrequencyToStepperCount(currentFrequency);
      stepper.moveTo(currPosition);         // Move stepper to new position then increment for next pass...
      stepper.run();
      if (stepper.distanceToGo() == 0 ) {
        currPosition++;
      }
      SendFrequency(currentFrequency);
      MyDelay(1000L);
      currentSWR = SWRValue = ReadSWRValue();
      ShowSubmenuData(tft, SWRValue);

      //      stepper.moveTo(currPosition);
      //      stepper.run();
      //      SWRValue = ReadSWRValue();
      //      AutoFineTune(bandIndex);
      //      stepper.moveTo(currPosition);
      //      stepper.run();
      tft.fillRect(insetMargin, halfScreen, PIXELWIDTH * .80, 40, ILI9341_BLACK);
      tft.setCursor(insetMargin, halfScreen);
      tft.print(currentFrequency);
      //      tft.setTextSize(2);
      //      ShowSubmenuData(tft, SWRValue);                 // Update screen SWR and freq
      //      tft.setTextSize(4);
      frequencyEncoderMovement = 0L;                   // Reset encoder flag

      tft.setTextSize(4);
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(insetMargin + (insetPad + incrementPad) + digitSpacing * 6, halfScreen - 35);     // Assume 1KHz increment
      tft.print((char) DIGITINDICATOR);
      tft.setTextColor(ILI9341_GREEN);
      tft.setCursor(insetMargin, halfScreen);
      tft.print(currentFrequency);

      //  DoFastMoveToFrequency(bandIndex);
      tft.setTextSize(2);
      ShowSubmenuData(tft, SWRValue);                 // Update screen SWR and freq
      tft.setTextSize(4);
    }
    //
    //    ChangeFrequency(whichBandOption);
    // Set DDS the edge frequency...
    //    MyDelay(1000L);                                                   // Let DDS stablize...
    //    currentSWR = ReadSWRValue();
  }
#ifdef DEBUG
  Serial.print("In NewAutoFineTune() currPosition = ");
  Serial.print(currPosition);
  Serial.print(" currentSWR = ");
  Serial.print(currentSWR);
  Serial.print(" currentFrequency = ");
  Serial.println(currentFrequency);
#endif

  return;

  /*
          stepper.moveTo(currPosition);         // Move stepper to new position then increment for next pass...
          stepper.run();
          if (stepper.distanceToGo() == 0 ) {
            currPosition++;
          }
  */
  oldSWR = minSWR = SWR = 10.0;                               // Initialize
  minStepperPos = currPosition;
  while (digitalRead(FREQUENCYENCODERSWITCH) != LOW) {        // This ends setting the frequency

  }

  currentSWR = ReadSWRValue();          // Get the current value
#ifdef DEBUG
  Serial.print("In top AutoFineTune() currentStepperPos = ");
  Serial.print(currentStepperPos);
  Serial.print(" currentSWR = ");
  Serial.print(currentSWR);
  Serial.print(" oldSWR = ");
  Serial.println(oldSWR);
#endif
  // SWR from lower band edge
  currentStepperPos = currPosition;
  // We're adding to the current stepper position
  currentStepperPos += POSITIONDELTA;                   // Add a few steps and see what changes
  movesMade         = 0;
  //  minSWR            = currentSWR;
  //  minStepperPos     = currentStepperPos;
  while (true) {
    currentSWR = ReadSWRValue();          // Get the current value
    if (currentSWR < minSWR)
      minSWR = currentSWR;

    oldStepperPos = currentStepperPos;
    if (currentSWR < oldSWR) {
      currentStepperPos += POSITIONDELTA;                   // Add a few steps and see what changes
      oldSWR = currentSWR;
    } else {
      if (currentSWR > oldSWR) {
        currentStepperPos -= POSITIONDELTA;                   // Add a few steps and see what changes
        oldSWR = currentSWR;
      } else {
        if (currentSWR > oldSWR) {
          currentStepperPos += POSITIONDELTA;                   // Add a few steps and see what changes
          oldSWR = currentSWR;
        } else {
          if (currentSWR < oldSWR) {
            currentStepperPos -= POSITIONDELTA;                   // Add a few steps and see what changes
            oldSWR = currentSWR;
          }
        }
      }
#ifdef DEBUG
      Serial.print("In while currentStepperPos = ");
      Serial.print(currentStepperPos);
      Serial.print("   oldStepperPos = ");
      Serial.print(oldStepperPos);
      Serial.print("   minSWR = ");
      Serial.print(minSWR);
      Serial.print("   currentSWR = ");
      Serial.print(currentSWR);
      Serial.print("   oldSWR = ");
      Serial.println(oldSWR);
#endif
      if (currentSWR < minSWR) {
        minStepperPos = currentStepperPos;
        minSWR        = currentSWR;
      }
      stepper.moveTo(currentStepperPos);
      stepper.run();
      movesMade++;
      if (movesMade > 25) {
        break;
      }
      if (currentSWR < 1.03) {                // Good enough?
        break;
      }
    }
  }
  currPosition = minStepperPos;
  SWRValue     = minSWR;

#ifdef DEBUG
  Serial.print("Bottom*** currPosition = ");
  Serial.print(currPosition);
  Serial.print("  movesMade = ");
  Serial.print(movesMade);
  Serial.print("  SWRValue = ");
  Serial.print(SWRValue);
  Serial.print("   minStepperPos = ");
  Serial.print(minStepperPos);
  Serial.print("   minSWR = ");
  Serial.println(minSWR);
#endif
}

/*****
  Purpose: Resets stepper motor to 0 position

  Parameter list:
    void

  Return value:
    void

  CAUTION:
*****/
void ResetStepperToCount()
{

#ifdef DEBUG
  Serial.println("In ResetStepperToCount()");
#endif
  digitalWrite(ZEROSWITCH, HIGH);               // Force a read
  //  while (digitalRead(ZEROSWITCH) != LOW) {      // move to zero position

  stepper.setMaxSpeed(FASTMOVESPEED);
  stepper.setAcceleration(1100);

  while (true) {      // move to zero position
    if (digitalRead(MAXSWITCH) == LOW) 
      break;
    stepper.moveTo(currPosition);
    stepper.run();
    currPosition++;
    if (stepper.distanceToGo() == 0) {
      break;
    }
  }
  stepper.setMaxSpeed(NORMALMOVESPEED);
  currPosition = 0L;
}
