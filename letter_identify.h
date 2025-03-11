#ifndef LETTER_IDENTIFY_H
#define LETTER_IDENTIFY_H

#include <Arduino.h>

// Function to identify a letter from an analog reading
String identify(int analogRead) {
  switch (analogRead) {
      case 920 ... 940: return "A";
      case 765 ... 785: return "E";
      case 370 ... 390: return "R";
      case 335 ... 355: return "S";
      case 307 ... 327: return "T";
      default: return String(analogRead);
  }
}

#endif // LETTER_IDENTIFY_H 