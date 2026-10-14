#include "WordSet.h"
#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdlib.h>
#endif

using namespace wordset_data;

namespace {

// Reads the Rice-coded bit stream out of flash, one bit at a time.
// The stream carries a trailing pad byte, so fetching one byte ahead is safe.
struct BitReader {
  const uint8_t *p;
  uint8_t cur;
  uint8_t bit_index;

  void seek(uint16_t byte_offset) {
    p = kStream + byte_offset;
    cur = pgm_read_byte(p);
    bit_index = 0;
  }

  // Named nextBit, not bit: Arduino.h defines bit() as a macro.
  uint8_t nextBit() {
    const uint8_t b = (cur >> (7 - bit_index)) & 1;
    if (++bit_index == 8) {
      bit_index = 0;
      cur = pgm_read_byte(++p);
    }
    return b;
  }

  // Rice code: unary quotient terminated by a 0, then k low bits.
  uint16_t rice(uint8_t k) {
    uint16_t q = 0;
    while (nextBit()) ++q;
    uint16_t r = 0;
    for (uint8_t i = 0; i < k; ++i) r = (uint16_t)(r << 1) | nextBit();
    return (uint16_t)(q << k) | r;
  }
};

bool wellFormed(const char *w) {
  if (w == 0) return false;
  for (uint8_t i = 0; i < 5; ++i)
    if (w[i] < 'a' || w[i] > 'z') return false;
  return w[5] == '\0';
}

uint16_t bucketOf(const char *w) {
  return (uint16_t)(w[0] - 'a') * 26 + (uint16_t)(w[1] - 'a');
}

// The last three letters, base 26. Fits in 16 bits (26^3 = 17,576).
uint16_t suffixOf(const char *w) {
  return ((uint16_t)(w[2] - 'a') * 26 + (uint16_t)(w[3] - 'a')) * 26 +
         (uint16_t)(w[4] - 'a');
}

// Rice parameter for a bucket, stored as a nibble.
uint8_t riceK(uint16_t bucket) {
  const uint8_t packed = pgm_read_byte(&kBucketK[bucket >> 1]);
  return (bucket & 1) ? (packed & 0x0F) : (packed >> 4);
}

void writeWord(char *out, uint16_t bucket, uint16_t suffix) {
  out[0] = 'a' + (char)(bucket / 26);
  out[1] = 'a' + (char)(bucket % 26);
  out[4] = 'a' + (char)(suffix % 26); suffix /= 26;
  out[3] = 'a' + (char)(suffix % 26); suffix /= 26;
  out[2] = 'a' + (char)(suffix % 26);
  out[5] = '\0';
}

// Position of `word` within its bucket, or kNotFound.
uint8_t positionInBucket(const char *word, uint16_t bucket) {
  const uint8_t n = pgm_read_byte(&kBucketCount[bucket]);
  if (n == 0) return 0xFF;

  const uint16_t target = suffixOf(word);
  const uint8_t k = riceK(bucket);
  BitReader br;
  br.seek(pgm_read_word(&kBucketOffset[bucket]));

  uint16_t base = 0;
  for (uint8_t i = 0; i < n; ++i) {
    const uint16_t v = base + br.rice(k);
    if (v == target) return i;
    if (v > target) return 0xFF;   // the bucket is sorted, so we are past it
    base = v + 1;
  }
  return 0xFF;
}

// Index of the first word of `bucket` within the whole guess list.
uint16_t bucketStartIndex(uint16_t bucket) {
  uint16_t start = 0;
  for (uint16_t b = 0; b < bucket; ++b) start += pgm_read_byte(&kBucketCount[b]);
  return start;
}

// Which answers have already been drawn this power cycle. Bits past
// WORDSET_ANSWER_COUNT are pinned to 1 so they are never selected.
const uint16_t kUsedBytes = (WORDSET_ANSWER_COUNT + 7) / 8;
uint8_t used_bits[kUsedBytes];
uint16_t used_total = 0xFFFF;   // 0xFFFF means "not initialised yet"

uint8_t popcount8(uint8_t v) {
  v = (uint8_t)(v - ((v >> 1) & 0x55));
  v = (uint8_t)((v & 0x33) + ((v >> 2) & 0x33));
  return (uint8_t)((v + (v >> 4)) & 0x0F);
}

}  // namespace

namespace WordSet {

bool isValidGuess(const char *word) {
  if (!wellFormed(word)) return false;
  return positionInBucket(word, bucketOf(word)) != 0xFF;
}

bool guessAt(uint16_t index, char *out) {
  if (index >= WORDSET_GUESS_COUNT) return false;

  // Walk the count table to find the bucket holding this index.
  uint16_t bucket = 0, seen = 0;
  for (;; ++bucket) {
    const uint8_t n = pgm_read_byte(&kBucketCount[bucket]);
    if (seen + n > index) break;
    seen += n;
  }

  const uint8_t k = riceK(bucket);
  BitReader br;
  br.seek(pgm_read_word(&kBucketOffset[bucket]));

  uint16_t base = 0, v = 0;
  for (uint16_t i = seen; i <= index; ++i) {
    v = base + br.rice(k);
    base = v + 1;
  }
  writeWord(out, bucket, v);
  return true;
}

bool answerAt(uint16_t index, char *out) {
  if (index >= WORDSET_ANSWER_COUNT) return false;

  // Find the (index+1)-th set bit in the answer bitmap: a byte at a time,
  // then bit by bit within the byte that contains it.
  uint16_t byte_pos = 0, seen = 0;
  for (;; ++byte_pos) {
    const uint8_t pc = popcount8(pgm_read_byte(&kAnswerBits[byte_pos]));
    if (seen + pc > index) break;
    seen += pc;
  }
  const uint8_t bits = pgm_read_byte(&kAnswerBits[byte_pos]);
  uint16_t guess_index = (uint16_t)byte_pos * 8;
  for (uint8_t b = 0; b < 8; ++b) {
    if (bits & (0x80 >> b)) {
      if (seen == index) { guess_index += b; break; }
      ++seen;
    }
  }
  return guessAt(guess_index, out);
}

bool isAnswer(const char *word) {
  if (!wellFormed(word)) return false;
  const uint16_t bucket = bucketOf(word);
  const uint8_t pos = positionInBucket(word, bucket);
  if (pos == 0xFF) return false;
  const uint16_t index = bucketStartIndex(bucket) + pos;
  return (pgm_read_byte(&kAnswerBits[index >> 3]) & (0x80 >> (index & 7))) != 0;
}

void resetUsed() {
  for (uint16_t i = 0; i < kUsedBytes; ++i) used_bits[i] = 0;
  // Pin the padding bits of the final byte.
  const uint8_t spare = (uint8_t)(kUsedBytes * 8 - WORDSET_ANSWER_COUNT);
  for (uint8_t b = 0; b < spare; ++b)
    used_bits[kUsedBytes - 1] |= (uint8_t)(1 << b);
  used_total = 0;
}

uint16_t remainingAnswers() {
  if (used_total == 0xFFFF) resetUsed();
  return (uint16_t)(WORDSET_ANSWER_COUNT - used_total);
}

bool pickUnusedAnswer(char *out) {
  const uint16_t left = remainingAnswers();
  if (left == 0) return false;

#ifdef ARDUINO
  uint16_t skip = (uint16_t)random(left);
#else
  uint16_t skip = (uint16_t)(rand() % left);
#endif

  // Walk to the skip-th clear bit, a byte at a time then within the byte.
  uint16_t byte_pos = 0;
  for (;; ++byte_pos) {
    const uint8_t free_here = (uint8_t)(8 - popcount8(used_bits[byte_pos]));
    if (free_here > skip) break;
    skip = (uint16_t)(skip - free_here);
  }
  uint8_t bit = 0;
  for (uint8_t b = 0; b < 8; ++b) {
    if (used_bits[byte_pos] & (0x80 >> b)) continue;
    if (skip == 0) { bit = b; break; }
    --skip;
  }

  used_bits[byte_pos] |= (uint8_t)(0x80 >> bit);
  ++used_total;
  return answerAt((uint16_t)(byte_pos * 8 + bit), out);
}

void randomAnswer(char *out) {
#ifdef ARDUINO
  answerAt((uint16_t)random(WORDSET_ANSWER_COUNT), out);
#else
  answerAt((uint16_t)(rand() % WORDSET_ANSWER_COUNT), out);
#endif
}

}  // namespace WordSet
