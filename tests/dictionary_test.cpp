// Exhaustive verification of the encoded word set, compiled for the host.
#include "../dictionary.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <algorithm>
#include <chrono>

static std::vector<std::string> load(const char *path) {
  std::vector<std::string> v;
  std::ifstream f(path);
  if (!f) { std::fprintf(stderr, "cannot open %s\n", path); std::exit(1); }
  std::string s;
  while (f >> s) if (s.size() == 5) v.push_back(s);
  return v;
}

static int failures = 0;
static void check(bool ok, const char *what) {
  if (!ok) { std::printf("  FAIL: %s\n", what); ++failures; }
}

int main() {
  auto guesses = load("words/guesses.txt");
  auto answers = load("words/answers.txt");
  guesses.insert(guesses.end(), answers.begin(), answers.end());
  std::sort(guesses.begin(), guesses.end());
  guesses.erase(std::unique(guesses.begin(), guesses.end()), guesses.end());
  std::set<std::string> gset(guesses.begin(), guesses.end());
  std::set<std::string> aset(answers.begin(), answers.end());

  std::printf("loaded %zu guesses, %zu answers\n", guesses.size(), answers.size());
  std::printf("flash footprint: %d bytes\n\n", WORDSET_FLASH_BYTES);

  check(WordSet::guessCount() == guesses.size(), "guessCount");
  check(WordSet::answerCount() == answers.size(), "answerCount");

  // Target filtering must never narrow the accepted-guess dictionary.
  for (const char *plural : {"years", "books", "cries", "jeans", "pants"}) {
    check(WordSet::isValidGuess(plural), "plural was removed from guesses");
    check(!WordSet::isAnswer(plural), "regular plural is still a target");
  }
  for (const char *irregular : {"fungi", "women", "media", "wives", "lives"}) {
    check(WordSet::isValidGuess(irregular), "irregular plural was removed from guesses");
    check(WordSet::isAnswer(irregular), "irregular plural was incorrectly excluded from targets");
  }
  for (const char *singular : {"virus", "bonus", "focus"})
    check(WordSet::isAnswer(singular), "singular ending in S was incorrectly filtered");

  for (const char *word : {"begum", "lough", "laird", "synod", "sahib", "septa",
                            "taxon", "sager", "rives", "copra", "ketch", "quint", "cloys", "trued"}) {
    check(WordSet::isValidGuess(word), "curation narrowed the broad guess dictionary");
    check(!WordSet::isAnswer(word), "explicitly rejected obscure word became a target");
  }
  for (const char *word : {"spork", "swole", "gouda", "mochi", "towel", "unbox"}) {
    check(WordSet::isValidGuess(word), "new curated word is not a legal guess");
    check(WordSet::isAnswer(word), "curated approval is missing from encoded targets");
  }

  // Exercise the actual game adapter as well as the compressed decoder.
  Dictionary dictionary;
  for (const auto &word : guesses) {
    check(dictionary.isValid(String(word.c_str())), "adapter rejected lowercase guess");
    std::string upper = word;
    for (char &letter : upper) letter += 'A' - 'a';
    check(dictionary.isValid(String(upper.c_str())), "adapter rejected uppercase guess");
  }
  check(dictionary.isValid("ApPlE"), "mixed case rejected");
  for (const char *bad : {"", "abcd", "abcdef", "ab1ds", "ab ds", "zzzzs", "zzzzz"})
    check(!dictionary.isValid(bad), "adapter accepted malformed or nonexistent word");
  for (size_t i = 0; i < answers.size(); ++i) {
    const String target = dictionary.getWordAt(i);
    std::string lower = target.c_str();
    check(lower.size() == 5, "wrong target length");
    for (char &letter : lower) {
      check(letter >= 'A' && letter <= 'Z', "target must remain uppercase");
      letter += 'a' - 'A';
    }
    check(lower == answers[i], "adapter changed curated target selection");
    check(dictionary.isValid(target), "curated target is not an accepted guess");
    check(std::set<char>(lower.begin(), lower.end()).size() == 5, "target repeats a tile");
  }
  check(dictionary.getWordAt(-1).length() == 0, "negative answer index accepted");
  check(dictionary.getWordAt(answers.size()).length() == 0, "past-end answer index accepted");
  for (int i = 0; i < 10000; ++i) {
    const String target = dictionary.randomWord();
    std::string lower = target.c_str();
    for (char &letter : lower) letter += 'a' - 'A';
    check(aset.count(lower) != 0, "random target outside curated pool");
  }
  std::printf("0. game adapter accepts both cases, rejects invalid plurals, preserves every target\n");

  // 1. every guess word is accepted
  for (const auto &w : guesses)
    if (!WordSet::isValidGuess(w.c_str())) {
      std::printf("  FAIL: rejected valid guess '%s'\n", w.c_str()); ++failures; break;
    }
  std::printf("1. all %zu guess words accepted\n", guesses.size());

  // 2. guessAt enumerates the list in alphabetical order
  char buf[6];
  for (size_t i = 0; i < guesses.size(); ++i) {
    if (!WordSet::guessAt((uint16_t)i, buf) || guesses[i] != buf) {
      std::printf("  FAIL: guessAt(%zu) gave '%s', want '%s'\n", i, buf, guesses[i].c_str());
      ++failures; break;
    }
  }
  check(!WordSet::guessAt((uint16_t)guesses.size(), buf), "guessAt out of range");
  std::printf("2. guessAt round-trips all %zu words in order\n", guesses.size());

  // 3. answerAt enumerates exactly the answer set, in order
  std::vector<std::string> sorted_answers(aset.begin(), aset.end());
  for (size_t i = 0; i < sorted_answers.size(); ++i) {
    if (!WordSet::answerAt((uint16_t)i, buf) || sorted_answers[i] != buf) {
      std::printf("  FAIL: answerAt(%zu) gave '%s', want '%s'\n", i, buf,
                  sorted_answers[i].c_str());
      ++failures; break;
    }
  }
  check(!WordSet::answerAt((uint16_t)sorted_answers.size(), buf), "answerAt out of range");
  std::printf("3. answerAt round-trips all %zu answers in order\n", sorted_answers.size());

  // 4. isAnswer agrees with the answer set, tested over every guess word
  size_t mismatches = 0;
  for (const auto &w : guesses)
    if (WordSet::isAnswer(w.c_str()) != (aset.count(w) > 0)) ++mismatches;
  check(mismatches == 0, "isAnswer disagrees with the answer list");
  std::printf("4. isAnswer correct on all %zu guess words\n", guesses.size());

  // 5. malformed input is rejected rather than crashing
  const char *bad[] = {"", "abc", "abcdef", "ABCDE", "ab1de", "ab de", 0};
  for (int i = 0; bad[i]; ++i)
    check(!WordSet::isValidGuess(bad[i]), "malformed input accepted");
  check(!WordSet::isValidGuess(nullptr), "null accepted");
  std::printf("5. malformed input rejected\n");

  // 6. pickUnusedAnswer never repeats and eventually covers the whole set
  {
    WordSet::resetUsed();
    check(WordSet::remainingAnswers() == answers.size(), "remainingAnswers after reset");
    std::set<std::string> drawn;
    for (size_t i = 0; i < answers.size(); ++i) {
      if (!WordSet::pickUnusedAnswer(buf)) {
        std::printf("  FAIL: pool exhausted early at draw %zu\n", i); ++failures; break;
      }
      if (!drawn.insert(buf).second) {
        std::printf("  FAIL: '%s' drawn twice at draw %zu\n", buf, i); ++failures; break;
      }
      if (!aset.count(buf)) {
        std::printf("  FAIL: '%s' is not an answer word\n", buf); ++failures; break;
      }
      if (WordSet::remainingAnswers() != answers.size() - i - 1) {
        std::printf("  FAIL: remainingAnswers wrong after draw %zu\n", i); ++failures; break;
      }
    }
    check(drawn.size() == answers.size(), "draws did not cover the answer set");
    check(drawn == aset, "drawn set != answer set");
    check(!WordSet::pickUnusedAnswer(buf), "pool kept dealing past exhaustion");
    check(WordSet::remainingAnswers() == 0, "remainingAnswers not zero when exhausted");
    WordSet::resetUsed();
    check(WordSet::remainingAnswers() == answers.size(), "resetUsed did not refill");
    check(WordSet::pickUnusedAnswer(buf), "no draw available after reset");
    std::printf("6. pickUnusedAnswer dealt all %zu answers with no repeat, "
                "refused a %zu-th, and refilled\n", answers.size(), answers.size() + 1);
  }

  // 7. the draw is not biased toward one end of the pool
  {
    const int kRounds = 200;
    std::vector<int> hits(answers.size(), 0);
    std::vector<std::string> sorted_a(aset.begin(), aset.end());
    for (int r = 0; r < kRounds; ++r) {
      WordSet::resetUsed();
      WordSet::pickUnusedAnswer(buf);      // first draw of a fresh pool
      auto it = std::lower_bound(sorted_a.begin(), sorted_a.end(), std::string(buf));
      hits[it - sorted_a.begin()]++;
    }
    int first_half = 0;
    for (size_t i = 0; i < answers.size() / 2; ++i) first_half += hits[i];
    check(first_half > kRounds / 4 && first_half < 3 * kRounds / 4,
          "first draw looks biased across the pool");
    std::printf("7. first draw landed in the lower half %d/%d times\n", first_half, kRounds);
    WordSet::resetUsed();
  }

  // 8. exhaustive: no false positives anywhere in the 26^5 space
  char w[6]; w[5] = '\0';
  size_t accepted = 0;
  auto t0 = std::chrono::steady_clock::now();
  for (int a = 0; a < 26; ++a) { w[0] = 'a' + a;
    for (int b = 0; b < 26; ++b) { w[1] = 'a' + b;
      for (int c = 0; c < 26; ++c) { w[2] = 'a' + c;
        for (int d = 0; d < 26; ++d) { w[3] = 'a' + d;
          for (int e = 0; e < 26; ++e) { w[4] = 'a' + e;
            if (WordSet::isValidGuess(w)) {
              ++accepted;
              if (!gset.count(w)) {
                std::printf("  FAIL: false positive '%s'\n", w); ++failures;
              }
            }
  }}}}}
  auto t1 = std::chrono::steady_clock::now();
  double secs = std::chrono::duration<double>(t1 - t0).count();
  check(accepted == guesses.size(), "accepted count != guess list size");
  std::printf("8. swept all 11,881,376 strings: %zu accepted, 0 false positives\n", accepted);
  std::printf("   (%.2f s, %.0f ns per lookup on the host)\n\n", secs, secs * 1e9 / 11881376.0);

  std::printf(failures ? "%d CHECK(S) FAILED\n" : "all checks passed\n", failures);
  return failures ? 1 : 0;
}
