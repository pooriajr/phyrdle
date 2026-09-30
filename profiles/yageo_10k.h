#ifndef PROFILE_YAGEO_10K_H
#define PROFILE_YAGEO_10K_H

const char HARDWARE_PROFILE_NAME[] = "yageo-10k";

// Vcc -> letter resistor -> ADC node -> 10k fixed resistor -> GND.
// Adafruit 442 / Yageo 0603 letter resistors: +/-5%; fixed resistor: +/-1%.
// Assumes a 10-bit ADC with Vcc as its reference.
// ADC = 1023 * Rfixed / (Rletter + Rfixed).
// Detection ranges expand the original outward-rounded tolerance bands.
// Each gap is split at floor((lower band's high + upper band's low) / 2).
// The lower letter includes that count; the upper letter starts one count later.
// Bounds are inclusive, with no overlaps or unassigned counts above empty.
// Resistor-tolerance formulas used for the original bands:
// Lower bound: 1023 * 9900 / (Rletter * 1.05 + 9900).
// Upper bound: 1023 * 10100 / (Rletter * 0.95 + 10100).
// Expanded ranges allow extra ADC error, but sufficiently large errors can
// identify a neighboring letter. Verify on assembled hardware.

// An empty slot reads 0–20. P starts at 21; Q extends to 1023.
const uint16_t EMPTY_SLOT_MAX_ADC = 20;

const LetterAdcRange LETTER_ADC_RANGES[] PROGMEM = {
  {376,  437, 'A'},  // 15000 ohms
  { 79,  102, 'B'},  // 100000 ohms
  {644,  697, 'C'},  // 5100 ohms
  {789,  829, 'D'},  // 2700 ohms
  {234,  308, 'E'},  // 27000 ohms
  { 57,   78, 'F'},  // 150000 ohms
  { 43,   56, 'G'},  // 200000 ohms
  {489,  537, 'H'},  // 10000 ohms
  {946,  970, 'I'},  // 680 ohms
  {923,  945, 'J'},  // 1000 ohms
  {103,  121, 'K'},  // 82000 ohms
  {744,  788, 'L'},  // 3300 ohms
  {873,  902, 'M'},  // 1500 ohms
  {587,  643, 'N'},  // 6800 ohms
  {830,  872, 'O'},  // 2000 ohms
  { 21,   42, 'P'},  // 270000 ohms
  {987, 1023, 'Q'},  // 330 ohms
  {438,  488, 'R'},  // 12000 ohms
  {538,  586, 'S'},  // 8200 ohms
  {309,  375, 'T'},  // 20000 ohms
  {698,  743, 'U'},  // 4300 ohms
  {122,  149, 'V'},  // 68000 ohms
  {150,  180, 'W'},  // 51000 ohms
  {903,  922, 'X'},  // 1200 ohms
  {181,  233, 'Y'},  // 43000 ohms
  {971,  986, 'Z'},  // 430 ohms
};

const uint8_t LETTER_ADC_RANGE_COUNT =
    sizeof(LETTER_ADC_RANGES) / sizeof(LETTER_ADC_RANGES[0]);

#endif // PROFILE_YAGEO_10K_H
