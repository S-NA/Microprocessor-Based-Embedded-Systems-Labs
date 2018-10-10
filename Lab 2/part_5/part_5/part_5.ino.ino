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
  unsigned long duty_cycle;
  const byte pin;
} signal_generator_def = { 0, 0, 0, 0, 0, 8 };
typedef struct signal_generator_s signal_generator;

struct led_s {
  unsigned long pwm_high;
  unsigned long pwm_low;
  unsigned long period;
  float frequency;
  unsigned long duty_cycle;
  const byte pin;
} led_def = { 0, 0, 0, 0, 0, 8 };
typedef struct led_s led;

force_resistor *ForceResistor = &force_resistor_def;
signal_generator *SignalGenerator = &signal_generator_def;
led *LED = &led_def;

void setup() {
  Serial.begin(9600);
  pinMode(ForceResistor->pin, INPUT);
  pinMode(SignalGenerator->pin, INPUT);
  TCCR1A = _BV(COM1B1) | _BV(WGM11) | _BV(WGM11); /* mode */
  TCCR1B = _BV(CS11)   | _BV(WGM13) | _BV(WGM12); /* mode */

  ICR1 = 0x07D0; /* 2000 */
  OCR1B = 0x02FE; /* 766 */
}

void loop() {
  ForceResistor->value = map(analogRead(ForceResistor->pin), 0, 1023, 0, 255);
  /* Signal Generator */
  SignalGenerator->pwm_high = pulseIn(SignalGenerator->pin, HIGH);
  SignalGenerator->pwm_low = pulseIn(SignalGenerator->pin, LOW);
  SignalGenerator->period = (SignalGenerator->pwm_low + SignalGenerator->pwm_high);
  SignalGenerator->frequency = (1 / (SignalGenerator->period / 1000000.0));
  SignalGenerator->duty_cycle = (float(SignalGenerator->pwm_high) / float(SignalGenerator->period)) * 100.0;

  Serial.println(ForceResistor->value);
  Serial.println(SignalGenerator->frequency);
}
