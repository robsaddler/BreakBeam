/*
  GateTest - the first three things, and nothing else.

  Before porting any timing logic, prove on the bench that:
    1. the ESP32 flashes and talks to the PC,
    2. the beam sensor reads the same numbers it did on the Uno,
    3. the SRF radio answers, on a serial port of its very own.

  That third point is the whole reason for moving off the Uno. On the Uno the radio
  and the USB cable shared one port, and nearly every bug in this project came from
  that. Here the radio is on Serial2 and the USB is on Serial. They never meet, so
  there is no echo war, no shredded commands, and no upload that fails because a
  shield is wired up.

  Wiring, ESP32 30-pin DOIT DevKit V1
  ------------------------------------
    SRF shield 3V3  -> 3V3        the radio itself
    SRF shield 5V   -> VIN        the shield's two analogue switches
    SRF shield GND  -> GND
    SRF shield D0   -> GPIO16     labelled RX2. Radio talks, ESP32 listens.
    SRF shield D1   -> GPIO17     labelled TX2. ESP32 talks, radio listens.
    SRF shield D8   -> GPIO4      radio enable

    Sensor: phototransistor long leg -> 3V3, short leg -> GPIO34,
            10k from GPIO34 to GND.

  Note the sensor runs on 3.3 V here, not 5 V. The divider behaves the same way and
  the readings land in the same place, because the reading is a fraction of whatever
  the supply is, not an absolute voltage.

  GPIO34 is not an arbitrary choice. The ESP32 has two banks of analogue pins and one
  of them stops working the moment wifi is switched on. 34 is in the bank that
  survives it, which matters because wifi is the point of the exercise.

  Type anything into the serial monitor and it goes straight out to the radio, so you
  can poke at it by hand. Everything the radio says comes back.
*/

static const int PIN_SENSOR   = 34;   // ADC1, survives wifi, input only
static const int PIN_RADIO_EN = 4;
static const int PIN_RADIO_RX = 16;   // silkscreen RX2
static const int PIN_RADIO_TX = 17;   // silkscreen TX2

static const unsigned long RADIO_BAUD = 115200;   // SRF factory default

bool radioOk = false;

// The SRF ends its AT replies with a carriage return. Collect until one arrives and
// say whether the line matched.
bool waitForAtReply(const char* want, unsigned long timeoutMs) {
  char buf[16]; uint8_t n = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < timeoutMs) {
    if (!Serial2.available()) { delay(1); continue; }
    char c = (char)Serial2.read();
    if (c == '\n') { n = 0; continue; }
    if (c == '\r') { buf[n] = 0; if (strcmp(buf, want) == 0) return true; n = 0; continue; }
    if (n < sizeof(buf) - 1) buf[n++] = c;
  }
  return false;
}

// One second of silence, three plus signs, one second of silence, then "OK".
bool atEnter() {
  Serial2.flush();
  delay(1100);
  Serial2.print("+++");
  return waitForAtReply("OK", 2000);
}

void radioSelfTest() {
  Serial.println(F("Radio self-test on Serial2..."));
  if (atEnter()) {
    Serial2.print("ATDN\r");
    waitForAtReply("OK", 1000);
    radioOk = true;
    Serial.println(F("Radio: OK - the SRF answered at 115200 on its own serial port."));
    return;
  }
  // An older module may still be at 9600. Move it up and keep it there.
  Serial2.begin(9600, SERIAL_8N1, PIN_RADIO_RX, PIN_RADIO_TX);
  if (atEnter()) {
    Serial.println(F("Radio answered at 9600 - switching it to 115200..."));
    Serial2.print("ATBD1C200\r"); waitForAtReply("OK", 1000);
    Serial2.print("ATWR\r");      waitForAtReply("OK", 1000);
    Serial2.print("ATAC\r");      delay(200);
    Serial2.begin(RADIO_BAUD, SERIAL_8N1, PIN_RADIO_RX, PIN_RADIO_TX);
    delay(100);
    Serial2.print("ATDN\r");      waitForAtReply("OK", 1000);
    radioOk = true;
    Serial.println(F("Radio: OK - now at 115200."));
    return;
  }
  Serial2.begin(RADIO_BAUD, SERIAL_8N1, PIN_RADIO_RX, PIN_RADIO_TX);
  Serial.println(F("Radio: NO ANSWER. Check the six wires, and that D8 reaches GPIO4."));
}

void setup() {
  Serial.begin(115200);
  delay(400);
  Serial.println();
  Serial.println(F("=============== BreakBeam ESP32 bench test ==============="));

  // Read 0 to 1023 like the Uno did, rather than the ESP32's native 0 to 4095, so
  // every threshold measured on the breadboard carries straight over.
  analogReadResolution(10);

  pinMode(PIN_RADIO_EN, OUTPUT);
  digitalWrite(PIN_RADIO_EN, HIGH);          // connect the radio
  Serial2.begin(RADIO_BAUD, SERIAL_8N1, PIN_RADIO_RX, PIN_RADIO_TX);
  delay(50);

  radioSelfTest();

  long sum = 0;
  for (int i = 0; i < 64; i++) { sum += analogRead(PIN_SENSOR); delay(2); }
  Serial.print(F("Sensor baseline on GPIO34: ")); Serial.println(sum / 64);
  Serial.println(F("Aim a light at it: about 1000 on target, under 200 blocked."));
  Serial.println(F("Anything you type goes out to the radio."));
}

void loop() {
  static unsigned long lastMs = 0;
  unsigned long nowMs = millis();

  if (nowMs - lastMs >= 250) {
    lastMs = nowMs;
    int v = analogRead(PIN_SENSOR);
    Serial.print(F("sensor=")); Serial.print(v);
    Serial.print(F("  radio=")); Serial.println(radioOk ? F("OK") : F("no answer"));
  }

  // Hand-poke the radio from the serial monitor, and see whatever it says back.
  while (Serial.available())  Serial2.write(Serial.read());
  while (Serial2.available()) Serial.write(Serial2.read());
}
