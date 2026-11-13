#ifndef PROFILE_ECO_10K_H
#define PROFILE_ECO_10K_H

// Vcc -> letter resistor -> ADC -> fixed resistor -> GND.
// Fixed resistor: 10000 ohms; assumed letter tolerance: 5%; fixed tolerance: 1%.
// 10-bit ADC, Vcc reference. Inclusive midpoint-expanded detection ranges.
// All counts above the empty threshold identify as a letter.
// A is nominally 19.67 counts, so the legacy empty threshold of 20 cannot be used.
// Validate the empty threshold and error allowance on assembled hardware.
const char HARDWARE_PROFILE_NAME[] = "eco-10k";
const uint16_t EMPTY_SLOT_MAX_ADC = 0;

const LetterAdcRange LETTER_ADC_RANGES[] PROGMEM = {
  {  1,   33, 'A'},  // 510000 ohms; tolerance ADC 18.57–20.89; extra error ±12
  { 34,   69, 'B'},  // 200000 ohms; tolerance ADC 46.06–51.64; extra error ±12
  { 70,  111, 'C'},  // 100000 ohms; tolerance ADC 88.14–98.31; extra error ±12
  {112,  150, 'D'},  // 68000 ohms; tolerance ADC 124.57–138.32; extra error ±11
  {151,  189, 'E'},  // 49900 ohms; tolerance ADC 162.58–179.68; extra error ±9
  {190,  223, 'F'},  // 39000 ohms; tolerance ADC 199.17–219.14; extra error ±3
  {224,  257, 'G'},  // 33000 ohms; tolerance ADC 227.33–249.27; extra error ±3
  {258,  297, 'H'},  // 27000 ohms; tolerance ADC 264.78–289.02; extra error ±6
  {298,  342, 'I'},  // 22000 ohms; tolerance ADC 306.90–333.30; extra error ±8
  {343,  387, 'J'},  // 18000 ohms; tolerance ADC 351.66–379.86; extra error ±7
  {388,  427, 'K'},  // 15000 ohms; tolerance ADC 394.84–424.32; extra error ±2
  {428,  478, 'L'},  // 13000 ohms; tolerance ADC 430.05–460.24; extra error ±2
  {479,  537, 'M'},  // 10000 ohms; tolerance ADC 496.46–527.16; extra error ±9
  {538,  585, 'N'},  // 8200 ohms; tolerance ADC 547.15–577.55; extra error ±7
  {586,  632, 'O'},  // 6800 ohms; tolerance ADC 594.35–623.93; extra error ±8
  {633,  676, 'P'},  // 5600 ohms; tolerance ADC 641.81–670.06; extra error ±5
  {677,  716, 'Q'},  // 4700 ohms; tolerance ADC 682.69–709.39; extra error ±5
  {717,  753, 'R'},  // 3900 ohms; tolerance ADC 723.67–748.45; extra error ±4
  {754,  787, 'S'},  // 3300 ohms; tolerance ADC 757.78–780.68; extra error ±3
  {788,  822, 'T'},  // 2700 ohms; tolerance ADC 795.27–815.82; extra error ±6
  {823,  853, 'U'},  // 2200 ohms; tolerance ADC 829.46–847.60; extra error ±5
  {854,  891, 'V'},  // 1800 ohms; tolerance ADC 859.01–874.88; extra error ±5
  {892,  930, 'W'},  // 1200 ohms; tolerance ADC 907.50–919.24; extra error ±10
  {931,  960, 'X'},  // 820 ohms; tolerance ADC 941.15–949.75; extra error ±10
  {961,  993, 'Y'},  // 510 ohms; tolerance ADC 970.50–976.17; extra error ±9
  {994, 1023, 'Z'},  // 120 ohms; tolerance ADC 1010.14–1011.58; extra error ±11
};

const uint8_t LETTER_ADC_RANGE_COUNT =
    sizeof(LETTER_ADC_RANGES) / sizeof(LETTER_ADC_RANGES[0]);

#endif // PROFILE_ECO_10K_H
