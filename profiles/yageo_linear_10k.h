#ifndef PROFILE_YAGEO_LINEAR_10K_H
#define PROFILE_YAGEO_LINEAR_10K_H

// Vcc -> letter resistor -> ADC -> fixed resistor -> GND.
// Fixed resistor: 10000 ohms; letter tolerance: 5%; fixed tolerance: 1%.
// 10-bit ADC, Vcc reference. Inclusive midpoint-expanded detection ranges.
// All counts above the empty threshold identify as a letter.
// Validate the empty threshold and error allowance on assembled hardware.
const char HARDWARE_PROFILE_NAME[] = "yageo-linear-10k";
const uint16_t EMPTY_SLOT_MAX_ADC = 0;

const LetterAdcRange LETTER_ADC_RANGES[] PROGMEM = {
  {1, 20, 'A'},  // 820000 ohms; tolerance ADC 11.63–13.09; extra error ±6
  {21, 38, 'B'},  // 330000 ohms; tolerance ADC 28.42–31.93; extra error ±6
  {39, 56, 'C'},  // 200000 ohms; tolerance ADC 46.06–51.64; extra error ±4
  {57, 77, 'D'},  // 150000 ohms; tolerance ADC 60.50–67.71; extra error ±3
  {78, 111, 'E'},  // 100000 ohms; tolerance ADC 88.14–98.31; extra error ±10
  {112, 161, 'F'},  // 68000 ohms; tolerance ADC 124.57–138.32; extra error ±12
  {162, 215, 'G'},  // 43000 ohms; tolerance ADC 183.97–202.79; extra error ±12
  {216, 257, 'H'},  // 33000 ohms; tolerance ADC 227.33–249.27; extra error ±7
  {258, 308, 'I'},  // 27000 ohms; tolerance ADC 264.78–289.02; extra error ±6
  {309, 374, 'J'},  // 20000 ohms; tolerance ADC 327.76–355.06; extra error ±18
  {375, 437, 'K'},  // 15000 ohms; tolerance ADC 394.84–424.32; extra error ±12
  {438, 488, 'L'},  // 12000 ohms; tolerance ADC 450.12–480.57; extra error ±7
  {489, 537, 'M'},  // 10000 ohms; tolerance ADC 496.46–527.16; extra error ±7
  {538, 585, 'N'},  // 8200 ohms; tolerance ADC 547.15–577.55; extra error ±7
  {586, 643, 'O'},  // 6800 ohms; tolerance ADC 594.35–623.93; extra error ±8
  {644, 696, 'P'},  // 5100 ohms; tolerance ADC 663.89–691.35; extra error ±4
  {697, 743, 'Q'},  // 4300 ohms; tolerance ADC 702.58–728.40; extra error ±5
  {744, 787, 'R'},  // 3300 ohms; tolerance ADC 757.78–780.68; extra error ±6
  {788, 829, 'S'},  // 2700 ohms; tolerance ADC 795.27–815.82; extra error ±7
  {830, 871, 'T'},  // 2000 ohms; tolerance ADC 843.98–861.02; extra error ±9
  {872, 902, 'U'},  // 1500 ohms; tolerance ADC 882.59–896.51; extra error ±5
  {903, 930, 'V'},  // 1200 ohms; tolerance ADC 907.50–919.24; extra error ±4
  {931, 960, 'W'},  // 820 ohms; tolerance ADC 941.15–949.75; extra error ±10
  {961, 982, 'X'},  // 510 ohms; tolerance ADC 970.50–976.17; extra error ±5
  {983, 1001, 'Y'},  // 330 ohms; tolerance ADC 988.41–992.20; extra error ±5
  {1002, 1023, 'Z'},  // 120 ohms; tolerance ADC 1010.14–1011.58; extra error ±8
};

const uint8_t LETTER_ADC_RANGE_COUNT = sizeof(LETTER_ADC_RANGES) / sizeof(LETTER_ADC_RANGES[0]);

#endif
