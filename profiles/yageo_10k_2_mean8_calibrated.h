#ifndef PROFILE_MEASURED_CALIBRATION_H
#define PROFILE_MEASURED_CALIBRATION_H

// Calibrated from yageo_10k_2.h; measured envelopes, not resistor tolerance bounds.
// Requires the same eight-sample moving average (mean8) used during calibration.
// Inclusive midpoint boundaries maximize clearance between measured envelopes.
// Assumes stable readings identify the physical letter correctly unless manually labeled.
// Applies only to the measured tiles/slots/conditions; validate before normal use.
// Minimum observed extra clearance: 0 counts; requested reserve: 3.
const char HARDWARE_PROFILE_NAME[] = "yageo_10k_2_mean8_calibrated";
const uint16_t EMPTY_SLOT_MAX_ADC = 0;

const LetterAdcRange LETTER_ADC_RANGES[] PROGMEM = {
  {374, 436, 'A'}, // measured 407..411; 63 windows; slots 4
  {76, 109, 'B'}, // measured 89..90; 60 windows; slots 5
  {650, 701, 'C'}, // measured 681..685; 14 windows; slots 2
  {795, 837, 'D'}, // measured 810..816; 17 windows; slots 2
  {256, 306, 'E'}, // measured 274..275; 7 windows; slots 1
  {52, 75, 'F'}, // measured 59..62; 8 windows; slots 5
  {35, 51, 'G'}, // measured 43..44; 14 windows; slots 4
  {487, 540, 'H'}, // measured 508..513; 11 windows; slots 1
  {953, 974, 'I'}, // measured 963..970; 10 windows; slots 4
  {919, 952, 'J'}, // measured 935..941; 8 windows; slots 3
  {1, 16, 'K'}, // measured 7..7; 15 windows; slots 1
  {17, 34, 'L'}, // measured 25..25; 7 windows; slots 5
  {110, 160, 'M'}, // measured 128..131; 7 windows; slots 4
  {161, 212, 'N'}, // measured 189..191; 14 windows; slots 5
  {213, 255, 'O'}, // measured 234..237; 5 windows; slots 1,3
  {307, 373, 'P'}, // measured 338..340; 8 windows; slots 2
  {437, 486, 'Q'}, // measured 462..464; 11 windows; slots 3
  {541, 592, 'R'}, // measured 568..571; 8 windows; slots 4
  {593, 649, 'S'}, // measured 614..617; 10 windows; slots 1
  {702, 748, 'T'}, // measured 718..722; 10 windows; slots 2
  {749, 794, 'U'}, // measured 774..779; 10 windows; slots 5
  {838, 879, 'V'}, // measured 858..863; 13 windows; slots 3
  {880, 918, 'W'}, // measured 896..901; 14 windows; slots 3
  {975, 991, 'X'}, // measured 979..986; 79 windows; slots 2
  {992, 1009, 'Y'}, // measured 996..1003; 78 windows; slots 1
  {1010, 1023, 'Z'}, // measured 1015..1023; 81 windows; slots 3
};
const uint8_t LETTER_ADC_RANGE_COUNT = sizeof(LETTER_ADC_RANGES) / sizeof(LETTER_ADC_RANGES[0]);

#endif
