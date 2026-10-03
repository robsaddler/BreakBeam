# BreakBeam - laser speed gates for sprint timing

Two Arduino Unos, two Ciseco R017 SRF radio shields, two laser pens and two LDRs
become a pair of wireless timing gates. The athlete breaks the START beam, runs,
breaks the FINISH beam, and the finish gate prints the time on your PC.

**Start here:** [docs/GUIDE.md](docs/GUIDE.md) - the step-by-step build guide.

## Layout

| Path | What it is |
|---|---|
| `BreakBeam/BreakBeam.ino` | The firmware. One image for both gates; the role is set once over USB and remembered. |
| `platformio.ini` | PlatformIO project (open this folder in VS Code). Also builds with the Arduino IDE: open `BreakBeam/BreakBeam.ino`. |
| `tools/gate.ps1` | PowerShell helper to send a command to a gate and watch its output without an IDE. |
| `docs/GUIDE.md` | The absolute-beginner build guide, phase by phase. |
| `ciseco-master/` | Original Ciseco documentation and schematics (not in git). The R017 datasheet and schematic are the key ones. |

## How the timing works (short version)

The finish gate is the timekeeper. Ten times a second it radios a 12-byte poll to
the start gate, which answers "my beam was broken N microseconds ago". The finish
gate knows when it sent the poll and how long the round trip took, so it can put
the start event on its own clock to within about a millisecond. When its own beam
breaks it subtracts the two. No clock sync, no drift problem.

## Serial commands (115200 baud, newline endings)

| Type | Effect |
|---|---|
| `role solo` | Both beams on this one board (A0 starts, A1 stops). No radio or shield needed. |
| `role start` / `role finish` | Set this board's job for radio operation. Saved to EEPROM, board reboots. |
| `display none\|lcd\|i2clcd\|tm1637\|max7219` | Which display is wired up. Remembered. |
| `t` | Re-initialise the display and show 12.34. Fixes a garbled screen. |
| `a` | Align mode: live sensor numbers 5x a second. Use while pointing the laser. |
| `l`, `lap on`, `lap off` | Lap mode, **solo role only**: time between two breaks of the start beam alone. Works with a single sensor and is remembered across resets. |
| `r` | Re-arm. |
| `s` | Buzzer on/off. |
| `h` | Help. |

## Roadmap

1. **Phase 1 - radio link** (done, 15-16 ms round trip): shields on, no other wiring.
2. **Phase 2 - beam sensor**: LDR + laser on a breadboard, one board at a time (shield off). Align mode and lap mode.
3. **Phase 2B - both beams on one board**: two sensors joined by a long cable, plus a display. The whole product except the radio, and it needs no soldering.
4. **Phase 3 - two gates over the radio** (done, 3 Oct 2026): first wireless time 4.095 s, start event synced to within 8 ms. The shield needs only six wires (3V3, 5V, GND, D0, D1, D8), so no soldering was needed.
5. **Phase 4 - outdoors**: hoods for the LDRs, tripods, battery power, faster sensor (phototransistor).
6. **Phase 5 - productise**: 3D-printed housings (Bambu P2S), display on the finish gate, logging app.
