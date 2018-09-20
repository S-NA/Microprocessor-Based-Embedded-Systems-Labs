/**
 * Lab 2, Part 1 (5%)
 * Develop a UNO sketch that reads two values, pin and value, from the PC
 * keyboard by  using the IDE serial monitor and then calls an Arduino function
 * analogWrite(pin, value),  where pin is 3, 5, 6, 9, 10, or 11, while value is
 * 0, 1, 2, …, 255, so as to generate a square wave on the corresponding digital
 * pin. Use an oscilloscope to observe the waveforms on digital pins. Record the
 * frequency on each pin.√  Note that UNO digital pins 5 and 6 are connected to
 * the 8-bit timer 0, pins 9 and 10 are connected to the 16-bit timer 1, and
 * pins 3 and 11 are connected to the 8-bit timer 2.
 *
 * Remember: Show your lab instructor! (They will want the freq. of each pin.)
 */
struct param_s {
  uint8_t pins[6];
  uint8_t values[256];
} param_def = {{3, 5, 6, 9, 10, 11}, {}};
typedef struct param_s param;

param *Parameters = &param_def;

void setup() {
  /* Set baud rate for serial communication, e.g. PC->Arduino. */
  Serial.begin(9600);

  /* Set up all our pins into output mode. */
  for (uint8_t pin : Parameters->pins) {
    pinMode(pin, OUTPUT);
  }

  /* Initalize the values 0..255, since we have no short hand.
   * Warning, uint16_t only holds up to 65536.
   * sizeof(arr)/sizeof(type of arr) = ordinal count. */
  uint16_t size = sizeof(Parameters->values) / sizeof(uint8_t);
  for (uint16_t indx = 0; indx < size; indx++)
    Parameters->values[indx] = indx;
}

void loop() {
  for (uint8_t pin : Parameters->pins) {
    for (uint8_t value : Parameters->values)
      analogWrite(pin, value);
    delay(5000); /* Delay five seconds to write down freq. */
  }
}