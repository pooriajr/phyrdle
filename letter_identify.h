#ifndef LETTER_IDENTIFY_H
#define LETTER_IDENTIFY_H

#include <Arduino.h>

// Function to identify a letter from an analog reading
String identify(int analogRead) {
  switch (analogRead) {
      case 920 ... 940: return "A";
      case 881 ... 901: return "B";
      case 843 ... 863: return "C";
      case 815 ... 835: return "D";
      case 765 ... 785: return "E";
      case 739 ... 759: return "F";
      case 703 ... 720: return "G";
      case 685 ... 702: return "H";
      case 665 ... 675: return "I";
      case 640 ... 660: return "J";
      case 594 ... 614: return "K";
      case 561 ... 581: return "L";
      case 519 ... 535: return "M";
      case 501 ... 518: return "N";
      case 475 ... 495: return "O";
      case 449 ... 469: return "P";
      case 403 ... 423: return "Q";
      case 370 ... 390: return "R";
      case 335 ... 355: return "S";
      case 307 ... 327: return "T";
      case 285 ... 305: return "U";
      case 267 ... 284: return "V";
      case 230 ... 250: return "W";
      case 182 ... 196: return "X";
      case 167 ... 181: return "Y";
      case 117 ... 137: return "Z";
      default: return String(analogRead);
  }
}

#endif // LETTER_IDENTIFY_H 