/**
 * NHD_0216K3Z_FL_GBW_V3.h
 * An Arduino Uno R3 library for NHD-0216K3Z-FL-GBW-V3
 *
 * Installation:
 * Copy both NHD_0216K3Z_FL_GBW_V3.{cpp,hpp} into
 * (OS X) ~/Documents/Arduino/libraries/libnhdlcd
 * (Linux) ~/Arduino/libraries/libnhdlcd
 * (Windows) C:\Users\%USERNAME%\My Documents\Arduino\libraries\libnhdlcd
 *
 * Copyright (c) 2018 S. Nordin Abouzahra <nordinabouzahra@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

/* Used internally. */
typedef uint8_t const pin_t;
typedef uint8_t cmd_t;

/* For better or for worse, this is gets thrown into the global namespace. */
enum Command {
  display_on,
  display_off,
  set_cursor,
  cursor_home,
  underline_cursor_on,
  underline_cursor_off,
  move_cursor_left_one_place,
  move_cursor_right_one_place,
  blinking_cursor_on,
  blinking_cursor_off,
  backspace,
  clear_screen,
  set_contrast,
  set_backlight_brightness,
  load_custom_character,
  move_display_left_one_place,
  move_display_right_one_place,
  change_rs232_baud_rate,
  change_i2c_address,
  display_firmware_version_number,
  display_rs232_baud_rate,
  display_i2c_address
};

class NHD0216K3Z {
private:
  pin_t data_pin;
  pin_t clock_pin;
  pin_t slave_select_pin;

public:
  NHD0216K3Z(pin_t data, pin_t clock, pin_t slave_select);
  void cmdWrite(Command cmd) const noexcept;
  void cmdWrite(Command cmd, const byte *pms, uint8_t len = 1) const noexcept;
  void charWrite(byte ch, uint8_t bit_order = MSBFIRST) const noexcept;
  void strWrite(const char *str) const noexcept;
  void init() const noexcept;
};

/* Generic class name. */
typedef NHD0216K3Z Display;
/* Allows (P){0x40, 0x42} otherwise (const byte[]) {...} */
typedef const byte P[];

/**
 * TODOs:
 *
 * For cmdWrite(Command cmd, const byte *pms, uint8_t len);
 * Instead of the anonymous array, perhaps use variadic arugments?
 *
 * For enum Command {...}, come up with a more elegant solution.
 * One that perhaps combines the value and the delay to reduce lookups.
 *
 * Document all the code.
 */
