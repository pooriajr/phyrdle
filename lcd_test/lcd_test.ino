#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Most common I2C address is 0x27 (sometimes 0x3F)
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  lcd.init();            // or lcd.begin(20, 4) on some library versions
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hello!");
  lcd.setCursor(0, 1);
  lcd.print("J204A 20x4");
}

void loop() {}
