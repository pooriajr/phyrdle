#ifndef RNG_SEED_H
#define RNG_SEED_H

#include <Arduino.h>
#include <EEPROM.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

// Bytes 0-3 hold the 32-bit RNG state that survives power-off.
// seedRandomOnce() updates it once per boot. EEPROM.put writes a byte only
// when that byte changes, so a boot costs at most four write cycles.
static const int RNG_EEPROM_ADDR = 0;

static volatile uint8_t wdtJitterSample = 0;
static volatile uint8_t wdtJitterReady = 0;

ISR(WDT_vect) {
  // Timer1 runs from the CPU clock. The watchdog runs from its own RC
  // oscillator, so this low byte jitters from boot to boot.
  wdtJitterSample = TCNT1L;
  wdtJitterReady = 1;
}

// Interrupt mode, ~16 ms. The two stores must stay adjacent: the hardware
// accepts the mode write only within four cycles of setting WDCE.
static void armWatchdogJitterInterrupt() {
  MCUSR &= ~(1 << WDRF);
  __asm__ __volatile__(
      "in __tmp_reg__, __SREG__\n\t"
      "cli\n\t"
      "wdr\n\t"
      "sts %[wdt], %[change]\n\t"
      "sts %[wdt], %[mode]\n\t"
      "out __SREG__, __tmp_reg__\n\t"
      :
      : [wdt] "M"(_SFR_MEM_ADDR(WDTCSR)),
        [change] "r"((uint8_t)((1 << WDCE) | (1 << WDE))),
        [mode] "r"((uint8_t)(1 << WDIE))
      : "r0", "memory");
  sei();
}

static unsigned long harvestWatchdogJitter() {
  uint8_t savedTCCR1A = TCCR1A;
  uint8_t savedTCCR1B = TCCR1B;
  uint16_t savedTCNT1 = TCNT1;

  TCCR1A = 0;
  TCCR1B = (1 << CS10);

  unsigned long jitter = 0;
  for (uint8_t i = 0; i < 8; i++) {
    wdtJitterReady = 0;
    armWatchdogJitterInterrupt();
    unsigned long started = millis();
    while (!wdtJitterReady && (millis() - started) < 40) {
    }
    // Stop the watchdog before the next timeout. Leaving it armed in reset
    // mode would reboot the board; leaving it interrupting would stretch
    // LED bit timing later in the game.
    wdt_disable();
    jitter ^= (unsigned long)wdtJitterSample << ((i & 3) * 8);
  }

  TCCR1B = 0;
  TCNT1 = savedTCNT1;
  TCCR1A = savedTCCR1A;
  TCCR1B = savedTCCR1B;
  return jitter;
}

// Optional extra only. A floating pin often reads the same value every time,
// so these bits must not be the thing the seed depends on.
// A0-A3 and A6 are letter slots. A4 and A5 are I2C SDA/SCL for the LCD.
// A7 is the analog input this wiring leaves unconnected.
static unsigned long harvestAnalogExtra() {
  analogRead(A7);
  unsigned long bits = 0;
  for (uint8_t i = 0; i < 32; i++) {
    bits = (bits << 1) | ((unsigned long)analogRead(A7) & 1UL);
  }
  return bits;
}

// Call once from setup(), before the first randomWord().
static void seedRandomOnce() {
  unsigned long state = 0;
  EEPROM.get(RNG_EEPROM_ADDR, state);

  unsigned long seed = state ^ harvestWatchdogJitter() ^ harvestAnalogExtra();
  // randomSeed(0) is ignored. avr-libc's generator wants a non-zero 31-bit seed.
  seed &= 0x7FFFFFFFUL;
  if (seed == 0) {
    seed = 1;
  }
  randomSeed(seed);

  unsigned long next = (unsigned long)random();
  if (next == 0) {
    next = 1;
  }
  EEPROM.put(RNG_EEPROM_ADDR, next);
}

#endif // RNG_SEED_H
