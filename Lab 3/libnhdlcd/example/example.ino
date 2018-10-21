#include <NHD-0216K3Z-FL-GBW-V3.hpp>

Display lcd(13, 12, 11);

void setup() {
  Serial.begin(9600);
  lcd.cmdWrite(clear_screen);
  /* lcd.init() is called by the constructor. */
}

void loop() {
  lcd.cmdWrite(clear_screen);
  lcd.cmdWrite(blinking_cursor_on);
  delay(1000);
  while (Serial.available()) {
    if (Serial.available() <= 16)
      lcd.charWrite(Serial.read());
    else {
      for (uint8_t i = 0; i < 16; ++i) lcd.charWrite(Serial.read());
      lcd.cmdWrite(set_cursor, (P) {0x40});
      while (Serial.available()) lcd.charWrite(Serial.read());
    }
  }
  delay(5000);

  /* Does not work... Why?
   *  lcd.cmdWrite(load_custom_character,
   *               (P){0x00, 0x04, 0x00, 0x04, 0x08, 0x10, 0x11, 0x0E, 0x00},
   * 9); lcd.charWrite(0x00);
   */
}