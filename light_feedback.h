#ifndef LIGHT_FEEDBACK_H
#define LIGHT_FEEDBACK_H

#include <stdint.h>

// Animation timing is independent of scoring and never blocks input scans.
class LightFeedback {
public:
  static const uint8_t SLOT_COUNT = 5;
  static const uint8_t NEUTRAL_WHITE = 40;
  static const uint8_t PULSE_WHITE = 160;
  static const uint8_t ABSENT_WHITE = 100;
  static const uint16_t RECOGNITION_MS = 200;
  static const uint16_t REVEAL_STEP_MS = 67; // About 1.5x the previous reveal speed.

  static const uint16_t DENIED_PULSE_MS = 500;
  static const uint8_t DENIED_DIM_RED = 48;
  static const uint16_t DENIED_PULSES_MS = 1000;
  static const uint16_t DENIED_HOLD_MS = 2000;

  void reset() {
    denied_ = false;
    deniedStarted_ = 0;
    revealing_ = false;
    revealStarted_ = 0;
    for (uint8_t i = 0; i < SLOT_COUNT; ++i) {
      letters_[i] = 0;
      pulsing_[i] = false;
      pulseStarted_[i] = 0;
    }
  }

  LightFeedback() { reset(); }

  void update(const char letters[SLOT_COUNT], bool valid, uint32_t now,
              bool denied = false) {
    bool changed = false;
    for (uint8_t i = 0; i < SLOT_COUNT; ++i) {
      if (letters[i] != letters_[i]) {
        changed = true;
        letters_[i] = letters[i];
        // An unrecognized ADC reading must not acknowledge a letter.
        pulsing_[i] = letters[i] >= 'A' && letters[i] <= 'Z';
        pulseStarted_[i] = now;
      }
      if (pulsing_[i] && uint32_t(now - pulseStarted_[i]) >= RECOGNITION_MS) {
        pulsing_[i] = false;
      }
    }
    if (!denied) {
      denied_ = false;
    } else if (changed || !denied_) {
      denied_ = true;
      deniedStarted_ = now;
    }
    if (changed || !valid) {
      revealing_ = false;
    }
    if (valid && !revealing_) {
      revealing_ = true;
      revealStarted_ = now;
    }
  }

  uint8_t neutralBrightness(uint8_t slot, uint32_t now) const {
    const uint32_t elapsed = uint32_t(now - pulseStarted_[slot]);
    if (!pulsing_[slot] || elapsed >= RECOGNITION_MS) {
      return NEUTRAL_WHITE;
    }
    const uint16_t half = RECOGNITION_MS / 2;
    const uint16_t ramp = elapsed < half ? elapsed : RECOGNITION_MS - elapsed;
    return NEUTRAL_WHITE + uint32_t(PULSE_WHITE - NEUTRAL_WHITE) * ramp / half;
  }

  bool isRevealed(uint8_t slot, uint32_t now) const {
    return revealing_ && uint32_t(now - revealStarted_) >=
        uint32_t(RECOGNITION_MS) + uint32_t(slot) * REVEAL_STEP_MS;
  }

  // The winning rainbow starts at slot zero immediately, with no recognition
  // delay or green result stage. Elapsed time is measured from the win.
  static bool rainbowSlotVisible(uint8_t slot, uint32_t elapsed) {
    return elapsed >= uint32_t(slot) * REVEAL_STEP_MS;
  }

  // Two soft red pulses over one second, then two seconds of steady red.
  // Stay faintly lit at each trough and join the hold/repeat without a jump.
  uint8_t deniedBrightness(uint32_t now) const {
    if (!denied_) return 0;
    const uint16_t phase = uint32_t(now - deniedStarted_) %
                          (DENIED_PULSES_MS + DENIED_HOLD_MS);
    if (phase >= DENIED_PULSES_MS) return 255;
    const uint16_t withinPulse = phase % DENIED_PULSE_MS;
    const uint16_t half = DENIED_PULSE_MS / 2;
    const uint16_t fade = withinPulse < half ? withinPulse : DENIED_PULSE_MS - withinPulse;
    return 255 - uint16_t(255 - DENIED_DIM_RED) * fade / half;
  }

  bool resultComplete(uint32_t now) const {
    return revealing_ && uint32_t(now - revealStarted_) >=
        uint32_t(RECOGNITION_MS) + uint32_t(SLOT_COUNT) * REVEAL_STEP_MS;
  }

private:
  bool denied_;
  uint32_t deniedStarted_;
  char letters_[SLOT_COUNT];
  bool pulsing_[SLOT_COUNT];
  uint32_t pulseStarted_[SLOT_COUNT];
  bool revealing_;
  uint32_t revealStarted_;
};

#endif // LIGHT_FEEDBACK_H
