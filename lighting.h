#ifndef LIGHTING_H
#define LIGHTING_H

#include <Arduino.h>
#include <FastLED.h>
#include "slot.h"  // Include slot.h to access the slot states

// FastLED setup
#define LED_PIN     5     // Pin connected to the WS2812B LED strip
#define NUM_LEDS    5     // Number of LEDs in the strip
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

// Define the LED array directly in this file
CRGB leds[NUM_LEDS];

// Define colors and brightness levels
#define DIM_WHITE  CRGB(10, 10, 10)   // Dim white for empty slots
#define BRIGHT_WHITE CRGB(255, 255, 255) // Bright white for full slots

// Initialize the LED strip
void setupLighting() {
  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(50);  // Set brightness to 50 (0-255)
  
  // Clear all LEDs
  FastLED.clear();
  FastLED.show();
}

// Forward declaration of helper function
void updateLEDForSlot(int ledIndex, Slot &slot);

// Update LEDs based on slot states
// Maps slot 1 to LED 1 (index 0), slot 2 to LED 3 (index 2), and slot 3 to LED 5 (index 4)
void updateSlotLEDs(Slot slots[], int slotCount) {
  // Clear all LEDs first
  FastLED.clear();
  
  // Map slots to specific LEDs (1->1, 2->3, 3->5)
  // Only update if we have enough slots and LEDs
  if (slotCount >= 1 && NUM_LEDS >= 1) {
    // Slot 1 -> LED 1 (index 0)
    updateLEDForSlot(0, slots[0]);
  }
  
  if (slotCount >= 2 && NUM_LEDS >= 3) {
    // Slot 2 -> LED 3 (index 2)
    updateLEDForSlot(2, slots[1]);
  }
  
  if (slotCount >= 3 && NUM_LEDS >= 5) {
    // Slot 3 -> LED 5 (index 4)
    updateLEDForSlot(4, slots[2]);
  }
  
  // Update the LED strip
  FastLED.show();
}

// Helper function to update a single LED based on slot state
void updateLEDForSlot(int ledIndex, Slot &slot) {
  switch (slot.state) {
    case EMPTY:
      leds[ledIndex] = DIM_WHITE;
      break;
    case FULL:
      leds[ledIndex] = BRIGHT_WHITE;
      break;
    case MISPLACED:
      leds[ledIndex] = CRGB::Orange;
      break;
    case CORRECT:
      leds[ledIndex] = CRGB::Green;
      break;
    case ABSENT:
      leds[ledIndex] = CRGB::Red;
      break;
    case WIN:
      // Rainbow effect could be implemented here
      leds[ledIndex] = CRGB::Purple;
      break;
    case INVALID:
      leds[ledIndex] = CRGB::Red;
      break;
    default:
      leds[ledIndex] = CRGB::Black;
      break;
  }
}

// Light up specific LEDs (first, third, and fifth) - keeping this for testing
void lightSpecificLEDs() {
  // Clear all LEDs first
  FastLED.clear();
  
  // Light up the first, third, and fifth LEDs
  leds[0] = CRGB::Red;    // First LED (index 0) - Red
  leds[2] = CRGB::Green;  // Third LED (index 2) - Green
  leds[4] = CRGB::Blue;   // Fifth LED (index 4) - Blue
  
  // Update the LED strip
  FastLED.show();
}

// Set all LEDs to a specific color
void setAllLEDs(CRGB color) {
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
}

// Set a specific LED to a color
void setLED(int index, CRGB color) {
  if (index >= 0 && index < NUM_LEDS) {
    leds[index] = color;
    FastLED.show();
  }
}

#endif // LIGHTING_H 