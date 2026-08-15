#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include <Arduino.h>

// Hardware profile IDs. Change PHYRDLE_HARDWARE_PROFILE to select the
// resistor configuration that will be compiled and uploaded from the IDE.
#define PHYRDLE_PROFILE_LEGACY_1K 1
#define PHYRDLE_PROFILE_WIDE_4K7 2

// ---------------------------------------------------------------------------
// Active hardware profile -- change this one line in the Arduino IDE.
// ---------------------------------------------------------------------------
#ifndef PHYRDLE_HARDWARE_PROFILE
#define PHYRDLE_HARDWARE_PROFILE PHYRDLE_PROFILE_WIDE_4K7
#endif

struct LetterAdcRange {
  uint16_t minimum;
  uint16_t maximum;
  char letter;
};

#if PHYRDLE_HARDWARE_PROFILE == PHYRDLE_PROFILE_LEGACY_1K
#include "profiles/legacy_1k.h"
#elif PHYRDLE_HARDWARE_PROFILE == PHYRDLE_PROFILE_WIDE_4K7
#include "profiles/wide_4k7.h"
#else
#error "Unknown PHYRDLE_HARDWARE_PROFILE selection"
#endif

#endif // HARDWARE_PROFILE_H
