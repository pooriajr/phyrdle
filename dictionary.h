#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <Arduino.h>

// List of 5-letter words that use the letters E, T, S, A, R
// These words are common and use some combination of these letters
const char* WORD_LIST[] = {
  "STARE", "TEARS", "RATES"
};

const int WORD_COUNT = sizeof(WORD_LIST) / sizeof(WORD_LIST[0]);

class Dictionary {
  public:
    // Constructor
    Dictionary() {
      // Initialize random seed using an unconnected analog pin
      randomSeed(analogRead(A5));
    }
    
    // Get a random word from the list
    String randomWord() {
      int index = random(WORD_COUNT);
      return String(WORD_LIST[index]);
    }
    
    // Check if a word is valid (exists in our dictionary)
    bool isValid(String word) {
      // Convert input to uppercase for comparison
      word.toUpperCase();
      
      // Check if the word exists in our list
      for (int i = 0; i < WORD_COUNT; i++) {
        if (word.equals(WORD_LIST[i])) {
          return true;
        }
      }
      
      return false;
    }
};

#endif // DICTIONARY_H 