#ifndef SLOT_H
#define SLOT_H

#include <Arduino.h>
#include "letter_identify.h"  // Include the letter identification header

// Slot states
enum slotState {
  EMPTY,    // no tile in the slot, show a dim white light
  FULL,     // a tile in this slot but other slots are empty, show a bright white light
  MISPLACED, // the letter is in the word, but not here, orange light
  CORRECT,  // this letter is here, green light
  ABSENT,   // this letter isn't in the word, red light
  WIN,      // win, game over, rainbow
  INVALID   // word doesn't exist in the database, blink red lights 
};

// Slot class
class Slot {
  public: 
    int pin;
    slotState state;
    String letter;
    int signalValue; // Store the current analog signal value
    
    // Constructor with default values
    Slot() : pin(0), state(EMPTY), letter(""), signalValue(0) {}
    
    // Constructor with pin
    Slot(int p) : pin(p), state(EMPTY), letter(""), signalValue(0) {}
    
    // Method to read analog signal and identify letter
    void readLetter() {
      // Read the analog value from the pin
      signalValue = analogRead(pin);
      
      // Use the letter_identify functionality to map the signal to a letter
      if (signalValue > 100) {  // Threshold to determine if a tile is present
        letter = identify(signalValue);  // Using the identify function from letter_identify.h
        state = FULL;
      } else {
        letter = "";
        state = EMPTY;
      }
    }
};

#endif // SLOT_H 