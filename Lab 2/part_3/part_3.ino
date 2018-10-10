/**
 * Lab 2, Part 3 (10%)
 * Modify the previous sketch so that the square wave is generated on digital
 * pin 10 instead. Again use the fast PWM mode. Use an oscilloscope to observe
 * the waveforms. Record the frequency range that you are able to support. The
 * range should be as large as possible.√ Note that pin 10 is connected to the
 * timer 1 channel B output compare pin.
 *
 * Remember: Show your lab instructor!
 * (They will want the range of frequencies you can generate.)
 */

const unsigned pin = 10;

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

timer1 *Timer1    = &timer1_def;

void setup() {
  /* Set baud rate for serial communication, e.g. PC->Arduino. */
  Serial.begin(9600);

  /* Set our pin into output mode. */
  pinMode(pin, OUTPUT);

  /* Note, we overwrite the entire register to set the mode.
   * _BV(x) = x bit value */
  TCCR1A = _BV(COM1B1) | _BV(WGM11) | _BV(WGM11); /* mode */
  TCCR1B = _BV(CS11)   | _BV(WGM13) | _BV(WGM12); /* mode */

  ICR1 = 0x07D0; /* 2000 */
  OCR1B = 0x02FE; /* 766 */
}

void do_pin_10_fast_pwm(long double freq, long double duty) {
  Serial.println("Setting pin 10...");
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

  Serial.println(Timer1->max_top);
  Serial.println(Timer1->prescaler);
  Serial.println(Timer1->n_top);
  Serial.println(Timer1->top);
  Serial.println(Timer1->freq);
  Serial.println(Timer1->duty);
  Serial.println(Timer1->mode);
  Serial.println(OCR1B);
}

void loop() {
  if (Serial.available() >= 2) {
    Timer1->freq = double(Serial.parseFloat());
    Timer1->duty = Serial.parseInt();
    /* Clear the serial line of junk. */
    while (Serial.available()) { Serial.read(); }
    do_pin_10_fast_pwm(Timer1->freq, Timer1->duty);
    /* String() does not like long double... */
    Serial.println("Timer1->freq = " + String(double(Timer1->freq)) + " Timer1->duty = " + String(double(Timer1->duty)));
  }
}
