#ifndef BEENHERE
#include "MagLoop.h"
#endif

/*****
  Purpose: To erase the display below the top two menu lines

  Argument list:
    void

  Return value:
    void
*****/
void EraseBelowMenu()
{
  tft.fillRect(0, 56, 340, 280, ILI9341_BLACK);
}

/*****
  Purpose: To erase the display page

  Argument list:
    void

  Return value:
    void
*****/
void ErasePage()
{
  tft.fillScreen(ILI9341_BLACK);
}

/*****
  Purpose: To erase the display page

  Argument list:
    void

  Return value:
    void
*****/
void ErasePrompts()
{
  tft.fillRect(0, PIXELHEIGHT / 2 + 30, 340, 280, ILI9341_BLACK);
}



/*****
  Purpose: To display the main menu page

  Argument list:
    Adafruit_ILI9341 tft      the display object
    int whichMenuPage         the currently displayed menu page
    float SWR                 the current SWR value

  Return value:
    int                       the menu selected
*****/
int ShowMainDisplay(Adafruit_ILI9341 tft, int whichMenuPage, float SWR)
{
  int lastMenuPage = 0;
  int i;


  tft.fillScreen(ILI9341_BLACK);

  for (i = 0; i < 3; i++) {
    if (i == 0) {
      tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
    } else {
      tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    }
    tft.setCursor(i * 100, 0);
    tft.print(menuOptions[i]);
  }
  return lastMenuPage;
}


/*****
  Purpose: To display the SWR amd frequency data

  Argument list:
    Adafruit_ILI9341 tft      the display object
    float SWR                 the current SWR value

  Return value:
    void
*****/
void ShowSubmenuData(Adafruit_ILI9341 tft, float SWR)
{
  tft.drawFastHLine(0, 25, 320, ILI9341_RED);
  tft.setCursor(0, 30);
  tft.print("SWR: ");
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  if (SWR > 5.0 || SWR < 1.0) {                     // Real or bogus SWR?
    tft.print("??");                                //...bogus
  } else {
    tft.print(SWR);                                 //...real
  }
  UpdateFrequency(tft);
  tft.drawFastHLine(0, 50, 320, ILI9341_RED);
  /*
    tft.fillRect(215, 30, PIXELWIDTH, 20, ILI9341_BLACK);

    tft.setCursor(120, 30);
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.print("Freq: ");
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.print(currentFrequency);
  */
}


/*****
  Purpose: To get a main menu choice

  Argument list:
    Adafruit_ILI9341 tft      the display object
    float SWR                 the current SWR value

  Return value:
    int                       the menu selected
*****/
int MakeMenuSelection(Adafruit_ILI9341 tft)
{
  int i, index = 0;
  /*
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    for (i = 0; i < MAXMENUES; i++) {
      tft.setCursor(i * 90, 0);
      tft.print(menuOptions[i]);
    }

    MyDelay(2000);

    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);     // Highlight current menu
    tft.setCursor(menuIndex * 90, 0);
    tft.print(menuOptions[menuIndex]);
    MyDelay(2000);
  */
  while (digitalRead(MENUENCODERSWITCH) != LOW) {
    if (menuEncoderMovement) {                             // Must be i (CW) or -1 (CCW)

      if (menuEncoderMovement == 1) {
        index++;
        if (index == MAXMENUES) {        // wrap to first index
          index = 0;
        }
      }
      if (menuEncoderMovement == -1) {
        index--;
        if (index < 0) {        // wrap to first index
          index = MAXMENUES - 1;
        }
      }
      menuEncoderMovement = 0;
      tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
      for (i = 0; i < MAXMENUES; i++) {
        tft.setCursor(i * 100, 0);
        tft.print(menuOptions[i]);
      }

      tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
      tft.setCursor(index * 100, 0);
      tft.print(menuOptions[index]);
    }
  }
  tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
  tft.setCursor(index * 100, 0);
  tft.print(menuOptions[index]);

  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
#ifdef DEBUG
  Serial.print("Leaving MakeMenuSelection(), menuIndex = ");
  Serial.println(index);
#endif
  return index;
}

/*****
  Purpose: To get a band menu choice

  Argument list:
    Adafruit_ILI9341 tft      the display object

  Return value:
    int                       the menu selected
*****/
int SelectBand(Adafruit_ILI9341 tft)
{
  char *bands[] = {"40M", "30M", "20M"};
  int currBand[] = {40, 30, 20};
  int i, index, where = 0;

  tft.fillRect(0, 52, PIXELWIDTH, PIXELHEIGHT, ILI9341_BLACK);

  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  for (int i = 0; i < 3; i++) {
    tft.setCursor(100, 110 + i * 30);
    tft.print(bands[i]);
  }

  tft.setCursor(100, 110);
  tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
  tft.print(bands[0]);


  digitalWrite(MENUENCODERSWITCH, HIGH);
  MyDelay(100L);
  //  Serial.println("In SelectBand, before while");
  index = 0;
  while (true) {
    if (menuEncoderMovement) {
      if (menuEncoderMovement == 1) {
        index++;
        if (index == 3) {        // wrap to first index
          index = 0;
        }
      }
      if (menuEncoderMovement == -1) {
        index--;
        if (index < 0) {        // wrap to last index
          index = 2;
        }
      }
      menuEncoderMovement = 0;
      tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
      for (int i = 0; i < 3; i++) {
        tft.setCursor(100, 110 + i * 30);
        tft.print(bands[i]);
      }

      tft.setTextColor(ILI9341_BLUE, ILI9341_WHITE);
      tft.setCursor(100, 110 + index * 30);
      tft.print(bands[index]);
    }
    if (digitalRead(MENUENCODERSWITCH) == LOW)
      break;
  }

  MyDelay(500L);
  currentBand = currBand[index];
  return index;
}



/*****
  Purpose: To display the axes for a graph

  Paramter list:
    int whichBandOption     // The band being used

  Return value:
    void
*****/
void GraphAxis(int whichBandOption)
{
  char buff[10];
  int chunks, tickCount;
  int yTick, xTick, yDotIncrement, xDotIncrement;
  int tcolor, bcolor;
  int i, k, yIncrement, xIncrement;
  float freqCount, freqEnd, pip;

  switch (whichBandOption) {
    case 0:
      freqCount = 7.0;
      freqEnd = 7.3;
      pip = 0.1;
      chunks = 3;
      xDotIncrement = 10;
      break;

    case 1:
      freqCount = 10.1;
      freqEnd = 10.15;
      pip = 0.02;
      chunks = 3;
      xDotIncrement = 20;
      break;

    case 2:
      freqCount = 14.0;
      freqEnd = 14.35;
      pip = 0.10;
      chunks = 4;
      xDotIncrement = 50;
      break;

  }

  tcolor = ILI9341_YELLOW;
  bcolor = ILI9341_BLACK;
  tickCount = 5;

  tft.fillRect(0, 52, PIXELWIDTH, PIXELHEIGHT, ILI9341_BLACK);

  tft.drawLine(XAXISSTART, YAXISSTART, XAXISSTART, YAXISEND, ILI9341_DARKGREY);    // Y axis
  tft.drawLine(XAXISSTART, YAXISEND,   XAXISEND,   YAXISEND, ILI9341_DARKGREY);    // Y axis

  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);              // Lable X axis
  tft.setCursor(XAXISEND, YAXISEND - 15);
  tft.print("Mhz");

  yIncrement = (YAXISEND - YAXISSTART) / 5;                     // Spacing for graph tick marks
  yTick = YAXISSTART + 5;                                       // on Y axis
  //  xDotIncrement = 10;

  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  for (i = YAXISSTART; i < YAXISEND; i += yIncrement, yTick += yIncrement) {
    tft.drawLine(XAXISSTART - 5, yTick, XAXISSTART, yTick, ILI9341_DARKGREY);    // Y axis
    for (k = XAXISSTART + 10; k < XAXISEND; k += xDotIncrement) {
      tft.drawPixel(k, yTick, ILI9341_DARKGREY);    // Y dotted axis
    }
    tft.setCursor(0, yTick - 5);
    tft.print(tickCount--);
    tft.print(".0");
  }

  xIncrement = (XAXISEND - XAXISSTART - 10) / chunks;            // Spacing for graph tick marks
  xTick = XAXISSTART - 10;                                       // on X axis
  //  yDotIncrement = xIncrement / 2;

  //  PlotStartingFrequency(whichBandOption);
  PlotNewStartingFrequency(whichBandOption);
  for (i = 0; i < chunks + 1; i++, xTick += xIncrement) {
    tft.drawLine(XAXISSTART + i * xIncrement, YAXISEND, XAXISSTART + i * xIncrement, YAXISEND + 5, ILI9341_DARKGREY);    // Y axis
    tft.setCursor(XAXISSTART - 25 + i * xIncrement, YAXISEND + 10);
    if (freqCount > freqEnd) {
      freqCount = freqEnd;
    }
    if (whichBandOption == 2) {    // 20M
#ifdef DEBUG
      Serial.print("XAXISSTART - 25 + i * xIncrement = ");
      Serial.print(XAXISSTART - 25 + i * xIncrement);
      Serial.print("  freqCount = ");
      Serial.println(freqCount);
#endif
      if (freqCount < 14.15 || freqCount < 14.25 || freqCount < 14.31 || freqCount < 14.4) {
        if (freqCount > 14.35) {
          freqCount = 14.35;
          dtostrf(freqCount, 5, 2, buff);
        } else {
          dtostrf(freqCount, 4, 1, buff);
        }
        tft.print(buff);
      }
    } else {
      tft.print(freqCount);
    }
    freqCount += pip;
  }
  tft.setTextSize(2);

  for (k = XAXISSTART; k <= XAXISEND; k += xIncrement / 2) {      // Draw horizontal dotted grid lines
    for (i = YAXISSTART; i < YAXISEND; i += 10) {
      tft.drawPixel(k, i, ILI9341_DARKGREY);    // Y dotted axis
    }
  }
  for (k = XAXISSTART; k <= XAXISEND; k += xIncrement / 4) {     // Draw horizontal dotted grid lines
    for (i = YAXISSTART; i < YAXISEND; i += 10) {
      tft.drawPixel(k, i, ILI9341_DARKGREY);    // Y dotted axis
    }
  }

  /*
    int delta = HIGHEND40M - LOWEND40M;               // Hz = 300000
    Serial.print("delta = ");
    Serial.println(delta);
    xIncrement = XAXISEND - XAXISSTART;               // pixels = 230
    Serial.print("1 xIncrement = ");
    Serial.println(xIncrement);
    xIncrement = delta / xIncrement;                  // Hz per pixel
    Serial.print("2 xIncrement = ");
    Serial.println(xIncrement);
    delta = (currentFrequency - LOWEND40M) / xIncrement;  // 50000
    Serial.print("2 delta = ");
    Serial.println(delta);
    xIncrement = delta - XAXISSTART;
    Serial.print("3 xIncrement = ");
    Serial.println(xIncrement);
    tft.drawLine(XAXISSTART + delta, YAXISSTART, XAXISSTART + delta, YAXISEND, ILI9341_MAGENTA);    // Y axis


    Serial.print("x1 = ");
    Serial.print(XAXISSTART - 15);
    Serial.print("   y1 = ");
    Serial.print(YAXISSTART + i * increment);
    Serial.print("   XAXISSTART = ");
    Serial.println(XAXISSTART);
  */



  /*
    for (i = ylo; i <= yhi; i += yinc) {
      // compute the transform
      temp =  (i - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

      if (i == 0) {
        tft.drawLine(gx, temp, gx + w, temp, acolor);
      }
      else {
        tft.drawLine(gx, temp, gx + w, temp, gcolor);
      }

      tft.setTextSize(1);
      tft.setTextColor(tcolor, bcolor);
      tft.setCursor(gx - 40, temp);
      // precision is default Arduino--this could really use some format control
      tft.println(i);
    }

    hedge = xlo;
    bump = (xhi - xlo) / 5.0;


    // draw x scale
    for (int i = 0; i < ELEMENTS(pip); i++) {
      temp = pip[i];
      tft.drawLine(temp, UPPERPLOTMARGIN, temp, gy, GREEN);
      tft.setTextSize(1);
      tft.setTextColor(tcolor, bcolor);
      tft.setCursor(temp - 10, gy + 10);
      f = (int) (hedge * .0001);
      FormatFrequency(f, buff);
      hedge += bump;
      tft.println(buff);
    }
    lastX = temp;
    //now draw the labels
    tft.setTextSize(1);
    tft.setTextColor(acolor, bcolor);
    tft.setCursor(430, gy + 10);
    tft.println(xlabel);

    tft.setTextSize(1);
    tft.setTextColor(acolor, bcolor);
    tft.setCursor(5, gy - h - 15);
    tft.println(ylabel);
  */
}
/*****
  Purpose: Numbers the X axis

  Argument list:
    int whichBandOption       the selected band

  Return value:
    int                       the menu selected
*****/
void PlotStartingFrequency(int whichBandOption)
{
  int delta, tickCount;
  long highEnd, lowEnd, midPoint;
  float freqCount;

  switch (whichBandOption) {
    case 0:
      freqCount = 7.0;
      highEnd = 7300000L;
      lowEnd  = 7000000L;
      break;
    case 1:
      freqCount = 10.1;
      highEnd = 10150000L;
      lowEnd  = 10100000L;
      break;
    case 2:
      freqCount = 14.0;
      highEnd = 14350000L;
      lowEnd  = 14000000L;
      break;

  }

  midPoint = ((highEnd - lowEnd) / 2) + lowEnd;

  delta = (XAXISEND - XAXISSTART) / 2 - 5;
  if (whichBandOption == 2)
    delta -= 15;
  tft.drawLine(XAXISSTART + delta, YAXISSTART, XAXISSTART + delta, YAXISEND, ILI9341_MAGENTA);    // Y axis

  tft.fillRect(215, 30, PIXELWIDTH, 20, ILI9341_BLACK);
  tft.setCursor(215, 30);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.print(midPoint);
}


/*****
  Purpose: Sets the frequency to be used with the mag loop

  Argument list:
    int whichBandOption       the selected band

  Return value:
    void
*****/
void SetFrequency(int whichBandOption)
{
  while (true) {
    if (menuEncoderState) {
      if (menuEncoderState == 1) {
        currentFrequency += FREQUENCYTUNINGINCREMENT;
        if (currentFrequency >= bandEdges[whichBandOption][1]) {     // Freq too high for band?
          currentFrequency = bandEdges[whichBandOption][1] - 250L;   // Close as you can get (CW)
        }
      }
      if (menuEncoderState == -1) {
        currentFrequency -= FREQUENCYTUNINGINCREMENT;
        if (currentFrequency <= bandEdges[whichBandOption][0]) {     // Freq too high for band?
          currentFrequency = bandEdges[whichBandOption][0] + 250L;   // Close as you can get (CW)
        }
      }
      menuEncoderState = 0;
      /*
            tft.fillRect(215, 30, PIXELWIDTH, 20, ILI9341_BLACK);
            //  tft.fillRect(215, 30, PIXELWIDTH, PIXELHEIGHT, ILI9341_BLACK);
            tft.setCursor(215, 30);
            tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
            tft.print(currentFrequency);
      */
      UpdateFrequency(tft);
    }
    if (digitalRead(MENUENCODERSWITCH) == LOW)
      break;
  }
  tft.fillRect(0, 52, PIXELWIDTH, PIXELHEIGHT, ILI9341_BLACK);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.setCursor(10, 120);
  tft.print("New frequency: ");
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.print(currentFrequency);

  MyDelay(100L);
}

/*****
  Purpose: Plot frequency given by user

  Argument list:
    int whichBandOption       the selected band

  Return value:
    int                       the menu selected
*****/
void PlotNewStartingFrequency(int whichBandOption)
{
  int delta, tickCount;
  int x, y;
  long highEnd, lowEnd, midPoint;
  float freqCount;

  switch (whichBandOption) {
    case 0:
      freqCount = 7.0;
      highEnd = 7300000L;
      lowEnd  = 7000000L;
      x = ((currentFrequency - lowEnd) / 10 ) * hertzPerPixel[0];
      break;

    case 1:
      freqCount = 10.1;
      highEnd = 10150000L;
      lowEnd  = 10100000L;
      x = ((currentFrequency - lowEnd) / 10 ) * hertzPerPixel[1];
      break;
    case 2:
      freqCount = 14.0;
      highEnd = 14350000L;
      lowEnd  = 14000000L;
      x = ((currentFrequency - lowEnd) / 10 ) * hertzPerPixel[2];
      break;

  }

  tft.drawLine(XAXISSTART + x, YAXISSTART, XAXISSTART + x, YAXISEND, ILI9341_MAGENTA);    // Y axis
  UpdateFrequency(tft);
  /*
    tft.fillRect(215, 30, PIXELWIDTH, 20, ILI9341_BLACK);
    tft.setCursor(215, 30);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.print(currentFrequency);
  */
}

/*****
  Purpose: To display the graph with set frequency and variable tuning

  Paramter list:
    int whichBandOption     // The band being used

  Return value:
    void
*****/
void GraphNoXAxisLables(int whichBandOption)
{
  char limitLo[6];
  char limitHi[6];
  int chunks, tickCount;
  int yTick, xTick;
  int tcolor, bcolor;
  int i, k, yIncrement, xIncrement;
  long freqCount, freqEnd, pip, yDotIncrement, xDotIncrement, xOffset;

  /*
    tft.fillRect(215, 30, PIXELWIDTH, 20, ILI9341_BLACK);
    tft.setCursor(215, 30);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.print(currentFrequency);
  */
  switch (whichBandOption) {
    case 0:
      freqCount = LOWEND40M;
      freqEnd = HIGHEND40M;
      strcpy(limitLo, "7.0");
      strcpy(limitHi, "7.3");
      pip = 0.1;
      chunks = 3;
      xDotIncrement = 10;
      break;

    case 1:
      freqCount = LOWEND30M;
      freqEnd = HIGHEND30M;
      strcpy(limitLo, "10.1");
      strcpy(limitHi, "10.15");
      pip = 0.02;
      chunks = 3;
      xDotIncrement = 20;
      break;

    case 2:
      freqCount = LOWEND20M;
      freqEnd = HIGHEND30M;
      strcpy(limitLo, "14.0");
      strcpy(limitHi, "14.35");
      pip = 0.10;
      chunks = 4;
      xDotIncrement = 50;
      break;
  }

  tcolor = ILI9341_YELLOW;
  bcolor = ILI9341_BLACK;
  tickCount = 5;

  tft.fillRect(0, 52, PIXELWIDTH, PIXELHEIGHT, ILI9341_BLACK);

  tft.drawLine(XAXISSTART, YAXISSTART, XAXISSTART, YAXISEND, ILI9341_DARKGREY);    // Y axis
  tft.drawLine(XAXISSTART, YAXISEND,   XAXISEND,   YAXISEND, ILI9341_DARKGREY);    // X axis

  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);              // Lable X axis
  tft.setCursor(XAXISSTART - 25, YAXISEND + 10);
  tft.print(limitLo);
  tft.setCursor(XAXISEND   - 25, YAXISEND + 10);
  tft.print(limitHi);
  tft.setCursor(XAXISEND   - 10, YAXISEND - 10);
  tft.print("Mhz");

  yIncrement = (YAXISEND - YAXISSTART) / 5;                     // Spacing for graph tick marks
  yTick = YAXISSTART + 5;                                       // on Y axis

  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  for (i = YAXISSTART; i < YAXISEND; i += yIncrement, yTick += yIncrement) {
    tft.drawLine(XAXISSTART - 5, yTick, XAXISSTART, yTick, ILI9341_DARKGREY);    // Y axis
    for (k = XAXISSTART + 10; k < XAXISEND; k += xDotIncrement) {
      tft.drawPixel(k, yTick, ILI9341_DARKGREY);    // Y dotted axis
    }
    tft.setCursor(0, yTick - 5);
    tft.print(tickCount--);
    tft.print(".0");
  }

  xIncrement = (XAXISEND - XAXISSTART - 10) / chunks;            // Spacing for graph tick marks
  xTick = XAXISSTART - 10;                                       // on X axis

  // Find the number of 10 Hz blocks in frequency spread. Example: currentFrequency = 7050000L
  //    Because each pixel is 1304 Hz on 40M, 50000 /  1304 = 38, so plot from XAXISSTART + 38
  xOffset = (currentFrequency - freqCount) / hertzPerPixel[whichBandOption];  // Hz from lower band edge to target frequency
  // divided by pixels/FREQUENCYTUNINGINCREMENT Hz
#ifdef DEBUG
  Serial.print("xOffset = ");
  Serial.print(xOffset);
  Serial.print("   freqCount = ");
  Serial.print(freqCount);
  Serial.print("   hertzPerPixel[whichBandOption] = ");
  Serial.println(hertzPerPixel[whichBandOption]);
#endif
  tft.drawLine(XAXISSTART + xDotIncrement, YAXISEND,   XAXISSTART + xDotIncrement, YAXISEND + 5, ILI9341_DARKGREY);   // Y axis
  tft.drawLine(XAXISSTART + xOffset - 6, YAXISSTART, XAXISSTART + xOffset - 6, YAXISEND,     ILI9341_MAGENTA);    // Y axis

  //  tft.setCursor(XAXISSTART - 30 + xDotIncrement, YAXISEND + 10);
  //  tft.print((float) ((float)currentFrequency / (float)1000000L));


  /*
    for (i = 0; i < chunks + 1; i++, xTick += xIncrement) {
      tft.drawLine(XAXISSTART + i * xIncrement, YAXISEND, XAXISSTART + i * xIncrement, YAXISEND + 5, ILI9341_DARKGREY);    // Y axis
      tft.setCursor(XAXISSTART - 25 + i * xIncrement, YAXISEND + 10);
      if (freqCount > freqEnd) {
        freqCount = freqEnd;
      }
      if (whichBandOption == 2) {    // 20M
        Serial.print("XAXISSTART - 25 + i * xIncrement = ");
        Serial.print(XAXISSTART - 25 + i * xIncrement);
        Serial.print("  freqCount = ");
        Serial.println(freqCount);
        if (freqCount < 14.15 || freqCount < 14.25 || freqCount < 14.31 || freqCount < 14.4) {
          if (freqCount > 14.35) {
            freqCount = 14.35;
            dtostrf(freqCount, 5, 2, buff);
          } else {
            dtostrf(freqCount, 4, 1, buff);
          }
          tft.print(buff);
        }
      } else {
        tft.print(freqCount);
      }
      freqCount += pip;
    }
  */
  tft.setTextSize(2);

  /*
    for (k = XAXISSTART; k <= XAXISEND; k += xIncrement / 2) {      // Draw horizontal dotted grid lines
      for (i = YAXISSTART; i < YAXISEND; i += 5) {
        tft.drawPixel(k, i, ILI9341_DARKGREY);    // Y dotted axis
      }
    }
  */
  //  PlotNewStartingFrequency(whichBandOption);

}



/*****
  Purpose: Displays the horizontal line for measured SWR

  Paramter list:
    int whichBandOption     // The band being used

  Return value:
    void
*****/
void PlotGrid(int whichBandOption)
{
  char limitLo[6];
  char limitHi[6];
  int chunks, tickCount;
  int yTick, xTick, yDotIncrement, xDotIncrement;

  int i, k, yIncrement, xIncrement;
  long freqCount, freqEnd, pip;

  UpdateFrequency(tft);

  switch (whichBandOption) {
    case 0:                         // 40M
      freqCount = LOWEND40M;
      freqEnd = HIGHEND40M;
      strcpy(limitLo, "7.0");
      strcpy(limitHi, "7.3");
      pip = 0.1;
      chunks = 3;
      xDotIncrement = 10;
      break;

    case 1:                         // 30M
      freqCount = LOWEND30M;
      freqEnd = HIGHEND30M;
      strcpy(limitLo, "10.1");
      strcpy(limitHi, "10.15");
      pip = 0.02;
      chunks = 3;
      xDotIncrement = 20;
      break;

    case 2:                         // 20M
      freqCount = LOWEND20M;
      freqEnd = HIGHEND20M;
      strcpy(limitLo, "14.0");
      strcpy(limitHi, "14.35");
      pip = 0.10;
      chunks = 4;
      xDotIncrement = 50;
      break;
  }
  yIncrement = (YAXISEND - YAXISSTART) / 5;                     // Spacing for graph tick marks
  yTick = YAXISSTART + 5;                                       // on Y axis

  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
  for (i = YAXISSTART; i < YAXISEND; i += yIncrement, yTick += yIncrement) {
    for (k = XAXISSTART + 10; k < XAXISEND; k += 10) {
      tft.drawPixel(k, yTick, ILI9341_DARKGREY);    // Y dotted axis
    }
  }

  for (k = XAXISSTART + 10; k < XAXISEND; k += 30) {
    for (i = YAXISSTART + 10; i < YAXISEND; i += xDotIncrement) {
      tft.drawPixel(k, i, ILI9341_DARKGREY);                      // Y dotted axis
    }
  }
}

/*****
  Purpose: To rewrite the frequency display

  Argument list:
    Adafruit_ILI9341 tft      the display object

  Return value:
    int                       the menu selected
*****/
void UpdateFrequency(Adafruit_ILI9341 tft)
{
  tft.fillRect(120, 30, PIXELWIDTH, 20, ILI9341_BLACK);

  tft.setCursor(120, 30);
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  tft.print("Freq: ");
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.print(currentFrequency);
}

/*****
  Purpose: Update the SWR value

  Argument list:
    Adafruit_ILI9341 tft      the display object
    float SWR                 the current SWR value

  Return value:
    void
*****/
void UpdateSWR(Adafruit_ILI9341 tft, float SWR, char msg[])
{
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(60, 30);
  if (strlen(msg) > 0) {
    tft.print(msg);
  } else {
    if (SWR > 1.0 && SWR < 5.0) {
      tft.print(SWR);
    } else {
      tft.print("> 5");
    }
  }
}

/*****
  Purpose: To display the SWR amd frequency data

  Argument list:
    Adafruit_ILI9341 tft      the display object
    float SWR                 the current SWR value

  Return value:
    void
*****/
void UpdatePositionValue(Adafruit_ILI9341 tft, int currPos)
{
  tft.fillRect(200, 55, 240, 20, ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setCursor(260, 55);
  tft.println(currPos);
}

/*****
  Purpose: To display the SWR plot a +/- 5K from minimum SWR

  Argument list:
    float currentSWR                 the current SWR value

  Return value:
    void
*****/
void PlotFinalSWR(float currentSWR)
{
  int i;
  long testFreq = currentFrequency - 3000L;
  float tempSWR;

  for (i = 0; i < 6000; i += 100) {
    SendFrequency(testFreq);
    tempSWR = ReadSWRValue();
    Serial.print("testFreq = ");
    Serial.print(testFreq);
    Serial.print("   tempSWR = ");
    Serial.println(tempSWR);
    testFreq += 100L;
  }
}

/*****
  Purpose: To allow variable increments on frequency

  Argument list:
    int bandIndex       // which of the three bands was selected?

  Return value:
    void
*****/
void ChangeFrequency(int bandIndex)
{
  int i, changeDigit, digitSpacing, halfScreen, incrementPad, insetMargin, insetPad;
  long defaultIncrement;

  insetPad         = 0;                         // Used to align digit indicator
  incrementPad     = 0;
  digitSpacing     = 20;
  insetMargin      = 20;
  defaultIncrement = 100L;
  halfScreen   = PIXELHEIGHT / 2 - 25;

  if (bandIndex == 0) {                 // 40M??
    insetPad -= digitSpacing;            // smaller number, so less spacing to a given digit
  }
  EraseBelowMenu();
  Serial.print("In ChangeFrequency() bandIndex = ");
  Serial.print(bandIndex);
  Serial.print("   insetMargin = ");
  Serial.println(insetMargin);

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);                      // Messages
  tft.setCursor(insetMargin, halfScreen + 60);
  tft.print("Increment: Menu encoder");
  tft.setCursor(insetMargin, halfScreen + 90);
  tft.print("   To end: Freq encoder");

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

  while (digitalRead(FREQUENCYENCODERSWITCH) != LOW && digitalRead(MAXSWITCH) != LOW) {
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
      } else {
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
    }
  }
  tft.setTextSize(2);                         // Back to normal
  tft.setTextColor(ILI9341_WHITE);
}
