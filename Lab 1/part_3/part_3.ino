/**
 * Lab 1, Part 3 (25%)
 * Follow instructions on https://www.arduino.cc/en/Tutorial/Debounce so that
 * Remember: Show your lab instructor!
 */

/* macros to make easy changes */
#define BTN_PIN 2
#define LED_PIN LED_BUILTIN

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

struct sound_s {
  const byte    output;
  bool          playing;
  uint8_t       position;
} snd_def = {11, false, 0};
typedef sound_s snd;


/* initalize structs with default values. */
btn *Toggle   = &btn_def;
led *LED      = &led_def;
dbn *Debounce = &dbn_def;
snd *Sound    = &snd_def;

void playMusic() {
  char note[] = "CDEFGABH";
  uint8_t pitch[] = { 0, 2, 4, 5, 7, 9, 11, 12 };
  uint8_t songs[] = {262, 164, 262, 164, 262, 164 };
  double  freq[]  = { 261.6, 293.6, 329.6, 349.2, 392, 440, 493.8, 523.2 };
  tone(Sound->output, songs[Sound->position], 4 * 1.30);
  Sound->position += 1;
}

void setup() {
  pinMode(Toggle->pin, INPUT_PULLUP);
  pinMode(LED->pin, OUTPUT);
  digitalWrite(
      LED->pin,
      LED->state); /* Our inital default for the LED is off, i.e. 0*/
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

  if (Sound->playing) playMusic();
  
  digitalWrite(LED->pin, LED->state);
  Toggle->prev_state = Toggle->pressed;
}
