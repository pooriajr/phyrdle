#ifndef LIGHTING_H
#define LIGHTING_H

#include <Arduino.h>
#include <FastLED.h>
#include "light_feedback.h"
#include "slot.h"  // Include slot.h to access the slot states

// FastLED setup
#ifndef USE_FOUR_PIN_LED_STRIP
#define USE_FOUR_PIN_LED_STRIP false // fallback value, to change, set in arduino_sketch.ino
#endif

#ifndef THREE_PIN_LED_DATA_PIN
#define THREE_PIN_LED_DATA_PIN 2
#endif

#ifndef FOUR_PIN_LED_DATA_PIN
#define FOUR_PIN_LED_DATA_PIN 11
#endif

#ifndef FOUR_PIN_LED_CLOCK_PIN
#define FOUR_PIN_LED_CLOCK_PIN 13
#endif

#define NUM_LEDS    5     // Number of LEDs in the strip
#define THREE_PIN_LED_TYPE WS2812B
#define FOUR_PIN_LED_TYPE APA102
#define THREE_PIN_COLOR_ORDER GRB
#define FOUR_PIN_COLOR_ORDER BGR

// Define the LED array directly in this file
CRGB leds[NUM_LEDS];

// Empty and occupied slots share a quiet baseline after recognition.
LightFeedback lightFeedback;

// Variables for rainbow animation
uint8_t gHue = 0; // rotating "base color" used by rainbow effect
uint32_t lastRainbowFrameAt = 0;
const uint8_t NORMAL_BRIGHTNESS = 50;
const uint8_t RAINBOW_BRIGHTNESS = 200;
const uint16_t RAINBOW_FRAME_MS = 100;

// Initialize the LED strip
void setupLighting() {
  // Initialize FastLED
#if USE_FOUR_PIN_LED_STRIP
  FastLED.addLeds<FOUR_PIN_LED_TYPE, FOUR_PIN_LED_DATA_PIN, FOUR_PIN_LED_CLOCK_PIN, FOUR_PIN_COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
#else
  FastLED.addLeds<THREE_PIN_LED_TYPE, THREE_PIN_LED_DATA_PIN, THREE_PIN_COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
#endif
  FastLED.setBrightness(NORMAL_BRIGHTNESS);
  
  // Clear all LEDs
  FastLED.clear();
  FastLED.show();
}

// Forward declaration of helper function
void updateLEDForSlot(int ledIndex, Slot &slot, uint32_t now);

// Update LEDs based on slot states
// Maps each slot directly to the corresponding LED (slot 1 to LED 1, slot 2 to LED 2, etc.)
void updateSlotLEDs(Slot slots[], int slotCount) {
  // Clear all LEDs first
  FastLED.clear();

  const uint32_t now = millis();
  char letters[LightFeedback::SLOT_COUNT] = {};
  bool valid = slotCount == LightFeedback::SLOT_COUNT;
  bool denied = slotCount == LightFeedback::SLOT_COUNT;
  for (int i = 0; i < min(slotCount, int(LightFeedback::SLOT_COUNT)); ++i) {
    letters[i] = slots[i].letter.length() ? slots[i].letter[0] : 0;
    const slotState state = slots[i].state;
    denied = denied && state == INVALID;
    valid = valid && (state == ABSENT || state == MISPLACED ||
                      state == CORRECT);
  }
  lightFeedback.update(letters, valid, now, denied);

  // Update LEDs for each slot
  // Only update if we have enough slots and LEDs
  int maxSlots = min(slotCount, NUM_LEDS);
  
  for (int i = 0; i < maxSlots; i++) {
    updateLEDForSlot(i, slots[i], now);
  }
  
  // Update the LED strip
  FastLED.show();
}

// Helper function to update a single LED based on slot state
void updateLEDForSlot(int ledIndex, Slot &slot, uint32_t now) {
  const uint8_t white = lightFeedback.neutralBrightness(ledIndex, now);
  const bool result = slot.state == ABSENT || slot.state == MISPLACED ||
                      slot.state == CORRECT;
  if (result && !lightFeedback.isRevealed(ledIndex, now)) {
    leds[ledIndex] = CRGB(white, white, white);
    return;
  }
  switch (slot.state) {
    case EMPTY:
    case FULL:
    case WIN: // Winning guesses use the rainbow renderer directly.
      leds[ledIndex] = CRGB(white, white, white);
      break;
    case MISPLACED:
      leds[ledIndex] = CRGB::DarkOrange;
      break;
    case CORRECT:
      leds[ledIndex] = CRGB::Green;
      break;
    case ABSENT:
      leds[ledIndex] = CRGB(LightFeedback::ABSENT_WHITE,
                            LightFeedback::ABSENT_WHITE,
                            LightFeedback::ABSENT_WHITE);
      break;
    case INVALID:
      leds[ledIndex] = CRGB(lightFeedback.deniedBrightness(now), 0, 0);
      break;
    default:
      leds[ledIndex] = CRGB::Black;
      break;
  }
}

// Reveal the animated rainbow from left to right as soon as a win is detected.
void updateRainbowAnimation(uint32_t now, uint32_t winStarted) {
  // Refresh the entry sweep frequently while preserving the rainbow's hue speed.
  if (uint32_t(now - lastRainbowFrameAt) >= RAINBOW_FRAME_MS) {
    gHue += 10;
    lastRainbowFrameAt = now;
  }
  FastLED.setBrightness(RAINBOW_BRIGHTNESS);
  fill_rainbow(leds, NUM_LEDS, gHue, 15);

  // Keep slots ahead of the sweep at their original neutral brightness, even
  // though the rainbow uses a higher global brightness.
  const uint8_t neutral = uint16_t(LightFeedback::NEUTRAL_WHITE) *
                          NORMAL_BRIGHTNESS / RAINBOW_BRIGHTNESS;
  for (uint8_t i = 0; i < NUM_LEDS; ++i) {
    if (!LightFeedback::rainbowSlotVisible(i, uint32_t(now - winStarted))) {
      leds[i] = CRGB(neutral, neutral, neutral);
    }
  }
  FastLED.show();
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
