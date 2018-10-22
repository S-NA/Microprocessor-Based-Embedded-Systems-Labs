#include <SoftwareSerial.h>
#include <nhdlcd.hpp>

#define LOG(x) Serial.println(x)

nhdlcd lcd(13 /* data */, 12 /* clock */, 11 /* slave select */);
SoftwareSerial bluetooth(2 /* Bluetooth Tx */, 3 /* Bluetooth Rx */);
bool ranOnce = false;

void setup() {
  /* Start serial at 9600 baudrate. */
  Serial.begin(9600);
  /* Start bluetooth serial at 9600 baudrate. */
  bluetooth.begin(9600);
}

void loop() {
  if (!ranOnce) {
    lcd.write(Command::ClearScreen);
  }

  if (bluetooth.available()) {
    char ch = bluetooth.read();
    if (!(ch - '\n')) {
      lcd.write(ch);
      LOG("Wrote to LCD: " + ch);
    } else /* treat a newline as a clearscreen */
    {
      lcd.write(Command::ClearScreen);
    }
  }

  delay(100);
  ranOnce = true;
}
