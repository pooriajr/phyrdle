#include "../light_feedback.h"
#include <cassert>
#include <cstdio>

int main() {
  LightFeedback feedback;
  char letters[5] = {};
  feedback.update(letters, false, 0);
  assert(feedback.neutralBrightness(0, 0) == LightFeedback::NEUTRAL_WHITE);
  assert(!feedback.resultComplete(1000));

  letters[0] = 'A';
  feedback.update(letters, false, 10);
  assert(feedback.neutralBrightness(0, 110) == LightFeedback::PULSE_WHITE);
  assert(feedback.neutralBrightness(1, 110) == LightFeedback::NEUTRAL_WHITE);
  assert(!feedback.isRevealed(0, 110));
  feedback.update(letters, false, 210);
  assert(feedback.neutralBrightness(0, 210) == LightFeedback::NEUTRAL_WHITE);
  feedback.update(letters, false, 500);
  assert(feedback.neutralBrightness(0, 600) == LightFeedback::NEUTRAL_WHITE);

  // Non-winning valid guesses share the faster recognition/reveal schedule.
  const char complete[5] = {'A', 'B', 'C', 'D', 'E'};
  feedback.update(complete, true, 1000);
  assert(feedback.neutralBrightness(4, 1100) == LightFeedback::PULSE_WHITE);
  assert(!feedback.isRevealed(0, 1199));
  for (uint8_t i = 0; i < 5; ++i) {
    assert(!feedback.isRevealed(i, 1199 + i * 67));
    assert(feedback.isRevealed(i, 1200 + i * 67));
  }
  assert(!feedback.resultComplete(1534));
  feedback.update(complete, true, 1535);
  assert(feedback.resultComplete(1535));
  feedback.update(complete, true, 5000);
  assert(feedback.resultComplete(5000)); // No replay for unchanged guesses.

  // A direct replacement restarts the reveal and pulses only that slot.
  char replacement[5] = {'A', 'B', 'C', 'D', 'F'};
  feedback.update(replacement, true, 5100);
  assert(!feedback.isRevealed(0, 5100));
  assert(feedback.neutralBrightness(4, 5200) == LightFeedback::PULSE_WHITE);
  assert(feedback.neutralBrightness(0, 5200) == LightFeedback::NEUTRAL_WHITE);

  // Removing a tile interrupts an active reveal immediately.
  replacement[2] = 0;
  feedback.update(replacement, false, 5350);
  for (uint8_t i = 0; i < 5; ++i) assert(!feedback.isRevealed(i, 5350));
  assert(!feedback.resultComplete(9000));
  assert(feedback.neutralBrightness(2, 5350) == LightFeedback::NEUTRAL_WHITE);
  feedback.update(complete, true, 5400);
  assert(!feedback.isRevealed(0, 5599));
  assert(feedback.isRevealed(0, 5600));

  // Invalid words never reveal results; unknown readings never pulse.
  replacement[2] = '?';
  feedback.update(replacement, false, 6000);
  assert(feedback.neutralBrightness(2, 6100) == LightFeedback::NEUTRAL_WHITE);
  assert(!feedback.isRevealed(0, 9000));
  assert(!feedback.resultComplete(9000));

  feedback.reset();
  assert(!feedback.resultComplete(9000));
  assert(feedback.neutralBrightness(4, 9000) == LightFeedback::NEUTRAL_WHITE);

  // Arduino millis() wraps at 32 bits: both animations must still finish.
  const uint32_t start = UINT32_MAX - 50;
  feedback.update(complete, true, start);
  assert(feedback.neutralBrightness(0, start + 100u) == LightFeedback::PULSE_WHITE);
  assert(!feedback.isRevealed(0, start + 199u));
  assert(feedback.isRevealed(0, start + 200u));
  assert(!feedback.resultComplete(start + 534u));
  assert(feedback.resultComplete(start + 535u));
  feedback.update(complete, true, start + 535u);
  assert(feedback.neutralBrightness(0, start + 535u) == LightFeedback::NEUTRAL_WHITE);
  assert(LightFeedback::ABSENT_WHITE > LightFeedback::NEUTRAL_WHITE);
  // Winning guesses enter the animated rainbow immediately, from left to right.
  assert(LightFeedback::REVEAL_STEP_MS == 67);
  assert(LightFeedback::rainbowSlotVisible(0, 0));
  for (uint8_t i = 1; i < 5; ++i) {
    assert(!LightFeedback::rainbowSlotVisible(i, i * 67 - 1));
    assert(LightFeedback::rainbowSlotVisible(i, i * 67));
  }
  assert(LightFeedback::rainbowSlotVisible(4, uint32_t((start + 268u) - start)));
  // Two soft red pulses over 1 s, then a 2 s steady hold, repeated.
  feedback.reset();
  feedback.update(complete, false, 10000, true);
  assert(feedback.deniedBrightness(10000) == 255);
  assert(feedback.deniedBrightness(10250) == 48);
  assert(feedback.deniedBrightness(10500) == 255);
  assert(feedback.deniedBrightness(10750) == 48);
  assert(feedback.deniedBrightness(11000) == 255);
  uint8_t previous = feedback.deniedBrightness(10000);
  unsigned troughs = 0;
  // Multiple cycles and unchanged input scans must not restart the animation.
  for (uint32_t elapsed = 5; elapsed <= 6500; elapsed += 5) {
    feedback.update(complete, false, 10000 + elapsed, true);
    const uint8_t brightness = feedback.deniedBrightness(10000 + elapsed);
    const int change = int(brightness) - previous;
    assert(change >= -5 && change <= 5); // No abrupt on/off or phase jumps.
    assert(brightness >= 48);
    if (brightness == 48) ++troughs;
    if (elapsed % 3000 >= 1000) assert(brightness == 255);
    assert(!feedback.isRevealed(0, 10000 + elapsed));
    previous = brightness;
  }
  assert(troughs == 5); // Two per full cycle, one in the last half-second.
  assert(feedback.deniedBrightness(12999) == 255);
  assert(feedback.deniedBrightness(13000) == 255);
  assert(feedback.deniedBrightness(13250) == 48);
  // A changed invalid guess restarts the pulse phase, even during a hold.
  replacement[2] = 'X';
  feedback.update(replacement, false, 19000, true);
  assert(feedback.deniedBrightness(19000) == 255);
  assert(feedback.deniedBrightness(19250) == 48);
  // Removing a tile cancels the hold immediately.
  feedback.update(replacement, false, 20000, true);
  assert(feedback.deniedBrightness(20000) == 255);
  replacement[2] = 0;
  feedback.update(replacement, false, 20005);
  assert(feedback.deniedBrightness(20005) == 0);
  // A valid word cancels the pulse phase and begins the normal reveal.
  feedback.update(complete, false, 20100, true);
  feedback.update(complete, true, 20230);
  assert(feedback.deniedBrightness(20230) == 0);
  assert(feedback.isRevealed(0, 20430));
  // Pulse/hold transitions and repetition survive millis() rollover.
  feedback.update(complete, false, start, true);
  assert(feedback.deniedBrightness(start + 250u) == 48);
  assert(feedback.deniedBrightness(start + 500u) == 255);
  assert(feedback.deniedBrightness(start + 750u) == 48);
  assert(feedback.deniedBrightness(start + 1000u) == 255);
  assert(feedback.deniedBrightness(start + 2999u) == 255);
  assert(feedback.deniedBrightness(start + 3000u) == 255);
  assert(feedback.deniedBrightness(start + 3250u) == 48);
  feedback.reset();
  assert(feedback.deniedBrightness(0) == 0);
  std::puts("Light feedback timing tests passed.");
}
