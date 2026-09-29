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
| `role start` / `role finish` | Set this board's job. Saved to EEPROM, board reboots. |
| `a` | Align mode: live sensor numbers 5x a second. Use while pointing the laser. |
| `l` | Lap mode (finish gate): time between two breaks of its own beam. Tests one gate alone. |
| `r` | Re-arm. |
| `s` | Buzzer on/off. |
| `h` | Help. |

## Roadmap

1. **Phase 1 - radio link**: shields on, no other wiring. Finish gate reports `link OK rtt N ms`.
2. **Phase 2 - beam sensor**: LDR + laser on a breadboard, one board at a time (shield off). Align mode and lap mode.
3. **Phase 3 - two gates in the garage**: both together. Walk through both beams, read the time.
4. **Phase 4 - outdoors**: hoods for the LDRs, tripods, battery power, faster sensor (phototransistor).
5. **Phase 5 - productise**: 3D-printed housings (Bambu P2S), display on the finish gate, logging app.
