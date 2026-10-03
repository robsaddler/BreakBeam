# BreakBeam build guide (absolute beginner edition)

Read this in order. Each phase ends with a test that must pass before you go on.
Nothing here needs soldering at all. The shields have no sockets on top, but the
radio only uses six Arduino pins, so six jumper wires replace them (see Phase 3).

---

## Words used in this guide

Nothing here assumes you have met any of this before. Skim it once and come back when
a word trips you up.

**Jumper wire**, usually just "jumper". A short flexible wire with a stiff metal pin
moulded onto each end, sold in bundles of rainbow colours. The pins push into
breadboard holes and into the Arduino's black sockets. They are called jumpers because
they jump a connection from one place to another. The colours mean nothing electrically
and are purely so you can follow your own work. By convention red is used for power and
black for ground, which is worth copying.

- **Male** end: a pin that sticks out. Goes *into* a hole.
- **Female** end: a small socket. Goes *over* a pin.
- **Male to male** jumpers have pins at both ends. These are what you want for
  breadboard work, and are what most kits contain.

So "jumper from the Uno's 5V to the + rail" means: take one jumper wire, push one end
into the Arduino hole labelled 5V, and push the other end into any hole along the
breadboard strip marked +.

**Breadboard.** The white plastic block covered in small holes. It lets you build a
circuit by pushing parts into holes instead of soldering. Inside, hidden metal clips
join certain holes together, which is the whole trick.

**Rail.** The long strip of holes running down each long edge of the breadboard,
marked with a red line and a **+**, or a blue line and a **−**. Every hole along one
rail is joined to every other hole on that same rail. We put 5 volts on the + rail and
ground on the − rail, so that power is available all along the board.

**Row.** In the middle section of the breadboard, a short run of five holes side by
side. All five are joined to each other and to nothing else. Rows are usually numbered
along the edge. When this guide says "put both of these in the same row", it means pick
any row and push both legs into it, which connects them.

Which row number you pick never matters. What matters is the opposite: **a row you are
using for one job must not contain anything else**, or those two things get joined
together whether you wanted it or not. Before using a row, look along all five of its
holes and check they are empty.

One more thing that often surprises people: the channel down the middle of the board
splits every row in two. The five holes above the channel and the five below it are
numbered the same but are **not** connected. So "row 10" above the channel and "row 10"
below it are two separate rows you can use for different jobs.

**Pin.** Two meanings, both common.
1. A metal leg sticking out of a component or a board.
2. A labelled connection point on the Arduino, such as 5V, GND, A0 or D12. The labels
   are printed on the board right next to the black sockets.

**Leg** or **lead.** The wire sticking out of a component such as a resistor or a
sensor. Same thing as pin, meaning 1.

**GND** stands for ground. It is the zero-volt side of the circuit, the common return
path that everything connects back to. Think of it as the drain that all the water
flows back down. The Arduino has several holes labelled GND and they are all the same
point, so use whichever is convenient.

**5V** is the Arduino's five-volt power output, the other side of the circuit. Current
flows out of 5V, through your components, and back into GND.

**A0 to A5** are the Arduino's *analogue input* pins. Analogue means they do not just
read on or off, they measure a voltage and report it as a number from 0 (zero volts) to
1023 (five volts). This is how the board can tell "a bit of light" from "lots of light".
Our sensor goes to A0.

**D0 to D13** are the *digital* pins. They deal in just on or off, and can act as
inputs or outputs. We use them to drive the display and the buzzer.

**Resistor.** A small component that restricts how much current can flow. Its value is
in **ohms**, written Ω, and larger numbers restrict more. The value is printed as
coloured bands around the body rather than as a number, so:

| Value | Bands to look for |
|---|---|
| 150 Ω | brown, green, brown |
| 330 Ω | orange, orange, brown |
| 470 Ω | yellow, violet, brown |
| 1 kΩ (1000 Ω) | brown, black, red |
| 4.7 kΩ | yellow, violet, red |
| 10 kΩ (10,000 Ω) | brown, black, orange |

Read the bands from the end that has them grouped closest together. The lone gold or
silver band at the other end is the tolerance and you can ignore it. A resistor has no
right way round.

**How close do you have to get?** For the two jobs in this build that just limit
current, lighting the display's backlight and lighting an indicator LED, anything from
about 150 to 470 ohm is fine. A smaller value means brighter, a larger one dimmer.
Only the sensor's 10 kohm actually sets behaviour, and even that is adjustable.

**Polarity** means a component cares which way round it goes. A resistor and an LDR do
not. An LED, a buzzer and a phototransistor do, and putting them in backwards means
they do not work.

**LDR**, light dependent resistor, sometimes photoresistor. A small orange disc whose
resistance falls when light hits it. Cheap, slow, no polarity.

**Phototransistor.** Does the same job far faster, which is why it is better for
timing. It has polarity. The TEPT5700 is the one you bought.

**Potentiometer**, usually "pot". An adjustable resistor with a knob or a slot for a
screwdriver, and three legs. Turning it varies the resistance between the middle leg
and the outer two. We use one to set the display's contrast.

**Flashing** or **uploading**. Copying the program from the PC onto the Arduino over
the USB lead. The program then stays on the board, even with the power off, until it is
replaced.

**Serial monitor.** A window on the PC showing the text the Arduino prints over the USB
lead, and letting you type text back. This is how you give the board commands such as
`role solo`.

**Baud** is the speed the board and the PC talk at, in bits per second. Both ends have
to agree or you get gibberish. Ours is 115200.

**Firmware.** The program running on the Arduino, as opposed to software running on the
PC. Same idea, different word, because it lives inside a device.

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

## Phase 2 - one board, one beam, one display (no soldering)

**Goal:** the LCD shows a time when you wave your hand through a laser beam twice.

Everything here plugs together. Nothing is soldered. Allow about 40 minutes.

### Before you start: the lasers

Your pens are marked **Class III**, one green at 532 nm and one red at 650 nm. Both
work fine for this. Three rules, and they are not optional:

- Never look into the beam or at the front end of the pen, and never point either at
  a person or an animal.
- Work with the beam at bench height, pointing at a wall, not across a room at head
  height.
- The green one is the riskier of the two, because cheap green pens are often
  stronger than their label claims. Use the **red** one for this bench test.

### Parts, all from your photos

| Part | Which one | How many |
|---|---|---|
| Arduino Uno + USB cable | Either board | 1 |
| Breadboard | The white one | 1 |
| TEPT5700 phototransistor | The new ones. Clear 5 mm body, two legs of **different lengths** | 1 |
| LDR | Small orange disc. Keep one to hand as a fallback only | 1 |
| 10 kohm resistor | Bands brown, black, orange | 1 |
| 330 ohm resistor | Bands orange, orange, brown. Anything from 150 to 470 ohm works here | 1 |
| LCD | The **1602A**, the one that already has a black 16-pin strip soldered on. Not the big 2004A. | 1 |
| Contrast control | A small potentiometer if you have one, otherwise a 1 kohm resistor (brown, black, red) | 1 |
| Jumper wires | Male to male | about 12 |
| Laser pen | The red 650 nm one | 1 |

Leave the buzzer, the radio shields and the phototransistors out for now. One new
thing at a time.

### How a breadboard works

If any word below is unfamiliar, the glossary at the top of this guide defines every
term used here. The short version:

- The two long strips down each edge, marked **+** and **-** with a red and a blue
  line, are each joined all the way along. These are the rails. One carries 5 V, the
  other carries ground.
- In the middle, each **short row of five holes** is joined to itself and to nothing
  else. The rows are numbered down the side.
- The channel down the middle splits the board. A row on the left of the channel is
  **not** joined to the row on the right of it.

So two legs in the same five-hole row are connected to each other, and legs in
different rows are not. That is the whole idea.

### Stage 1: power rails

**Unplug the USB cable first.** Build everything with the power off.

1. Jumper wire from the Uno's **5V** pin to the breadboard's **+** rail.
2. Jumper wire from one of the Uno's **GND** pins to the **-** rail.

The Uno has two GND pins side by side. Either one is fine.

### Stage 2: the LCD

Push the LCD into the breadboard so all 16 of its pins go into 16 separate rows, near
one end of the board. Press it in square and firm. The pins are numbered 1 to 16 and
the board has them printed next to the strip: **VSS VDD VO RS RW E D0 D1 D2 D3 D4 D5
D6 D7 A K**.

Now run these wires. The row you work from is whichever row that LCD pin landed in.

| LCD pin | Label | Connect to |
|---|---|---|
| 1 | VSS | **-** rail |
| 2 | VDD | **+** rail |
| 3 | VO | contrast, see below |
| 4 | RS | Arduino **D12** |
| 5 | RW | **-** rail |
| 6 | E | Arduino **D11** |
| 7, 8, 9, 10 | D0 to D3 | nothing at all |
| 11 | D4 | Arduino **D2** |
| 12 | D5 | Arduino **D3** |
| 13 | D6 | Arduino **D4** |
| 14 | D7 | Arduino **D5** |
| 15 | A | one leg of the 330 ohm resistor, its other leg to the **+** rail |
| 16 | K | **-** rail |

The LCD has pins called D4 to D7 and the Arduino has pins called D2 to D5. They are
different things that happen to share a naming style. Follow the table and the numbers
climb together, which makes it hard to get wrong.

**Contrast, LCD pin 3 (VO).** If you have a small potentiometer, push it into three
rows, run its middle leg to LCD pin 3, one outer leg to the **+** rail and the other
to the **-** rail. With no potentiometer, run a 1 kohm resistor from LCD pin 3 to the
**-** rail instead, which gives a fixed and usually readable contrast.

### Stage 3: the light sensor

Unlike the LDR, **the phototransistor has a right way round**. Look at its two legs:
one is longer than the other. The long leg is the collector and goes to the positive
side. Get this right before you power up.

Pick any **completely empty row** for this and call it the sensor row. Not one of the
sixteen rows the LCD's pins are sitting in, and not one holding any other leg or wire.
Check all five of its holes are free before you start. The row number itself is
irrelevant; it just has to be a row nothing else is using.

1. **Long leg** into the **+** rail. **Short leg** into the **sensor row**.
2. Push the 10 kohm resistor between the **sensor row** and the **-** rail.
3. Jumper wire from the **sensor row** to the Arduino's **A0**.

Those three things, and only those three things, share the sensor row. That shared row
is the middle of the divider, and it is what A0 measures. Anything else pushed into it
would be wired straight onto the sensor's output.

That is the whole sensor, and it is the same shape of circuit as the LDR version: the
sensor and the resistor make a divider, and A0 reads the middle of it. More light means
more current through the phototransistor, which pulls A0 higher. Darkness lets it fall
towards 0. The firmware watches for a sudden fall.

If you get the legs the wrong way round nothing catches fire, because the 10 kohm
limits the current to a fraction of a milliamp. The symptom is simply a reading that
sits low and barely reacts to light. Turn it round and carry on.

### Stage 4: switch on

1. Plug the USB cable into the Uno and the PC.
2. Tell Claude, and the firmware gets flashed.
3. Open the serial monitor at **115200 baud** with **Newline** line endings. From
   PowerShell in the repo folder this also works:

   ```powershell
   .\tools\gate.ps1 -Port COM3 -Listen 30
   ```

4. Type `role solo` and press Enter. The board reboots.
5. Type `display lcd` and press Enter. The LCD should show **BreakBeam** on the top
   line and **12.34 s** underneath.
6. Nothing on the LCD, or just a row of solid blocks? Turn the potentiometer slowly
   from one end to the other. There is a narrow band where the text appears. If you
   used the fixed resistor, try a smaller one, down to 330 ohm.
7. Type `t` at any time to send 12.34 to the display again.

### Stage 5: the beam

1. Type `a` and press Enter for align mode. Numbers arrive five times a second, like
   `A0=612 base=610 trips<488 [OK]`.
2. Cup your hand over the sensor. `A0` should fall. Take your hand away and it climbs
   back. If it barely moves at all, either the short leg, the 10 kohm leg and the A0
   wire are not all in the sensor row, or something else has crept into that row, or
   the phototransistor is in backwards.
3. Wrap a rubber band round the red pen's button so it stays on. Stand it on the bench
   about 30 cm away and aim the dot at the face of the phototransistor. `A0` should
   jump high and the board's **L** light should come on.

**What good numbers look like.** A phototransistor is far more sensitive than an LDR,
so expect a big gap rather than a gentle one:

| Condition | Expected `A0` |
|---|---|
| Room light only, no laser | low, roughly 20 to 200 |
| Laser dot on the sensor | very high, often pinned at 1023 |
| Finger in the beam | back down to the room-light figure |

Pinned at 1023 **with the laser on is fine and is what you want**, because the
firmware only ever looks for a sudden fall. The one case that needs fixing is room
light alone reading above about 700, which means a lamp or a window is shining
straight in. Shade the sensor, or swap the 10 kohm for 1 kohm.
4. Put a finger in the beam. The L light goes out.
5. Type `a` again to stop the numbers.

### Stage 6: get a time

1. Type `l` and press Enter for lap mode.
2. Wave your hand through the beam. The serial monitor says the clock started.
3. Wave again. You get a time, on screen and on the LCD:

   ```
   === RUN 1   TIME 2.318 s ===
   CSV,1,2.318
   ```

4. Time a few against a phone stopwatch. They should agree to about a tenth, which is
   your reaction time rather than any error in the gate.

**Pass:** the laser on the sensor lights the L light, and two waves give a time on the
LCD.

### Then one more thing

**Add the buzzer.** First work out which of your three buzzers is which, with the
multimeter on resistance across the two legs:

| Reading | What it is | What to do |
|---|---|---|
| Open circuit, or megohms | Passive piezo | Perfect. Use this one. |
| 15 to 50 ohm | Passive electromagnetic | Usable, but put 330 ohm in series or it pulls too much current from the pin. |
| A few hundred ohm to a few kohm | Active, with its own oscillator | It only makes one fixed pitch. Fine as a beeper, but it ignores the tune. |

Wire the chosen one with its **+** leg through a 330 ohm resistor to Arduino **D9**,
and its other leg to the **-** rail. The 330 ohm is harmless with a piezo and protects
the pin with the others.

**If the phototransistor misbehaves**, the LDR is the fallback. It drops into the same
two holes, has no polarity so it cannot go in backwards, and needs no other change.
It is slower, which costs accuracy at speed, but it will prove the rest of the chain.

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
| D7 | 330 Ω to an LED, LED short leg to − rail. Lit when the START beam lands. |
| D6 | 330 Ω to a second LED, short leg to − rail. Lit when the FINISH beam lands. |
| D9 | buzzer +, buzzer − to − rail |

A second LED is worth wiring: one LED per beam means you can see both are aligned
without reading the screen.

### Adding a display

Pick whichever you actually own and wire only that one. Tell the board with the
`display` command and it remembers.

| You have | Command | Wiring |
|---|---|---|
| 16x2 LCD, the 16-pin one (the 1602A) | `display lcd` | As wired in Phase 2: RS->D12, E->D11, D4->D2, D5->D3, D6->D4, D7->D5 |
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
