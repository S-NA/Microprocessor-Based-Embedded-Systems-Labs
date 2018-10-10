struct force_resistor_s {
  unsigned value;
  const byte pin;
} force_resistor_def = { 0, A5 };
typedef struct force_resistor_s force_resistor;

struct signal_generator_s {
  unsigned long pwm_high;
  unsigned long pwm_low;
  unsigned long period;
  float frequency;
  unsigned long duty_cycle; /* likely uneeded */
  const byte pin;
} signal_generator_def = { 0, 0, 0, 0, 0, 8 };
typedef struct signal_generator_s signal_generator;

struct led_s {
  unsigned long pwm_high;
  unsigned long pwm_low;
  unsigned long period;
  float frequency;
  unsigned long duty_cycle;
  unsigned long currentTime;
  const byte pin;
} led_def = { 0, 0, 0, 0, 0, 0, 8 };
typedef struct led_s led;

force_resistor *ForceResistor = &force_resistor_def;
signal_generator *SignalGenerator = &signal_generator_def;
led *LED = &led_def;

void setup() {
  Serial.begin(9600);
  pinMode(ForceResistor->pin, INPUT);
  pinMode(SignalGenerator->pin, INPUT);
  pinMode(LED->pin, OUTPUT);
  LED->currentTime = millis(); /* zero... */
}

void loop() {
  /* Force resistor value increases with force to it, used for analogWrite() */
  ForceResistor->value = map(analogRead(ForceResistor->pin), 0, 1023, 0, 255);
  /* Signal generator, we will likely sample more to get a more accurate freq before.  */
  SignalGenerator->pwm_high = pulseIn(SignalGenerator->pin, HIGH);
  SignalGenerator->pwm_low = pulseIn(SignalGenerator->pin, LOW);
  SignalGenerator->period = (SignalGenerator->pwm_low + SignalGenerator->pwm_high);
  SignalGenerator->frequency = (1 / (SignalGenerator->period / 1000000.0));
  /* Duty cycle uneeded, keep in case we deal with this using registers instead of analogWrite() */
  SignalGenerator->duty_cycle = (float(SignalGenerator->pwm_high) / float(SignalGenerator->period)) * 100.0;

  /* if (currentTime + (500 / (SignalGenerator->frequency / 1000) > millis() ?... 
   *  analogWrite(SignalGenerator->pin, ForceResistor->value);
   *  on ? off : logic
   *  currentTime = millis()
   */

  Serial.println(ForceResistor->value);
  Serial.println(SignalGenerator->frequency);
}
