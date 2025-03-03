#ifndef LETTER_IDENTIFY_H
#define LETTER_IDENTIFY_H

#include <Arduino.h>

// Function to identify a letter from an analog reading
String identify(int analogRead) {
  switch (analogRead) {
      case 102 ... 123: return "A";
      case 133 ... 153: return "B";
      case 164 ... 184: return "C";
      case 194 ... 215: return "D";
      case 225 ... 250: return "E";
      case 256 ... 276: return "F";
      case 286 ... 307: return "G";
      case 317 ... 338: return "H";
      case 348 ... 368: return "I";
      case 379 ... 399: return "J";
      case 409 ... 430: return "K";
      case 440 ... 460: return "L";
      case 471 ... 491: return "M";
      case 501 ... 522: return "N";
      case 532 ... 552: return "O";
      case 563 ... 583: return "P";
      case 593 ... 614: return "Q";
      case 624 ... 644: return "R";
      case 666 ... 684: return "S";
      case 685 ... 706: return "T";
      case 716 ... 737: return "U";
      case 747 ... 767: return "V";
      case 777 ... 798: return "W";
      case 808 ... 829: return "X";
      case 839 ... 859: return "Y";
      case 870 ... 890: return "Z";
      default: return String(analogRead);
  }
}

#endif // LETTER_IDENTIFY_H 