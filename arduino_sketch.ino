#include "dictionary.h" // Include our new dictionary header
#include "letter_identify.h"

// LED strip selection:
//   false = 3-pin WS2812B strip on THREE_PIN_LED_DATA_PIN
//   true  = 4-pin APA102/DotStar-style strip with DI/CI pins below
#define USE_FOUR_PIN_LED_STRIP true
#define THREE_PIN_LED_DATA_PIN 2
#define FOUR_PIN_LED_DATA_PIN 11  // DI
#define FOUR_PIN_LED_CLOCK_PIN 13 // CI

#include "lighting.h" // Include the lighting header
#include "logging.h"
#include "slot.h"
#include <FastLED.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define CHECK_INTERVAL 5 // milliseconds between input scans
#define WIN_ANIMATION_INTERVAL 100 // preserve the current rainbow speed
#define SLOT_COUNT 5
#define WIN_TIMEOUT 5000  // 3 seconds in milliseconds
#define LOG_INTERVAL 1000 // 1 second in milliseconds
#define LCD_ADDR 0x27
#define LCD_COLS 20
#define LCD_ROWS 4
#define ENABLE_RUNTIME_DIAGNOSTICS false

// Forward declaration of global variables
extern Dictionary dictionary;
extern Slot slots[];

// Variables to track win state
bool gameWon = false;
unsigned long winTimestamp = 0;
unsigned long lastLogTimestamp = 0; // For controlling log frequency
unsigned long lastLcdUpdate = 0;    // For controlling LCD refresh rate
String lastReportedWord = "";       // Avoid repeatedly logging an unchanged word
bool lcdEnabled = ENABLE_RUNTIME_DIAGNOSTICS;

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

class Word {
private:
  Slot *slots;
  int slotCount;
  String word;

public:
  enum WordStatus {
    INCOMPLETE, // One or more slots are empty
    INVALID,    // All slots have letters but the word is not in the dictionary
    VALID       // All slots have letters and the word is in the dictionary
  };

  WordStatus status;

  // Constructor
  Word(Slot *s, int count) {
    slots = s;
    slotCount = count;
    updateWord();
  }

  // Update the word based on current slot letters
  void updateWord() {
    word = "";
    bool hasEmptySlot = false;

    // Concatenate letters from all slots
    for (int i = 0; i < slotCount; i++) {
      if (slots[i].letter == "") {
        hasEmptySlot = true;
      } else {
        word += slots[i].letter;
      }
    }

    // Determine word status
    if (hasEmptySlot) {
      status = INCOMPLETE;
    } else if (word.length() == slotCount && dictionary.isValid(word)) {
      status = VALID;
    } else {
      status = INVALID;
    }
  }

  // Get the current word
  String getWord() { return word; }
};

class Result {
private:
  Word *wordPtr;
  String targetWord;
  Slot *slots;

public:
  // Constructor
  Result(Word *w, String target, Slot *s) {
    wordPtr = w;
    targetWord = target;
    slots = s;
    evaluateWord();
  }

  // Evaluate the word and update slot states
  void evaluateWord() {
    // Handle based on word status
    switch (wordPtr->status) {
    case Word::INCOMPLETE:
      // For incomplete words, keep slots as EMPTY or FULL
      // This is already handled by the Slot::readLetter() method
      break;

    case Word::INVALID:
      // For invalid words, mark all slots as INVALID
      for (int i = 0; i < SLOT_COUNT; i++) {
        slots[i].state = INVALID;
      }
      break;

    case Word::VALID:
      // For valid words, evaluate each letter against the target word
      evaluateLetters();
      break;
    }
  }

  // Evaluate each letter against the target word
  void evaluateLetters() {
    String currentWord = wordPtr->getWord();

    // First pass: mark correct letters
    for (int i = 0; i < SLOT_COUNT; i++) {
      if (i < currentWord.length() && i < targetWord.length()) {
        if (currentWord[i] == targetWord[i]) {
          slots[i].state = CORRECT;
        }
      }
    }

    // Second pass: mark misplaced and absent letters
    for (int i = 0; i < SLOT_COUNT; i++) {
      // Skip already marked correct letters
      if (slots[i].state == CORRECT)
        continue;

      if (i < currentWord.length()) {
        // Check if the letter exists elsewhere in the target word
        bool found = false;
        for (int j = 0; j < targetWord.length(); j++) {
          if (currentWord[i] == targetWord[j] && j != i) {
            found = true;
            break;
          }
        }

        if (found) {
          slots[i].state = MISPLACED;
        } else {
          slots[i].state = ABSENT;
        }
      }
    }

    // Check if all letters are correct (win condition)
    bool allCorrect = true;
    for (int i = 0; i < SLOT_COUNT; i++) {
      if (slots[i].state != CORRECT) {
        allCorrect = false;
        break;
      }
    }

    if (allCorrect) {
      // Set all slots to WIN state
      for (int i = 0; i < SLOT_COUNT; i++) {
        slots[i].state = WIN;
      }
    }
  }
};

class Board {
public:
  // Constructor
  Board() {
    // Nothing to initialize here
  }

  // Light up the slots based on their current state
  void light(Slot slots[], int slotCount) {
    // Use the updateSlotLEDs function from lighting.h
    updateSlotLEDs(slots, slotCount);
  }
};

// Global variables
Dictionary dictionary; // initialize a global dictionary
Board board;
String targetWord;
Slot slot1(A0); // Using analog pin A0
Slot slot2(A1); // Using analog pin A1
Slot slot3(A2); // Using analog pin A2
Slot slot4(A3); // Using analog pin A3
Slot slot5(A6); // Using analog pin A6
Slot slots[SLOT_COUNT] = {slot1, slot2, slot3, slot4, slot5};

void clearLcdLine(uint8_t row) {
  lcd.setCursor(0, row);
  for (int i = 0; i < LCD_COLS; i++) {
    lcd.print(' ');
  }
}

void printAnalogLine(uint8_t row, const char *label1, int value1,
                     const char *label2, int value2) {
  char line[LCD_COLS + 1];
  if (label2 != nullptr) {
    snprintf(line, sizeof(line), "%s:%4d(%s) %s:%4d(%s)", label1, value1, identify(value1).c_str(), label2,
             value2, identify(value2).c_str());
  } else {
    snprintf(line, sizeof(line), "%s:%4d(%s)  %s", label1, value1, identify(value1).c_str(), targetWord.c_str());
  }
  clearLcdLine(row);
  lcd.setCursor(0, row);
  lcd.print(line);
}

void setupLcd() {
  if (!lcdEnabled) {
    return;
  }
  lcd.init(); // or lcd.begin(LCD_COLS, LCD_ROWS) on some library versions
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Phyrdle LCD");
  lcd.setCursor(0, 1);
  lcd.print("J204A 20x4");
}

void updateLcdAnalogReadings(unsigned long currentTime) {
  if (!lcdEnabled) {
    return;
  }
  if (currentTime - lastLcdUpdate < LOG_INTERVAL) {
    return;
  }
  lastLcdUpdate = currentTime;

  int a0 = analogRead(A0);
  int a1 = analogRead(A1);
  int a2 = analogRead(A2);
  int a3 = analogRead(A3);
  int a6 = analogRead(A6);

  printAnalogLine(0, "0", a0, "1", a1);
  printAnalogLine(1, "2", a2, "3", a3);
  printAnalogLine(2, "6", a6, nullptr, 0);
}

// Function to test randomization by printing multiple random words
void testRandomization() {
  Serial.println("Testing randomization - printing 10 random words:");

  for (int i = 0; i < 10; i++) {
    // Get a random word
    String word = dictionary.randomWord();

    // Print the word
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(word);

    // Small delay
    delay(50);
  }

  Serial.println("Randomization test complete");
}

void setup() {
  analogReference(INTERNAL);

  // Initialize serial communication
  Serial.begin(9600);

  // Wait a moment for the serial connection to establish
  delay(1000);

  // Initialize timestamp for logging
  lastLogTimestamp = millis();

  // Print a test message
  Serial.println("Phyrdle starting up...");
  Serial.println("Serial monitor test - if you can see this, serial "
                 "communication is working!");

  // Test randomization
  testRandomization();

  // Initialize the game with a random word
  resetGame();

  // Initialize the LED lighting
  setupLighting();

  setupLcd();
}

void loop() {
  // Current time
  unsigned long currentTime = millis();

  // Check if we're in a win state and if the timeout has elapsed
  if (gameWon && (currentTime - winTimestamp >= WIN_TIMEOUT)) {
    // Reset the game after win timeout
    resetGame();
    Serial.println("Game reset with new word after win!");
  }

  // If game is won, update with rainbow animation and skip the rest of the loop
  if (gameWon) {
    // Use the rainbow animation instead of the regular LED update
    updateRainbowAnimation();
    delay(WIN_ANIMATION_INTERVAL);
    return;
  }

  // Read the state of each slot and identify letters
  for (int i = 0; i < SLOT_COUNT; i++) {
    slots[i].readLetter();
  }

  // Check if all slots have letters (potential word to check)
  bool allSlotsHaveLetters = true;
  for (int i = 0; i < SLOT_COUNT; i++) {
    if (slots[i].state != FULL) {
      allSlotsHaveLetters = false;
      break;
    }
  }

  // If all slots have letters, check the word
  if (allSlotsHaveLetters) {
    check_word();
  } else {
    lastReportedWord = "";
    // Otherwise, just update LEDs based on current slot states
    board.light(slots, SLOT_COUNT);
  }

  // Log only once per second
  if (ENABLE_RUNTIME_DIAGNOSTICS &&
      currentTime - lastLogTimestamp >= LOG_INTERVAL) {
    // Print target word
    Serial.print("Target word: ");
    Serial.println(targetWord);

    // Use the logging function to print slot states and identified letters
    printSlotStates(slots, SLOT_COUNT);

    // Update the last log timestamp
    lastLogTimestamp = currentTime;
  }

  // Conditionally update the LCD with analog readings
  if (!gameWon) {
    updateLcdAnalogReadings(currentTime);
  }

  // Add a delay to avoid flooding the serial monitor
  delay(CHECK_INTERVAL);
}

void check_word() {
  // Create a Word object from the slots
  Word currentWord(slots, SLOT_COUNT);

  // Create a Result object to evaluate the word against the target
  Result result(&currentWord, targetWord, slots);

  // Light up the board based on the evaluation
  board.light(slots, SLOT_COUNT);

  String currentWordText = currentWord.getWord();

  // Only log when the completed word changes. Reprinting it on every fast
  // input scan can fill the serial buffer and delay subsequent ADC reads.
  if (currentWord.status != Word::INCOMPLETE &&
      currentWordText != lastReportedWord) {
    Serial.print("Word: ");
    Serial.print(currentWordText);
    Serial.print(" - Status: ");

    switch (currentWord.status) {
    case Word::INCOMPLETE:
      Serial.println("INCOMPLETE");
      break;
    case Word::INVALID:
      Serial.println("INVALID");
      break;
    case Word::VALID:
      Serial.println("VALID");
      break;
    }

    lastReportedWord = currentWordText;
  }

  // Check if the game has been won
  bool allWin = true;
  for (int i = 0; i < SLOT_COUNT; i++) {
    if (slots[i].state != WIN) {
      allWin = false;
      break;
    }
  }

  // If all slots are in WIN state, set the game to won and record the timestamp
  if (allWin) {
    gameWon = true;
    winTimestamp = millis();

    // Always log win messages
    Serial.println("Game won! Resetting in 3 seconds...");
  }
}

// Function to reset the game with a new word
void resetGame() {
  // Reset game state
  gameWon = false;

  // Reset logging timestamp
  lastLogTimestamp = millis();

  // Reset LED brightness back to normal
  FastLED.setBrightness(50);

  // Improve randomization by using multiple analog reads
  // This creates a more unpredictable seed value
  long randomSeed1 = analogRead(A5); // Use an unconnected analog pin
  delay(5);                          // Small delay to get different readings
  long randomSeed2 = analogRead(A5);
  delay(5);
  long randomSeed3 = analogRead(A5);

  // Combine the readings to create a more random seed
  randomSeed((randomSeed1 * randomSeed2) ^ randomSeed3 ^ millis());

  // Pick a new target word
  targetWord = dictionary.randomWord();

  // Always log the new target word
  Serial.print("New target word: ");
  Serial.println(targetWord);

  // Reset all slots to EMPTY state
  for (int i = 0; i < SLOT_COUNT; i++) {
    slots[i].state = EMPTY;
    slots[i].letter = "";
  }
}
