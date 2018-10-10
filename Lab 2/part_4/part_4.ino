/**
 * Lab 2, Part 4 (10%)
 * Modify the previous sketch so that, in addition to the previous
 * functionality, the code also continuously measures the frequency and duty
 * cycle of a square wave input from digital generator_pin 8, and then prints out the
 * measured values to the serial monitor, when the measured values are quite
 * different from the previous value. To verify the code operation, connect
 * generator_pin 10 to generator_pin 8 and use an oscilloscope to observe the waveforms.
 * √√ Hint: you may use the Arduino function pulseIn().
 *
 * Remember: Show your lab instructor!
 * (Verify the code in operation through the oscilloscope.)
 */

const unsigned generator_pin = 10;
const unsigned measure_pin = 8;

/* Table 19-9. Waveform Generation Mode Bit Description.
 * Mode 3, Fast PWM, TOP: 0xFF, TOV Flag set on MAX.*/
struct timer1_s {
  const double max_top;
  long prescaler;
  double n_top;
  double top;
  double freq;
  double duty;
  unsigned char mode;
} timer1_def = { 65536, 1, 0, 0, 0, 0, 0 };
typedef struct timer1_s timer1;

struct sample_s {
  unsigned long pwm_high;
  unsigned long pwm_low;
  unsigned long period;
  const byte pin;
  float curr_freq;
  float prev_freq;
  uint8_t curr_duty;
  uint8_t prev_duty;
  uint8_t relative_delta;
} sample_def = { 0, 0, 0, 8, 0, 0, 0, 0, 25 };
typedef struct sample_s sample;

timer1 *Timer1 = &timer1_def;
sample *Sample = &sample_def;

void setup() {
  /* Set baud rate for serial communication, e.g. PC->Arduino. */
  Serial.begin(9600);

  /* Set our generator_pin into output mode. */
  pinMode(generator_pin, OUTPUT);

  /* Note, we overwrite the entire register to set the mode.
   * _BV(x) = x bit value */
  TCCR1A = _BV(COM1B1) | _BV(WGM11) | _BV(WGM11); /* mode */
  TCCR1B = _BV(CS11)   | _BV(WGM13) | _BV(WGM12); /* mode */

  ICR1 = 0x07D0; /* 2000 */
  OCR1B = 0x02FE; /* 766 */
}

void do_generator_pin_10_fast_pwm(long double freq, long double duty) {
  Serial.println("Setting generator_pin 10...");
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
  /* TODO: Fix to handle floating point numbers. (0.5, 0.24, ...) */
  Timer1->top = 16000000.0 / freq / Timer1->prescaler - 1;
  unsigned int duty_cycle = (duty / 100) * Timer1->top;
  ICR1 = Timer1->top;
  OCR1B = duty_cycle;
}

float get_relative_delta(float a, float b) {
  return ((max(a, b) - min(a, b)) / max(a, b)) * 100.0;
}

uint8_t get_relative_delta(uint8_t a, uint8_t b) {
  return ((max(a, b) - min(a, b)) / max(a, b)) * 100;
}

void loop() {
  if (Serial.available() >= 2) {
    Timer1->freq = double(Serial.parseFloat());
    Timer1->duty = Serial.parseInt();
    /* Clear the serial line of junk. */
    while (Serial.available()) { Serial.read(); }
    do_generator_pin_10_fast_pwm(Timer1->freq, Timer1->duty);
    /* String() does not like long double... */
    Serial.println("Setting Timer1->freq = " + String(double(Timer1->freq)) + " Timer1->duty = " + String(double(Timer1->duty)));
  }
  const byte cycles = 64;
  Sample->pwm_low = 0;
  Sample->pwm_high = 0;
  for (byte i = 0; i < cycles; i++) {
    Sample->pwm_low += pulseInLong(measure_pin, LOW);
    Sample->pwm_high += pulseInLong(measure_pin, HIGH);
  }
  Sample->pwm_high /= cycles;
  Sample->pwm_low /= cycles;

  Sample->period = (Sample->pwm_low + Sample->pwm_high);
  Sample->curr_freq = (1 / (Sample->period / 1000000.0));
  Sample->curr_duty = (float(Sample->pwm_high) / float(Sample->period)) * 100.0;

  if (isfinite(Sample->curr_freq) && isfinite(Sample->prev_freq) && isfinite(Sample->curr_duty) && isfinite(Sample->prev_duty)) {
    float frequency_delta = get_relative_delta(Sample->curr_freq, Sample->prev_freq);
    uint8_t duty_delta = get_relative_delta(Sample->curr_duty, Sample->prev_duty);
    if (((frequency_delta) >= Sample->relative_delta || (duty_delta) >= Sample->relative_delta)) {
      Serial.println("Measured values: (" + String(Sample->curr_freq) + ", " + String(Sample->curr_duty) + ")");
    }
  }

  Sample->prev_freq = Sample->curr_freq;
  Sample->prev_duty = Sample->curr_duty;
  
}

