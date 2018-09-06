/**
 * Lab 1, Part 2 (10%)
 * Follow instructions on https://www.arduino.cc/en/Tutorial/Debounce so that
 * the LED is toggled  whenever the push-button is depressed and then released.
 * (Toggling occurs when the button is released.) You need to handle the
 * debouncing with software.
 * 
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

/* initalize structs with default values. */
btn *Toggle   = &btn_def;
led *LED      = &led_def;
dbn *Debounce = &dbn_def;

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
      }
    }
  }

  digitalWrite(LED->pin, LED->state);
  Toggle->prev_state = Toggle->pressed;
}
