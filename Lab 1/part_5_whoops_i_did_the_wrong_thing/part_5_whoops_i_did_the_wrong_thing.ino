/**
 * Lab 1, Part 3 (25%)
 * Connect a speaker to the UNO board (through a resistor if too loud) and
 * modify the previous program  so as to play through a short song segment
 * repetitively. The push-button allows the toggling between playing of the song
 * and turning the song off. The program should be designed so that the song can
 * be represented as a character string (or multiple character strings) and it
 * should include tones of different duration√√. Note that a character string is
 * the same as an array of characters. You may impose conditions on how long the
 * push-button needs to be pressed down.
 *
 * Conditions:
 * Hold the button for at least 1.75 second or change the note duration and
 * delay.
 *
 * Remember: Show your lab instructor!
 */

/* macros to make easy changes */
#define BTN_PIN 2
#define LED_PIN LED_BUILTIN

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
char music_composition[] = "abcdefgh";
int note;
double pitch;

/* btn { pressed, state, prev_state, pin } */
struct btn_s {
  int       pressed;
  int       state;
  int       prev_state;
  const int pin;
} btn_def = {1, 1, 1, BTN_PIN};
typedef struct btn_s btn;

/* led { state, pin } */
struct led_s {
  int       state;
  const int pin;
} led_def = {0, LED_PIN};
typedef struct led_s led;

/* debounce { delay, last_time } */
struct debounce_s {
  const unsigned delay;
  unsigned long  last_time; /* old millis() */
} dbn_def = {10, 0};
typedef debounce_s dbn;

struct sound_s {
  const byte output;
  bool       playing;
  uint8_t    position;
} snd_def = {11, false, 0};
typedef sound_s snd;

/* initalize structs with default values. */
btn *Toggle   = &btn_def;
led *LED      = &led_def;
dbn *Debounce = &dbn_def;
snd *Sound    = &snd_def;

uint8_t get_pitch(char note) {
  static const uint8_t lookup[8] = {
      0x9 /* a */
      ,
      0xB /* b */
      ,
      0x0 /* c */
      ,
      0x2 /* d */
      ,
      0x4 /* e */
      ,
      0x5 /* f */
      ,
      0x7 /* g */
      ,
      0xC /* C */
  };      /* Numerical pitch mapped to note. */
  return lookup[note - 'a'];
}

// typedef const char* song;

void play_music(const char *composition) {
  uint8_t pitch = get_pitch(composition[Sound->position++]);
  double frequency = 261.2 * pow(2.0, pitch / 12.0);
  tone(Sound->output, frequency, 500);
  if (Sound->position < 8) {
    //    TODO: Clean this up.
  } else {
    Sound->position = 0;
  }
  delay(1000);
  noTone(Sound->output);
}

void setup() {
  pinMode(Toggle->pin, INPUT_PULLUP);
  pinMode(LED->pin, OUTPUT);
  digitalWrite(LED->pin,
               LED->state); /* Our inital default for the LED is off, i.e. 0*/
  Serial.begin(9600);
}

void loop() {
  Toggle->pressed = digitalRead(Toggle->pin);

  if ((Toggle->prev_state) != (Toggle->pressed)) {
    /* If the last button state does not match the current reading, update
       last debounce time. */
    Debounce->last_time = millis();
  }

  /* currentUptime - oldUptime > delay */
  if ((millis() - (Debounce->last_time)) > (Debounce->delay)) {
    /* The button is still changed after delay. */
    if (Toggle->state != Toggle->pressed) {
      Toggle->state = Toggle->pressed;
      if (Toggle->state) {
        LED->state = !LED->state;
        Sound->playing = !Sound->playing;
      }
    }
  }

    if (kpd.getKeys()) {
    for (int i = 0; i < 8; i++) {
      if (kpd.key[i].stateChanged) {
        Serial.println("Key pressed.");
        if (kpd.key[i].kchar >= '1' && kpd.key[i].kchar <= '8') {
          note = kpd.key[i].kchar - '1';
          switch (note) {
            case 1:
              note = 'c';
              break;
              case 2:
              note = 'd';
              break;
              case 3:
              note = 'e';
              break;
              case 4:
              note = 'f';
              break;
              case 5:
              note = 'g';
              break;
              case 6:
              note = 'a';
              break;
              case 7:
              note = 'b';
              break;
              case 8:
              note = 'h';
              break;
          }
          pitch = 261.2 * pow(2.0, (get_pitch(note) / 12.0));
          switch (kpd.key[i].kstate) {
            case PRESSED:
            music_composition[i] = pitch;
//              tone(speaker, pitch);
              break;
            case HOLD:
              break;
            case RELEASED:
//              noTone(speaker);
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

  if (Sound->playing)
    play_music(
        music_composition); /* string's if is hard-coded to handle a max of only 8
                               notes. */

  /* TODO: add support for strings > 8 notes.*/

  digitalWrite(LED->pin, LED->state);
  Toggle->prev_state = Toggle->pressed;
}
