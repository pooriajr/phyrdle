#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <Arduino.h>
#include "src/wordset/WordSet.h"

const int WORD_LENGTH = 5;

class Dictionary {
 public:
  // Targets come from the Wikipedia-ranked, distinct-letter pool. RNG is seeded in setup().
  String randomWord() {
    return getWordAt(random(WordSet::answerCount()));
  }

  bool isValid(const String &word) {
    if (word.length() != WORD_LENGTH) return false;
    char lower[WORD_LENGTH + 1];
    for (int i = 0; i < WORD_LENGTH; ++i) {
      char letter = word[i];
      if (letter >= 'A' && letter <= 'Z') letter += 'a' - 'A';
      if (letter < 'a' || letter > 'z') return false;
      lower[i] = letter;
    }
    lower[WORD_LENGTH] = '\0';
    return WordSet::isValidGuess(lower);
  }

  // Preserve the game's uppercase target-word API.
  String getWordAt(int index) {
    char word[WORD_LENGTH + 1];
    if (index < 0 || !WordSet::answerAt(index, word)) return "";
    for (int i = 0; i < WORD_LENGTH; ++i) word[i] += 'A' - 'a';
    return String(word);
  }
};

#endif // DICTIONARY_H
