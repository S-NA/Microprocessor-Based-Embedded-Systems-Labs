/**
 * Lab 1, Part 1 (5%)
 * Connect the UNO to a USB port of the PC. Create a project that turns
 * on or off the on-board LED  depending on whether a push-button connected to a
 * digital input pin of the board is depressed or not. In  addition, whenever
 * the push-button is depressed and then released, a letter “A” should be
 * displayed inside  the IDE serial monitor window. Make sure the serial
 * monitor and the code use the same baud rate.  No need  to demonstrate this
 * code.
 */

/* macros to make easy changes */
#define BTN_PIN 2
#define LED_PIN LED_BUILTIN

/* btn { pressed, state, prev_state, pin } */
struct btn_s {
  int pressed;
  const int pin;
} btn_def = {1, BTN_PIN};
typedef struct btn_s btn;

/* led { state, pin } */
struct led_s {
  int state;
  const int pin;
} led_def = {0, LED_PIN};
typedef struct led_s led;

/* initalize structs with default values. */
btn *Button   = &btn_def;
led *LED      = &led_def;

void setup() {
  pinMode(Button->pin, INPUT_PULLUP);
  pinMode(LED->pin, OUTPUT);
  digitalWrite(
      LED->pin,
      LED->state); /* Our inital default for the LED is off, i.e. 0*/
  Serial.begin(9600);
}

void loop() {
  Button->pressed = digitalRead(Button->pin);
  if (!Button->pressed) {
    LED->state = HIGH;
    Serial.println("A");
  } else {
    LED->state = LOW;
  }
  digitalWrite(LED->pin, LED->state);
}
