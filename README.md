# litterbox-ventilation
Automatic litterbox ventilation system for cats with asthma and dust sensitivity

For asthmatic cats, very small environmental factors can trigger flare ups of asthma, causing an undesirable reaction of increased stress levels, and ultimately resulting in a potentially harmful asthma attack. I have found that the leading factor for triggering attacks is air quality and humidity. Litterbox usage tends to create a lot of dry dust particulate in an enclosed area, irritating the throat and causing attacks. This project aims to mitigate this vector of attacks by detecting litter box usage, and venting the area (by way of a mounted fan) for the duration of usage.

# Components

* 1x microcontroller - anything works, but this is designed to use a particle xenon, for built in RGB status LEDs
* 1x IR break beam sensors - I used 5mm (not 3mm) as the range is wide enough to comfortably span the large litterbox I have
* 1x 5v Fan - 120mm works for me. Optimizing for low noise
* USB Battery pack with 1 type-a plug
* 1x MOSFET N-Channel - only needs to handle low current, 5v
* 1x breadboard

# Design

```
                                +-----+
                                | USB |
                                +--+--+
                                   |
                                   |
 5V Bus                            |                                          GND Bus
+---+                       +------+-----------+                             +---+
|   |                       |     USB          |                             |   |
| +-------------------------+ VUSB         GND +-------------------------------+ |
|   |                       |                  |                             |   |
|   |                       |                  |                             |   |
|   |                       |                  |                             |   |
|   |                       |     +--<----+D7  |                             |   |
|   |                       |     +-+          |                             |   |
|   |                       |   RGB LED        |                             |   |
|   |                       |                  |                             |   |
|   |                       |              D6  +--------------+              |   |
|   |                       |                  |              |    +-----------+ |
| +-----------+             |                  |              |    |         |   |
|   |         |             |                  |              |    |         |   |
|   |         |             |              D5  +-------+      |    |     +-----+ |
| +-------+   |             |                  |       |      |    |     |   |   |
|   |     |   |             |                  |       |      |    |     |   | +--------+
| +----+  |   |             |   Particle       |       v      |    |     |   |   |      |
|   |  |  |   |             |    Xenon         |      +++     |    |     |   |   |      |
+---+  |  |   |             +------------------+  220 | |     |    |     |   +---+      |
       |  |   |                                   OHM +++     |    |     |              |
       |  |   |                                        |      |    |     |              |
       |  |   |                                        |      |    |     |              |
       |  |   |                                        |      |    |     |              |
       |  |   |       +---------+                      |      |    |     |              |
       |  |   |       | 5V FAN  |                 Gate |      |    |     |              |
       |  |   |       |         |                      |      |    |     |              |
       +--------------+ +     - +---------+            |      |    |     |              |
          |   |       |         |         |            |      |    |     |              |
          |   |       +---------+         |       +----v--+   |    |     |              |
          |   |                           |       |       |   |    |     |              |
          |   |        +--------+         +------->       +-----------------------------+
          |   |        |  IR    |          Drain  |       |   |    |     |      Source
          +------------+ +    - +--+              +-------+   |    |     |
              |        |  LED   |  |               N-Channel  |    |     |
              |        +--------+  |               MOSFET 30v |    |     |
              |                    |                          |    |     |
              |                    +-------------------------------+     |
              |                                               |          |
              |             +---------------------------------+          |
              |             |                                            |
              |       +-----+------+                                     |
              |       |   Signal   |                                     |
              +-------+ +        - +-------------------------------------+
                      |  IR Sensor |
                      +------------+

```

**Important Notes**

* D7 is the internal mono-color (blue) LED
  * off means IR beam is unbroken
  * blue means IR beam is broken
* RGB onboard LED is used for house status indicator
  * Off means house is unoccupied
  * Green means occupied
  * Red means cooldown
* D6 is hooked to the signal wire of the IR Sensor
* D5 is hooked to a 220Ohm resistor, to the gate of the N-ch MOSFET
* MOSFET is configured as follows for fan control:
  * Gate: 220 ohm resistor to D5
  * Drain: Ground pin on fan
  * Source: Ground bus
* Fan Pins:
  * 5v: 5v bus
  * Ground: Drain on MOSFET

# Build

```
$ make build
particle cloud compile xenon project.properties src/litterbox-ventilation --target 1.5.2 --saveTo firmware.bin

Compiling code for xenon
Targeting version: 1.5.2

Including:
    project.properties
    src/litterbox-ventilation/litterbox-ventilation.ino

attempting to compile firmware
downloading binary from: /v1/binaries/5f009ecce674958f35eef72a
saving to: firmware.bin
Memory use:
   text    data     bss     dec     hex filename
   9384     112    1100   10596    2964 /workspace/target/workspace.elf

Compile succeeded.
Saved firmware to: /Users/gabe/code/litterbox-ventilation/firmware.bin
```

# Flashing

```
# put device in DFU mode
$ make flash
...
```
