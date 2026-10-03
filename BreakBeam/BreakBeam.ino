/*
  BreakBeam - laser speed-gate firmware for Arduino Uno (+ Ciseco R017 SRF radio shield).

  ONE firmware, THREE jobs. Tell a board its job once over the USB serial monitor
  (115200 baud, "newline" line endings) and it remembers it in EEPROM:

      role solo      - BOTH beams on this one board. No radio, no shield needed.
                       A0 = start beam, A1 = finish beam, joined by a long cable.
                       This is the bench/garage test.
      role start     - remote start gate, talks over the radio.
      role finish    - timekeeper, talks over the radio, reports to your PC.

  An optional display shows the time as seconds to 2 decimal places. Tell the board
  which one you have (also remembered):

      display none | lcd | i2clcd | tm1637 | max7219

  How the radio timing works (start/finish roles)
  -----------------------------------------------
  The FINISH gate is the master. Ten times a second it sends a 12-byte poll. The
  START gate replies "my beam broke N microseconds ago". The master knows when it
  sent the poll and the round-trip time, so it places the start event on its OWN
  clock to about a millisecond. No clock sync, so crystal drift cannot matter.

  Radio facts (R017 datasheet + Ciseco AT reference)
  ---------------------------------------------------
  * The radio's serial is on D0/D1, SHARED with USB. Everything printed goes to the
    PC and over the air. Radio frames start with '@' and are exactly 12 bytes, so
    the SRF sends them at once (packet size 12; shorter data waits 16 ms).
  * D8 HIGH connects the radio. Solo role leaves it alone.
  * Factory: 115200 baud, PANID 5AA5, 868.3 MHz (EU). "+++" enters AT mode.
  * A gate NEVER answers text it does not recognise: on a shared serial line that
    would start an endless echo war between the two gates.

  Wiring - the core (same on every board)
  ---------------------------------------
    A0  START beam sensor: LDR from 5V to A0, 10k from A0 to GND.
        (more light = bigger number. A phototransistor drops in the same way:
         long leg to 5V, short leg to A0, 10k from A0 to GND.)
    A1  FINISH beam sensor, wired identically. Solo role only.
    D7  LED via 330R to GND: lit when the START beam is landing. D13 copies it.
    D6  LED via 330R to GND: lit when the FINISH beam is landing. Solo role only.
    D9  piezo buzzer: + to D9, - to GND.
    D8  radio enable. Used by the shield. Connect nothing.

  Wiring - the display (pick ONE)
  --------------------------------
    lcd      16x2 LCD, the 16-pin one (e.g. 1602A). Straight through, no crossover:
             LCD RS->D12, E->D11, D4->D2, D5->D3, D6->D4, D7->D5, RW->GND.
             Plus VSS->GND, VDD->5V, VO->contrast pot (or 1k to GND),
             A->5V via 330R, K->GND. LCD pins D0-D3 are left unconnected.
    i2clcd   16x2 LCD with an I2C backpack soldered on the back (4 pins).
             SDA=A4  SCL=A5  plus 5V and GND. Address 0x27 or 0x3F, found for you.
    tm1637   4-digit "clock" module, 4 pins. CLK=D2  DIO=D3  plus 5V and GND.
             Most of these have a colon instead of decimal points, so 12.34 s
             shows as 12:34. That is the display, not a bug.
    max7219  8-digit 7-segment module, 5 pins. DIN=D11  CS=D10  CLK=D12  5V  GND.
*/

#include <Arduino.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include <Wire.h>

// ---------------------------------------------------------------- pins
static const uint8_t PIN_BEAM_A   = A0;
static const uint8_t PIN_BEAM_B   = A1;
static const uint8_t PIN_LED_A    = 7;
static const uint8_t PIN_LED_B    = 6;
static const uint8_t PIN_BUZZER   = 9;
static const uint8_t PIN_RADIO_EN = 8;

// ---------------------------------------------------------------- tunables
static const unsigned long SERIAL_BAUD      = 115200; // SRF shield factory default
static const unsigned long LOCKOUT_MS       = 800;    // ignore re-breaks (arms, legs)
static const int           MIN_DROP         = 80;     // light must fall this many ADC counts...
static const uint8_t       DROP_PERCENT     = 20;     // ...or this % of baseline, whichever is larger
static const int           BEAM_MIN_LEVEL   = 300;    // baseline below this = no beam on the sensor
static const unsigned long POLL_INTERVAL_MS = 100;    // master -> slave poll rate when idle
static const unsigned long POLL_TIMEOUT_MS  = 80;     // give up waiting for a reply
static const uint8_t       SYNC_SAMPLES     = 6;      // rapid polls used to pin down the start
static const unsigned long RUN_TIMEOUT_MS   = 60000;  // no finish within this = abandon
static const unsigned long COOLDOWN_MS      = 1500;   // after a result, ignore beams this long
static const unsigned long STATUS_EVERY_MS  = 2000;   // status line interval
static const uint8_t       LINK_LOST_AFTER  = 5;      // consecutive missed polls = link down

// ---------------------------------------------------------------- EEPROM
enum Role : uint8_t { ROLE_UNSET = 0, ROLE_START = 1, ROLE_FINISH = 2, ROLE_SOLO = 3 };
enum DispType : uint8_t { DISP_NONE = 0, DISP_LCD = 1, DISP_I2CLCD = 2, DISP_TM1637 = 3, DISP_MAX7219 = 4 };

static const int     EE_MAGIC_ADDR = 0, EE_ROLE_ADDR = 1, EE_DISP_ADDR = 2, EE_LAP_ADDR = 3;
static const uint8_t EE_MAGIC      = 0xB4;
Role     role = ROLE_UNSET;
DispType disp = DISP_NONE;

void loadSettings() {
  if (EEPROM.read(EE_MAGIC_ADDR) != EE_MAGIC) return;
  uint8_t r = EEPROM.read(EE_ROLE_ADDR);
  if (r == ROLE_START || r == ROLE_FINISH || r == ROLE_SOLO) role = (Role)r;
  uint8_t d = EEPROM.read(EE_DISP_ADDR);
  if (d <= DISP_MAX7219) disp = (DispType)d;
}
void saveRole(Role r) {
  EEPROM.update(EE_MAGIC_ADDR, EE_MAGIC);
  EEPROM.update(EE_ROLE_ADDR, (uint8_t)r);
}
void saveDisp(DispType d) {
  EEPROM.update(EE_MAGIC_ADDR, EE_MAGIC);
  EEPROM.update(EE_DISP_ADDR, (uint8_t)d);
}
const char* roleName(Role r) {
  return r == ROLE_START ? "START" : r == ROLE_FINISH ? "FINISH" : r == ROLE_SOLO ? "SOLO (both beams, no radio)" : "NOT SET";
}
const char* dispName(DispType d) {
  return d == DISP_LCD ? "lcd" : d == DISP_I2CLCD ? "i2clcd" : d == DISP_TM1637 ? "tm1637" : d == DISP_MAX7219 ? "max7219" : "none";
}
void rebootNow() {
  Serial.println(F("Rebooting..."));
  Serial.flush();
  wdt_enable(WDTO_250MS);
  while (true) {}
}

// ---------------------------------------------------------------- sound
bool soundOn = true;
void beep(unsigned int hz, unsigned long ms) { if (soundOn) tone(PIN_BUZZER, hz, ms); }

// ================================================================ DISPLAY DRIVERS
// Each driver is hand-written so no extra libraries need installing.

// ---- shared: turn microseconds into four digits "SS.hh" -------------------
// Fills d[0..3] with the digits of SS.hh (capped at 99.99 s).
void timeDigits(unsigned long us, uint8_t* d) {
  unsigned long h = (us + 5000UL) / 10000UL;      // hundredths of a second
  if (h > 9999UL) h = 9999UL;
  d[0] = (uint8_t)((h / 1000UL) % 10);
  d[1] = (uint8_t)((h / 100UL) % 10);
  d[2] = (uint8_t)((h / 10UL) % 10);
  d[3] = (uint8_t)(h % 10);
}
// Text form, always 5 chars + nul, e.g. " 9.87" or "12.34"
void timeText(unsigned long us, char* out) {
  uint8_t d[4]; timeDigits(us, d);
  out[0] = d[0] ? ('0' + d[0]) : ' ';
  out[1] = '0' + d[1];
  out[2] = '.';
  out[3] = '0' + d[2];
  out[4] = '0' + d[3];
  out[5] = 0;
}

// ---- TM1637 (4-digit module, 2 wires) ------------------------------------
static const uint8_t TM_CLK = 2, TM_DIO = 3;
static const uint8_t SEG7[10] = { 0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F };
void tmStart() { pinMode(TM_DIO, OUTPUT); digitalWrite(TM_DIO, HIGH); digitalWrite(TM_CLK, HIGH); delayMicroseconds(5); digitalWrite(TM_DIO, LOW); delayMicroseconds(5); }
void tmStop()  { digitalWrite(TM_CLK, LOW); delayMicroseconds(5); pinMode(TM_DIO, OUTPUT); digitalWrite(TM_DIO, LOW); delayMicroseconds(5); digitalWrite(TM_CLK, HIGH); delayMicroseconds(5); digitalWrite(TM_DIO, HIGH); delayMicroseconds(5); }
void tmByte(uint8_t b) {
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(TM_CLK, LOW); delayMicroseconds(5);
    digitalWrite(TM_DIO, (b & 1) ? HIGH : LOW); b >>= 1; delayMicroseconds(5);
    digitalWrite(TM_CLK, HIGH); delayMicroseconds(5);
  }
  digitalWrite(TM_CLK, LOW); pinMode(TM_DIO, INPUT_PULLUP); delayMicroseconds(5);  // read ack
  digitalWrite(TM_CLK, HIGH); delayMicroseconds(5);
  digitalWrite(TM_CLK, LOW); pinMode(TM_DIO, OUTPUT); delayMicroseconds(5);
}
void tmInit() {
  pinMode(TM_CLK, OUTPUT); pinMode(TM_DIO, OUTPUT);
  tmStart(); tmByte(0x88 | 0x05); tmStop();    // display on, medium brightness
}
void tmRaw(const uint8_t* seg) {
  tmStart(); tmByte(0x40); tmStop();                                  // auto-increment address
  tmStart(); tmByte(0xC0); for (uint8_t i = 0; i < 4; i++) tmByte(seg[i]); tmStop();
  tmStart(); tmByte(0x88 | 0x05); tmStop();
}

// ---- MAX7219 (8-digit 7-segment module, 3 wires) -------------------------
static const uint8_t MX_DIN = 11, MX_CS = 10, MX_CLK = 12;
void mxSend(uint8_t reg, uint8_t val) {
  digitalWrite(MX_CS, LOW);
  shiftOut(MX_DIN, MX_CLK, MSBFIRST, reg);
  shiftOut(MX_DIN, MX_CLK, MSBFIRST, val);
  digitalWrite(MX_CS, HIGH);
}
void mxInit() {
  pinMode(MX_DIN, OUTPUT); pinMode(MX_CS, OUTPUT); pinMode(MX_CLK, OUTPUT);
  digitalWrite(MX_CS, HIGH);
  mxSend(0x0F, 0x00);   // display test off
  mxSend(0x09, 0xFF);   // BCD decode on all digits
  mxSend(0x0A, 0x08);   // medium brightness
  mxSend(0x0B, 0x07);   // scan all 8 digits
  mxSend(0x0C, 0x01);   // wake up
  for (uint8_t i = 1; i <= 8; i++) mxSend(i, 0x0F);   // blank
}

// ---- 16x2 LCD, 16-pin parallel (HD44780 in 4-bit mode, hand-written) -----
static const uint8_t LCD_RS = 12, LCD_E = 11, LCD_D4 = 2, LCD_D5 = 3, LCD_D6 = 4, LCD_D7 = 5;
void pNibble(uint8_t n, bool rs) {
  digitalWrite(LCD_RS, rs);
  digitalWrite(LCD_D4, n & 1); digitalWrite(LCD_D5, (n >> 1) & 1);
  digitalWrite(LCD_D6, (n >> 2) & 1); digitalWrite(LCD_D7, (n >> 3) & 1);
  digitalWrite(LCD_E, HIGH); delayMicroseconds(2);
  digitalWrite(LCD_E, LOW);  delayMicroseconds(60);
}
void pCmd(uint8_t c)  { pNibble(c >> 4, false); pNibble(c & 0x0F, false); }
void pChar(uint8_t c) { pNibble(c >> 4, true);  pNibble(c & 0x0F, true); }
// full = a cold start: claim the pins, wait for the module to power up, clear the
// screen. Anything else is a re-assert of the settings on a display that is already
// running, which costs about 6 ms and is how a garbled screen puts itself right.
void pLcdInit(bool full) {
  if (full) {
    const uint8_t pins[] = { LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7 };
    for (uint8_t i = 0; i < 6; i++) { pinMode(pins[i], OUTPUT); digitalWrite(pins[i], LOW); }
    delay(50);
  }
  pNibble(0x03, false); delay(5);
  pNibble(0x03, false); delayMicroseconds(200);
  pNibble(0x03, false); delayMicroseconds(200);
  pNibble(0x02, false); delayMicroseconds(200);   // into 4-bit mode
  pCmd(0x28);   // 2 lines, 5x8 font
  pCmd(0x0C);   // display on, cursor off
  pCmd(0x06);   // entry mode: advance right
  if (full) { pCmd(0x01); delay(3); }
}
void pLine(uint8_t row, const char* s) {
  pCmd(row ? 0xC0 : 0x80);
  uint8_t n = 0;
  while (s[n] && n < 16) { pChar((uint8_t)s[n]); n++; }
  while (n < 16) { pChar(' '); n++; }
}

// ---- 16x2 LCD on an I2C backpack (PCF8574) -------------------------------
uint8_t i2cAddr = 0;
void i2cRaw(uint8_t v) { Wire.beginTransmission(i2cAddr); Wire.write(v); Wire.endTransmission(); }
void i2cNibble(uint8_t nib, bool rs) {
  uint8_t d = (uint8_t)((nib << 4) | (rs ? 0x01 : 0x00) | 0x08);   // 0x08 = backlight on
  i2cRaw(d | 0x04); delayMicroseconds(2);      // E high
  i2cRaw(d);        delayMicroseconds(60);     // E low
}
void i2cCmd(uint8_t c)  { i2cNibble(c >> 4, false); i2cNibble(c & 0x0F, false); }
void i2cChar(uint8_t c) { i2cNibble(c >> 4, true);  i2cNibble(c & 0x0F, true); }
bool i2cFind() {
  const uint8_t candidates[] = { 0x27, 0x3F, 0x26, 0x3E, 0x20, 0x38 };
  for (uint8_t i = 0; i < sizeof(candidates); i++) {
    Wire.beginTransmission(candidates[i]);
    if (Wire.endTransmission() == 0) { i2cAddr = candidates[i]; return true; }
  }
  return false;
}
void i2cLcdInit() {
  Wire.begin();
  if (!i2cFind()) { Serial.println(F("I2C LCD: nothing answered. Check SDA=A4, SCL=A5, 5V, GND.")); return; }
  Serial.print(F("I2C LCD found at 0x")); Serial.println(i2cAddr, HEX);
  delay(50);
  i2cNibble(0x03, false); delay(5);
  i2cNibble(0x03, false); delayMicroseconds(200);
  i2cNibble(0x03, false); delayMicroseconds(200);
  i2cNibble(0x02, false); delayMicroseconds(200);   // 4-bit mode
  i2cCmd(0x28);   // 2 lines, 5x8 font
  i2cCmd(0x0C);   // display on, cursor off
  i2cCmd(0x06);   // entry mode: advance right
  i2cCmd(0x01); delay(3);   // clear
}
void i2cLine(uint8_t row, const char* s) {
  if (!i2cAddr) return;
  i2cCmd(row ? 0xC0 : 0x80);
  uint8_t n = 0;
  while (s[n] && n < 16) { i2cChar((uint8_t)s[n]); n++; }
  while (n < 16) { i2cChar(' '); n++; }
}

// ---- the display API the rest of the sketch uses -------------------------
void dispInit() {
  switch (disp) {
    case DISP_TM1637:  tmInit(); break;
    case DISP_MAX7219: mxInit(); break;
    case DISP_LCD:     pLcdInit(true); break;
    case DISP_I2CLCD:  i2cLcdInit(); break;
    default: break;
  }
}
// A display that has lost its settings prints nonsense and never recovers on its
// own, so re-assert them periodically. This rides along with a full redraw, which
// only happens on a state change and always after the timestamp that matters has
// already been taken, so the few milliseconds cost nothing.
unsigned long lastLcdKeepAliveMs = 0;
void lcdKeepAlive() {
  if (disp != DISP_LCD) return;
  unsigned long now = millis();
  if (now - lastLcdKeepAliveMs < 2000) return;
  lastLcdKeepAliveMs = now;
  pLcdInit(false);
}

// Line 1 on an LCD; ignored by the 4-digit displays.
void dispTitle(const char* s) {
  lcdKeepAlive();
  if (disp == DISP_LCD)    pLine(0, s);
  if (disp == DISP_I2CLCD) i2cLine(0, s);
}
void dispTime(unsigned long us) {
  char txt[8]; timeText(us, txt);
  switch (disp) {
    case DISP_TM1637: {
      uint8_t d[4]; timeDigits(us, d);
      uint8_t seg[4];
      seg[0] = d[0] ? SEG7[d[0]] : 0x00;
      seg[1] = SEG7[d[1]] | 0x80;            // 0x80 lights the colon on these modules
      seg[2] = SEG7[d[2]];
      seg[3] = SEG7[d[3]];
      tmRaw(seg);
      break;
    }
    case DISP_MAX7219: {
      uint8_t d[4]; timeDigits(us, d);
      for (uint8_t i = 1; i <= 4; i++) mxSend(i, 0x0F);            // blank digits 1-4
      mxSend(8, d[0] ? d[0] : 0x0F);
      mxSend(7, d[1] | 0x80);                                       // decimal point
      mxSend(6, d[2]);
      mxSend(5, d[3]);
      break;
    }
    case DISP_LCD:
    case DISP_I2CLCD: { char b[17]; snprintf(b, sizeof(b), "%s s", txt);
                        if (disp == DISP_LCD) pLine(1, b); else i2cLine(1, b); break; }
    default: break;
  }
}
void dispDashes(const char* lcdText) {
  switch (disp) {
    case DISP_TM1637:  { uint8_t seg[4] = { 0x40, 0x40, 0x40, 0x40 }; tmRaw(seg); break; }
    case DISP_MAX7219: { for (uint8_t i = 1; i <= 8; i++) mxSend(i, 0x0A); break; }   // 0x0A = '-'
    case DISP_LCD:     pLine(1, lcdText); break;
    case DISP_I2CLCD:  i2cLine(1, lcdText); break;
    default: break;
  }
}

// While the clock runs we repaint only the digits, not the whole line. A full
// 16-character line takes about a millisecond, and the main loop is blocked for
// that long, which would blunt the timing of a beam break landing in the middle
// of it. Five characters is a fifth of the cost.
void dispTimeLive(unsigned long us) {
  char txt[8]; timeText(us, txt);
  switch (disp) {
    case DISP_LCD:    pCmd(0xC0);  for (uint8_t i = 0; txt[i]; i++) pChar((uint8_t)txt[i]);  break;
    case DISP_I2CLCD: i2cCmd(0xC0); for (uint8_t i = 0; txt[i]; i++) i2cChar((uint8_t)txt[i]); break;
    default:          dispTime(us); break;    // the 4-digit modules are cheap to rewrite whole
  }
}
// While nothing is being timed the screen doubles as an alignment aid: it says
// whether the beam is landing, and when it is not it shows the live light level
// against the level needed, so the laser can be walked onto the sensor without a PC.
bool holdingResult = false;      // a finished time is on screen; do not overwrite it
unsigned long beamGoneSinceMs = 0;
static const unsigned long BEAM_GONE_GRACE_MS = 1500;  // a hand passing through is not a lost beam
int  lastShownBucket = -1;
int  lastShownPresent = -1;
void dispArmed(bool present, int reading) {
  int bucket = reading / 10;
  if ((int)present == lastShownPresent && bucket == lastShownBucket) return;
  lastShownPresent = (int)present; lastShownBucket = bucket;
  if (present) { dispTitle("BreakBeam"); dispDashes("ready"); }
  else {
    char b[17];
    snprintf(b, sizeof(b), "%d of %d", reading, BEAM_MIN_LEVEL);
    dispTitle("Aim the laser"); dispDashes(b);
  }
}

unsigned long lastLiveMs = 0;
static const unsigned long LIVE_EVERY_MS = 50;   // 20 updates a second: the hundredths visibly race
void liveClockTick(bool running, unsigned long startUs, unsigned long nowMs) {
  if (!running) { lastLiveMs = nowMs; return; }
  if (nowMs - lastLiveMs < LIVE_EVERY_MS) return;
  lastLiveMs = nowMs;
  dispTimeLive(micros() - startUs);
}

// ================================================================ BEAM SENSOR
class BeamSensor {
public:
  uint8_t       pinIn, pinLed;
  int           reading        = 0;
  long          baseline16     = 0;      // baseline * 16, fixed point
  bool          blocked        = false;
  unsigned long blockedSinceMs = 0;
  unsigned long lastBaselineMs = 0;
  unsigned long lastTriggerUs  = 0;
  unsigned long lastTriggerMs  = 0;
  bool          hasEvent       = false;
  uint8_t       eventCount     = 0;      // +1 per trigger, wraps 0..15

  BeamSensor(uint8_t in, uint8_t led) : pinIn(in), pinLed(led) {}

  int  baseline() const { return (int)(baseline16 / 16); }
  int  dropNeeded() const {
    int pct = (int)((long)baseline() * DROP_PERCENT / 100);
    return pct > MIN_DROP ? pct : MIN_DROP;
  }
  bool beamPresent() const { return !blocked && baseline() >= BEAM_MIN_LEVEL; }
  const char* stateText() const { return beamPresent() ? "OK" : (blocked ? "BLOCKED" : "TOO DIM/NOT ALIGNED"); }

  void seed() {
    long sum = 0;
    for (int i = 0; i < 64; i++) { sum += analogRead(pinIn); delay(2); }
    baseline16 = (sum / 64) * 16;
    reading = (int)(sum / 64);
    blocked = false;
  }

  // Call constantly. Returns true exactly once per new beam-break event.
  bool update() {
    reading = analogRead(pinIn);
    unsigned long nowUs = micros();
    unsigned long nowMs = millis();
    bool triggered = false;

    if (!blocked) {
      // Only a beam that is actually present can be broken. Without this test an
      // unconnected pin, which drifts and mirrors its neighbour, fires phantom
      // breaks and stops the clock the instant it starts.
      if (baseline() >= BEAM_MIN_LEVEL && reading < baseline() - dropNeeded()) {
        blocked = true;
        blockedSinceMs = nowMs;
        bool lockedOut = hasEvent && (nowMs - lastTriggerMs) < LOCKOUT_MS;
        if (!lockedOut) {
          lastTriggerUs = nowUs;
          lastTriggerMs = nowMs;
          hasEvent = true;
          eventCount = (eventCount + 1) & 0x0F;
          triggered = true;
        }
      }
    } else {
      if (reading > baseline() - dropNeeded() / 2) blocked = false;   // beam is back
      else if (nowMs - blockedSinceMs > 5000) seed();                  // stuck: re-learn "normal"
    }

    // Slowly follow slow light changes (clouds, lamps) while the beam is unbroken.
    if (!blocked && nowMs - lastBaselineMs >= 20) {
      lastBaselineMs = nowMs;
      baseline16 += ((long)reading * 16 - baseline16) / 64;
    }
    if (pinLed != 255) digitalWrite(pinLed, beamPresent());
    return triggered;
  }
  // Age of the last event in 10 us units, or -1 if none / older than 9.99 s.
  long ageTensOfMicros() const {
    if (!hasEvent) return -1;
    unsigned long age = micros() - lastTriggerUs;
    if (age > 9999990UL) return -1;
    return (long)(age / 10);
  }
};
BeamSensor beamA(PIN_BEAM_A, PIN_LED_A);    // start beam, and the only beam in radio roles
BeamSensor beamB(PIN_BEAM_B, PIN_LED_B);    // finish beam, solo role only

// ---------------------------------------------------------------- serial lines
char    lineBuf[24];
uint8_t lineLen = 0;
bool readLine() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') { lineBuf[lineLen] = 0; lineLen = 0; return true; }
    if (lineLen < sizeof(lineBuf) - 1) lineBuf[lineLen++] = c;
    else lineLen = 0;
  }
  return false;
}

// ---------------------------------------------------------------- radio helpers
bool waitForAtReply(const char* want, unsigned long timeoutMs) {
  char buf[16]; uint8_t n = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < timeoutMs) {
    if (!Serial.available()) continue;
    char c = (char)Serial.read();
    if (c == '\n') { n = 0; continue; }   // a frame from the other gate may land mid-test
    if (c == '\r') { buf[n] = 0; if (strcmp(buf, want) == 0) return true; n = 0; continue; }
    if (n < sizeof(buf) - 1) buf[n++] = c;
  }
  return false;
}
bool atEnter() { Serial.flush(); delay(1100); Serial.print(F("+++")); return waitForAtReply("OK", 2000); }
bool atCommand(const char* cmd) { Serial.print(cmd); Serial.print('\r'); return waitForAtReply("OK", 1000); }
bool radioOk = false;
void radioSelfTest() {
  Serial.println(F("Radio self-test (you will see +++ATDN below - that is normal)..."));
  if (atEnter()) {
    radioOk = true; atCommand("ATDN");
    Serial.println(F("Radio: OK - SRF shield answered at 115200 baud."));
    return;
  }
  Serial.begin(9600);                       // older radios ship at 9600; move it to 115200
  if (atEnter()) {
    Serial.println(F("Radio answered at 9600 - switching it to 115200 (ATBD 1C200)..."));
    atCommand("ATBD1C200"); atCommand("ATWR");
    Serial.print(F("ATAC\r")); Serial.flush(); delay(200);
    Serial.begin(SERIAL_BAUD); delay(100); atCommand("ATDN");
    radioOk = true;
    Serial.println(F("Radio: OK - now at 115200 baud."));
    return;
  }
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("Radio: NO ANSWER. Is the SRF shield fitted? (Beam sensing still works.)"));
}

// ---------------------------------------------------------------- misc helpers
void printSeconds(unsigned long us) {
  unsigned long ms = (us + 500) / 1000;
  Serial.print(ms / 1000); Serial.print('.');
  unsigned int frac = ms % 1000;
  if (frac < 100) Serial.print('0');
  if (frac < 10)  Serial.print('0');
  Serial.print(frac);
}
char hexDigit(uint8_t v) { return "0123456789ABCDEF"[v & 0x0F]; }

bool alignMode = false, lapMode = false;
unsigned long lastAlignPrintMs = 0;
unsigned int  runNumber = 0;

void alignPrint() {
  if (!alignMode || millis() - lastAlignPrintMs < 200) return;
  lastAlignPrintMs = millis();
  Serial.print(F("A0=")); Serial.print(beamA.reading);
  Serial.print(F(" base=")); Serial.print(beamA.baseline());
  Serial.print(F(" trips<")); Serial.print(beamA.baseline() - beamA.dropNeeded());
  Serial.print(F(" [")); Serial.print(beamA.stateText()); Serial.print(']');
  if (role == ROLE_SOLO) {
    Serial.print(F("   A1=")); Serial.print(beamB.reading);
    Serial.print(F(" base=")); Serial.print(beamB.baseline());
    Serial.print(F(" trips<")); Serial.print(beamB.baseline() - beamB.dropNeeded());
    Serial.print(F(" [")); Serial.print(beamB.stateText()); Serial.print(']');
  }
  Serial.println();
}
void printHelp() {
  Serial.println(F("Commands (type, then Enter):"));
  Serial.println(F("  role solo | role start | role finish   set this board's job, then it reboots"));
  Serial.println(F("  display none | lcd | i2clcd | tm1637 | max7219"));
  Serial.println(F("  a   ALIGN mode on/off: live sensor numbers 5x a second"));
  Serial.println(F("  l   LAP mode on/off: time between two breaks of the START beam alone"));
  Serial.println(F("  t   re-initialise the display and show 12.34 (fixes a garbled screen)"));
  Serial.println(F("  r   reset / re-arm"));
  Serial.println(F("  s   buzzer on/off"));
  Serial.println(F("  h   this help"));
}
bool setDisplay(const char* name) {
  DispType d;
  if      (!strcmp(name, "none"))    d = DISP_NONE;
  else if (!strcmp(name, "lcd"))     d = DISP_LCD;
  else if (!strcmp(name, "i2clcd"))  d = DISP_I2CLCD;
  else if (!strcmp(name, "tm1637"))  d = DISP_TM1637;
  else if (!strcmp(name, "max7219")) d = DISP_MAX7219;
  else return false;
  disp = d; saveDisp(d); dispInit();
  Serial.print(F("Display set to ")); Serial.println(dispName(disp));
  dispTitle("BreakBeam"); dispTime(12340000UL);
  return true;
}
// Returns true if the line was handled. Never prints "unknown": on a shared
// serial line an echo would start a war with the other gate.
bool handleCommonCommand(const char* line) {
  if (!strcmp(line, "role solo"))   { saveRole(ROLE_SOLO);   Serial.println(F("Role saved: SOLO"));   rebootNow(); }
  if (!strcmp(line, "role start"))  { saveRole(ROLE_START);  Serial.println(F("Role saved: START"));  rebootNow(); }
  if (!strcmp(line, "role finish")) { saveRole(ROLE_FINISH); Serial.println(F("Role saved: FINISH")); rebootNow(); }
  if (!strncmp(line, "display ", 8)) return setDisplay(line + 8);
  if (!strcmp(line, "a")) { alignMode = !alignMode; Serial.println(alignMode ? F("ALIGN mode ON") : F("ALIGN mode OFF")); return true; }
  if (!strcmp(line, "s")) { soundOn = !soundOn;     Serial.println(soundOn ? F("Buzzer ON") : F("Buzzer OFF")); return true; }
  if (!strcmp(line, "t")) {   // re-initialise first: a garbled display is usually one that lost its setup
    dispInit(); dispTitle("BreakBeam"); dispTime(12340000UL);
    Serial.print(F("Display re-initialised and sent 12.34: ")); Serial.println(dispName(disp));
    return true;
  }
  if (!strcmp(line, "h") || !strcmp(line, "?")) { printHelp(); return true; }
  return false;
}

// ================================================================ result reporting
unsigned long tStartUs = 0, tFinishUs = 0;
void reportRun(unsigned long elapsed, const char* note) {
  runNumber++;
  Serial.print(F("=== RUN ")); Serial.print(runNumber); Serial.print(F("   TIME "));
  printSeconds(elapsed); Serial.print(F(" s"));
  if (note) { Serial.print(F("   ")); Serial.print(note); }
  Serial.println(F(" ==="));
  Serial.print(F("CSV,")); Serial.print(runNumber); Serial.print(','); printSeconds(elapsed); Serial.println();
  char title[17]; snprintf(title, sizeof(title), "Run %u", runNumber);
  dispTitle(title); dispTime(elapsed);
  holdingResult = true; lastShownBucket = -1; lastShownPresent = -1;
  beep(2000, 120); delay(150); beep(2600, 200);
}

// ================================================================ SOLO role
enum RunState : uint8_t { ARMED, SYNCING, RUNNING, COOLDOWN };
RunState      state = ARMED;
unsigned long runStartMs = 0, cooldownStartMs = 0, lastStatusMs = 0;
unsigned long lapStartUs = 0; bool lapArmed = false;

void armSolo() {
  state = ARMED;
  Serial.println(lapMode ? F("[ARMED] LAP mode: break the START beam to begin.")
                         : F("[ARMED] waiting for the START beam (A0)..."));
  lastShownBucket = -1; lastShownPresent = -1;
}
void handleLap(unsigned long nowUs) {
  if (!lapArmed) { lapArmed = true; lapStartUs = nowUs; Serial.println(F("LAP: clock started - break the beam again to stop it")); holdingResult = false; dispTitle("Lap running"); dispTime(0); beep(1200, 60); }
  else {
    // Second break stops the clock and LEAVES the result on screen. Restarting
    // straight away, as this used to, wiped the time after a single frame and
    // there was no way to read it. The next break begins a fresh lap.
    reportRun(nowUs - lapStartUs, "lap");
    lapArmed = false;
  }
}
void soloLoop() {
  unsigned long nowMs = millis();
  bool a = beamA.update();
  bool b = beamB.update();
  digitalWrite(LED_BUILTIN, beamA.beamPresent() && (lapMode || beamB.beamPresent()));

  if (a) {
    beep(1500, 60);
    if (lapMode) handleLap(beamA.lastTriggerUs);
    else switch (state) {
      case ARMED:
      case COOLDOWN:
        tStartUs = beamA.lastTriggerUs; state = RUNNING; runStartMs = nowMs;
        Serial.println(F("START! clock running"));
        holdingResult = false; dispTitle("Running"); dispTime(0);
        break;
      case RUNNING:
        tStartUs = beamA.lastTriggerUs; runStartMs = nowMs;
        Serial.println(F("START beam again - clock restarted."));
        break;
      default: break;
    }
  }
  if (b && !lapMode) {
    beep(2000, 80);
    if (state == RUNNING) {
      tFinishUs = beamB.lastTriggerUs;
      reportRun(tFinishUs - tStartUs, nullptr);
      state = COOLDOWN; cooldownStartMs = nowMs;
    } else if (state == ARMED) {
      Serial.println(F("FINISH beam (A1) broken but no start was seen - ignored."));
    }
  }

  bool clockRunning = lapMode ? lapArmed : (state == RUNNING);
  liveClockTick(clockRunning, lapMode ? lapStartUs : tStartUs, nowMs);
  // A lost beam wins the screen over a held result, but only once it has been gone
  // for a moment. Without the grace period the hand that stopped the clock is still
  // in the beam when the result is written, and wipes it instantly.
  if (!clockRunning) {
    bool present = beamA.beamPresent();
    if (present) beamGoneSinceMs = 0;
    else if (beamGoneSinceMs == 0) beamGoneSinceMs = nowMs;
    bool longGone = !present && (nowMs - beamGoneSinceMs > BEAM_GONE_GRACE_MS);
    if (longGone) holdingResult = false;
    if (!holdingResult) dispArmed(present, beamA.reading);
  }

  if (readLine() && lineBuf[0] && lineBuf[0] != '@') {
    if      (!strcmp(lineBuf, "r")) { lapArmed = false; armSolo(); }
    else if (!strcmp(lineBuf, "l")) {
      lapMode = !lapMode; lapArmed = false;
      EEPROM.update(EE_LAP_ADDR, lapMode ? 1 : 0);   // sticky, so a reset does not lose it
      Serial.println(lapMode ? F("LAP mode ON: one beam (A0) only, timed between breaks") : F("LAP mode OFF: A0 starts, A1 stops"));
      armSolo();
    }
    else handleCommonCommand(lineBuf);
  }

  if (state == RUNNING && nowMs - runStartMs > RUN_TIMEOUT_MS) { Serial.println(F("No finish within 60 s - run abandoned.")); armSolo(); }
  if (state == COOLDOWN && nowMs - cooldownStartMs > COOLDOWN_MS) armSolo();
  if (!alignMode && (state == ARMED || state == COOLDOWN) && nowMs - lastStatusMs >= STATUS_EVERY_MS) {
    lastStatusMs = nowMs;
    Serial.print(state == ARMED ? F("[ARMED] ") : F("[COOLDOWN] "));
    Serial.print(F("START beam: ")); Serial.print(beamA.stateText()); Serial.print(F(" (A0=")); Serial.print(beamA.reading); Serial.print(')');
    if (!lapMode) { Serial.print(F(" | FINISH beam: ")); Serial.print(beamB.stateText()); Serial.print(F(" (A1=")); Serial.print(beamB.reading); Serial.print(')'); }
    Serial.println();
  }
  alignPrint();
}

// ================================================================ START role (radio slave)
void startGateLoop() {
  if (beamA.update()) {
    beep(1500, 60);
    Serial.print(F("START beam broken (event ")); Serial.print(beamA.eventCount); Serial.println(')');
  }
  digitalWrite(LED_BUILTIN, beamA.beamPresent());
  if (readLine()) {
    if (lineBuf[0] == '@' && lineBuf[1] == 'P' && lineBuf[2] != 0) {
      char status = beamA.beamPresent() ? 'A' : (beamA.blocked ? 'B' : 'N');
      long age = beamA.ageTensOfMicros();
      char out[13];
      if (age >= 0) snprintf(out, sizeof(out), "@R%c%c%c%06ld", lineBuf[2], status, hexDigit(beamA.eventCount), age);
      else          snprintf(out, sizeof(out), "@R%c%c%c------", lineBuf[2], status, beamA.hasEvent ? hexDigit(beamA.eventCount) : '-');
      Serial.write(out, 11); Serial.write('\n');
    } else if (lineBuf[0] != '@' && lineBuf[0]) {
      handleCommonCommand(lineBuf);
    }
  }
  alignPrint();
}

// ================================================================ FINISH role (radio master)
bool          linkUp = false, seenSlaveOnce = false;
uint8_t       pollSeq = 0, missedPolls = 0, syncSamples = 0;
bool          awaitingReply = false, pendingFinish = false;
unsigned long tPollSentUs = 0, pollSentMs = 0, nextPollMs = 0;
unsigned long lastRttUs = 0, bestRttUs = 0;
char          lastSeenEvt = '-', slaveStatus = '?';

void armFinish() { state = ARMED; pendingFinish = false; Serial.println(F("[ARMED] waiting for the START beam...")); lastShownBucket = -1; lastShownPresent = -1; }
void sendPoll() {
  pollSeq = (pollSeq + 1) & 0x0F;
  char out[13];
  snprintf(out, sizeof(out), "@P%c--------", hexDigit(pollSeq));
  tPollSentUs = micros();
  Serial.write(out, 11); Serial.write('\n');    // exactly 12 bytes = one SRF packet
  pollSentMs = millis();
  awaitingReply = true;
}
void finalizeRun() {
  unsigned long elapsed = tFinishUs - tStartUs;
  if (elapsed > 0x7FFFFFFFUL) Serial.println(F("!! Finish came before start? Ignored. Re-arming."));
  else {
    char note[40];
    snprintf(note, sizeof(note), "(start +/-%lu ms)", (bestRttUs / 2 + 500) / 1000);
    reportRun(elapsed, note);
  }
  state = COOLDOWN; cooldownStartMs = millis(); pendingFinish = false;
}
void handleReply() {
  if (strlen(lineBuf) != 11) return;
  if (lineBuf[2] != hexDigit(pollSeq)) return;      // late reply to an older poll
  awaitingReply = false; missedPolls = 0;
  lastRttUs = micros() - tPollSentUs;
  if (!linkUp) { linkUp = true; Serial.print(F("Radio link UP (round trip ")); Serial.print(lastRttUs / 1000); Serial.println(F(" ms)")); }
  slaveStatus = lineBuf[3];
  char evt = lineBuf[4];
  bool ageValid = lineBuf[5] != '-';
  long ageUs = ageValid ? atol(lineBuf + 5) * 10L : -1;

  if (!seenSlaveOnce) { seenSlaveOnce = true; lastSeenEvt = evt; return; }

  if (evt != '-' && evt != lastSeenEvt) {
    lastSeenEvt = evt;
    if (!ageValid) return;
    if (state == COOLDOWN) return;                  // same athlete re-breaking the start
    if (state == RUNNING || state == SYNCING) Serial.println(F("New START while running - restarting the clock."));
    state = SYNCING; syncSamples = 0; bestRttUs = 0xFFFFFFFFUL; pendingFinish = false; runStartMs = millis();
  }
  if (state == SYNCING && evt == lastSeenEvt && ageValid) {
    unsigned long est = tPollSentUs + lastRttUs / 2 - (unsigned long)ageUs;
    if (lastRttUs < bestRttUs) { bestRttUs = lastRttUs; tStartUs = est; }
    if (++syncSamples >= SYNC_SAMPLES) {
      state = RUNNING; runStartMs = millis();
      Serial.print(F("START! clock running (sync +/-")); Serial.print((bestRttUs / 2 + 500) / 1000); Serial.println(F(" ms)"));
      holdingResult = false; dispTitle("Running"); dispTime(0);
      beep(1200, 60);
      if (pendingFinish) finalizeRun();
    }
  }
}
void finishGateLoop() {
  unsigned long nowMs = millis();
  if (beamA.update()) {
    unsigned long nowUs = beamA.lastTriggerUs;
    if (lapMode) { beep(2000, 80); handleLap(nowUs); }
    else {
      beep(2000, 80);
      switch (state) {
        case RUNNING:  tFinishUs = nowUs; finalizeRun(); break;
        case SYNCING:  tFinishUs = nowUs; pendingFinish = true; break;
        case ARMED:    Serial.println(F("FINISH beam broken but no start was seen - ignored. (Type 'l' for LAP mode.)")); break;
        case COOLDOWN: break;
      }
    }
  }
  digitalWrite(LED_BUILTIN, beamA.beamPresent());

  if (readLine()) {
    if (lineBuf[0] == '@') { if (lineBuf[1] == 'R') handleReply(); }
    else if (lineBuf[0]) {
      if      (!strcmp(lineBuf, "r")) { linkUp = false; seenSlaveOnce = false; lapArmed = false; armFinish(); }
      else if (!strcmp(lineBuf, "l")) {
        lapMode = !lapMode; lapArmed = false;
        EEPROM.update(EE_LAP_ADDR, lapMode ? 1 : 0);   // sticky, so a reset does not lose it
        Serial.println(lapMode ? F("LAP mode ON: this gate's own beam, timed between breaks") : F("LAP mode OFF"));
      }
      else handleCommonCommand(lineBuf);
    }
  }

  bool clockRunning2 = lapMode ? lapArmed : (state == RUNNING);
  liveClockTick(clockRunning2, lapMode ? lapStartUs : tStartUs, nowMs);
  if (!clockRunning2) {
    bool present = beamA.beamPresent();
    if (present) beamGoneSinceMs = 0;
    else if (beamGoneSinceMs == 0) beamGoneSinceMs = nowMs;
    bool longGone = !present && (nowMs - beamGoneSinceMs > BEAM_GONE_GRACE_MS);
    if (longGone) holdingResult = false;
    if (!holdingResult) dispArmed(present, beamA.reading);
  }

  if (awaitingReply && nowMs - pollSentMs > POLL_TIMEOUT_MS) {
    awaitingReply = false;
    if (missedPolls < 255) missedPolls++;
    if (linkUp && missedPolls >= LINK_LOST_AFTER) { linkUp = false; seenSlaveOnce = false; Serial.println(F("Radio link DOWN (no reply from the START gate)")); }
  }
  if (!awaitingReply && radioOk) {
    unsigned long interval = (state == SYNCING) ? 0 : POLL_INTERVAL_MS;
    if (nowMs - nextPollMs >= interval) { nextPollMs = nowMs; sendPoll(); }
  }

  if ((state == RUNNING || state == SYNCING) && nowMs - runStartMs > RUN_TIMEOUT_MS) { Serial.println(F("No finish within 60 s - run abandoned.")); armFinish(); }
  if (state == COOLDOWN && nowMs - cooldownStartMs > COOLDOWN_MS) armFinish();
  if (!awaitingReply && !alignMode && (state == ARMED || state == COOLDOWN) && nowMs - lastStatusMs >= STATUS_EVERY_MS) {
    lastStatusMs = nowMs;
    Serial.print(state == ARMED ? F("[ARMED] ") : F("[COOLDOWN] "));
    Serial.print(F("link "));
    if (linkUp) { Serial.print(F("OK rtt ")); Serial.print((lastRttUs + 500) / 1000); Serial.print(F("ms")); }
    else Serial.print(radioOk ? F("DOWN") : F("NO RADIO"));
    Serial.print(F(" | START beam: "));
    Serial.print(!linkUp ? "?" : slaveStatus == 'A' ? "OK" : slaveStatus == 'B' ? "BLOCKED" : "TOO DIM/NOT ALIGNED");
    Serial.print(F(" | FINISH beam: ")); Serial.print(beamA.stateText());
    Serial.print(F(" (A0=")); Serial.print(beamA.reading); Serial.println(')');
  }
  alignPrint();
}

// ================================================================ setup / loop
void setup() {
  pinMode(PIN_LED_A, OUTPUT); pinMode(PIN_LED_B, OUTPUT); pinMode(LED_BUILTIN, OUTPUT); pinMode(PIN_BUZZER, OUTPUT);
  Serial.begin(SERIAL_BAUD);
  delay(300);
  loadSettings();
  if (EEPROM.read(EE_MAGIC_ADDR) == EE_MAGIC) lapMode = (EEPROM.read(EE_LAP_ADDR) == 1);
  Serial.println();
  Serial.println(F("================ BreakBeam speed gate ================"));
  Serial.print(F("Role: "));    Serial.println(roleName(role));
  Serial.print(F("Display: ")); Serial.println(dispName(disp));
  if (lapMode) Serial.println(F("LAP mode is ON (one beam, timed between breaks)"));
  if (role == ROLE_UNSET) Serial.println(F(">>> Type  role solo  (both beams, one board)  or  role start  or  role finish"));

  // D8 must always be driven, never left floating. With a shield attached a floating
  // D8 leaves its two analogue switches half open, which couples the radio onto the
  // shared serial lines and corrupts characters in both directions.
  bool wantRadio = (role == ROLE_START || role == ROLE_FINISH);
  pinMode(PIN_RADIO_EN, OUTPUT);
  digitalWrite(PIN_RADIO_EN, wantRadio ? HIGH : LOW);
  if (wantRadio) radioSelfTest();
  else Serial.println(F("Radio held off in this role (D8 driven low)."));

  dispInit();
  dispTitle("BreakBeam"); dispDashes("ready");

  beamA.seed();
  Serial.print(F("START sensor (A0) baseline: ")); Serial.print(beamA.baseline());
  Serial.println(beamA.baseline() >= BEAM_MIN_LEVEL ? F("  (beam seen)") : F("  (no beam - point the laser at the sensor)"));
  if (role == ROLE_SOLO) {
    beamB.seed();
    Serial.print(F("FINISH sensor (A1) baseline: ")); Serial.print(beamB.baseline());
    Serial.println(beamB.baseline() >= BEAM_MIN_LEVEL ? F("  (beam seen)") : F("  (no beam - point the laser at the sensor)"));
  }

  printHelp();
  if (role == ROLE_SOLO)   armSolo();
  if (role == ROLE_FINISH) armFinish();
  if (role == ROLE_START)  Serial.println(F("START gate ready - waiting for the finish gate to poll me."));
  beep(1000, 80);
}

void loop() {
  switch (role) {
    case ROLE_SOLO:   soloLoop();       break;
    case ROLE_START:  startGateLoop();  break;
    case ROLE_FINISH: finishGateLoop(); break;
    default: {
      beamA.update();
      static unsigned long lastNag = 0;
      if (millis() - lastNag > 3000) { lastNag = millis(); Serial.println(F(">>> Role not set. Type  role solo  or  role start  or  role finish")); }
      if (readLine() && lineBuf[0]) handleCommonCommand(lineBuf);
    }
  }
}
