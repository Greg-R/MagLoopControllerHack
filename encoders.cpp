#ifndef BEENHERE
#include "MagLoop.h"
#endif
#include <Arduino.h>

/*****
  Purpose: The ISR to detect the rotary encoder's direction of movement
  
  Parameter list: 
    void
  
  Return value:  
    void

  CAUTION: This encoder is used for multiple purposes, all sensing direction change
*****/
void menuISR() {
  unsigned char result = menuEncoder.process();

  menuEncoderMovement = 0;
  
  if (result == DIR_CW) {
    menuEncoderMovement = 1;
  } else if (result == DIR_CCW) {
    menuEncoderMovement = -1;
  }
}

/*****
   Purpose: ISR to read rotary encoder #1
  Paramter list: void
  Return value:  void
*****/
void frequencyISR() {
  unsigned char result = frequencyEncoder.process();
  
  frequencyEncoderMovement = 0;

  if (result == 0) {                  // No movement
    return;
  }

  if (result == DIR_CW) {             // Clockwise
    frequencyEncoderMovement++; 
  } else if (result == DIR_CCW) {
    frequencyEncoderMovement--;  
  }
}
