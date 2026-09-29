/*
  BreakBeam - laser speed-gate firmware for Arduino Uno + Ciseco R017 SRF radio shield.

  ONE firmware runs on BOTH gates.  You tell each board its job once, over the USB
  serial monitor (115200 baud, "newline" line ending), and it remembers it in EEPROM:

      role start     - the gate the athlete crosses first (remote, no PC needed)
      role finish    - the gate that keeps the clock and reports times to your PC

  How the timing works
  --------------------
  * Each gate watches a laser beam falling on an LDR (light dependent resistor).
    A sudden drop in light = beam broken = an event, time-stamped with micros().
  * The FINISH gate is the "master".  Ten times a second it sends a tiny 12-byte
    poll over the radio.  The START gate replies with "my beam was broken N
    microseconds ago".  The master knows when it sent the poll and how long the
    round trip took, so it can place the start event on its OWN clock to within
    about a millisecond.  No clock synchronisation, and the two boards' clocks
    drifting apart does not matter because the "age" is only ever a few ms old.
  * When the finish beam breaks, elapsed = finish time - start time.  Done.

  Radio facts (R017 datasheet + Ciseco AT reference)
  ---------------------------------------------------
  * The radio's serial is on D0/D1, SHARED with the USB link.  Everything this
    board prints goes to the PC and over the air; everything the other gate sends
    arrives mixed with anything typed on the PC.  The protocol copes: radio frames
    start with '@' and are exactly 12 bytes, so the SRF transmits them at once
    (its packet size is 12 bytes; shorter data waits for a 16 ms timeout).
  * D8 HIGH connects the radio.  Uploads work with the shield fitted because the
    bootloader leaves D8 low.
  * Factory defaults: 115200 baud, PANID 5AA5, 868.3 MHz (EU).  "+++" = AT mode.

  Pins (identical on both gates)
  ------------------------------
    A0  LDR divider: LDR from 5V to A0, 10k resistor from A0 to GND
        (more light on the LDR = bigger number)
    D7  LED via 220R to GND: ON = the laser is landing on the LDR (D13 "L" copies it)
    D9  piezo buzzer: + to D9, - to GND. Beeps when the beam is broken.
    D8  radio enable - used by the shield, connect nothing here.
*/

#include <Arduino.h>
#include <EEPROM.h>
#include <avr/wdt.h>

// ---------------------------------------------------------------- pins
static const uint8_t PIN_BEAM     = A0;
static const uint8_t PIN_LED      = 7;
static const uint8_t PIN_BUZZER   = 9;
static const uint8_t PIN_RADIO_EN = 8;

// ---------------------------------------------------------------- tunables
static const unsigned long SERIAL_BAUD      = 115200; // SRF shield factory default
static const unsigned long LOCKOUT_MS       = 800;    // ignore re-breaks (arms, legs) for this long
static const int           MIN_DROP         = 80;     // light must fall by this many ADC counts...
static const uint8_t       DROP_PERCENT     = 20;     // ...or this % of baseline, whichever is larger
static const int           BEAM_MIN_LEVEL   = 300;    // baseline below this = "no beam on the sensor"
static const unsigned long POLL_INTERVAL_MS = 100;    // master -> slave poll rate when idle
static const unsigned long POLL_TIMEOUT_MS  = 80;     // give up waiting for a reply after this
static const uint8_t       SYNC_SAMPLES     = 6;      // rapid polls used to pin down the start time
static const unsigned long RUN_TIMEOUT_MS   = 60000;  // no finish within this = abandon the run
static const unsigned long COOLDOWN_MS      = 1500;   // after a result, ignore beams for this long
static const unsigned long STATUS_EVERY_MS  = 2000;   // master prints a status line this often
static const uint8_t       LINK_LOST_AFTER  = 5;      // consecutive missed polls = link down

// ---------------------------------------------------------------- roles (kept in EEPROM)
enum Role : uint8_t { ROLE_UNSET = 0, ROLE_START = 1, ROLE_FINISH = 2 };
static const int     EE_MAGIC_ADDR = 0, EE_ROLE_ADDR = 1;
static const uint8_t EE_MAGIC      = 0xB3;
Role role = ROLE_UNSET;

Role loadRole() {
  if (EEPROM.read(EE_MAGIC_ADDR) != EE_MAGIC) return ROLE_UNSET;
  uint8_t r = EEPROM.read(EE_ROLE_ADDR);
  return (r == ROLE_START || r == ROLE_FINISH) ? (Role)r : ROLE_UNSET;
}
void saveRole(Role r) {
  EEPROM.update(EE_MAGIC_ADDR, EE_MAGIC);
  EEPROM.update(EE_ROLE_ADDR, (uint8_t)r);
}
const char* roleName(Role r) {
  return r == ROLE_START ? "START" : r == ROLE_FINISH ? "FINISH" : "NOT SET";
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

// ---------------------------------------------------------------- beam sensor
struct BeamSensor {
  int           reading        = 0;
  long          baseline16     = 0;      // baseline * 16 (fixed point)
  bool          blocked        = false;
  unsigned long blockedSinceMs = 0;
  unsigned long lastBaselineMs = 0;
  unsigned long lastTriggerUs  = 0;
  unsigned long lastTriggerMs  = 0;
  bool          hasEvent       = false;
  uint8_t       eventCount     = 0;      // +1 on every trigger, wraps 0..15

  int  baseline() const { return (int)(baseline16 / 16); }
  int  dropNeeded() const {
    int pct = (int)((long)baseline() * DROP_PERCENT / 100);
    return pct > MIN_DROP ? pct : MIN_DROP;
  }
  bool beamPresent() const { return !blocked && baseline() >= BEAM_MIN_LEVEL; }

  void seed() {
    long sum = 0;
    for (int i = 0; i < 64; i++) { sum += analogRead(PIN_BEAM); delay(2); }
    baseline16 = (sum / 64) * 16;
    reading = (int)(sum / 64);
    blocked = false;
  }

  // Call constantly. Returns true exactly once per new beam-break event.
  bool update() {
    reading = analogRead(PIN_BEAM);
    unsigned long nowUs = micros();
    unsigned long nowMs = millis();
    bool triggered = false;

    if (!blocked) {
      if (reading < baseline() - dropNeeded()) {
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
      if (reading > baseline() - dropNeeded() / 2) blocked = false;     // beam is back
      else if (nowMs - blockedSinceMs > 5000) seed();                    // stuck: re-learn "normal"
    }

    // Slowly follow slow changes in light (clouds, lamps) while the beam is unbroken.
    if (!blocked && nowMs - lastBaselineMs >= 20) {
      lastBaselineMs = nowMs;
      baseline16 += ((long)reading * 16 - baseline16) / 64;
    }
    bool led = beamPresent();
    digitalWrite(PIN_LED, led);
    digitalWrite(LED_BUILTIN, led);
    return triggered;
  }
  // Age of the last event in units of 10 us, or -1 if none / too old (> 9.99 s).
  long ageTensOfMicros() const {
    if (!hasEvent) return -1;
    unsigned long age = micros() - lastTriggerUs;
    if (age > 9999990UL) return -1;
    return (long)(age / 10);
  }
};
BeamSensor beam;

// ---------------------------------------------------------------- serial line reader
char    lineBuf[24];
uint8_t lineLen = 0;
bool readLine() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') { lineBuf[lineLen] = 0; lineLen = 0; return true; }
    if (lineLen < sizeof(lineBuf) - 1) lineBuf[lineLen++] = c;
    else lineLen = 0;   // garbage / too long: start again
  }
  return false;
}

// ---------------------------------------------------------------- radio (SRF) helpers
// Wait for a line ending in '\r' (the SRF's AT reply style). True if it equals `want`.
bool waitForAtReply(const char* want, unsigned long timeoutMs) {
  char buf[16]; uint8_t n = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < timeoutMs) {
    if (!Serial.available()) continue;
    char c = (char)Serial.read();
    if (c == '\n') { n = 0; continue; }   // a radio frame from the other gate may arrive mid-test: drop it
    if (c == '\r') { buf[n] = 0; if (strcmp(buf, want) == 0) return true; n = 0; continue; }
    if (n < sizeof(buf) - 1) buf[n++] = c;
  }
  return false;
}
bool atEnter() {                   // guard time, "+++", expect OK
  Serial.flush();
  delay(1100);
  Serial.print(F("+++"));
  return waitForAtReply("OK", 2000);
}
bool atCommand(const char* cmd) {  // e.g. "ATDN"; expects OK
  Serial.print(cmd); Serial.print('\r');
  return waitForAtReply("OK", 1000);
}
bool radioOk = false;
void radioSelfTest() {
  Serial.println(F("Radio self-test (you will see +++ATDN below - that is normal)..."));
  if (atEnter()) {
    radioOk = true;
    atCommand("ATDN");
    Serial.println(F("Radio: OK - SRF shield answered at 115200 baud."));
    return;
  }
  // Not at 115200? Older Ciseco radios default to 9600. Try that and move it to 115200.
  Serial.begin(9600);
  if (atEnter()) {
    Serial.println(F("Radio answered at 9600 - switching it to 115200 (ATBD 1C200)..."));
    atCommand("ATBD1C200");
    atCommand("ATWR");
    Serial.print(F("ATAC\r")); Serial.flush(); delay(200);
    Serial.begin(SERIAL_BAUD); delay(100);
    atCommand("ATDN");
    radioOk = true;
    Serial.println(F("Radio: OK - now at 115200 baud."));
    return;
  }
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("Radio: NO ANSWER. Is the SRF shield fitted? (Beam sensing still works.)"));
}

// ---------------------------------------------------------------- common helpers
void printSeconds(unsigned long us) {           // 1234567 us -> "1.235"
  unsigned long ms = (us + 500) / 1000;
  Serial.print(ms / 1000); Serial.print('.');
  unsigned int frac = ms % 1000;
  if (frac < 100) Serial.print('0');
  if (frac < 10)  Serial.print('0');
  Serial.print(frac);
}
char hexDigit(uint8_t v) { return "0123456789ABCDEF"[v & 0x0F]; }

bool alignMode = false;
unsigned long lastAlignPrintMs = 0;
void alignPrint() {
  if (!alignMode || millis() - lastAlignPrintMs < 200) return;
  lastAlignPrintMs = millis();
  Serial.print(F("A0=")); Serial.print(beam.reading);
  Serial.print(F("  baseline=")); Serial.print(beam.baseline());
  Serial.print(F("  triggers below=")); Serial.print(beam.baseline() - beam.dropNeeded());
  Serial.print(F("  beam="));
  Serial.println(beam.beamPresent() ? "OK" : (beam.blocked ? "BLOCKED" : "TOO DIM"));
}
void printHelp() {
  Serial.println(F("Commands (type, then Enter):"));
  Serial.println(F("  role start | role finish   set this board's job (it then reboots)"));
  Serial.println(F("  a   toggle ALIGN mode: live sensor numbers 5x per second"));
  Serial.println(F("  l   toggle LAP mode (finish gate): time between two breaks of ITS OWN beam"));
  Serial.println(F("  r   reset / re-arm"));
  Serial.println(F("  s   toggle buzzer"));
  Serial.println(F("  h   this help"));
}
// Returns true if the line was a command it handled.
bool handleCommonCommand(const char* line) {
  if (strcmp(line, "role start") == 0)  { saveRole(ROLE_START);  Serial.println(F("Role saved: START"));  rebootNow(); }
  if (strcmp(line, "role finish") == 0) { saveRole(ROLE_FINISH); Serial.println(F("Role saved: FINISH")); rebootNow(); }
  if (strcmp(line, "a") == 0) { alignMode = !alignMode; Serial.println(alignMode ? F("ALIGN mode ON") : F("ALIGN mode OFF")); return true; }
  if (strcmp(line, "s") == 0) { soundOn = !soundOn; Serial.println(soundOn ? F("Buzzer ON") : F("Buzzer OFF")); return true; }
  if (strcmp(line, "h") == 0 || strcmp(line, "?") == 0) { printHelp(); return true; }
  return false;
}

// ================================================================ START gate (slave)
void startGateLoop() {
  if (beam.update()) {
    beep(1500, 60);
    Serial.print(F("START beam broken (event ")); Serial.print(beam.eventCount); Serial.println(')');
  }
  if (readLine()) {
    if (lineBuf[0] == '@' && lineBuf[1] == 'P' && lineBuf[2] != 0) {
      // Poll "@P<seq>--------"  ->  reply "@R<seq><status><evt><age6>"  (12 bytes with the newline)
      char status = beam.beamPresent() ? 'A' : (beam.blocked ? 'B' : 'N');
      long age = beam.ageTensOfMicros();
      char out[13];
      if (age >= 0) snprintf(out, sizeof(out), "@R%c%c%c%06ld", lineBuf[2], status, hexDigit(beam.eventCount), age);
      else          snprintf(out, sizeof(out), "@R%c%c%c------", lineBuf[2], status, beam.hasEvent ? hexDigit(beam.eventCount) : '-');
      Serial.write(out, 11); Serial.write('\n');
    } else if (lineBuf[0] != '@' && lineBuf[0]) {
      handleCommonCommand(lineBuf);   // anything else is a PC command or the other gate's chatter: never answer it
    }
  }
  alignPrint();
}

// ================================================================ FINISH gate (master)
enum RunState : uint8_t { ARMED, SYNCING, RUNNING, COOLDOWN };
RunState      state = ARMED;
bool          linkUp = false, seenSlaveOnce = false, lapMode = false;
uint8_t       pollSeq = 0, missedPolls = 0, syncSamples = 0;
bool          awaitingReply = false, pendingFinish = false;
unsigned long tPollSentUs = 0, pollSentMs = 0, nextPollMs = 0, lastStatusMs = 0;
unsigned long lastRttUs = 0, bestRttUs = 0;
unsigned long tStartUs = 0, tFinishUs = 0, runStartMs = 0, cooldownStartMs = 0;
unsigned long lapStartUs = 0; bool lapArmed = false;
char          lastSeenEvt = '-', slaveStatus = '?';
unsigned int  runNumber = 0;

void reArm() { state = ARMED; pendingFinish = false; Serial.println(F("[ARMED] waiting for the START beam...")); }

void sendPoll() {
  pollSeq = (pollSeq + 1) & 0x0F;
  char out[13];
  snprintf(out, sizeof(out), "@P%c--------", hexDigit(pollSeq));
  tPollSentUs = micros();
  Serial.write(out, 11); Serial.write('\n');   // exactly 12 bytes = one SRF packet, sent immediately
  pollSentMs = millis();
  awaitingReply = true;
}
void finalizeRun() {
  unsigned long elapsed = tFinishUs - tStartUs;
  runNumber++;
  if (elapsed > 0x7FFFFFFFUL) {
    Serial.println(F("!! Finish came before start? Ignored. Re-arming."));
  } else {
    Serial.print(F("=== RUN ")); Serial.print(runNumber); Serial.print(F("   TIME "));
    printSeconds(elapsed); Serial.print(F(" s"));
    Serial.print(F("   (start-time uncertainty about +/-")); Serial.print((bestRttUs / 2 + 500) / 1000);
    Serial.println(F(" ms) ==="));
    Serial.print(F("CSV,")); Serial.print(runNumber); Serial.print(','); printSeconds(elapsed); Serial.println();
    beep(2000, 120); delay(150); beep(2600, 200);
  }
  state = COOLDOWN; cooldownStartMs = millis(); pendingFinish = false;
}
void handleReply() {
  // "@R<seq><status><evt><age6>"
  if (strlen(lineBuf) != 11) return;
  if (lineBuf[2] != hexDigit(pollSeq)) return;     // late reply to an older poll: ignore
  awaitingReply = false; missedPolls = 0;
  lastRttUs = micros() - tPollSentUs;
  if (!linkUp) {
    linkUp = true;
    Serial.print(F("Radio link UP (round trip ")); Serial.print(lastRttUs / 1000); Serial.println(F(" ms)"));
  }
  slaveStatus = lineBuf[3];
  char evt = lineBuf[4];
  bool ageValid = lineBuf[5] != '-';
  long ageUs = ageValid ? atol(lineBuf + 5) * 10L : -1;

  if (!seenSlaveOnce) { seenSlaveOnce = true; lastSeenEvt = evt; return; }   // history is not news

  if (evt != '-' && evt != lastSeenEvt) {            // a NEW start event
    lastSeenEvt = evt;
    if (!ageValid) return;                            // too old to use
    if (state == COOLDOWN) return;                    // just finished: same athlete re-breaking the start
    if (state == RUNNING || state == SYNCING) Serial.println(F("New START while running - restarting the clock."));
    state = SYNCING; syncSamples = 0; bestRttUs = 0xFFFFFFFFUL; pendingFinish = false; runStartMs = millis();
  }
  if (state == SYNCING && evt == lastSeenEvt && ageValid) {
    unsigned long est = tPollSentUs + lastRttUs / 2 - (unsigned long)ageUs;
    if (lastRttUs < bestRttUs) { bestRttUs = lastRttUs; tStartUs = est; }
    if (++syncSamples >= SYNC_SAMPLES) {
      state = RUNNING; runStartMs = millis();
      Serial.print(F("START! clock running (sync +/-")); Serial.print((bestRttUs / 2 + 500) / 1000); Serial.println(F(" ms)"));
      beep(1200, 60);
      if (pendingFinish) finalizeRun();
    }
  }
}
void handleFinishBeam() {
  unsigned long nowUs = micros();
  if (lapMode) {
    if (!lapArmed) { lapArmed = true; lapStartUs = nowUs; Serial.println(F("LAP: clock started on this beam")); beep(1200, 60); }
    else { Serial.print(F("LAP TIME ")); printSeconds(nowUs - lapStartUs); Serial.println(F(" s")); lapStartUs = nowUs; beep(2000, 150); }
    return;
  }
  beep(2000, 80);
  switch (state) {
    case RUNNING:  tFinishUs = nowUs; finalizeRun(); break;
    case SYNCING:  tFinishUs = nowUs; pendingFinish = true; break;
    case ARMED:    Serial.println(F("FINISH beam broken but no start was seen - ignored. (Type 'l' for single-gate LAP mode.)")); break;
    case COOLDOWN: break;
  }
}
void printStatus() {
  Serial.print(state == ARMED ? F("[ARMED] ") : state == COOLDOWN ? F("[COOLDOWN] ") : F("[RUNNING] "));
  Serial.print(F("link "));
  if (linkUp) { Serial.print(F("OK rtt ")); Serial.print((lastRttUs + 500) / 1000); Serial.print(F("ms")); }
  else Serial.print(radioOk ? F("DOWN") : F("NO RADIO"));
  Serial.print(F(" | START beam: "));
  Serial.print(!linkUp ? "?" : slaveStatus == 'A' ? "OK" : slaveStatus == 'B' ? "BLOCKED" : "TOO DIM/NOT ALIGNED");
  Serial.print(F(" | FINISH beam: "));
  Serial.print(beam.beamPresent() ? "OK" : (beam.blocked ? "BLOCKED" : "TOO DIM/NOT ALIGNED"));
  Serial.print(F(" (A0=")); Serial.print(beam.reading); Serial.println(')');
}
void finishGateLoop() {
  unsigned long nowMs = millis();
  if (beam.update()) handleFinishBeam();

  if (readLine()) {
    if (lineBuf[0] == '@') { if (lineBuf[1] == 'R') handleReply(); }
    else if (lineBuf[0]) {
      if      (strcmp(lineBuf, "r") == 0) { linkUp = false; seenSlaveOnce = false; reArm(); }
      else if (strcmp(lineBuf, "l") == 0) { lapMode = !lapMode; lapArmed = false; Serial.println(lapMode ? F("LAP mode ON: break this gate's beam twice to get a time") : F("LAP mode OFF")); }
      else handleCommonCommand(lineBuf);   // unknown text is ignored silently (it may be the other gate's chatter)
    }
  }

  // Poll scheduling: strict request -> reply -> pause. Never talk over the slave.
  if (awaitingReply && nowMs - pollSentMs > POLL_TIMEOUT_MS) {
    awaitingReply = false;
    if (missedPolls < 255) missedPolls++;
    if (linkUp && missedPolls >= LINK_LOST_AFTER) {
      linkUp = false; seenSlaveOnce = false;
      Serial.println(F("Radio link DOWN (no reply from the START gate)"));
    }
  }
  if (!awaitingReply && radioOk) {
    unsigned long interval = (state == SYNCING) ? 0 : POLL_INTERVAL_MS;
    if (nowMs - nextPollMs >= interval) { nextPollMs = nowMs; sendPoll(); }
  }

  // Housekeeping
  if ((state == RUNNING || state == SYNCING) && nowMs - runStartMs > RUN_TIMEOUT_MS) {
    Serial.println(F("No finish within 60 s - run abandoned.")); reArm();
  }
  if (state == COOLDOWN && nowMs - cooldownStartMs > COOLDOWN_MS) reArm();
  if (!awaitingReply && !alignMode && (state == ARMED || state == COOLDOWN) && nowMs - lastStatusMs >= STATUS_EVERY_MS) {
    lastStatusMs = nowMs; printStatus();
  }
  alignPrint();
}

// ================================================================ setup / loop
void setup() {
  pinMode(PIN_LED, OUTPUT); pinMode(LED_BUILTIN, OUTPUT); pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RADIO_EN, OUTPUT); digitalWrite(PIN_RADIO_EN, HIGH);   // connect the SRF radio
  Serial.begin(SERIAL_BAUD);
  delay(300);
  role = loadRole();
  Serial.println();
  Serial.println(F("================ BreakBeam speed gate ================"));
  Serial.print(F("Role: ")); Serial.println(roleName(role));
  if (role == ROLE_UNSET) Serial.println(F(">>> Type  role start  or  role finish  and press Enter."));
  radioSelfTest();
  beam.seed();
  Serial.print(F("Beam sensor baseline: ")); Serial.print(beam.baseline());
  Serial.println(beam.baseline() >= BEAM_MIN_LEVEL ? F("  (beam seen)") : F("  (no beam yet - point the laser at the LDR)"));
  printHelp();
  if (role == ROLE_FINISH) reArm();
  if (role == ROLE_START)  Serial.println(F("START gate ready - waiting for the finish gate to poll me."));
  beep(1000, 80);
}

void loop() {
  switch (role) {
    case ROLE_START:  startGateLoop();  break;
    case ROLE_FINISH: finishGateLoop(); break;
    default: {
      beam.update();
      static unsigned long lastNag = 0;
      if (millis() - lastNag > 3000) { lastNag = millis(); Serial.println(F(">>> Role not set. Type  role start  or  role finish")); }
      if (readLine() && lineBuf[0]) handleCommonCommand(lineBuf);
    }
  }
}
