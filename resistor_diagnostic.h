#ifndef RESISTOR_DIAGNOSTIC_H
#define RESISTOR_DIAGNOSTIC_H

#include "hardware_profile.h"
#include <stdio.h>
#define USE_FOUR_PIN_LED_STRIP true
#define FOUR_PIN_LED_DATA_PIN 11
#define FOUR_PIN_LED_CLOCK_PIN 13
#include "lighting.h"

// Match the game: eight-sample moving average, Vcc reference, 5 ms scans.
const uint8_t DIAG_PINS[] = {A0, A1, A2, A3, A6};
const uint8_t DIAG_WINDOW = 16;
AdcAverage diagAverages[5];
uint16_t diagReadings[5][DIAG_WINDOW];
uint8_t diagPosition = 0, diagCount = 0;
unsigned long diagLastScan = 0, diagLastLed = 0, diagLastLog = 0;
uint16_t diagLogLow[5] = {1023,1023,1023,1023,1023}, diagLogHigh[5] = {};
char diagTx[224];
uint8_t diagTxLength = 0, diagTxPosition = 0;

// Drain output only when UART buffer space is available, never delay ADC scans.
void diagnosticSerialDrain() {
  const int available = Serial.availableForWrite();
  const int remaining = diagTxLength - diagTxPosition;
  const int count = min(available, remaining);
  if (count > 0) {
    Serial.write((const uint8_t *)diagTx + diagTxPosition, count);
    diagTxPosition += count;
  }
}

// -1 = empty, -2 = unrecognized; otherwise a profile range index.
int8_t diagnosticRange(uint16_t adc) {
  if (adc <= EMPTY_SLOT_MAX_ADC) return -1;
  for (uint8_t i = 0; i < LETTER_ADC_RANGE_COUNT; ++i) {
    if (adc >= pgm_read_word(&LETTER_ADC_RANGES[i].minimum) &&
        adc <= pgm_read_word(&LETTER_ADC_RANGES[i].maximum)) return i;
  }
  return -2;
}

CRGB diagnosticBoundsColor(uint16_t low, uint16_t high, bool ready) {
  const int8_t range = diagnosticRange(low);
  if (range == -2 || diagnosticRange(high) != range) return CRGB::Red;
  if (!ready) return CRGB::Blue;
  if (range == -1) return CRGB(16,16,16);
  const uint16_t minimum = pgm_read_word(&LETTER_ADC_RANGES[range].minimum);
  const uint16_t maximum = pgm_read_word(&LETTER_ADC_RANGES[range].maximum);
  const uint16_t clearance = min(low-minimum, maximum-high);
  return uint32_t(clearance)*4 >= maximum-minimum && maximum>minimum
      ? CRGB::Green : CRGB::Yellow;
}

CRGB diagnosticColor(const uint16_t *samples, uint8_t count) {
  if (!count) return CRGB::Blue;
  uint16_t low=1023, high=0;
  for(uint8_t i=0;i<count;++i){low=min(low,samples[i]);high=max(high,samples[i]);}
  return diagnosticBoundsColor(low,high,count==DIAG_WINDOW);
}

// Latest readings plus full-second extrema/status; stream without blocking.
void diagnosticLog() {
  int length = 0;
  for (uint8_t slot = 0; slot < 5; ++slot) {
    const uint16_t raw = diagReadings[slot][(diagPosition + DIAG_WINDOW - 1) % DIAG_WINDOW];
    const int8_t range = diagnosticRange(raw);
    const char letter = range >= 0 ? pgm_read_byte(&LETTER_ADC_RANGES[range].letter) : (range == -1 ? '-' : '?');
    const CRGB color=diagnosticBoundsColor(diagLogLow[slot],diagLogHigh[slot],diagCount==DIAG_WINDOW);
    const char state=color==CRGB::Red?'R':color==CRGB::Green?'G':color==CRGB::Yellow?'Y':diagCount<DIAG_WINDOW?'B':'E';
    length += snprintf_P(diagTx + length, sizeof(diagTx) - length,
        PSTR("%sslot%u=%u(%c,%c,%u,%u)"), slot ? " | " : "", slot+1, raw, letter, state, diagLogLow[slot], diagLogHigh[slot]);
    diagLogLow[slot]=1023; diagLogHigh[slot]=0;
  }
  length += snprintf_P(diagTx + length, sizeof(diagTx) - length, PSTR(" filter=mean8"));
  diagTxLength = min((int)sizeof(diagTx) - 2, length);
  diagTx[diagTxLength++] = '\n';
  diagTxPosition = 0;
}

void setup() {
  analogReference(DEFAULT);
  Serial.begin(9600);
  Serial.print(F("Resistor diagnostic: "));
  Serial.println(HARDWARE_PROFILE_NAME);
  setupLighting();
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
}

void loop() {
  diagnosticSerialDrain();
  const unsigned long now = millis();
  if (now - diagLastScan < 5) return;
  diagLastScan = now;
  for (uint8_t slot = 0; slot < 5; ++slot) {
    const uint16_t value = diagAverages[slot].update(analogRead(DIAG_PINS[slot]), now);
    diagReadings[slot][diagPosition] = value;
    diagLogLow[slot]=min(diagLogLow[slot],value);diagLogHigh[slot]=max(diagLogHigh[slot],value);

  }
  diagPosition = (diagPosition + 1) % DIAG_WINDOW;
  if (diagCount < DIAG_WINDOW) ++diagCount;
  if (now - diagLastLed >= 25) {
    diagLastLed = now;
    for (uint8_t slot = 0; slot < 5; ++slot)
      leds[slot] = diagnosticColor(diagReadings[slot], diagCount);
    FastLED.show();
  }
  if (now - diagLastLog >= 1000 && diagTxPosition == diagTxLength) {
    diagLastLog = now;
    diagnosticLog();
  }
}
#endif
