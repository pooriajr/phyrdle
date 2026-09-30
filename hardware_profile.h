#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#include <Arduino.h>

// Hardware profile IDs. Change PHYRDLE_HARDWARE_PROFILE to select the
// resistor configuration that will be compiled and uploaded from the IDE.
#define PHYRDLE_PROFILE_LEGACY_1K 1
#define PHYRDLE_PROFILE_WIDE_4K7 2
#define PHYRDLE_PROFILE_YAGEO_10K 3

// ---------------------------------------------------------------------------
// Active hardware profile -- change this one line in the Arduino IDE.
// ---------------------------------------------------------------------------
#ifndef PHYRDLE_HARDWARE_PROFILE
#define PHYRDLE_HARDWARE_PROFILE PHYRDLE_PROFILE_YAGEO_10K
#endif

struct LetterAdcRange {
  uint16_t minimum;
  uint16_t maximum;
  char letter;
};

// CLI uploaders discover profile headers directly; IDE numeric defaults still work.
#ifdef PHYRDLE_PROFILE_FILE
#define PHYRDLE_STRINGIFY_INNER(value) #value
#define PHYRDLE_STRINGIFY(value) PHYRDLE_STRINGIFY_INNER(value)
#include PHYRDLE_STRINGIFY(PHYRDLE_PROFILE_FILE)
#elif PHYRDLE_HARDWARE_PROFILE == PHYRDLE_PROFILE_LEGACY_1K
#include "profiles/legacy_1k.h"
#elif PHYRDLE_HARDWARE_PROFILE == PHYRDLE_PROFILE_WIDE_4K7
#include "profiles/wide_4k7.h"
#elif PHYRDLE_HARDWARE_PROFILE == PHYRDLE_PROFILE_YAGEO_10K
#include "profiles/yageo_10k.h"
#else
#error "Unknown PHYRDLE_HARDWARE_PROFILE selection"
#endif

#endif // HARDWARE_PROFILE_H
