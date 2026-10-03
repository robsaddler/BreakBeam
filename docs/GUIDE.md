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

## Using the multimeter

A meter turns "I think that wire is connected" into "that wire is connected". Worth
ten minutes to learn, because it finds in seconds what guessing finds in an hour.

These notes are written for a VC99+, but any cheap meter works the same way.

### Setting it up for voltage

Leads first, and for everything in this guide they never move:

- **Black** lead into the socket marked **COM**.
- **Red** lead into the socket marked **VΩHz**.

Never put the red lead into the **A** or **mA** sockets while measuring voltage. Those
are deliberately near-zero resistance, so across a power supply they are a short
circuit. They are only for measuring current, which this project never needs.

Then turn the dial to **V** with a solid line and a dashed line under it, which means
DC volts. On a VC99 that is one click clockwise from OFF. Do not use **V~** beside it,
which is AC. The meter ranges itself, so there is nothing else to set.

### Probe tips do not fit breadboard holes

A probe tip is about 2 mm across and a breadboard hole takes a 0.6 mm pin. The tip will
not reach the metal clip inside, so you get a false reading of zero, and forcing it
splays the clip and ruins that row for good.

**Use a jumper wire as a test point.** Push a spare male-to-male jumper into a free hole
in the row you want to measure and leave its other end standing in the air. Touch the
probe to that free pin. It is electrically the same point and it gives you something
big enough to hit. Taping the jumper to the probe makes a usable extended tip.

The Arduino's own black sockets have exactly the same problem, so measure them the same
way: a jumper into the socket, probe onto its free end.

If the kit has crocodile clip leads, clip one onto the ground jumper. That gives you a
hands-free black probe and leaves a hand spare for turning a potentiometer.

### Measuring a voltage

Voltage is always a measurement *between two points*, never at one point on its own. In
practice one of those points is nearly always ground, so:

1. Power the circuit up. Voltage readings need the power **on**.
2. Park the **black** probe on the **−** rail and leave it there for the whole session.
3. Move the **red** probe to each point you want to know about.

The number you read is how far above ground that point sits.

### Measuring resistance and continuity

The opposite rule applies: resistance and continuity need the power **off**. Unplug the
USB first. Measuring resistance on a live circuit gives nonsense, because the meter
works by pushing its own tiny current through the thing and seeing what happens.

Turn the dial to the **Ω** position. The same dial position usually carries continuity
and diode testing too, cycled with the blue button, and the display shows a small
speaker symbol when continuity is selected. Touch the probes together: a short beep
means the meter is working.

Continuity is the fastest way to answer "is this wire really in the row I think it is".
One probe at each end, beep means yes.

Resistance is how to identify the three buzzers later in this guide, and how to confirm
a resistor's value when you cannot face reading the colour bands.

### If the meter behaves oddly

A blank screen, a continuous beep or wandering numbers usually means a tired battery,
and these meters take a 9 V PP3 behind a cover on the back. Prove the meter before you
trust it: set it to DC volts and measure a known 9 V battery across its two terminals.
A healthy new one reads about 9.5 V.

Most of these meters also switch themselves off after a few idle minutes. Click the
dial to OFF and back to wake it.

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

**The contrast and the backlight are two separate circuits.** They sit next to each
other in the table above, which makes it easy to wire one into the other. The pot has
nothing to do with the backlight, and the 330 ohm has nothing to do with the contrast.

```
CONTRAST  -  the pot, three legs, and nothing else
   + rail  --- one outer leg
               MIDDLE leg --- LCD pin 3
   - rail  --- other outer leg

BACKLIGHT  -  nothing to do with the pot
   + rail  --- [ 330 ohm ] --- LCD pin 15
   - rail  ---------------------  LCD pin 16
```

Both outer legs of the pot must reach the rails, one to **+** and one to **-**, so the
middle leg can slide between 0 V and 5 V. Running LCD pin 15 to a pot leg instead feeds
the backlight through the pot's whole track, which passes so little current that the
screen stays dark and pin 15 measures almost 0 V.

With no potentiometer, run a 1 kohm resistor from LCD pin 3 to the **-** rail instead,
or even wire pin 3 straight to the **-** rail, which is maximum contrast and usually
readable.

**A note on pot values.** Most kits ship a 10 kohm pot. A larger one such as 50 kohm
still works, but it squeezes the whole useful range into a sliver of rotation close to
the **-** end, so turn it slowly and all the way before deciding it does nothing.

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

### What the display tells you

Once a display is wired, the board can be used without a PC at all.

**Waiting, beam not landing.** The screen becomes an alignment aid. The first number
is what the sensor sees right now, the second is what it needs:

```
Aim the laser
268 of 300
```

Move the light until the first number climbs past the second. Properly aimed, a
TEPT5700 with a red laser on it reads about 1000.

**Waiting, beam landing.** `BreakBeam` over `ready`. That is the green light.

**Timing.** The clock counts up twenty times a second, so the hundredths visibly race.

**Finished.** The time stays on screen until the next run starts. A hand passing
through the beam does not wipe it; only the beam going genuinely missing, for more
than a second and a half, switches back to the aim screen.

### Stage 6: get a time

1. Type `l` and press Enter for lap mode.
2. Wave your hand through the beam. The clock starts and the LCD counts up.
3. Wave again. The clock stops and the time stays on screen:

   ```
   === RUN 1   TIME 2.318 s ===
   CSV,1,2.318
   ```

4. A third wave starts a fresh lap. There is an 0.8 second lockout after each break,
   so your hand leaving the beam cannot count as the next one.
5. Time a few against a phone stopwatch. They should agree to about a tenth, which is
   your reaction time rather than any error in the gate.

**A laser is not required for this stage.** Aiming a 3 mm dot onto a 5 mm sensor by
hand is the fiddliest thing in this whole project, and it has nothing to do with the
timing. A desk lamp or a phone torch held 5 to 10 cm away gives a huge, steady reading
and needs no aiming at all, and your hand passing in front of it breaks the beam just
the same. Save the laser for when the parts are mounted and cannot drift.

**Pass:** the laser on the sensor lights the L light, and two waves give a time on the
LCD.

**Result on 3 Oct 2026:** passed on the first board. With the TEPT5700 and a 10 kohm
resistor the beam read **1022** with the red laser on it and about **181** blocked, a
five-fold margin with nothing marginal about it. Three hand-waved lap times came out at
1.410 s, 0.821 s and 1.480 s.

Two things learned on the way. The sensor reads low in room light, around 190, and the
board correctly says TOO DIM until the laser is actually on it; that is not a fault.
And an unwired analogue pin mirrors its neighbour, so with only A0 connected, A1 reports
almost the same number and will fire spurious finish events. That is exactly why this
stage uses lap mode, which looks at A0 alone.

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

## Phase 4 - the permanent build, ready for a case

Breadboards are for proving a circuit, not for keeping one. This phase moves each gate
onto a soldered prototype shield in a vertical stack, with every part that has to point
somewhere on the end of a lead. Do this before designing the case, because the case has
to fit the result.

### Power: one 9 V PP3 per gate

Into the barrel jack, which wants 7 to 12 volts, so 9 V is right.

**Never put two 9 V batteries in series.** 18 volts is past what the Uno's regulator is
built for; it will run hot and may fail.

What to expect per gate:

| | Current | Runtime on a PP3 |
|---|---|---|
| Finish gate, with display | about 80 mA | 4 to 5 hours |
| Start gate, no display | about 65 mA | 5 to 6 hours |

Fit a **switch in the battery's positive lead**, between the battery and the barrel
plug. Without one the only way to turn a gate off is to unplug the battery, and in a
sealed case that means opening it every time.

A USB power bank is the cheaper long-term option, running for days and costing nothing
per session, so leave room in the case for either. A PP3 is 48 x 27 x 18 mm and a small
power bank is about 95 x 45 x 22 mm, so design the compartment for the power bank and a
battery will fit anywhere.

### The stack

```
   SRF radio shield     <- top, antenna in clear air
   Prototype shield     <- the whole circuit lives here
   Arduino Uno          <- bottom, barrel jack and USB at one end
```

The radio goes on **top** for a reason. Its own datasheet warns that covering the
antenna section cuts the range, and a board sitting above it does exactly that. This
order also means nothing has to stack above the radio, so the stackable headers you
bought are not needed here at all.

Overall stack is roughly **69 x 53 mm and 40 mm tall**. Measure your own before cutting
any CAD, since header heights vary.

### What goes on the prototype shield

The guiding rule: **anything that has to point somewhere lives on a lead, not on the
board.** The sensor has to look down the lane and the laser has to shine along it, so
neither can be soldered flat to a shield buried in a case.

Both gates:

| Fit | Connects to | Why |
|---|---|---|
| 3-pin female header, marked SENSOR | 5V, A0, GND | the sensor head plugs in here on a short lead |
| Buzzer, with 330 ohm in series | D9 and GND | no aiming needed, so it can live on the board |
| LED, with 330 ohm in series | D7 and GND | beam-is-landing light, visible on the outside of the case |

Finish gate only:

| Fit | Connects to | Why |
|---|---|---|
| 16-pin female header, marked LCD | RS to D12, E to D11, D4 to D2, D5 to D3, D6 to D4, D7 to D5, plus 5V, GND, RW to GND | the screen mounts in the case lid on a ribbon |
| 10 kohm trimmer pot | between 5V and GND, wiper to LCD pin 3 | contrast. A small trimmer you set once and forget, not the big panel pot |
| 330 ohm | 5V to LCD pin 15 | backlight |

Use the shield's printed 5 V and ground rails rather than running those by hand.

**Solder sockets, not components, wherever a part might fail.** A phototransistor
soldered directly to the board is a part you will regret the first time one dies.

### The sensor head

This is the part that goes inside the printed hood and points at the far laser.

```
   in the hood                         to the SENSOR socket

   5V  ────┐
           │
         [ TEPT5700 ]  long leg up
           │
           ├──────────── signal
           │
        [ 10k ]
           │
   GND ────┘
```

Both the phototransistor **and** the 10 kohm live in the hood, with three wires back to
the board. That keeps the high-impedance middle of the divider short, so the long lead
carries a low-impedance signal and picks up far less noise.

Hood dimensions that work: **35 mm long, 8 mm bore**, with the sensor at the back and a
translucent diffuser disc across the mouth. The diffuser spreads the laser dot over the
whole sensor, so aiming stops being a 3 mm target, and it makes the dot visible from
behind, which is a genuinely useful alignment aid.

### Four tripods, four boxes: the through-beam layout

Each gate is a **pair** of tripods, laser on one side of the lane and sensor on the
other. This is a through-beam gate, the same arrangement commercial timing gates use,
and it gives the strongest and most reliable signal.

It also means you build **four boxes, not two**, and they are nothing like each other:

| Box | Contains | Count |
|---|---|---|
| Sensor box | Uno, prototype shield, radio shield, hooded sensor, battery, switch | 2, one per gate |
| Laser box | Laser module, battery, switch. **No electronics at all.** | 2, one per gate |

**The laser box needs its own power.** You cannot run a cable across the lane: that is
a trip hazard at hip height in front of sprinting athletes. The laser module takes 2.6
to 6 volts, so either works:

- **3 x AA in a holder**, giving 4.5 V. About £2, and a very long life at roughly 30 mA.
- **A USB power bank** with a cut-down USB lead, giving 5 V and rechargeable.

The laser box is otherwise trivial: battery, switch, laser, clamp, tripod thread.
Nothing in it can go wrong, which is worth remembering when something does.

**Alignment is now the main job**, because each gate is two independent tripods rather
than one rigid bracket. What makes it quick:

1. Set both tripods to the same height and level them.
2. Coarse aim by eye, landing the dot on the face of the sensor hood.
3. Fine aim by watching feedback. The finish gate shows the live number on its aim
   screen. The start gate has no display, so its **L** light is the only indicator:
   bring that out to the case surface and make it visible from several metres.

The diffuser disc across the hood mouth matters most here. It turns the target from a
5 mm lens into the whole hood mouth, which across a two metre lane is the difference
between fiddly and quick.

### The laser end: a printed clamp for the pen

The laser pens already carry their own batteries, so the laser box needs no electronics
and no power supply of its own. It is a printed clamp with a tripod thread, and that is
all.

The clamp has one job beyond holding the pen still: **it should press the pen's button
for you.** Tape was what kept slipping on the bench, and a printed part can do it
properly.

Design that as a **thumbscrew bearing on the button**. Screw it in and the laser is on,
back it off and it is off. That gives a real on/off control without modifying the pen
and nothing to creep. Put it where you can reach it with the pen clamped and the box on
a tripod.

The rest of it:

- **A split clamp with a pinch screw**, not a plain printed hole. A hole sized to grip
  comes out wrong by a few tenths once printed, and anything that merely slides in will
  rotate in time. A degree of rotation loses the sensor across a two metre lane.
- **Measure the barrel with calipers** where the clamp grips. Pen lasers run about 12 to
  14 mm, and if your two differ, print two clamps rather than one compromise.
- **Line the clamp with something soft**, a strip of inner tube or a few layers of tape,
  so it can be nipped tight without marking the barrel.
- **A captive 1/4 inch 20 UNC nut** in the base, as a hex pocket you push the nut into.
  Do not try to print the thread.
- **Support the pen at two points** along its length. A single ring lets it pivot.

Two things to expect. Those pens run on small button cells, and with the button held on
you get a few hours at best, fading rather than stopping cleanly. Carry spares and check
the reading at the start of a session rather than mid-sprint. And all the aiming
adjustment now comes from the tripod head, so put your finest pan-and-tilt heads on the
laser ends.

**If you later tire of button cells**, a 5 V laser module removes them: about £13 for a
Class 2 one with an automatic power control driver, which also holds its brightness
steady as it warms up. That would need a battery in the laser box, 3 x AA or a USB power
bank, since the module has none of its own. Not needed to get going.

### Case design notes

Measure everything yourself before committing, but these are the figures to design
around:

| Part | Approximate size |
|---|---|
| Uno plus two shields | 69 x 53 x 40 mm |
| 1602 LCD module | 80 x 36 x 13 mm, viewing window 65 x 16 mm |
| PP3 battery | 48 x 27 x 18 mm |
| Sensor hood | 35 mm long, 8 mm bore |
| Laser pen | measure the barrel with calipers, typically 12 to 14 mm |
| SMA bulkhead connector | needs a 6.5 mm hole |
| Tripod thread | 1/4 inch 20 UNC nut, captive |

Five things the case has to get right:

1. **Each box must hold its own aim without creeping.** With laser and sensor on
   separate tripods, nothing you print can enforce their alignment, so what the cases
   must do instead is stop the parts moving relative to their own tripod. A laser that
   can rotate a degree inside its clamp loses the sensor across a two metre lane.
2. **A bubble level recess on top of every box, including the laser boxes.** With four
   tripods to set up, levelling all four to the same height is what turns alignment
   from guesswork into a routine.
3. **Keep metal away from the antenna end**, and put the SMA connector where a tripod
   head cannot shadow it.
4. **The USB socket must stay reachable** without opening the case, for firmware
   updates and for the finish gate to talk to a laptop.
5. **The L light and the power switch on the outside.** On the start gate that LED is
   the only indication the beam is landing.

### Build order, with a test after each step

Do not solder the lot and then look for faults. Build one gate at a time, in this
order, testing as you go:

1. Fit the headers so the shield stacks cleanly. Stack it, power up, confirm the board
   still boots and reports its role.
2. Solder the **sensor socket** and make up the sensor head. Test with `a` for align
   mode: you want about 1000 with the laser on it and under 200 blocked.
3. Solder the **buzzer and LED**. Break the beam and check both react.
4. Finish gate only: solder the **LCD socket and trimmer**. Set the contrast once.
5. Fit the **SRF shield on top** and confirm the radio self-test passes.
6. Both gates together: confirm the link comes up and you get a real time.
7. Only then design the case around what you are holding.

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
| LCD completely dark, no backlight | Backlight not fed, or fed through the pot | Measure LCD pin 15: it should be 3 to 5 V. Near 0 V means the 330 ohm is not bridging a + rail hole to pin 15's row, or pin 15 has been wired to a pot leg. |
| LCD lights but shows nothing | Contrast | Sweep the pot slowly end to end. With a 50 kohm pot the readable band is close to the - end. Or wire pin 3 straight to the - rail. |
| LCD shows gibberish characters | The four data wires are in the wrong order, RW is not grounded, or RS and E are swapped | LCD pins 11, 12, 13, 14 go to Arduino D2, D3, D4, D5, climbing together. LCD pin 5 (RW) must be firmly in the - rail. LCD pin 4 to D12 and pin 6 to D11. |
| Gates print each other's messages, or garbage fragments | Normal: the radio and USB share one serial line, and lost packets leave fragments | Harmless. Only lines starting with `@` are protocol; everything else is chatter. |
| Mangled characters in every line, and uploads fail | The radio is driving the shared serial lines when it should be isolated | D8 must be driven, never left floating: a floating D8 leaves the shield's two analogue switches half open. The firmware now drives it low in any role that does not want the radio. If it persists, a shield wire is on the wrong pin - D0 and D1 belong on the Arduino-style header, not the separate programming group next to the radio chip, which bypasses the switches entirely. |

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
