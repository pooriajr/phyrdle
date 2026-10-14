// WordSet -- a 5-letter word dictionary for word games on small MCUs.
//
// Two sets live in one structure:
//   * the GUESS set, every 5-letter word accepted as a legal guess;
//   * the ANSWER set, the curated Phyrdle target words to pick from.
//
// Answers cost one bit per guess word on top of the guess set: an answer is
// just a guess word that is flagged. Letters are never stored twice.
//
// Words are lowercase a-z, five letters, NUL-terminated on output, so every
// buffer passed in must hold six chars.
#pragma once
#include <stdint.h>
#include "wordset_data.h"

namespace WordSet {

// Number of words in each set.
inline uint16_t guessCount()  { return WORDSET_GUESS_COUNT; }
inline uint16_t answerCount() { return WORDSET_ANSWER_COUNT; }

// True if `word` is a legal guess. `word` must be 5 lowercase letters;
// anything else returns false. Decodes at most one bucket, ~19 codes
// on average and 210 in the worst case.
bool isValidGuess(const char *word);

// Write answer number `index` (0 .. answerCount()-1) into `out[6]`.
// Returns false and leaves `out` untouched if `index` is out of range.
bool answerAt(uint16_t index, char *out);

// Write a uniformly random answer into `out[6]`. May repeat a previous
// word. Seed the Arduino RNG yourself (e.g. randomSeed(analogRead(A0)))
// before the first call.
void randomAnswer(char *out);

// Write a uniformly random answer that has not been handed out since the
// last reset, and mark it used. Returns false, leaving `out` untouched,
// once every answer has been drawn -- call resetUsed() to start over.
//
// The used-word set lives in RAM and so clears on power-up or reset.
// It costs ceil(WORDSET_ANSWER_COUNT/8) bytes when used (268 here).
// Phyrdle uses randomWord() with replacement, so this tracker is linked out.
bool pickUnusedAnswer(char *out);

// Forget which answers have been drawn.
void resetUsed();

// How many answers pickUnusedAnswer() can still return.
uint16_t remainingAnswers();

// Write guess word number `index` (0 .. guessCount()-1) into `out[6]`,
// in alphabetical order. Returns false if `index` is out of range.
bool guessAt(uint16_t index, char *out);

// True if `word` is in the answer subset. Useful for filtering or stats.
bool isAnswer(const char *word);

}  // namespace WordSet
