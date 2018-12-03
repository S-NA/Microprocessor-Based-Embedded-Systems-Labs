#include "nhdlcd.hpp"

unsigned char get_cmd_value(Command);
unsigned get_cmd_delay(Command);
constexpr unsigned char get_upper_eight_bits(unsigned);
constexpr unsigned char get_lower_eight_bits(unsigned);

nhdlcd::nhdlcd(pin_t datPin, pin_t clkPin, pin_t slsPin)
    : data{datPin}, clock{clkPin}, slaveSelect{slsPin} {
  setup();
}

nhdlcd::nhdlcd(pin_t datPin, pin_t clkPin, pin_t slsPin, bool msk)
    : data{datPin}, clock{clkPin}, slaveSelect{slsPin},
      bufferCharacters{msk}, chIdx{0}, packNextCharacter{false}, charCount{0} {
  setup();
}

void nhdlcd::setup() {
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(slaveSelect, OUTPUT);
}

void nhdlcd::write(unsigned char ch, int bitOrder) {
  //  if (bufferCharacters && (ch >= 0x20  && ch <= 0x7F)) {
  //    if (chIdx == 32) {
  //      chIdx = 0; /* naive */
  //    }
  //    if (packNextCharacter) {
  //      /* unpack, and keep in mind this is the lower eight bits */
  //      unsigned char tmp = characterBuffer[chIdx - 1];
  //      unsigned packedCharacter = (ch << 8) + tmp;
  //      characterBuffer[chIdx++] = packedCharacter;
  //      packNextCharacter = false;
  //    } else {
  //      characterBuffer[chIdx++] = ch;
  //      packNextCharacter = true;
  //    }
  //  }
  /* bitOrder uses MSBFIRST and LSBFIRST macros, of which are 1 and 0 */
  unsigned char compareValue = bitOrder ? 0x80U : 0x01U;

  /**
   * To all who read this, the comments below are needlessly detailed.
   * Why you ask, for myself when I forget what the fsck is SPI.
   */
  /* Before sending data, we set slave select line low which activates it. */
  digitalWrite(slaveSelect, LOW);

  // shift out data
  for (uint8_t i = 0U; i < 8U; ++i) {
    // send bit to data pin
    digitalWrite(data, (ch & compareValue) ? HIGH : LOW);

    /* Shift compare value, again this relies on the fact that the macros
     * MSBFIRST and LSBFIRST are defined to 1 and 0 respectively.
     */
    compareValue = bitOrder ? compareValue >> 1 : compareValue << 1;

    // trigger clk rising edge(toggle low then high)
    digitalWrite(clock, LOW);
    // wait before trigger
    delayMicroseconds(4U);
    digitalWrite(clock, HIGH);
    /* Due to the LCD we are using specifying a max 100KHz for SPI
     * wait some time so we can bit bang at such a frequency.
     */
    delayMicroseconds(14U);
  }
  /* Finished sending data, set slave select line high, disabling it. */
  digitalWrite(slaveSelect, HIGH);
  delayMicroseconds(110U); /* TODO: is this what the datasheet means? */
}

void nhdlcd::write(Command cmd) {
  unsigned delayTime = get_cmd_delay(cmd); /* in μs */
  if (cmd == Command::ClearScreen)
    charCount = 0;
  write(0xFEU);
  write(get_cmd_value(cmd));
  delayMicroseconds(delayTime);
}

void nhdlcd::write(Command cmd, int argv...) {
  unsigned delayTime = get_cmd_delay(cmd); /* in μs */
  va_list args;
  va_start(args, argv);

  uint8_t count = 1;
  if (cmd == Command::LoadCustomCharacter)
    count = 9;

  write(0xFE);
  write(get_cmd_value(cmd));
  for (uint8_t i = 0; i < count; ++i) {
    unsigned char ch = (count == 1) ? argv : va_arg(args, int);
    Serial.println(ch);
    write(ch);
  }
  va_end(args);

  delayMicroseconds(delayTime);
}

void nhdlcd::write(const char *str) {
  uint8_t i = 0;
  while (str[i]) {
    write(str[i++]);
    ++charCount;
  }
}

void nhdlcd::hMarquee() {
  for (unsigned i = 0; i < max(charCount, 16); ++i) {
    write(Command::MoveDisplayRightOnePlace);
    delay(50);
  }
  delay(100);
  for (unsigned i = 0; i < max(charCount, 16); ++i) {
    write(Command::MoveDisplayLeftOnePlace);
    delay(50);
  }
  delay(100);
  /* Why do I do this to myself? */
  //  bool prevState = false;
  //  if (bufferCharacters) {
  //    write(Command::ClearScreen);
  //    prevState = bufferCharacters;
  //    bufferCharacters = false;
  //    for (int i = 0; i < 31; ++i) {
  //      if (i == 16) write(Command::SetCursor, 0x40);
  //      if (characterBuffer[i] < 0x20 || characterBuffer[i] > 0x7F) break;
  //      unsigned char firstChar = get_lower_eight_bits(characterBuffer[i]);
  //      unsigned char lastChar  = get_upper_eight_bits(characterBuffer[i]);
  //      write(firstChar);
  //      write(lastChar);
  //    }
  //  }
  //  bufferCharacters = prevState;
}
void nhdlcd::vMarquee() { /* requires software buffer */ }
void nhdlcd::blink(unsigned char reps, Level restore) {
  for (int i = 0; i < reps; ++i) {
    write(Command::SetBacklightBrightness, 1);
    delay(100);
    write(Command::SetBacklightBrightness, restore);
    delay(100);
  }
}

/**
 * Internal helper functions for the command interface.
 */
unsigned char get_cmd_value(Command cmd) {
  /* NHD-0216K3Z-FL-GBW-V3.pdf page 7 */
  static const unsigned char lookup[22] = {
      0x41, 0x42, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4E,
      0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x61, 0x62, 0x70, 0x71, 0x72};
  return lookup[static_cast<uint8_t>(cmd)];
}

unsigned get_cmd_delay(Command cmd) {
  switch (cmd) {
  case Command::CursorHome:
  case Command::UnderlineCursorOn:
  case Command::UnderlineCursorOff:
  case Command::ClearScreen:
    return 2000;
  case Command::SetContrast:
    return 501;
  case Command::LoadCustomCharacter:
    return 201;
  case Command::ChangeRS232BaudRate:
  case Command::ChangeI2CAddress:
    return 3500;
  case Command::DisplayFirmwareVersionNumber:
  case Command::DisplayI2CAddress:
    return 4500;
  case Command::DisplayRS232BaudRate:
    return 10500;
  default:
    return 110;
  }
}

constexpr unsigned char get_upper_eight_bits(unsigned int packedCharacter) {
  return packedCharacter >> 8;
}

constexpr unsigned char get_lower_eight_bits(unsigned int packedCharacter) {
  return packedCharacter;
}
