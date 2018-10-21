#include "NHD-0216K3Z-FL-GBW-V3.hpp"

NHD0216K3Z::NHD0216K3Z(pin_t data, pin_t clock, pin_t slave_select)
    : data_pin{data}, clock_pin{clock}, slave_select_pin{slave_select} {
  init();
}

/**
 * Kindly sourced from Prof.
 */
void NHD0216K3Z::charWrite(byte ch, uint8_t bit_order) const noexcept {
  byte compareValue = 0x80;
  // initialize compareValue
  if (bit_order == MSBFIRST) {
    compareValue = 0x80;
  } else {
    compareValue = 0x01;
  }

  // enable slave select
  digitalWrite(slave_select_pin, LOW);

  // shift out data
  for (uint8_t i = 0; i < 8; ++i) {
    // send bit to data pin
    digitalWrite(data_pin, (ch & compareValue) ? HIGH : LOW);

    // shift compare value
    if (bit_order == MSBFIRST) {
      compareValue = compareValue >> 1;
    } else {
      compareValue = compareValue << 1;
    }

    // trigger clk rising edge(toggle low then high)
    digitalWrite(clock_pin, LOW);
    // wait before trigger
    delayMicroseconds(4);
    digitalWrite(clock_pin, HIGH);
    // wait, LCD can only handle up to 100KHz
    delayMicroseconds(14);
  }
  // disable slave select
  digitalWrite(slave_select_pin, HIGH);
}

cmd_t get_cmd_value(Command cmd) {
  /* NHD-0216K3Z-FL-GBW-V3.pdf page 7 */
  static const cmd_t lookup[22] = {
      0x41, 0x42, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4E,
      0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x61, 0x62, 0x70, 0x71, 0x72};
  return lookup[cmd];
}

unsigned get_cmd_delay(Command cmd) {
  switch (cmd) {
  case cursor_home:
  case underline_cursor_on:
  case underline_cursor_off:
  case clear_screen:
    return 2000;
  case set_contrast:
    return 501;
  case load_custom_character:
    return 201;
  case change_rs232_baud_rate:
  case change_i2c_address:
    return 3500;
  case display_firmware_version_number:
  case display_i2c_address:
    return 4500;
  case display_rs232_baud_rate:
    return 10500;
  default:
    return 110;
  }
}

void NHD0216K3Z::cmdWrite(Command cmd) const noexcept {
  unsigned delay_time = get_cmd_delay(cmd); /* in μs */
  charWrite(0xFE);                          /* command prefix */
  charWrite(get_cmd_value(cmd));            /* actual command */
  delayMicroseconds(delay_time);
}

/**
 * cmdWrite(Command, uint8_t * const)
 * Takes a Command (enum), and an array of parameters.
 */
void NHD0216K3Z::cmdWrite(Command cmd, const byte *pms, uint8_t len) const
    noexcept {
  unsigned delay_time = get_cmd_delay(cmd); /* in μs */
  charWrite(0xFE);                          /* command prefix */
  charWrite(get_cmd_value(cmd));            /* actual command */
  for (uint8_t i = 0; i < len; ++i)
    charWrite(pms[i]);
  delayMicroseconds(delay_time);
  // cmdWrite(backspace); /* delete magical garbage? why? */
}

void NHD0216K3Z::strWrite(const char *str) const noexcept {
  uint8_t i = 0;
  while (str[i]) {
    charWrite(str[i++]);
  }
}

void NHD0216K3Z::init() const noexcept {
  pinMode(data_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(slave_select_pin, OUTPUT);
}