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
// Maps each slot directly to the corresponding LED (slot 1 to LED 1, slot 2 to LED 2, etc.)
void updateSlotLEDs(Slot slots[], int slotCount) {
  // Clear all LEDs first
  FastLED.clear();
  
  // Update LEDs for each slot
  // Only update if we have enough slots and LEDs
  int maxSlots = min(slotCount, NUM_LEDS);
  
  for (int i = 0; i < maxSlots; i++) {
    updateLEDForSlot(i, slots[i]);
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