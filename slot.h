#ifndef SLOT_H
#define SLOT_H

#include <Arduino.h>
#include "letter_identify.h"  // Include the letter identification header

const uint8_t ADC_SAMPLE_COUNT = 9;
const uint8_t REQUIRED_STABLE_READS = 3;

// Read a channel several times and return the median. Discarding the first
// conversion gives the ADC sample-and-hold capacitor time to settle after the
// input multiplexer switches channels.
int readFilteredAnalog(int pin) {
  int samples[ADC_SAMPLE_COUNT];

  analogRead(pin);

  for (uint8_t i = 0; i < ADC_SAMPLE_COUNT; i++) {
    samples[i] = analogRead(pin);
  }

  // Insertion sort is small and predictable for this nine-element array.
  for (uint8_t i = 1; i < ADC_SAMPLE_COUNT; i++) {
    int value = samples[i];
    int8_t j = i - 1;

    while (j >= 0 && samples[j] > value) {
      samples[j + 1] = samples[j];
      j--;
    }
    samples[j + 1] = value;
  }

  return samples[ADC_SAMPLE_COUNT / 2];
}

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
    String candidateLetter;
    uint8_t candidateReadCount;
    
    // Constructor with default values
    Slot() : pin(0), state(EMPTY), letter(""), signalValue(0),
             candidateLetter(""), candidateReadCount(0) {}
    
    // Constructor with pin
    Slot(int p) : pin(p), state(EMPTY), letter(""), signalValue(0),
                  candidateLetter(""), candidateReadCount(0) {}
    
    // Method to read analog signal and identify letter
    void readLetter() {
      signalValue = readFilteredAnalog(pin);

      String observedLetter = signalValue > 100 ? identify(signalValue) : "";

      // Ignore readings that fall between calibrated letter ranges. A brief
      // noisy sample should not replace a previously accepted letter.
      if (observedLetter == "?") {
        candidateReadCount = 0;
        state = letter.length() > 0 ? FULL : EMPTY;
        return;
      }

      if (observedLetter == candidateLetter) {
        if (candidateReadCount < REQUIRED_STABLE_READS) {
          candidateReadCount++;
        }
      } else {
        candidateLetter = observedLetter;
        candidateReadCount = 1;
      }

      if (candidateReadCount >= REQUIRED_STABLE_READS) {
        letter = candidateLetter;
      }

      state = letter.length() > 0 ? FULL : EMPTY;
    }
};

#endif // SLOT_H
