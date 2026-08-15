#ifndef PROFILE_LEGACY_1K_H
#define PROFILE_LEGACY_1K_H

const char HARDWARE_PROFILE_NAME[] = "legacy-1k";
const uint16_t EMPTY_SLOT_MAX_ADC = 100;
const uint16_t INSERT_STABLE_MS = 25;
const uint16_t REPLACE_STABLE_MS = 150;

// These are the original calibrated ranges, preserved exactly for existing
// hardware that uses the 1k fixed resistor configuration.
const LetterAdcRange LETTER_ADC_RANGES[] PROGMEM = {
  {920, 940, 'A'},
  {881, 901, 'B'},
  {843, 863, 'C'},
  {815, 835, 'D'},
  {765, 785, 'E'},
  {739, 759, 'F'},
  {703, 720, 'G'},
  {685, 702, 'H'},
  {665, 675, 'I'},
  {640, 660, 'J'},
  {594, 614, 'K'},
  {561, 581, 'L'},
  {519, 535, 'M'},
  {501, 518, 'N'},
  {475, 495, 'O'},
  {449, 469, 'P'},
  {403, 423, 'Q'},
  {370, 390, 'R'},
  {335, 355, 'S'},
  {307, 327, 'T'},
  {285, 305, 'U'},
  {267, 284, 'V'},
  {230, 250, 'W'},
  {182, 196, 'X'},
  {167, 181, 'Y'},
  {117, 137, 'Z'}
};

const uint8_t LETTER_ADC_RANGE_COUNT =
    sizeof(LETTER_ADC_RANGES) / sizeof(LETTER_ADC_RANGES[0]);

#endif // PROFILE_LEGACY_1K_H
