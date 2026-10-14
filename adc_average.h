#ifndef ADC_AVERAGE_H
#define ADC_AVERAGE_H
#include <stdint.h>

// Eight scans at ~5 ms spacing: ~40 ms to fully follow a changed input.
// Reset after a long scan interruption rather than blend stale tile readings.
class AdcAverage {
  uint16_t values[8] = {};
  uint16_t sum = 0;
  uint8_t next = 0, count = 0;
  uint32_t last = 0;
  bool wasEmpty = true;
public:
  void reset() { sum = 0; next = 0; count = 0; }
  uint16_t update(uint16_t raw, uint32_t now, uint16_t emptyThreshold = 0) {
    const bool empty = raw <= emptyThreshold;
    // Never average an empty slot together with an inserted tile.
    if (count && (empty != wasEmpty || uint32_t(now-last) > 25)) reset();
    wasEmpty = empty;
    last = now;
    if (count == 8) sum -= values[next];
    else ++count;
    values[next] = raw;
    sum += raw;
    next = (next + 1) % 8;
    return (sum + count/2) / count;
  }
};
#endif
