#ifndef LETTER_IDENTIFY_H
#define LETTER_IDENTIFY_H

#include <Arduino.h>
#include "hardware_profile.h"

// Return an empty string for an empty slot, the identified letter for a valid
// range, or "?" for an unrecognized reading between configured ranges.
String identify(int analogReading) {
  if (analogReading <= EMPTY_SLOT_MAX_ADC) {
    return "";
  }

  for (uint8_t i = 0; i < LETTER_ADC_RANGE_COUNT; i++) {
    uint16_t minimum = pgm_read_word(&LETTER_ADC_RANGES[i].minimum);
    uint16_t maximum = pgm_read_word(&LETTER_ADC_RANGES[i].maximum);

    if (analogReading >= minimum && analogReading <= maximum) {
      return String((char)pgm_read_byte(&LETTER_ADC_RANGES[i].letter));
    }
  }

  return "?";
}

#endif // LETTER_IDENTIFY_H 
