/**
 * Lab 1, Part 5 (30%)
 * Combine the previous two projects. The keypad is used to enter a new song
 * segment which replaces a previous song segment. You need to support the input
 * of tones at least from C to high C. The push-button still controls whether to
 * play the song. Demonstrate the ability to repetitively enter and play new
 * song
 * segments√√.
 *
 * Conditions:
 * Hold the button for at least 1.75 second or change the note duration and
 * delay.
 * Toggled the song to stop playing and press the pound symbol on the keypad to
 * program in a replacement composition.
 *
 * Remember: Show your lab instructor!
 */

/* Macros to make easy changes. */
#define BTN_PIN 12
#define LED_PIN LED_BUILTIN

/* Library taken from: <https://playground.arduino.cc/code/keypad> */
#include <Keypad.h>
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
    {'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'}};
byte rowPins[ROWS] = {5, 6, 7, 8};
byte colPins[COLS] = {2, 3, 4};
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/* btn { pressed, state, prev_state, pin } */
struct btn_s {
  int pressed;
  int state;
  int prev_state;
  const int pin;
} btn_def = {1, 1, 1, BTN_PIN};
typedef struct btn_s btn;

/* led { state, pin } */
struct led_s {
  int state;
  const int pin;
} led_def = {0, LED_PIN};
typedef struct led_s led;

/* debounce { delay, last_time } */
struct debounce_s {
  const unsigned delay;
  unsigned long last_time; /* old millis() */
} dbn_def = {10, 0};
typedef debounce_s dbn;

/* sound { output, playing, position }*/
struct sound_s {
  const byte output;
  bool playing;
  uint8_t position;
} snd_def = {11, false, 0};
typedef sound_s snd;

/* music { composition, frequency, duration, octave, position } */
struct music_s {
  char composition[8];
  double frequency[8];
  double octave_frequency[8];
  uint8_t duration[8];
  uint8_t octave;
  uint8_t position;
} msc_def = {{'c', 'd', 'e', 'f', 'g', 'a', 'b', 'h'}, //{'h', 'd', 'd', 'f', 'a', 'f', 'g', 'h'},
             {},
             {},
             {16, 8, 8, 16, 8, 8, 16, 8},
             1,
             0};
typedef music_s msc;

/* Initalize structs with default values. */
btn *Toggle   = &btn_def;
led *LED      = &led_def;
dbn *Debounce = &dbn_def;
snd *Sound    = &snd_def;
msc *Music    = &msc_def;

uint8_t get_pitch(char note) {
  static const uint8_t lookup[8] =
  { 0x9 /* a */
  , 0xB /* b */
  , 0x0 /* c */
  , 0x2 /* d */
  , 0x4 /* e */
  , 0x5 /* f */
  , 0x7 /* g */
  , 0xC /* h (High C) */
  };      /* Numerical pitch mapped to note. */
  if (note > '8')
    return lookup[note - 'a'];
  return lookup[note - '1'];
}

/**
 * update_music(char*) - Given a char ptr, generate associated frequencies.
 * TODO:
 * We might want to also generate the higher octave frequencies by default.
 * This would allow us to not have more abstraction logic for dealing with
 * the touch sensor.
 */
void update_music(char *composition) {
  for (int i = 0; i < 8; i++) {
    uint8_t pitch = get_pitch(composition[i]);
    Music->octave_frequency[i] = 261.2 * pow(2, (2 * pitch) / 12.);
    Music->frequency[i] = 261.2 * pow(2, pitch / 12.);
  }
  Sound->position = 0; /* Sound device should start anew. */
  Music->position = 0; /* Ya, know I am not sure... */
}

void play_music() {
  uint8_t length = Music->duration[Sound->position];
  if (digitalRead(9) == HIGH) { /* Touch pad sensor, check. */
    tone(Sound->output, Music->octave_frequency[Sound->position++], 70 * length);
  } else {
    tone(Sound->output, Music->frequency[Sound->position++], 70 * length);
  }
  delay(10 * length);
  noTone(Sound->output);
  if (Sound->position > 7)
    Sound->position = 0;
}

void print_music_composition() {
  for (int i = 0; i < 8; i++) {
    Serial.print(Music->composition[i]);
  }
  Serial.println();
}

void setup() {
  pinMode(Toggle->pin, INPUT_PULLUP);
  pinMode(LED->pin, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, INPUT_PULLUP);
  digitalWrite(10, HIGH);
  digitalWrite(LED->pin,
               LED->state); /* Our inital default for the LED is off, i.e. 0*/
  update_music(Music->composition); /* Generates default frequencies. */
  Serial.begin(9600);
}

void loop() {
  Toggle->pressed = digitalRead(Toggle->pin);

  if ((Toggle->prev_state) != (Toggle->pressed)) {
    /* If the last button state does not match the current reading, update
     * last debounce time. */
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

  digitalWrite(LED->pin, LED->state);

  /**
   * The following code physically means, if the button toggles the sound off.
   * If the user then presses the pound key on the keypad, then read in 8 keys.
   * waitForKey() is a blocking call.
   *
   * TODO: Use a for-loop.
   */
  if (!Sound->playing) {
    noTone(Sound->output); /* TODO: does this improve the responsiveness? */
    char key = kpd.getKey();
    if (key == '#') {
      Music->composition[0] = kpd.waitForKey();
      Music->composition[1] = kpd.waitForKey();
      Music->composition[2] = kpd.waitForKey();
      Music->composition[3] = kpd.waitForKey();
      Music->composition[4] = kpd.waitForKey();
      Music->composition[5] = kpd.waitForKey();
      Music->composition[6] = kpd.waitForKey();
      Music->composition[7] = kpd.waitForKey();

      update_music(Music->composition);
      Serial.println("Music composition updated to: ");
      print_music_composition();
    }
  }

  /**
   * TODO: We might want to look into adding the octave support here.
   * if (Sound->player && digitalRead(touch_sensor) == HIGH) ...
   */
  if (Sound->playing) {
    play_music();
  }

  Toggle->prev_state = Toggle->pressed;
}
