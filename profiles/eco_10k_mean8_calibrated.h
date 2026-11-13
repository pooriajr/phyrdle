#ifndef PROFILE_MEASURED_CALIBRATION_H
#define PROFILE_MEASURED_CALIBRATION_H

// Calibrated from eco_10k.h; measured envelopes, not resistor tolerance bounds.
// Requires the same eight-sample moving average (mean8) used during calibration.
// Inclusive midpoint boundaries maximize clearance between measured envelopes.
// Assumes stable readings identify the physical letter correctly unless manually labeled.
// Applies only to the measured tiles/slots/conditions; validate before normal use.
// Minimum observed extra clearance: 1 counts; requested reserve: 3.
const char HARDWARE_PROFILE_NAME[] = "eco_10k_mean8_calibrated";
const uint16_t EMPTY_SLOT_MAX_ADC = 0;

const LetterAdcRange LETTER_ADC_RANGES[] PROGMEM = {
  {1, 31, 'A'}, // measured 17..17; 25 windows; slots 1
  {32, 69, 'B'}, // measured 46..46; 24 windows; slots 2
  {70, 111, 'C'}, // measured 92..92; 13 windows; slots 3
  {112, 151, 'D'}, // measured 130..131; 11 windows; slots 4
  {152, 189, 'E'}, // measured 171..171; 9 windows; slots 5
  {190, 224, 'F'}, // measured 208..209; 8 windows; slots 1
  {225, 259, 'G'}, // measured 240..240; 15 windows; slots 2
  {260, 299, 'H'}, // measured 278..279; 17 windows; slots 3
  {300, 344, 'I'}, // measured 320..321; 17 windows; slots 4
  {345, 390, 'J'}, // measured 367..368; 17 windows; slots 5
  {391, 432, 'K'}, // measured 413..414; 10 windows; slots 1
  {433, 483, 'L'}, // measured 450..450; 11 windows; slots 2
  {484, 541, 'M'}, // measured 516..517; 9 windows; slots 3
  {542, 591, 'N'}, // measured 565..567; 9 windows; slots 4
  {592, 639, 'O'}, // measured 615..616; 8 windows; slots 5
  {640, 683, 'P'}, // measured 662..664; 9 windows; slots 1
  {684, 723, 'Q'}, // measured 702..705; 8 windows; slots 2
  {724, 759, 'R'}, // measured 741..745; 9 windows; slots 3
  {760, 794, 'S'}, // measured 773..779; 10 windows; slots 4
  {795, 829, 'T'}, // measured 810..815; 13 windows; slots 5
  {830, 860, 'U'}, // measured 843..849; 16 windows; slots 1
  {861, 898, 'V'}, // measured 872..878; 22 windows; slots 2
  {899, 937, 'W'}, // measured 919..925; 18 windows; slots 3
  {938, 968, 'X'}, // measured 950..957; 13 windows; slots 4
  {969, 1001, 'Y'}, // measured 980..985; 9 windows; slots 5
  {1002, 1023, 'Z'}, // measured 1018..1022; 5 windows; slots 1
};
const uint8_t LETTER_ADC_RANGE_COUNT = sizeof(LETTER_ADC_RANGES) / sizeof(LETTER_ADC_RANGES[0]);

#endif
