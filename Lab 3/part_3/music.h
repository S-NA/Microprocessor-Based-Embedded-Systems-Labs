#pragma once

bool secondCompositionInitalized = false;
bool playSecond = false;

struct Composition {
  unsigned char note[32];
  unsigned char duration[32];
  unsigned noteCount;
} first = {{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'}, // {'c', 'd', 'e', 'f', 'g', 'a', 'b', 'h'},
           {128, 16, 128, 16, 16, 16, 16, 16},
           8},
  second;

constexpr unsigned getDuration(unsigned char length) { return 70 * length; }

constexpr unsigned getDelay(unsigned char length) { return 10 * length; }

uint8_t getPitch(unsigned char note) {
  static const uint8_t lookup[8] = {
      0x9 /* a */
      ,
      0xB /* b */
      ,
      0x0 /* c */
      ,
      0x2 /* d */
      ,
      0x4 /* e */
      ,
      0x5 /* f */
      ,
      0x7 /* g */
      ,
      0xC /* h (High C) */
  };      /* Numerical pitch mapped to note. */
  if (note > '8')
    return lookup[note - 'a'];
  return lookup[note - '1'];
}

float getFrequency(unsigned char note) {
  return 261.2 * pow(2, getPitch(note) / 12.);
}

float getOctaveUpFrequency(unsigned char note) {
  return 261.2 * pow(2, (2 * getPitch(note)) / 12.);
}

