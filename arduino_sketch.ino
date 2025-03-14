#include <FastLED.h>
#include "letter_identify.h"
#include "slot.h"
#include "logging.h"
#include "lighting.h"  // Include the lighting header
#include "dictionary.h" // Include our new dictionary header

#define CHECK_INTERVAL 100 // milliseconds
#define SLOT_COUNT 5

// Forward declaration of global variables
extern Dictionary dictionary;
extern Slot slots[];

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
  
  // Print a test message
  Serial.println("Phyrdle starting up...");
  Serial.println("Serial monitor test - if you can see this, serial communication is working!");
  
  // Print the target word
  targetWord = dictionary.randomWord();
  Serial.print("Target word: ");
  Serial.println(targetWord);
  
  // Initialize the LED lighting
  setupLighting();
}

void loop() {
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
  
  // Use the logging function to print slot states and identified letters
  printSlotStates(slots, SLOT_COUNT);
  
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
  
  // Print the result to the serial monitor
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
