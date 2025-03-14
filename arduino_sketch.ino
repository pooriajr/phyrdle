#include <FastLED.h>
#include "letter_identify.h"
#include "slot.h"
#include "logging.h"
#include "lighting.h"  // Include the lighting header
#include "dictionary.h" // Include our new dictionary header

#define CHECK_INTERVAL 100 // milliseconds
#define SLOT_COUNT 5
#define WIN_TIMEOUT 5000 // 3 seconds in milliseconds
#define LOG_INTERVAL 1000 // 1 second in milliseconds

// Forward declaration of global variables
extern Dictionary dictionary;
extern Slot slots[];

// Variables to track win state
bool gameWon = false;
unsigned long winTimestamp = 0;
unsigned long lastLogTimestamp = 0; // For controlling log frequency

class Word {
  private:
    Slot* slots;
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
    Word(Slot* s, int count) {
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
    String getWord() {
      return word;
    }
};

class Result {
  private:
    Word* wordPtr;
    String targetWord;
    Slot* slots;
    
  public:
    // Constructor
    Result(Word* w, String target, Slot* s) {
      wordPtr = w;
      targetWord = target;
      slots = s;
      evaluateWord();
    }
    
    // Evaluate the word and update slot states
    void evaluateWord() {
      // Handle based on word status
      switch(wordPtr->status) {
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
        if (slots[i].state == CORRECT) continue;
        
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
Slot slot1(A0);  // Using analog pin A0
Slot slot2(A1);  // Using analog pin A1
Slot slot3(A2);  // Using analog pin A2
Slot slot4(A3);  // Using analog pin A3
Slot slot5(A4);  // Using analog pin A4
Slot slots[SLOT_COUNT] = {slot1, slot2, slot3, slot4, slot5};

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Wait a moment for the serial connection to establish
  delay(1000);
  
  // Initialize timestamp for logging
  lastLogTimestamp = millis();
  
  // Print a test message
  Serial.println("Phyrdle starting up...");
  Serial.println("Serial monitor test - if you can see this, serial communication is working!");
  
  // Initialize the game with a random word
  resetGame();
  
  // Initialize the LED lighting
  setupLighting();
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
  
  // If game is won, just update LEDs and skip the rest of the loop
  if (gameWon) {
    board.light(slots, SLOT_COUNT);
    delay(CHECK_INTERVAL);
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
    // Otherwise, just update LEDs based on current slot states
    board.light(slots, SLOT_COUNT);
  }
  
  // Log only once per second
  if (currentTime - lastLogTimestamp >= LOG_INTERVAL) {
    // Print target word
    Serial.print("Target word: ");
    Serial.println(targetWord);
    
    // Use the logging function to print slot states and identified letters
    printSlotStates(slots, SLOT_COUNT);
    
    // Update the last log timestamp
    lastLogTimestamp = currentTime;
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
  
  // Only log for valid words or invalid words (not incomplete)
  if (currentWord.status != Word::INCOMPLETE) {
    Serial.print("Word: ");
    Serial.print(currentWord.getWord());
    Serial.print(" - Status: ");
    
    switch(currentWord.status) {
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
