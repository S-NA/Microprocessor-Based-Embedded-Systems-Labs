/**
 * Lab 2, Part 4 (10%)
 * Modify the previous sketch so that, in addition to the previous
 * functionality, the code also continuously measures the frequency and duty
 * cycle of a square wave input from digital pin 8, and then prints out the
 * measured values to the serial monitor, when the measured values are quite
 * different from the previous value. To verify the code operation, connect
 * pin 10 to pin 8 and use an oscilloscope to observe the waveforms.
 * √√ Hint: you may use the Arduino function pulseIn().
 *
 * Remember: Show your lab instructor!
 * (Verify the code in operation through the oscilloscope.)
 */

/* generator_pin is slightly useless due to function being tied to pin 10. */
const uint8_t generator_pin = 10;
const uint8_t sensor_pin    =  8;

/* Table 19-9. Waveform Generation Mode Bit Description.
 * Mode 3, Fast PWM, TOP: 0xFF, TOV Flag set on MAX.*/
struct timer1_s {
  const uint32_t max_top;
  uint16_t       prescaler;
  double n_top;
  double top;
  double freq;
  uint8_t     duty;
  unsigned char mode;
} timer1_def = { 65536, 1, 0, 0, 0, 0, 0 };
typedef struct timer1_s timer1;

struct sample_s {
  const uint8_t relative_delta;
  double  prev_freq;
  double  curr_freq;
  uint8_t prev_duty;
  uint8_t curr_duty;
} sample_def = { 5, 0, 0, 0, 0 };

timer1   *Timer1  = &timer1_def;
sample_s *Sample  = &sample_def;

void setup() {
  /* Set baud rate for serial communication, e.g. PC->Arduino. */
  Serial.begin(9600);

  /* Set our generator pin into output mode. */
  pinMode(generator_pin, OUTPUT);
  pinMode(sensor_pin, INPUT); /* We read in data. */

  /* Note, we overwrite the entire register to set the mode.
   * _BV(x) = x bit value */
  TCCR1A = _BV(COM1B1) | _BV(WGM11) | _BV(WGM11); /* mode */
  TCCR1B = _BV(CS11)   | _BV(WGM13) | _BV(WGM12); /* mode */

  ICR1 = 0x07D0; /* 2000 */
  OCR1B = 0x02FE; /* 766 */
}

void do_pin_10_fast_pwm(double freq, uint8_t duty) {
  Serial.println("Setting pin " + String(generator_pin) + "...");
  Timer1->n_top = 16000000.0 / freq;
  if (Timer1->n_top <= 1 * Timer1->max_top) Timer1->prescaler = 1;
  else if (Timer1->n_top <= 8 * Timer1->max_top) Timer1->prescaler = 8;
  else if (Timer1->n_top <= 64 * Timer1->max_top) Timer1->prescaler = 64;
  else if (Timer1->n_top <= 256 * Timer1->max_top) Timer1->prescaler = 256;
  else Timer1->prescaler = 1024;

  switch (Timer1->prescaler) {
  case 1: Timer1->mode = 0x01;
    break;
  case 8: Timer1->mode = 0x02;
    break;
  case 64: Timer1->mode = 0x03;
    break;
  case 256: Timer1->mode = 0x04;
    break;
  case 1024: Timer1->mode = 0x05;
    break;
  default: return;
  }

  TCCR1B = (TCCR1B & 0b11111000) | Timer1->mode;
  Timer1->top = 16000000.0 / freq / Timer1->prescaler - 1;
  uint8_t duty_cycle = (duty / 100) * Timer1->top;
  ICR1 = Timer1->top;
  OCR1B = duty_cycle;
}

double get_relative_delta(double a, double b) {
  return ((max(a, b) - min(a, b)) / max(a, b)) * 100.0;
}

uint8_t get_relative_delta(uint8_t a, uint8_t b) {
  return ((max(a, b) - min(a, b)) / max(a, b)) * 100;
}

void get_frequency_and_duty(uint8_t sensor_pin, double &freq, uint8_t &duty) {
  unsigned long pwm_high = pulseIn(sensor_pin, HIGH);
  unsigned long pwm_low  = pulseIn(sensor_pin, LOW);
  /* microseconds to seconds (by dividing by 1 million). */
  double period   = (pwm_high + pwm_low) / 1000000;
  freq = 1 / period;
  duty = (pwm_high / 1000000.0) / period;
}

void loop() {
  if (Serial.available() >= 2) {
    Timer1->freq = double(Serial.parseFloat());
    Timer1->duty = Serial.parseInt();
    /* Clear the serial line of junk. */
    while (Serial.available()) { Serial.read(); }
    do_pin_10_fast_pwm(Timer1->freq, Timer1->duty);
    Serial.println("Timer1->freq = " + String(Timer1->freq) + " Timer1->duty = " + String(Timer1->duty));
  }
  /**
   * Begin measuring a square wave input from digital pin 8 (sensor_pin).
   * When the instant change is > 5%, we print the measured values.
   */
  get_frequency_and_duty(sensor_pin, Sample->curr_freq, Sample->curr_duty);
  if (get_relative_delta(Sample->curr_freq, Sample->prev_freq) >= Sample->relative_delta
  ||  get_relative_delta(Sample->curr_duty, Sample->prev_duty) >= Sample->relative_delta) {
    Serial.println("Measured values: (" + String(Sample->curr_freq) + ", " + String(Sample->curr_duty) +")");
  }
  Sample->prev_freq = Sample->curr_freq;
  Sample->prev_duty = Sample->curr_duty;
}
