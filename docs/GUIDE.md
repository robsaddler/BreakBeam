# BreakBeam build guide (absolute beginner edition)

Read this in order. Each phase ends with a test that must pass before you go on.
Nothing here needs soldering at all. The shields have no sockets on top, but the
radio only uses six Arduino pins, so six jumper wires replace them (see Phase 3).

---

## Phase 0 - know your boards

### The four LEDs on an Arduino Uno

| Label on the board | Colour | Meaning |
|---|---|---|
| **ON** | green | Power. Should always be lit when plugged in. |
| **L** | orange | Wired to pin 13. Whatever program is loaded controls it. Blinking slowly = the factory "Blink" example. With BreakBeam loaded it means **"the beam is landing on the sensor"**. |
| **TX** | orange | Flickers when the board is *sending* data to the PC over USB. Steady-looking = sending constantly. |
| **RX** | orange | Flickers when the board is *receiving* data from the PC. |

Your board "A" was blinking L slowly: it still had the Blink example loaded. Your board
"B" had L on and TX flickering: it had a program that printed to the PC non-stop.
Both have now been overwritten with the BreakBeam firmware, so that history is gone.

### Which board is which

| PC port | Role | What its L LED does now |
|---|---|---|
| **COM3** | FINISH gate (the timekeeper, stays plugged into the PC) | Copies the beam sensor. With nothing wired to A0 it flickers randomly. |
| **COM4** | START gate | Same. |

Put a strip of masking tape on each and write **FINISH** and **START** on them. To find
out which physical board is COM3: unplug one board's USB cable and ask Claude which port
disappeared, or open Device Manager > Ports and watch which "Arduino Uno (COMx)" vanishes.

### The radio shields (Ciseco R017 SRF Shield)

Straight from the datasheet: *"There are no jumpers to worry about, no configuration
to be done, simply plug in and begin coding."* What you should know:

- The radio talks on the same two wires (D0/D1) as the USB link. Everything the board
  prints goes to the PC **and** over the air. The firmware is written for this.
- Pin **D8** switches the radio on. The firmware does that. Never wire anything to D8.
- Uploading new code works with the shield fitted. No need to remove it.
- Two small pads marked **RST** on the shield: if they are shorted at power-up the radio
  goes back to factory settings. Don't touch them unless the troubleshooting table says to.
- Factory settings: 115200 baud, network ID (PANID) 5AA5, 868 MHz (UK legal band).
  Both shields left the factory identical, which is exactly what we want.

---

## Phase 1 - prove the radio link (no wiring, no breadboard)

**Goal:** the FINISH gate says `Radio link UP (round trip N ms)`.

1. Unplug both USB cables.
2. Look at the top of a shield. Rob's shields have **empty holes** (no sockets), so
   Phase 3 will need stackable headers. Carry on regardless.
3. Fit a shield onto each Uno. The shield's pins point down into the Uno's black
   sockets. Line up the pin marked **D0/RX** (or the end of the 8-pin row nearest the
   USB socket) and the corner pins, check every pin has a hole, then press down evenly.
   The antenna end of the shield (the squiggly copper track) normally sits over the
   Uno's USB / power-jack end. If a pin bends, straighten it with pliers and try again.
4. Plug both boards into the PC.
5. Watch the FINISH gate (COM3). In VS Code with PlatformIO: open this folder, click
   the **plug icon** (Serial Monitor) in the bottom bar and pick COM3. In the Arduino
   IDE: Tools > Port > COM3, then the magnifier icon top-right, set **115200 baud** and
   **Newline**. Or, in PowerShell from this folder:

   ```powershell
   .\tools\gate.ps1 -Port COM3 -Listen 20
   ```

6. You want to see this sequence (the +++ATDN is the board testing the radio; the
   radio's own OK reply goes to the Uno, not to you):

   ```
   ================ BreakBeam speed gate ================
   Role: FINISH
   Radio self-test (you will see +++ATDN below - that is normal)...
   +++ATDNRadio: OK - SRF shield answered at 115200 baud.
   ...
   [ARMED] waiting for the START beam...
   Radio link UP (round trip 9 ms)
   [ARMED] link OK rtt 9ms | START beam: TOO DIM/NOT ALIGNED | FINISH beam: ...
   ```

   `TOO DIM/NOT ALIGNED` on both beams is correct at this stage: no sensors yet.
   You will also see a line like `@P7--------` ten times a second: that is the poll
   going out over the radio (the PC hears everything the board sends). Ignore them;
   `tools\gate.ps1` hides them unless you add `-Raw`. The shield's flashing red LED
   is its heartbeat.

   **Result on 28 Sep 2026:** both shields passed the self-test and the link came up
   at 15-16 ms round trip.

**Pass:** `link OK` with a round trip under about 30 ms. Now the two boards are talking
across the room. Try carrying the START gate (on a USB power bank or a phone charger)
to the far end of the garden: the link should hold.

If it fails, see Troubleshooting at the bottom.

---

## Phase 2 - the beam sensor, one board at a time (shield OFF)

**Goal:** a laser hitting the LDR lights the L LED, a hand through the beam gives a
`LAP TIME`.

Do this on the FINISH board first, then repeat on the START board. Take the shield off
for this phase so you can plug jumper wires straight into the Uno's sockets.

### Parts from the starter kit

| Part | How to recognise it | Quantity per gate |
|---|---|---|
| LDR (light dependent resistor, "photoresistor") | Round disc, 5 mm wide, wiggly line on the face, two legs. No polarity. | 1 |
| 10 kΩ resistor | Bands **brown, black, orange** (then gold/silver). | 1 |
| 220 Ω resistor | Bands **red, red, brown**. | 1 |
| LED, any colour | Long leg = **+** (anode). | 1 |
| Piezo buzzer (optional) | Black round can, "+" printed on top or a longer leg. | 1 |
| Breadboard | The white block with holes. | 1 |
| Jumper wires, male-male | The bendy wires with pins on both ends. | about 6 |

### How a breadboard works (30 seconds)

- The two long rows down each edge (marked **+** and **-**, red and blue lines) are
  connected along their whole length. We use one for 5 V and one for GND.
- In the middle, each **short row of 5 holes** (a, b, c, d, e or f, g, h, i, j) is
  connected together. Rows are **not** connected across the central trench.
- So: two legs pushed into the same 5-hole row are joined. Different rows are not.

### Wiring

```
   Uno 5V  ------------------------------- breadboard + rail
   Uno GND ------------------------------- breadboard - rail

   + rail --- [ LDR ] --- row 10 --- [ 10k ] --- - rail
                            |
                            +---------------------------- Uno A0

   Uno D7 --- [ 220R ] --- LED long leg   LED short leg --- - rail

   Uno D9 --- buzzer +                   buzzer - --------- - rail   (optional)
```

Step by step:

1. Jumper from Uno **5V** to the breadboard **+** rail. Jumper from Uno **GND** to the **-** rail.
2. LDR: one leg into the **+** rail, the other leg into **row 10** (any row, just remember it).
3. 10 kΩ resistor: one leg into **row 10**, the other into the **-** rail.
4. Jumper from **row 10** to Uno **A0**.
5. LED: long leg into row 20, short leg into the **-** rail. 220 Ω resistor from row 20
   to row 25. Jumper from row 25 to Uno **D7**.
6. Buzzer (optional): + leg to a jumper to Uno **D9**, other leg to the **-** rail.

Why it works: the LDR and the 10 k resistor form a voltage divider. Bright light makes
the LDR a low resistance, so A0 sees nearly 5 V and reads close to 1023. Dark makes the
LDR a high resistance and A0 falls towards 0. The firmware watches for a sudden drop.

### Test it

1. Plug the board in and open its serial monitor (COM3 for FINISH).
2. Type `a` and Enter: **align mode**. Numbers arrive 5 times a second:

   ```
   A0=612  baseline=610  triggers below=488  beam=OK
   ```

3. Cup your hand over the LDR: `A0` should fall a lot. Take it away: it climbs back.
   If it barely moves, the LDR or the 10 k is in the wrong row. Check row numbers.
4. Laser: wrap a rubber band or tape round the pen's button so it stays on. Stand it
   on a tripod or a blob of Blu-Tack about 1 m from the breadboard and aim the dot onto
   the LDR's face. `A0` should jump to 950+ and the L LED and your LED come on.
   Block the dot with a finger: the LEDs go out. That is the whole sensor.
5. Type `a` again to stop the numbers. Type `l` for **lap mode**. Wave your hand
   through the beam: `LAP: clock started`. Wave again: `LAP TIME 1.234 s`. Compare a
   few against a phone stopwatch. They should agree to within your reaction time.

**Pass:** laser on = LEDs on, hand through beam = a lap time. Now do the same on the
START board (COM4). Align mode works there too; lap mode is finish-only, so just check
that a hand through the beam prints `START beam broken (event N)`.

---

## Phase 2B - both beams on ONE board (the garage run)

**Goal:** walk through two beams 3 m apart and read a real time, with no radio and
no soldering. This is the whole product except the radio link, which Phase 1 already
proved.

The shields have no sockets on top, so a shield and jumper wires cannot be on the
same board until the stackable headers arrive. This phase sidesteps that: put both
beams on one Uno and join them with a long cable.

### What you need on top of Phase 2

| Part | Notes |
|---|---|
| A second LDR | Same as the first. If you only have one, skip to "Only one LDR?" below. |
| A second laser pen | Taped on, same as the first. |
| A long 3-core cable | An old ethernet patch lead with one end cut off is ideal: eight cores, you use three. Speaker wire, bell wire or an old USB cable also work. Length 3 to 5 m. |
| A display (optional) | See "Adding a display" below. |

### The remote sensor head

Only one sensor needs to travel. Keep the Uno and breadboard at the finish line with
its own sensor beside them, and run the cable to the start line.

Build the far end as a little sensor head so the noisy long wires carry a low
impedance signal rather than a high one:

```
   at the FAR end (start line):        at the BOARD end:

   5V  ----+                            cable core 1 --- breadboard + rail
           |                            cable core 2 --- breadboard - rail
         [ LDR ]                        cable core 3 --- Uno A0
           |
           +---- signal (core 3)
           |
        [ 10k ]
           |
   GND ----+
```

So: LDR and 10 kΩ both live at the far end, exactly the divider from Phase 2, and the
three cable cores carry 5 V out, ground out, and the divider's middle back to A0.
Twist the cores or just let them lie, either is fine over 5 m.

The near sensor is the Phase 2 circuit unchanged, but on **A1** instead of A0.

### Full wiring

| Uno pin | Goes to |
|---|---|
| 5V | breadboard + rail |
| GND | breadboard − rail |
| A0 | far sensor head, middle of its divider (via the long cable) |
| A1 | near sensor, middle of its divider |
| D7 | 220 Ω to an LED, LED short leg to − rail. Lit when the START beam lands. |
| D6 | 220 Ω to a second LED, short leg to − rail. Lit when the FINISH beam lands. |
| D9 | buzzer +, buzzer − to − rail |

A second LED is worth wiring: one LED per beam means you can see both are aligned
without reading the screen.

### Adding a display

Pick whichever you actually own and wire only that one. Tell the board with the
`display` command and it remembers.

| You have | Command | Wiring |
|---|---|---|
| 16x2 LCD, the 16-pin one with a 10 kΩ contrast pot | `display lcd` | RS=D12, E=D11, D4=D5, D5=D4, D6=D3, D7=D2, RW to GND, VSS to GND, VDD to 5V, VO to the pot wiper (pot ends to 5V and GND), LED+ via 220 Ω to 5V, LED− to GND |
| 16x2 LCD with a small board soldered on the back and only 4 pins | `display i2clcd` | SDA=A4, SCL=A5, VCC=5V, GND=GND. The address is found for you. |
| 4-digit "clock" module, 4 pins marked CLK and DIO | `display tm1637` | CLK=D2, DIO=D3, VCC=5V, GND=GND |
| 8-digit 7-segment module, 5 pins marked DIN CS CLK | `display max7219` | DIN=D11, CS=D10, CLK=D12, VCC=5V, GND=GND |
| None of these, or nothing yet | `display none` | Times still print to the PC |

Type `t` at any time and the board sends 12.34 to the display, so you can check the
wiring before you run anything.

Two notes. Most TM1637 clock modules have a colon rather than decimal points, so
12.34 s appears as 12:34. That is the module, not a fault. And if what you have is an
8x8 dot-matrix or a bare 12-pin 4-digit display, say so and the driver can be added.

### Run it

1. Upload the firmware, open the serial monitor on the board at 115200 baud with
   Newline endings.
2. Type `role solo` and Enter. The board reboots into two-beam mode and does not
   touch the radio.
3. Type `display lcd` (or whichever you have), then `t` to prove the display works.
4. Type `a` for align mode. You now get both sensors' numbers five times a second:

   ```
   A0=612 base=610 trips<488 [OK]   A1=598 base=596 trips<477 [OK]
   ```

5. Aim each laser at its LDR until both say `OK` and both LEDs are lit. Type `a`
   again to stop the numbers.
6. Walk through the start beam, then the finish beam:

   ```
   START! clock running
   === RUN 1   TIME 2.318 s ===
   CSV,1,2.318
   ```

**Pass:** repeatable times that match a stopwatch to within a couple of tenths, and
the same number on the display.

### Only one LDR?

Everything above still works with one beam. Wire it to A0, set `role solo`, then type
`l` for lap mode. The clock starts on the first break of that beam and stops on the
second, so you can time a there-and-back or just wave a hand twice. You still exercise
the sensor, the threshold logic, the buzzer, the display and the timing path, which is
most of what tonight is for.

---

## Phase 3 - two gates, one time

**Goal:** walk through START, then FINISH, and read `=== RUN 1  TIME 2.345 s ===`.

Each board now needs its shield **and** its sensor wiring at the same time.

### No soldering needed after all: the six-wire shield

The shield's schematic shows the radio is wired to only **six** Arduino pins. Every
other pin on the shield is a bare pass-through with nothing attached to it.

| Shield pin | Arduino pin | What it does |
|---|---|---|
| 3V3 | 3.3V | powers the SRF radio |
| 5V | 5V | powers the two analogue switches that gate the serial lines |
| GND | GND | ground |
| D0 | D0 | radio transmit into the Uno's receive |
| D1 | D1 | Uno transmit into the radio |
| D8 | D8 | radio enable. Held low by a 10k resistor, so uploads still work. |

So the shield does not have to sit on the Uno at all. Lay it on the bench, connect
those six pins with **female-to-male jumper wires** (the female end pushes onto the
shield's downward pins, the male end goes into the Uno's header), and every other
Arduino pin stays free for sensors and the display. Straight through, not crossed:
D0 to D0, D1 to D1.

Two practical notes.

The shield will not plug into a breadboard. Its two pin rows are about 48 mm apart
and a breadboard only spans about 23 mm across its terminal strips, and the Uno R3
layout offsets the D8 row by half a pitch so it cannot align to the grid anyway. It
does not need to: the jumper wires do the job.

If you only have male-to-male jumpers, there is a second route. Look closely at the
shield and you will see a spare empty hole about 2.5 mm inboard of every header pin.
Those are on the same nets, and are what the datasheet means by "Veroboard friendly
top layout". A male jumper pin pushed into one is a loose friction fit rather than a
proper connection, so tape the shield down and do not knock it, but it works for a
bench test.

This unblocks the two-gate build without waiting for the stackable headers. The
headers are still the right answer for the finished product, because a soldered
socket will not fall out on a windy track.


Then:

1. Set up both gates about 2 m apart in the garage, each with its laser on a tripod
   aimed across the "lane" onto its LDR. Start gate on a power bank, finish gate on
   the PC.
2. Open the finish gate's monitor. Both beams should report `OK`:

   ```
   [ARMED] link OK rtt 9ms | START beam: OK | FINISH beam: OK (A0=987)
   ```

   If the start beam says `BLOCKED` or `TOO DIM`, adjust the start laser until the
   start gate's L LED comes on.
3. Walk through the start beam, then the finish beam:

   ```
   START! clock running (sync +/-4 ms)
   === RUN 1   TIME 2.318 s   (start-time uncertainty about +/-4 ms) ===
   CSV,1,2.318
   [COOLDOWN] ...
   [ARMED] waiting for the START beam...
   ```

4. It re-arms itself 1.5 s after each result. Go again.

**Pass:** repeatable times that make sense. Ask a helper to time you with a stopwatch
for a sanity check; expect agreement within a couple of tenths (their reaction time,
not the gates).

---

## What accuracy to expect, and the limits of this prototype

- **Radio placement of the start event:** about ±1 ms typically; the firmware prints
  its own estimate (half the best round trip).
- **LDR response:** this is the weak link. An LDR takes several milliseconds to react
  to going dark, and the exact figure depends on the part. For a 20 m fly at 9 m/s
  that is a few centimetres of uncertainty. Good enough to prove the system; for
  production swap each LDR for a phototransistor or photodiode (microsecond response,
  about £1 each). The firmware needs no change: it just sees a faster drop on A0.
- **Lockout:** after a trigger, a gate ignores its beam for 0.8 s so arms and legs do
  not count twice. Two athletes closer than that will confuse it.
- **Sunlight:** outdoors, ambient light can swamp the laser. Fit a black tube 3-4 cm
  long over each LDR (a 3D-printed hood is the obvious first print) so it only sees
  along the line of the laser.
- **Laser safety:** use pens marked Class 2 (under 1 mW). Mount the beams at hip
  height or lower so nobody's eyes are in line, and never look into the beam. Keep
  children away from the pens.
- **Beam height and what triggers first:** a single beam at hip height usually
  triggers on the leading hand or knee, not the torso. Commercial gates add a second
  beam and require both to be broken. That is a later refinement.

---

## Phase 4 and 5 - outdoors, then productise

Once Phase 3 passes, in this order:

1. **Power:** USB power banks (most stay on at the Uno's 70-100 mA draw). Or 9 V PP3
   into the barrel jack for short sessions.
2. **Hoods for the LDRs** (first 3D print), then **phototransistor upgrade**.
3. **Laser modules instead of pens:** 5 V red laser diode modules (Class 2, 1 mW) can
   be powered from the Uno's 5 V pin, no taped buttons.
4. **Housing:** printed case per gate holding Uno + shield + sensor on one side and
   the laser on an adjustable mount, with a 1/4"-20 tripod nut. Antenna clear of metal.
5. **Display on the finish gate:** the starter kit's 16x2 LCD or a TM1637 4-digit
   display so you don't need the laptop at the track.
6. **Logging:** the finish gate already prints `CSV,run,time` lines. A tiny script on
   the laptop (or a phone over USB OTG) can save them per athlete.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| `Radio: NO ANSWER` with the shield fitted | Shield not seated, or shifted one pin over | Unplug USB, reseat, check every pin is in a hole. |
| Still `NO ANSWER` | Radio at a non-standard baud rate | The firmware already tries 9600 and fixes it. If still nothing: unplug, short the two RST pads on the shield with a screwdriver blade **while** plugging the USB in, release after 2 s. Reboots to factory 115200. |
| `Radio: OK` on both but never `link UP` | Different PANIDs, or one board is not set as START | Check the START board prints `Role: START`. Factory-reset both radios (RST pads) so they share PANID 5AA5. |
| `link DOWN` intermittently | Range or antenna shielded by metal | Keep shields away from metal tripod heads; add the 82 mm wire whip antenna (Phase 5). |
| A0 barely changes with hand / laser | LDR or 10 k in the wrong breadboard row | Re-check that the LDR leg, the 10 k leg and the A0 jumper share ONE row. |
| A0 stuck near 1023 even when blocked | Room too bright, or laser reflecting | Hood the LDR; move away from windows. |
| `FINISH beam broken but no start was seen` | Start gate did not trigger, or link down | Look at the status line: `START beam: OK`? `link OK`? |
| Times look about right but jittery by 10+ ms | Slow LDR, or a laser dot only half on the LDR | Centre the dot; upgrade to phototransistor. |
| Uploading fails: "not in sync" | Another program (serial monitor, gate.ps1) has the port open | Close it, retry. Shields do not need removing. |
| Commands typed in the first 5 s after reset are ignored | Radio self-test is running | Wait for the help text, type again. |
| Gates print each other's messages, or garbage fragments | Normal: the radio and USB share one serial line, and lost packets leave fragments | Harmless. Only lines starting with `@` are protocol; everything else is chatter. |

---

## Appendix A - uploading the firmware

**PlatformIO (VS Code):** File > Open Folder > this folder. First time, the extension
installs its toolchain (a few minutes). Then bottom bar: **tick** = build, **arrow** =
upload, **plug** = serial monitor. If both boards are plugged in, uncomment
`upload_port`/`monitor_port` in `platformio.ini` or pick the port when prompted.

**Arduino IDE 2:** File > Open > `BreakBeam/BreakBeam.ino`. Tools > Board > Arduino
Uno, Tools > Port > COM3. Click the arrow. Repeat with COM4.

**Command line (what Claude used):** the Arduino IDE bundles `arduino-cli`:

```powershell
& "$env:LOCALAPPDATA\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe" compile --fqbn arduino:avr:uno --upload -p COM3 .\BreakBeam
```

After a fresh upload the role is still remembered (it lives in EEPROM, which uploads
do not erase).

## Appendix B - settings you might tune

All near the top of `BreakBeam/BreakBeam.ino`:

| Constant | Default | Meaning |
|---|---|---|
| `LOCKOUT_MS` | 800 | Ignore re-breaks for this long after a trigger. |
| `MIN_DROP` / `DROP_PERCENT` | 80 / 20 | How much A0 must fall to count as a break. Raise if you get false triggers, lower if the beam is weak. |
| `BEAM_MIN_LEVEL` | 300 | Below this baseline the gate says TOO DIM. |
| `POLL_INTERVAL_MS` | 100 | How often the finish gate polls the start gate. |
| `COOLDOWN_MS` | 1500 | Pause after a result before re-arming. |
| `RUN_TIMEOUT_MS` | 60000 | Abandon a run with no finish after this. |
