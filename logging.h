#ifndef LOGGING_H
#define LOGGING_H

#include <Arduino.h>
#include "slot.h"

// Function to print the current state of all slots
void printSlotStates(Slot slots[], int slotCount) {
  Serial.println("Current slot states:");
  for (int i = 0; i < slotCount; i++) {
    Serial.print("Slot ");
    Serial.print(i + 1);
    // Log the exact raw and filtered samples used for this scan.
    Serial.print(F(" (Raw: "));
    Serial.print(slots[i].rawValue);
    Serial.print(F(", Mean[8]/classify: "));
    Serial.print(slots[i].signalValue);
    Serial.print(F(", Letter: "));
    if (slots[i].letter.length() == 0) {
      Serial.print(F("<empty>"));
    } else {
      Serial.print(slots[i].letter);
    }
    Serial.print(F(", Range: "));
    if (slots[i].signalValue <= EMPTY_SLOT_MAX_ADC) {
      Serial.print(F("<="));
      Serial.print(EMPTY_SLOT_MAX_ADC);
    } else {
      bool matched = false;
      // Mirror identify()'s inclusive, first-match lookup using the active
      // profile table, so diagnostic bounds cannot go stale independently.
      for (uint8_t r = 0; r < LETTER_ADC_RANGE_COUNT; r++) {
        uint16_t minimum = pgm_read_word(&LETTER_ADC_RANGES[r].minimum);
        uint16_t maximum = pgm_read_word(&LETTER_ADC_RANGES[r].maximum);
        if (slots[i].signalValue >= minimum &&
            slots[i].signalValue <= maximum) {
          Serial.print(minimum);
          Serial.print(F(".."));
          Serial.print(maximum);
          Serial.print(F(" inclusive"));
          matched = true;
          break;
        }
      }
      if (!matched) {
        Serial.print(F("none/unrecognized"));
      }
    }
    Serial.print(F("): "));

    // Print the state as text
    switch(slots[i].state) {
      case EMPTY:
        Serial.println("EMPTY");
        break;
      case FULL:
        Serial.println("FULL");
        break;
      case MISPLACED:
        Serial.println("MISPLACED");
        break;
      case CORRECT:
        Serial.println("CORRECT");
        break;
      case ABSENT:
        Serial.println("ABSENT");
        break;
      case WIN:
        Serial.println("WIN");
        break;
      case INVALID:
        Serial.println("INVALID");
        break;
      default:
        Serial.println("UNKNOWN");
        break;
    }
  }
}

#endif // LOGGING_H 