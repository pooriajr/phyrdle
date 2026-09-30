#ifndef PROFILE_10K_2_H
#define PROFILE_10K_2_H

// Vcc -> letter resistor -> ADC -> fixed resistor -> GND.
// Fixed resistor: 10000 ohms; letter tolerance: 5%; fixed tolerance: 1%.
// 10-bit ADC, Vcc reference. Inclusive midpoint-expanded detection ranges.
// All counts above the empty threshold identify as a letter.
// Validate the empty threshold and error allowance on assembled hardware.
const char HARDWARE_PROFILE_NAME[] = "yageo-10k-2";
const uint16_t EMPTY_SLOT_MAX_ADC = 0;

const LetterAdcRange LETTER_ADC_RANGES[] PROGMEM = {
  {1, 20, 'K'},  // 820000 ohms; tolerance ADC 11.63–13.09; extra error ±6
  {21, 38, 'L'},  // 330000 ohms; tolerance ADC 28.42–31.93; extra error ±6
  {39, 56, 'G'},  // 200000 ohms; tolerance ADC 46.06–51.64; extra error ±4
  {57, 77, 'F'},  // 150000 ohms; tolerance ADC 60.50–67.71; extra error ±3
  {78, 111, 'B'},  // 100000 ohms; tolerance ADC 88.14–98.31; extra error ±10
  {112, 161, 'M'},  // 68000 ohms; tolerance ADC 124.57–138.32; extra error ±12
  {162, 215, 'N'},  // 43000 ohms; tolerance ADC 183.97–202.79; extra error ±12
  {216, 257, 'O'},  // 33000 ohms; tolerance ADC 227.33–249.27; extra error ±7
  {258, 308, 'E'},  // 27000 ohms; tolerance ADC 264.78–289.02; extra error ±6
  {309, 374, 'P'},  // 20000 ohms; tolerance ADC 327.76–355.06; extra error ±18
  {375, 437, 'A'},  // 15000 ohms; tolerance ADC 394.84–424.32; extra error ±12
  {438, 488, 'Q'},  // 12000 ohms; tolerance ADC 450.12–480.57; extra error ±7
  {489, 537, 'H'},  // 10000 ohms; tolerance ADC 496.46–527.16; extra error ±7
  {538, 585, 'R'},  // 8200 ohms; tolerance ADC 547.15–577.55; extra error ±7
  {586, 643, 'S'},  // 6800 ohms; tolerance ADC 594.35–623.93; extra error ±8
  {644, 696, 'C'},  // 5100 ohms; tolerance ADC 663.89–691.35; extra error ±4
  {697, 743, 'T'},  // 4300 ohms; tolerance ADC 702.58–728.40; extra error ±5
  {744, 787, 'U'},  // 3300 ohms; tolerance ADC 757.78–780.68; extra error ±6
  {788, 829, 'D'},  // 2700 ohms; tolerance ADC 795.27–815.82; extra error ±7
  {830, 871, 'V'},  // 2000 ohms; tolerance ADC 843.98–861.02; extra error ±9
  {872, 910, 'W'},  // 1500 ohms; tolerance ADC 882.59–896.51; extra error ±10
  {911, 944, 'J'},  // 1000 ohms; tolerance ADC 924.90–935.05; extra error ±8
  {945, 966, 'I'},  // 680 ohms; tolerance ADC 954.18–961.50; extra error ±4
  {967, 982, 'X'},  // 510 ohms; tolerance ADC 970.50–976.17; extra error ±3
  {983, 1001, 'Y'},  // 330 ohms; tolerance ADC 988.41–992.20; extra error ±5
  {1002, 1023, 'Z'},  // 120 ohms; tolerance ADC 1010.14–1011.58; extra error ±8
};

const uint8_t LETTER_ADC_RANGE_COUNT = sizeof(LETTER_ADC_RANGES) / sizeof(LETTER_ADC_RANGES[0]);

#endif
