#ifndef PROFILE_WIDE_4K7_H
#define PROFILE_WIDE_4K7_H

const char HARDWARE_PROFILE_NAME[] = "wide-4.7k";

// An empty slot reads near zero. O is the lowest letter at approximately 55,
// so keep a gap between the empty threshold and O's accepted range.
const uint16_t EMPTY_SLOT_MAX_ADC = 25;

// The wider separation permits quicker confirmation than the legacy profile.
const uint16_t INSERT_STABLE_MS = 15;
const uint16_t REPLACE_STABLE_MS = 75;

// Initial ranges derived from "Phyrdle Resistor Math - 4.7k.csv". Each
// calculated ADC center is rounded to a 21-count window (center +/- 10).
// These can be refined later with readings from assembled hardware.
const LetterAdcRange LETTER_ADC_RANGES[] PROGMEM = {
  {278, 298, 'A'},
  {549, 569, 'B'},
  {687, 707, 'C'},
  { 83, 103, 'D'},
  {805, 825, 'E'},
  {170, 190, 'F'},
  {884, 904, 'G'},
  {730, 750, 'H'},
  {957, 977, 'I'},
  {487, 507, 'J'},
  {655, 675, 'K'},
  {591, 611, 'L'},
  {202, 222, 'M'},
  {920, 940, 'N'},
  { 45,  65, 'O'},
  {852, 872, 'P'},
  {426, 446, 'Q'},
  {118, 138, 'R'},
  {765, 785, 'S'},
  {317, 337, 'T'},
  {234, 254, 'U'},
  {518, 538, 'V'},
  {363, 383, 'W'},
  {624, 644, 'X'},
  {394, 414, 'Y'},
  {457, 477, 'Z'}
};

const uint8_t LETTER_ADC_RANGE_COUNT =
    sizeof(LETTER_ADC_RANGES) / sizeof(LETTER_ADC_RANGES[0]);

#endif // PROFILE_WIDE_4K7_H
