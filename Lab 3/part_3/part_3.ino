#include "music.h"
#include "nhdlcd.hpp"
#include "pins.hpp"
#include <Keypad.h>
#include <SoftwareSerial.h>

struct {
  int state;
  unsigned long previousMillis;
  unsigned long blinkInterval;
  bool enable;
} led_s = {LOW, 0, 500, true};

/* time state that can be done occasionally
 * this is good for lethargic code */
struct {
  const unsigned MAX_LOOP;
  unsigned loopCount;
} stateCounter = {10000, 0};

struct {
  const unsigned MAX_POS;
  unsigned pos;
  bool on;
} soundStream = {32, 0, true};

struct {
  unsigned char track0[32];
  unsigned char track1[32];
  unsigned trackSelect;
} song = {{'c', 'd', 'e', 'f', 'g', 'a', 'b', 'h', '#'},
          {'c', 'd', 'e', 'f', 'g', 'a', 'b', 'h', '#'},
          0};

unsigned getSSP() {
  if (soundStream.pos < soundStream.MAX_POS) {
    if (song.trackSelect == 0) {
      // validate track0
      if (song.track0[soundStream.pos] == '#') {
        soundStream.pos = 0;
        return soundStream.pos;
      }
    } else if (song.trackSelect == 1) {
      if (song.track1[soundStream.pos] == '#') {
        soundStream.pos = 0;
        return soundStream.pos;
      }
    }
    return soundStream.pos++;
  }
  soundStream.pos = 0;
  return soundStream.pos;
}

/**
 * NOTE, TODO, AND THAR BE DRAGONS:
 * Pin 11, and 3 are off limits for PWM output. (Due to use of tone() function.)
 */

SoftwareSerial BluetoothP(bluetooth::pin::tx, bluetooth::pin::rx);
nhdlcd DisplayP(lcd::pin::data, lcd::pin::clock, lcd::pin::slaveSelect);
Keypad KeypadP = Keypad(makeKeymap(keypad::keys), keypad::rowPins,
                        keypad::colPins, keypad::ROWS, keypad::COLS);

void setup() {
  BluetoothP.begin(115200); // The Bluetooth Mate defaults to 115200bps
  BluetoothP.print("$");    // Print three times individually
  BluetoothP.print("$");
  BluetoothP.print("$"); // Enter command mode
  delay(100);            // Short delay, wait for the Mate to send back CMD
  BluetoothP.println(
      "U,9600,N"); // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data
  // reliably
  BluetoothP.begin(9600); // Start bluetooth serial at 9600

  BluetoothP.println(F("Enter help for more information."));
  DisplayP.write(Command::ClearScreen);

  pinMode(A1, INPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, INPUT);

  Serial.begin(9600);
}

void loop() {
  if (stateCounter.MAX_LOOP ==
      ++stateCounter.loopCount) { // state to update occasionally
    updateState();
  }
  handleBluetooth();
  handleKeypad();
  handleLEDBrightness();
}

unsigned long getSGFrequency(byte pin) {
  //  const int SAMPLES = 16;
  //  unsigned long freq = 0;
  //  for (unsigned int j = 0; j < SAMPLES; j++) {
  //    unsigned long duration = pulseIn(pin, HIGH, 250000);
  //    freq += 500000 / (duration >= 50 ? duration : 100);
  //  }
  //  return freq / SAMPLES;
  unsigned long freq = 0;
  unsigned long duration = pulseIn(pin, HIGH, 250000);
  freq += 500000 / (duration >= 50 ? duration : 100);
  return freq;
}

void updateState() {
  noTone(speaker::pin);
  unsigned long freq = getSGFrequency(9);
  float adjFreq = freq / 1000.; // seconds
  led_s.blinkInterval = 1000 / adjFreq;
}

/**
 * LED blinking: https://www.arduino.cc/en/Tutorial/BlinkWithoutDelay
 * Convert Signal Generator read in to millis and use that^
 */
void handleLEDBrightness() {
  unsigned long currentMillis = millis();
  unsigned pressure =
      map(analogRead(forcesensitiveresistor::pin), 0, 1023, 0, 255);

  if (currentMillis - led_s.previousMillis > led_s.blinkInterval) {
    led_s.previousMillis = currentMillis;

    if (led_s.state == LOW)
      led_s.state = HIGH;
    else
      led_s.state = LOW;
    if (led_s.state) {
      if (song.trackSelect == 0) {
        if (digitalRead(A1) == HIGH) {
          if (soundStream.on) tone(A5, getOctaveUpFrequency(song.track0[getSSP()]));
        } else {
          if (soundStream.on) tone(A5, getFrequency(song.track0[getSSP()]));
        }
      } else {
        if (digitalRead(A1) == HIGH) {
          if (soundStream.on) tone(A5, getOctaveUpFrequency(song.track1[getSSP()]));
        } else {
          if (soundStream.on) tone(A5, getFrequency(song.track1[getSSP()]));
        }
      }
      if (led_s.enable)
        analogWrite(led::pin, pressure);
    } else {
      noTone(A5);
      analogWrite(led::pin, 0);
    }
  }
}

bool handleKeypad() {
  /* The playing of the default (or new) song stops when either the keypad or
   * the Bluetooth modem recieves some user actions. */
  char ch = KeypadP.getKey();
  if (ch != NO_KEY) {
    noTone(speaker::pin);
    DisplayP.write(Command::ClearScreen);
    sendToTrack0(song.track1);
    /* Keypad is used for users to input a new song segment. */
    if (ch != NO_KEY) {
      unsigned pos = 0;
      do {
        if (ch >= '1' && ch < '9') {
          DisplayP.write(ch);
          song.track1[pos] = ch;
          ch = KeypadP.waitForKey();
        } else if (ch == '#' && pos != 0) {
          song.track1[pos] = ch;
          DisplayP.write(Command::ClearScreen);
          BluetoothP.println();
          BluetoothP.print(F("Wrote new song 2: "));
          trackToBluetooth(song.track1);
          BluetoothP.println();
          break;
        } else if (ch == '#' && pos == 0) {
          DisplayP.write(Command::ClearScreen);
        }
      } while (++pos < 32);
    }
    return true;
  }
  return false;
}

void handleBluetooth() {
  if (!BluetoothP.available())
    return;
  noTone(speaker::pin);
  char command[16] = {}; /* zero initalize */
  for (int i = 0; i < 16; i++) {
    command[i] = BluetoothP.read();
    if (command[i] == 13) {
      command[i] = '\0';
      BluetoothP.println();
      break;
    }
    BluetoothP.print(command[i]);
    while (!BluetoothP.available())
      ;
  }
  if (strcmp(command, "dump") == 0) {
    // dump
    DisplayP.write(Command::ClearScreen);
    BluetoothP.println(F("Dumping data..."));
    BluetoothP.print(F("Dumping track0 (song1): "));
    trackToBluetooth(song.track0);
    BluetoothP.println();
    BluetoothP.print(F("Dumping track1 (song2): "));
    trackToBluetooth(song.track1);
    BluetoothP.println();
  } else if (strcmp(command, "select") == 0) {
    BluetoothP.println(F("Switching between selected song."));
    song.trackSelect = !song.trackSelect;
    DisplayP.write(Command::ClearScreen);
    DisplayP.write(Command::ClearScreen);
    DisplayP.write("Song ");
    DisplayP.write(char((song.trackSelect + 1) + 0x30));
    DisplayP.write(" selected.");
    DisplayP.blink(2);
  } else if (strcmp(command, "help") == 0) {
    BluetoothP.println(F("Welcome to this system integration lab..."));
    BluetoothP.println(F("Available commands are: dump, select, pause and help."));
  } else if (strcmp(command, "mode") == 0) {
    BluetoothP.println(F("Toggling LED mode."));
    DisplayP.write(Command::ClearScreen);
    DisplayP.write("Toggling LED mode.");
    DisplayP.blink(2);
    led_s.enable = !led_s.enable;
  } else if (strcmp(command, "pause") == 0) {
    BluetoothP.println("Sound " + String((soundStream.on ? "off" : "on")));
    DisplayP.write(Command::ClearScreen);
    if (soundStream.on)
      DisplayP.write("Sound is paused.");
    else
      DisplayP.write("Sound is playing.");
    soundStream.on = !soundStream.on;
  } else {
    BluetoothP.println(F("Invalid command entered, type help for more information. "));
  }
}

void trackToBluetooth(unsigned char *track) {
  unsigned pos = 0;
  char ch = track[pos];
  while (ch != '#' && pos < 32) {
    BluetoothP.print(ch);
    ch = track[++pos];
  }
}

void sendToTrack0(unsigned char *track) {
  unsigned pos = 0;
  char ch = track[pos];
  while (ch != '#' && pos < 32) {
    ch = song.track0[pos] = track[pos];
    ++pos;
  }
  song.track0[pos] = '#';
}
