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
  const byte pin;
} signal_generator_def = { 0, 0, 0, 0, 8 };
typedef struct signal_generator_s signal_generator;

struct led_s {
  float frequency;
  unsigned long currentTime;
  bool toggle;
  const byte pin;
} led_def = { 0, 0, 0, 10 };
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
  LED->frequency = SignalGenerator->frequency;
  if (LED->currentTime + (500 / (LED->frequency/1000)) > millis()) {
    if (LED->toggle) analogWrite(LED->pin, ForceResistor->value);
    else analogWrite(LED->pin, 0);
  } else {
    LED->toggle = !LED->toggle;
    LED->currentTime = millis();
  }
}
