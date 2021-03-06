#ifndef BEENHERE
#include "MagLoop.h"
#endif

/*****
  Purpose: To move the cap[acitor to the approximate location via the stepper motor

  Parameter list:
    Adafruit_ILI9341 tft      the display object
    int whichBandOption
    int submenuIndex          which of presets
    
  Return value:
    void

  CAUTION:

*****/
void ProcessPresets(Adafruit_ILI9341 tft, int whichBandOption, int submenuIndex)
{
  int i;

  EraseBelowMenu();

  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);       // Show presets for selected band
  for (i = 0; i < PRESETSPERBAND; i++) {
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    tft.setCursor(30, 70 + i * 20);
    tft.print(i + 1);
    tft.print(".");
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setCursor(65, 70 + i * 20);
    tft.print(presetFrequencies[whichBandOption][i]);
  }

  tft.setTextColor(ILI9341_MAGENTA, ILI9341_WHITE);
  tft.setCursor(65, 70 + submenuIndex * 20);
  tft.print(presetFrequencies[whichBandOption][submenuIndex]);

  digitalWrite(MENUENCODERSWITCH, HIGH);

  menuEncoderState = 0;
  MyDelay(100L);
  while (digitalRead(MENUENCODERSWITCH) != LOW) {
    if (menuEncoderMovement == 1) {                                  // Turning clockwise
#ifdef DEBUG      
      Serial.print("in enc = 1, submenuIndex = ");
      Serial.println(submenuIndex);
#endif      
      RestorePreviousPresetChoice(tft, submenuIndex, whichBandOption);
      MyDelay(200L);
      submenuIndex++;
      if (submenuIndex > PRESETSPERBAND - 1)
        submenuIndex = 0;
      HighlightNewPresetChoice(tft, submenuIndex, whichBandOption);
      menuEncoderMovement = 0;
    }
    if (menuEncoderMovement == -1) {                                      // Tuning counter-clockwise
      RestorePreviousPresetChoice(tft, submenuIndex, whichBandOption);
      submenuIndex--;
      if (submenuIndex < 0)
        submenuIndex = PRESETSPERBAND - 1;
      HighlightNewPresetChoice(tft, submenuIndex, whichBandOption);
      menuEncoderMovement = 0;
    }
  }
  currentFrequency = presetFrequencies[whichBandOption][submenuIndex];
  UpdateFrequency(tft);
#ifdef DEBUG   
Serial.print("Leaving ProcessPresets(), currentFrequency = ");
Serial.println(currentFrequency);  
#endif
}

/*****
  Purpose: To restore most recently highlighted preset choice

  Parameter list:
    Adafruit_ILI9341 tft      the display object
    int submenuIndex
    int whichBandOption

  Return value:
    void
*****/
void RestorePreviousPresetChoice(Adafruit_ILI9341 tft, int submenuIndex, int whichBandOption)
{
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);               // restore old background
  tft.setCursor(65, 70 + submenuIndex * 20);
  tft.print(presetFrequencies[whichBandOption][submenuIndex]);
}


/*****
  Purpose: To highlight new preset choice

  Parameter list:
    Adafruit_ILI9341 tft      the display object
    int submenuIndex
    int whichBandOption

  Return value:
    void
*****/
void HighlightNewPresetChoice(Adafruit_ILI9341 tft, int submenuIndex, int whichBandOption)
{
  tft.setTextColor(ILI9341_MAGENTA, ILI9341_WHITE);         // HIghlight new preset choice
  tft.setCursor(65, 70 + submenuIndex * 20);
  tft.print(presetFrequencies[whichBandOption][submenuIndex]);
}
