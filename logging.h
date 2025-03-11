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
    Serial.print(": ");
    
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