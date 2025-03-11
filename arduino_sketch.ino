#include <FastLED.h>
#include "letter_identify.h"
#include "slot.h"
#include "logging.h"
#include "lighting.h"  // Include the lighting header

#define CHECK_INTERVAL 100 // milliseconds
#define SLOT_COUNT 5

class Word {
  // this is a class that takes in an array of slots and concatenates the letters into a word.
  // if any of the slots is missing, the word is incomplete
  // if the word doesn't exist in the dictionary, it is invalid
  // if the word does exist in the dictionary, it is valid
};

class Dictionary {
  // this is a singleton class knows all the valid words
  // it can be asked for a random word
  // it can be fed a word to see if that word is valid
  public:
    String randomWord() {
      return "HELLO"; // Placeholder
    }
    
    bool isValid(String word) {
      return true; // Placeholder
    }
};

class Result {
  // takes in a word, evaluates for completion, validity, and correctness
  // updates slots accordingly

  // switch(word.status) {
  //   case incomplete:
  //     // all slots without letters "EMPTY" and the ones with letters "FULL"
  //     break;
  //   case invalid:
  //     // all slots INVALID
  //     break;
  //   case valid:
  //     // evaluate each slot for correctness
  // }
};

class Board {
  // handles lighting the slots
};

// Global variables
Dictionary dictionary; // initialize a global dictionary
Board board;
String targetWord;
Slot slot1(A0);  // Using analog pin A0
Slot slot2(A1);  // Using analog pin A1
Slot slot3(A2);  // Using analog pin A2
Slot slots[3] = {slot1, slot2, slot3};

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
  for (int i = 0; i < 3; i++) {
    slots[i].readLetter();
  }
  
  // Update LEDs based on slot states
  updateSlotLEDs(slots, 3);
  
  // Use the logging function to print slot states and identified letters
  printSlotStates(slots, 3);
  
  // Add a delay to avoid flooding the serial monitor
  delay(CHECK_INTERVAL);
}

// void check_word(){
//   word = Word()
//   result = Result(word)
//   board.light()
// }
