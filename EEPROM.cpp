//
//  Fast hack to solve compile errors when using STM32 Core and STMCubeProgrammer.
//  This does not yield a working system, it only demonstrates the incompatibilities.
//  Greg KF5N
//  January 2 2021


#ifndef BEENHERE
#include "MagLoop.h"
#endif

/*****

  The EEPROM memory map for the remote unit is as follows:

    Address                                             Description
      0             Band. On virgin chip, this value is likely 0xFF, in which case Calibrate option is automatically run. Otherwise, 40, 30, oe 20.
                    Keep in mind that an int is 4 bytes on the STM32.
      4  thru 24    These are the stepper position counts for each of the 3 band edges, lowest first (40Mlow, 40Mhigh, 30Mlow, 30Mhigh, 20Mlow,
                    20Mhigh). Variabke holding these is bandLimitPositionCounts[MAXBANDS][2]. These are a long data type
      28 thru 100   These are the stepper positions for the presets, using same order as the band edges (40M, 30M, 20M), each with 6 values.
                    Variable holding these is presetFrequencies[MAXBANDS][PRESETSPERBAND];

  Note that EEPROM is 16-bit oriented and writes can only be on word boundaries.

  From STM32 EEPROM.cpp library file:

  On writes, the possible return values are:
    enum : uint16_t
      {
        EEPROM_OK       = ((uint16_t)0x0000),
        EEPROM_OUT_SIZE     = ((uint16_t)0x0081),
        EEPROM_BAD_ADDRESS    = ((uint16_t)0x0082),
        EEPROM_BAD_FLASH    = ((uint16_t)0x0083),
        EEPROM_NOT_INIT     = ((uint16_t)0x0084),
        EEPROM_SAME_VALUE   = ((uint16_t)0x0085),
        EEPROM_NO_VALID_PAGE  = ((uint16_t)0x00AB)
      };

  On reads, the possible return values are:

        - EEPROM_OK: if variable was found
        - EEPROM_BAD_ADDRESS: if the variable was not found
        - EEPROM_NO_VALID_PAGE: if no valid page was found.

*****/

union {
  uint16_t myBytes[sizeof(uint16_t)];
  int val;
} myUnion;
/*****
  Purpose: The STM32 does not actually have any EEPROM, so we have to fake it with flash memory. This code defines where the
           page in flash memory resides that will be used as EEPROM

  Parameter list:
    void

  Return value:
    void
*****/


/*****
  Purpose: To write default values for EEPROM

  Parameter list:
    void

  Return value:
    void
*****/


/*****
  Purpose: Show EEPROM values, mainly a debugging tool

  Parameter list:
    void

  Return value:
    void
*****/
void ShowEEPROMValues()
{
  int i, j, k, offset, status;
  int intSize;
  int result;

  intSize = sizeof(uint16_t);

  offset = LASTBANDUSED;                                // Read default band...40M
  result = EEPROM.read(offset++);
#ifdef DEBUG
  Serial.print("Default band  = ");
  Serial.println(result);
#endif

  offset = OFFSETTOPOSITIONCOUNTS;                      // Read position counts for endpoints

  for (i = 0; i < MAXBANDS; i++) {
    for (k = 0; k < 2; k++) {                           // 2 = writing low and hi values
      for (j = 0; j < intSize; j++) {                   // Write int to EEPROM as intSize-d bytes
        myUnion.myBytes[j] = EEPROM.read(offset++);
      }
      bandLimitPositionCounts[i][k] = myUnion.val;
#ifdef DEBUG
      Serial.print("bandLimitPositionCounts[");
      Serial.print(i);
      Serial.print("][");
      Serial.print(k);
      Serial.print("] = ");
      Serial.println(bandLimitPositionCounts[i][k]);
#endif
    }
  }

  offset = OFFSETTOPRESETS;                                   // Starting EEPROM address for Band presets

  for (i = 0; i < MAXBANDS; i++) {
    for (k = 0; k < PRESETSPERBAND; k++) {
      for (j = 0; j < sizeof(uint16_t); j++) {
        myUnion.myBytes[j] = EEPROM.read(offset++);
      }
      presetFrequencies[i][k] = myUnion.val;
#ifdef DEBUG
      Serial.print("presetFrequencies[");
      Serial.print(i);
      Serial.print("][");
      Serial.print(k);
      Serial.print("] = ");
      Serial.println(presetFrequencies[i][k]);
#endif
    }
  }
}



/*****
  Purpose: Save band edges position counts

  Parameter list:
    void

  Return value:
    void
*****/
void WritePositionCounts()
{
  int i, intSize, j, k, offset, status;

  intSize = sizeof(uint16_t);
  offset = OFFSETTOPOSITIONCOUNTS;

  for (i = 0; i < MAXBANDS; i++) {
    for (k = 0; k < 2; k++) {
      myUnion.val = bandLimitPositionCounts[i][k];                  // Copy low limit into byte array
      for (j = 0; j < intSize; j++) {                               // Write int to EEPROM as intSize-d bytes
        EEPROM.write(offset++, myUnion.myBytes[j]);
      }
    }
  }
}

/*****
  Purpose: Read configuration data from EEPROM

  Parameter list:
    void

  Return value:
    void
*****/
void ReadPositionCounts()
{
  uint16_t i, j, k, offset, status;

  offset = OFFSETTOPOSITIONCOUNTS;

  for (i = 0; i < MAXBANDS; i++) {
    for (k = 0; k < 2; k++) {
      for (j = 0; j < sizeof(uint16_t); j++) {                                     // Write int to EEPROM as bytes
        myUnion.myBytes[j] = EEPROM.read(offset++);
      }
      bandLimitPositionCounts[i][k] = myUnion.val;                  // Copy int into byte array
      Serial.print("bandLimitPositionCounts[");
      Serial.print(i);
      Serial.print("][");
      Serial.print(k);
      Serial.print("] = ");
      Serial.println(bandLimitPositionCounts[i][k]);   
    }
  }
 
           // Do slope coefficients
  countPerHertz[0] = (float) ((float) bandLimitPositionCounts[0][1] - (float) bandLimitPositionCounts[0][0]) / (float) ((float) HIGHEND40M - (float) LOWEND40M);
  countPerHertz[1] = (float) ((float) bandLimitPositionCounts[1][1] - (float) bandLimitPositionCounts[1][0]) / (float) ((float) HIGHEND30M - (float) LOWEND30M);
  countPerHertz[2] = (float) ((float) bandLimitPositionCounts[2][1] - (float) bandLimitPositionCounts[2][0]) / (float) ((float) HIGHEND20M - (float) LOWEND20M);

}

/*****
  Purpose: Show slope coefficients for frequency calcs

  Parameter list:
    void

  Return value:
    void
*****/
void ShowSlopeCoefficients()
{
  int i;
  for (i = 0; i < 3; i++)
    Serial.println( countPerHertz[i], 6);   
}
/*****
  Purpose: Write the presets for each band

  Parameter list:
    void

  Return value:
    void
*****/
int WriteBandPresets()
{
  uint16_t i, j, k, offset, status;

  offset = OFFSETTOPRESETS;                                       // Starting EEPROM address for Bnd presets

  for (i = 0; i < MAXBANDS; i++) {
    for (k = 0; k < PRESETSPERBAND; k++) {
      myUnion.val = presetFrequencies[i][k];                     // Get stored value...
      for (j = 0; j < sizeof(uint16_t); j++) {                        // Write int to EEPROM as bytes
        EEPROM.write(offset++, myUnion.myBytes[j]);
        
      }
    }
  }
  return status;
}
/*****
  Purpose: Read the presets for each band from EEPROM

  Parameter list:
  void

  Return value:
  void
*****/
void ReadBandPresets()
{
  uint16_t i, j, k, offset, size, status;

  offset = OFFSETTOPRESETS;                                       // Starting EEPROM address for Bnd presets

  size = sizeof(uint16_t);

  for (i = 0; i < MAXBANDS; i++) {
    for (k = 0; k < PRESETSPERBAND; k++) {
      for (j = 0; j < size; j++) {                                     // Write int to EEPROM as bytes
        myUnion.myBytes[j] = EEPROM.read(offset++);
      }
      presetFrequencies[i][k] = myUnion.val;                  // Copy int into byte array
#ifdef DEBUG
      Serial.print("presetFrequencies[");
      Serial.print(i);
      Serial.print("][");
      Serial.print(k);
      Serial.print("] = ");
      Serial.println(presetFrequencies[i][k]);
#endif
    }
  }
}

/*****
  Purpose: Read the value for currentBand
  Parameter list:
  void

  Return value:
  void
*****/
void ReadCurrentBand()
{
  uint16_t i, offset, status;

  offset = LASTBANDUSED;                                                // Starting EEPROM address for Bnd presets

  for (i = 0; i < sizeof(uint16_t); i++) {                                // Write int to EEPROM as bytes
    myUnion.myBytes[i] = EEPROM.read(offset++);
  }
  currentBand = myUnion.val;
Serial.print("currentBand = ");
Serial.println(currentBand);

  if (currentBand == 40 || currentBand == 30 || currentBand == 20) {    // System not calibrated
    return;
  } else {
    DoNewCalibrate();        // Create default values on first time through here...                                                
  }
}

/*****
  Purpose: Write the value for currentBand to EEPROM
  Parameter list:
  void

  Return value:
  void
*****/
void WriteCurrentBand()
{
  uint16_t i, offset, status;

  offset = LASTBANDUSED;                                            // Starting EEPROM address for Bnd presets

  myUnion.val = currentBand;
  for (i = 0; i < sizeof(uint16_t); i++) {                               // Write int to EEPROM as bytes
    EEPROM.write(offset++, myUnion.myBytes[i]);

  }
}


/*****
  Purpose: Debugging read of EEPROM and print to Serial monitor

  Parameter list:
    void

  Return value:
    void
*****/
void ShowPositionCounts()
{
  int i, offset, status;

  offset = 0;
  for (i = 0; i < MAXBANDS; i++, offset += sizeof(int)) {
#ifdef DEBUG
    Serial.print("bandLimitPositionCounts[");
    Serial.print(i);
    Serial.print("][0] = ");
    Serial.println(bandLimitPositionCounts[i][0]);

    Serial.print("bandLimitPositionCounts[");
    Serial.print(i);
    Serial.print("][1] = ");
    Serial.println(bandLimitPositionCounts[i][1]);
#endif
  }
}

/*****
  Purpose: Reads all of the values stored in EEPROM

  Parameter list:
    void

  Return value:
    void
*****/
void ReadEEPROMData()
{
  ReadCurrentBand();              // Get last band used
  ReadPositionCounts();           // Get band edge positions
  ReadBandPresets();              // Get preset frequencies
}
