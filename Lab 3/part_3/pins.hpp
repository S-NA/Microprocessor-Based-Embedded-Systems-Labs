namespace lcd {
namespace pin {
const byte data = 13;
const byte clock = 12;
const byte slaveSelect = 10;
} // namespace pin
} // namespace lcd

namespace keypad {
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
    {'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'}};
byte rowPins[ROWS] = {6, 5, 4, 3}; // last four pins, then space
byte colPins[COLS] = {A0, 8, 7};    // first three pins, then space
} // namespace keypad

namespace bluetooth {
namespace pin {
const byte tx = 2;
const byte rx = 11;
} // namespace pin
} // namespace bluetooth

namespace speaker {
const byte pin = A5;
}

namespace led {
const byte pin = A4;
}

namespace forcesensitiveresistor {
const byte pin = A3;
}

namespace signalgenerator {
const byte pin = 9;
}

