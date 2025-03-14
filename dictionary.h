#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <Arduino.h>
#include <avr/pgmspace.h>
#include "word_list.h" // Include the word list header

// Define the word length constant
const int WORD_LENGTH = 5;

class Dictionary {
  private:
    char buffer[WORD_LENGTH + 1]; // Buffer to hold a word from PROGMEM
    
    // Helper function to load a word from PROGMEM into RAM
    void loadWord(int index) {
      // Calculate the starting position of the word in the packed array
      int position = index * WORD_LENGTH;
      
      // Copy the word from PROGMEM to RAM buffer
      for (int i = 0; i < WORD_LENGTH; i++) {
        buffer[i] = pgm_read_byte(&WORD_PACK[position + i]);
      }
      buffer[WORD_LENGTH] = '\0'; // Add null terminator
    }
    
    // Binary search for a word in the packed dictionary
    // Returns true if found, false otherwise
    bool binarySearch(const String &word) {
      int low = 0;
      int high = WORD_COUNT - 1;
      
      while (low <= high) {
        int mid = low + (high - low) / 2;
        
        // Load the middle word
        loadWord(mid);
        
        // Compare with the target word
        int comparison = word.compareTo(buffer);
        
        if (comparison == 0) {
          return true; // Word found
        } else if (comparison > 0) {
          low = mid + 1; // Search in the right half
        } else {
          high = mid - 1; // Search in the left half
        }
      }
      
      return false; // Word not found
    }
    
  public:
    // Constructor
    Dictionary() {
      // No initialization needed here - randomization is handled in resetGame()
    }
    
    // Get a random word from the list
    String randomWord() {
      int index = random(WORD_COUNT);
      loadWord(index);
      return String(buffer);
    }
    
    // Check if a word is valid (exists in our dictionary)
    bool isValid(String word) {
      // Convert input to uppercase for comparison
      word.toUpperCase();
      
      // Use binary search for efficient lookup
      return binarySearch(word);
    }
    
    // Get word at specific index (useful for testing)
    String getWordAt(int index) {
      if (index >= 0 && index < WORD_COUNT) {
        loadWord(index);
        return String(buffer);
      }
      return "";
    }
};

#endif // DICTIONARY_H 