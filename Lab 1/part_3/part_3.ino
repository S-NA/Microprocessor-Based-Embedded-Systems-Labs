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
  uint8_t pitch = get_pitch(composition[Sound->position]);
  unsigned int durations[] = { 280, 560, 280, 140, 280, 560, 280, 140 };
  double frequency = 261.2 * pow(2.0, pitch / 12.0);
  tone(Sound->output, frequency, durations[Sound->position++]);
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

  if (Sound->playing)
    play_music(
        "hcbdafge"); /* string's if is hard-coded to handle a max of only 8
                        notes. */
  /* TODO: add support for strings > 8 notes.*/

  digitalWrite(LED->pin, LED->state);
  Toggle->prev_state = Toggle->pressed;
}
