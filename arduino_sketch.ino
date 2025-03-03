#include <FastLED.h>
#include "letter_identify.h"

// ===== HARDWARE CONFIGURATION =====
#define LED_PIN 5
#define NUM_LEDS 2

// ===== GAME SETTINGS =====
// List of valid 2-letter words
const char* VALID_WORDS[] = {"AN", "AT", "IN", "IT", "NO", "ON", "TO", "TI"};
const int NUM_VALID_WORDS = sizeof(VALID_WORDS) / sizeof(VALID_WORDS[0]);

// ===== VISUAL SETTINGS =====
// LED colors for different states
#define COLOR_BREATHING CRGB(15, 15, 15)  // Dim white for breathing
#define COLOR_ACTIVE    CRGB(50, 50, 50)  // Bright white for active letter
#define COLOR_CORRECT   CRGB(0, 50, 0)    // Green for correct letter
#define COLOR_PRESENT   CRGB(50, 20, 0)   // Orange for letter in wrong position
#define COLOR_ABSENT    CRGB(0, 0, 0)     // Off for letter not in word
#define COLOR_INVALID   CRGB(50, 0, 0)    // Red for invalid word

// Breathing animation settings
#define BREATH_MIN 3       // Minimum brightness
#define BREATH_MAX 15      // Maximum brightness
#define BREATH_PERIOD 2500 // Complete breath cycle time (ms)
#define ANIMATION_SPEED 20 // Update interval (ms)

// ===== LETTER DETECTION SETTINGS =====
#define CONFIDENCE_MAX 15      // Maximum confidence level
#define CONFIDENCE_THRESHOLD 10 // Threshold to consider a letter stable
#define CONFIDENCE_GAIN 2      // How quickly confidence increases
#define CONFIDENCE_DECAY 1     // How quickly confidence decreases

// ===== GAME STATE =====
// Main game states
enum GameState {
  STATE_WAITING,       // Waiting for letters
  STATE_WORD_FEEDBACK, // Showing feedback for a word
  STATE_INVALID_WORD   // Showing invalid word indicator
};

// Global state variables
GameState gameState = STATE_WAITING;
String targetWord = "";
String currentLetters[2] = {"", ""};
bool letterChanged = false;
CRGB leds[NUM_LEDS];

// Letter confidence tracking
int letterConfidence[2][26] = {{0}}; // Confidence for each letter A-Z in each slot
char mostConfidentLetter[2] = {0, 0};
int maxConfidence[2] = {0, 0};

// Timing variables
unsigned long lastAnimationUpdate = 0;
unsigned long lastLogTime = 0;

// ===== SETUP =====
void setup() {
  // Initialize hardware
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  Serial.begin(9600);
  
  // Wait for serial to initialize
  delay(1000);
  
  // Pick a random target word
  selectRandomWord();
  
  // Start in waiting state
  gameState = STATE_WAITING;
  Serial.println("Game started! Target word: " + targetWord);
}

// ===== MAIN LOOP =====
void loop() {
  // 1. Read and process input
  readLetters();
  
  // 2. Process current state
  switch (gameState) {
    case STATE_WAITING:
      handleWaitingState();
      break;
      
    case STATE_WORD_FEEDBACK:
      handleWordFeedbackState();
      break;
      
    case STATE_INVALID_WORD:
      handleInvalidWordState();
      break;
  }
  
  // 3. Log status (if needed)
  logStatus();
  
  // Small delay for stability
  delay(10);
}

// ===== INPUT PROCESSING =====
// Read analog inputs and update letter confidence
void readLetters() {
  // Read analog values
  int analogValues[2];
  analogValues[0] = 1023 - analogRead(A0);
  analogValues[1] = 1023 - analogRead(A1);
  
  // Process each slot
  for (int slot = 0; slot < 2; slot++) {
    // Get letter from analog value
    String rawLetter = identify(analogValues[slot]);
    
    // Update confidence for this letter
    updateLetterConfidence(slot, rawLetter);
    
    // Check if the most confident letter changed
    String previousLetter = currentLetters[slot];
    
    // Update current letter if confidence threshold is met
    if (maxConfidence[slot] >= CONFIDENCE_THRESHOLD) {
      currentLetters[slot] = String(mostConfidentLetter[slot]);
    } else {
      currentLetters[slot] = "";
    }
    
    // Check if letter changed
    if (currentLetters[slot] != previousLetter) {
      letterChanged = true;
      
      // If in invalid word state, go back to waiting
      if (gameState == STATE_INVALID_WORD) {
        changeState(STATE_WAITING, "Letter changed");
      }
    }
  }
}

// Update confidence levels for a letter in a slot
void updateLetterConfidence(int slot, String rawLetter) {
  // Only process valid letters
  if (rawLetter.length() != 1 || rawLetter[0] < 'A' || rawLetter[0] > 'Z') {
    // Decay all confidences for this slot
    for (int i = 0; i < 26; i++) {
      letterConfidence[slot][i] = max(0, letterConfidence[slot][i] - CONFIDENCE_DECAY);
    }
    return;
  }
  
  // Get letter index (0-25 for A-Z)
  int letterIndex = rawLetter[0] - 'A';
  
  // Increase confidence for the detected letter
  letterConfidence[slot][letterIndex] = min(CONFIDENCE_MAX, 
                                           letterConfidence[slot][letterIndex] + CONFIDENCE_GAIN);
  
  // Decay confidence for all other letters
  for (int i = 0; i < 26; i++) {
    if (i != letterIndex) {
      letterConfidence[slot][i] = max(0, letterConfidence[slot][i] - CONFIDENCE_DECAY);
    }
  }
  
  // Find the most confident letter
  maxConfidence[slot] = 0;
  for (int i = 0; i < 26; i++) {
    if (letterConfidence[slot][i] > maxConfidence[slot]) {
      maxConfidence[slot] = letterConfidence[slot][i];
      mostConfidentLetter[slot] = 'A' + i;
    }
  }
}

// ===== STATE HANDLERS =====
// Handle the waiting state (breathing animation, detect words)
void handleWaitingState() {
  // Update breathing animation
  updateBreathingAnimation();
  
  // Check if we have a complete word
  if (hasValidLetters(0) && hasValidLetters(1) && letterChanged) {
    String word = currentLetters[0] + currentLetters[1];
    
    // Check if it's a valid word
    if (isValidWord(word)) {
      // Valid word - check against target
      changeState(STATE_WORD_FEEDBACK, "Valid word: " + word);
      checkWord();
    } else {
      // Invalid word
      changeState(STATE_INVALID_WORD, "Invalid word: " + word);
      showInvalidWord();
    }
    
    // Reset the letter changed flag
    letterChanged = false;
  }
}

// Handle the word feedback state (showing green/orange/off)
void handleWordFeedbackState() {
  // Only process if letters changed
  if (letterChanged && hasValidLetters(0) && hasValidLetters(1)) {
    String word = currentLetters[0] + currentLetters[1];
    
    if (isValidWord(word)) {
      // Check the new word
      checkWord();
    } else {
      // Invalid word
      changeState(STATE_INVALID_WORD, "Invalid word: " + word);
      showInvalidWord();
    }
    
    letterChanged = false;
  }
}

// Handle the invalid word state (showing red)
void handleInvalidWordState() {
  // Just keep showing red until letters change
  // (transition handled in readLetters)
}

// ===== VISUAL EFFECTS =====
// Update the breathing animation
void updateBreathingAnimation() {
  if (millis() - lastAnimationUpdate > ANIMATION_SPEED) {
    lastAnimationUpdate = millis();
    
    // Calculate breathing brightness
    float position = (float)(millis() % BREATH_PERIOD) / BREATH_PERIOD;
    float normalized = (sin(position * 2.0 * PI - PI/2) + 1.0) / 2.0;
    normalized = pow(normalized, 1.5);
    int brightness = BREATH_MIN + normalized * (BREATH_MAX - BREATH_MIN);
    
    // Update each LED
    for (int i = 0; i < NUM_LEDS; i++) {
      if (hasValidLetters(i)) {
        // Show white for active letter
        leds[i] = COLOR_ACTIVE;
      } else {
        // Show breathing animation
        leds[i] = CRGB(brightness, brightness, brightness);
      }
    }
    
    FastLED.show();
  }
}

// Check word against target and update LEDs
void checkWord() {
  // Get target letters
  String targetLetter0 = targetWord.substring(0, 1);
  String targetLetter1 = targetWord.substring(1, 2);
  
  // Check first letter
  if (currentLetters[0] == targetLetter0) {
    leds[0] = COLOR_CORRECT;
  } else if (currentLetters[0] == targetLetter1) {
    leds[0] = COLOR_PRESENT;
  } else {
    leds[0] = COLOR_ABSENT;
  }
  
  // Check second letter
  if (currentLetters[1] == targetLetter1) {
    leds[1] = COLOR_CORRECT;
  } else if (currentLetters[1] == targetLetter0) {
    leds[1] = COLOR_PRESENT;
  } else {
    leds[1] = COLOR_ABSENT;
  }
  
  FastLED.show();
  
  // Log the check
  Serial.println("\n**** WORD CHECK ****");
  Serial.print("Target: ");
  Serial.print(targetWord);
  Serial.print(" | Guess: ");
  Serial.println(currentLetters[0] + currentLetters[1]);
  Serial.println("******************\n");
  
  // Check for win
  if (currentLetters[0] == targetLetter0 && currentLetters[1] == targetLetter1) {
    Serial.println("\n🎉 CORRECT WORD! YOU WIN! 🎉");
    celebrate();
    selectRandomWord();
    changeState(STATE_WAITING, "Word solved! New word selected");
  }
}

// Show invalid word indicator
void showInvalidWord() {
  fill_solid(leds, NUM_LEDS, COLOR_INVALID);
  FastLED.show();
}

// Celebration effect
void celebrate() {
  for (int i = 0; i < 10; i++) {
    // Flash green
    fill_solid(leds, NUM_LEDS, COLOR_CORRECT);
    FastLED.show();
    delay(100);
    
    // Flash off
    fill_solid(leds, NUM_LEDS, COLOR_ABSENT);
    FastLED.show();
    delay(100);
  }
}

// ===== HELPER FUNCTIONS =====
// Change game state
void changeState(GameState newState, String reason) {
  String oldStateName = getStateName(gameState);
  gameState = newState;
  String newStateName = getStateName(gameState);
  
  Serial.println("\n==== STATE CHANGE ====");
  Serial.println("From: " + oldStateName + " to: " + newStateName);
  Serial.println("Reason: " + reason);
  Serial.println("======================\n");
}

// Get state name as string
String getStateName(GameState state) {
  switch (state) {
    case STATE_WAITING: return "WAITING";
    case STATE_WORD_FEEDBACK: return "WORD_FEEDBACK";
    case STATE_INVALID_WORD: return "INVALID_WORD";
    default: return "UNKNOWN";
  }
}

// Check if a slot has a valid letter
bool hasValidLetters(int slot) {
  return currentLetters[slot].length() == 1 && 
         currentLetters[slot][0] >= 'A' && 
         currentLetters[slot][0] <= 'Z';
}

// Check if a word is in our valid list
bool isValidWord(const String& word) {
  String upperWord = word;
  upperWord.toUpperCase();
  
  for (int i = 0; i < NUM_VALID_WORDS; i++) {
    if (upperWord == VALID_WORDS[i]) {
      return true;
    }
  }
  return false;
}

// Select a random word from our list
void selectRandomWord() {
  randomSeed(analogRead(A5));
  int randomIndex = random(NUM_VALID_WORDS);
  targetWord = String(VALID_WORDS[randomIndex]);
  Serial.print("Selected target word: ");
  Serial.println(targetWord);
}

// Log current status
void logStatus() {
  // Only log periodically
  if (millis() - lastLogTime < 2000) {
    return;
  }
  
  lastLogTime = millis();
  
  // Create status string
  String status = "State: " + getStateName(gameState);
  status += " | Target: " + targetWord;
  status += " | Letters: [";
  status += hasValidLetters(0) ? currentLetters[0] : "---";
  status += ", ";
  status += hasValidLetters(1) ? currentLetters[1] : "---";
  status += "]";
  
  Serial.println(status);
}

