// https://playground.arduino.cc/code/keypad

#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'}
};
byte rowPins[ROWS] = {5, 6, 7, 8};
byte colPins[COLS] = {2, 3, 4};

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
int note;
double pitch;
unsigned long loop_count;
unsigned long start_time;
byte speaker = 11;
int note_vals[] = {0, 2, 4, 5, 7, 9, 11, 12};
int octive = 1;
const int touch_sensor_pin = -1;
const int button_pin = 12;
int touch_sensor_pressed = 0;
int lasttouch_sensor_state = 0;
int touch_sensor_state = 0;
int touch_sensor_released = 1;
unsigned long last_debounce_time = 0;
unsigned long debounce_delay = 10;

/* btn { pressed, state, prev_state, pin } */
struct btn_s {
  int       pressed;
  int       state;
  int       prev_state;
  const int pin;
} btn_def = {1, 1, 1, 12};
typedef struct btn_s btn;


void setup() {
  Serial.begin(9600);
  loop_count = 0;
  start_time = millis();
  pinMode(touch_sensor_pin, INPUT_PULLUP);
}
void loop() {
  loop_count++;

  touch_sensor_state = digitalRead(touch_sensor_pin);

  touch_sensor();
  if (kpd.getKeys()) {
    for (int i = 0; i < LIST_MAX; i++) {
      if (kpd.key[i].stateChanged) {
        Serial.println("Key pressed.");
        if (kpd.key[i].kchar >= '1' && kpd.key[i].kchar <= '8') {
          note = kpd.key[i].kchar - '1';
          pitch = octive * 261.2 * pow(2.0, (note_vals[note] / 12.0));
          switch (kpd.key[i].kstate) {
            case PRESSED:
              tone(speaker, pitch);
              break;
            case HOLD:
              break;
            case RELEASED:
              noTone(speaker);
              break;
            case IDLE:
              break;
            default:
              break;
          }
        }
      }
    }
  }
}

void button() {

}

void touch_sensor() {
  int reading = digitalRead(touch_sensor_pin);
  while (reading == LOW) {
    delay(10);
    reading = digitalRead(touch_sensor_pin);
    if (reading != LOW) {
      Serial.println("Octave change.");
      octive = (octive == 1) ? 2 : 1;
    }
  }
}

