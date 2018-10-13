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

/**
 * Calculations are thrown off for when measuring PWM if the functions are not
 * inlined.
 */
inline void do_generator_pin_10_fast_pwm(void) __attribute__((always_inline));
inline void get_frequency_and_duty(void) __attribute__((always_inline));
inline float get_relative_delta(float, float) __attribute__ ((always_inline));
inline uint8_t get_relative_delta(uint8_t, uint8_t) __attribute__ ((always_inline));

const unsigned generator_pin = 10;
const unsigned measure_pin = 8;

/* Table 19-9. Waveform Generation Mode Bit Description.
 * Mode 3, Fast PWM, TOP: 0xFF, TOV Flag set on MAX.*/
struct timer1_s {
  const float max_top;
  long prescaler;
  float n_top;
  float top;
  float freq;
  float duty;
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
   * bit(x) = (1UL << x) */
  TCCR1A = bit(COM1B1) | bit(WGM11); /* select OCR1B, and fast PWM */
  TCCR1B = bit(CS11)   | bit(WGM13) | bit(WGM12); /* prescaler, and fast pwm */

  ICR1 = F_CPU / 1000 / 8; /* F_CPU / FREQ / PRESCALER */
  OCR1B = ICR1 * .5; /* FREQ / %DUTY */;
}

void do_generator_pin_10_fast_pwm() {
  Serial.println("Setting generator_pin 10...");
  Timer1->n_top = F_CPU / Timer1->freq;
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
  Timer1->top = Timer1->n_top / Timer1->prescaler - 1;
  unsigned long duty_cycle = (Timer1->duty / 100) * Timer1->top;
  ICR1 = Timer1->top;
  OCR1B = duty_cycle;
}

float get_relative_delta(float a, float b) {
  return ((max(a, b) - min(a, b)) / max(a, b)) * 100.0;
}

uint8_t get_relative_delta(uint8_t a, uint8_t b) {
  return ((max(a, b) - min(a, b)) / max(a, b)) * 100;
}

void get_frequency_and_duty() {
  const byte cycles = 255;
  Sample->pwm_low = 0;
  Sample->pwm_high = 0;
  for (byte i = 0; i < cycles; ++i) {
    Sample->pwm_low += pulseIn(measure_pin, LOW);
    Sample->pwm_high += pulseIn(measure_pin, HIGH);
  }
  Sample->pwm_low  /= cycles;
  Sample->pwm_high /= cycles;

  Sample->period = Sample->pwm_low + Sample->pwm_high;
  Sample->curr_freq = (1 / (Sample->period / 1000000.0));
  Sample->curr_duty = (Sample->pwm_high / float(Sample->period)) * 100.0;
}

void loop() {
  if (Serial.available() >= 2) {
    Timer1->freq = Serial.parseFloat();
    Timer1->duty = Serial.parseInt();
    /* Clear the serial line of junk. */
    while (Serial.available()) { Serial.read(); }
    do_generator_pin_10_fast_pwm();
    /* String() does not like long double... */
    Serial.println("Setting Timer1->freq = " + String(Timer1->freq) + " Timer1->duty = " + String(Timer1->duty));
  }

  get_frequency_and_duty();

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

