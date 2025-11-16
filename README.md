# Overview

**Drv7Seg2x595** is a single-class Arduino library for driving a multiplexed 7-segment display
using two daisy-chained 74HC595 shift register ICs.

## Concept

Typically, 7-segment displays come in models with 1 to 4 character positions (digits). Number of input pins for
any given model equals 8 + number of digits, thus 12 pins is the maximum. Driving a display requires a number o
individual signals equal to the number of input pins:
* 8 signals to turn ON and OFF individual segments (including a dot segment).
* 1 to 4 signals to turn ON and OFF whole digits.

Due to significant number of output signals required for driving a 7-segment display, output-extending devices,
such as output shift registers, are commonly used.

**74HC595**, sometimes simply called **595**, is a widely used 8-bit serial-in, parallel-out (SIPO) shift register
integrated circuit (IC) commonly employed to drive 7-segment displays. Despite being a SIPO register primarily, it
also features an auxiliary serial output that allows for **daisy-chaining**: connecting multiple 595s in such a way
that the serial output of a previous IC goes to the serial input of the next one and all ICs in the chain share
the same **clock/SCK** and **latch** signals. Two daisy-chained 595s form a 16-bit shift register, which is sufficient
for controlling any typical 7-segment display.

This library assumes that the 16-bit register consists of two bytes with distinct roles:
* **segment byte**, or **`seg_byte`**, controls individual display segments and makes it possible to output readable glyphs;
* **position byte**, or **`pos_byte`**, determines character positions the glyph will be output to. 1 to 4 bits out of 8 are used
the rest are NC (not connected).

The library API allows for any order of `seg_byte` and `pos_byte` placement within the register, that is, 
any of these bytes may be either upper or lower byte. 

`pos_byte` bits switch digits by electrically connecting and disconnecting the display's common pin to
a ground (GND, for a common cathode) or to a positive rail (VCC, for a common anode). Usually it is done
via a switching device (most commonly a transistor), since 595's ability to source/sink current itself for
a whole set of 7 LEDs gets close to exceeds its electrical limitations.

Here's a typical circuit diagram for the described arrangement (assumes a common cathode display):
![Circuit diagram (schematic)](assets/circuit_diagram_(schematic).png)

Wiring for a common-anode display is almost identical, the only difference being that the transistors' emitters
should connect to the display's common pins and their collectors should connect to the circuit's positive rail (VCC).

The wiring in your circuit **may** differ from the schematic provided in this README, and the library will still
be applicable. The only premise that must be followed is the distinction of `seg_byte` and `pos_byte` roles.

TODO: defaults in the examples sketch corresponds to the circuit diagram image.

## Multiplexing

Multiplexing is

## Single-digit displays

With a single-digit display there's usually no purpose in a switchable signal that turns the only digit ON and OFF
wholly. Instead, the common pin would go directly to GND (for common cathode) or VCC (for common anode). With such
configuration using daisy-chained 595s would be commonly regarded as redundant. However, the library still supports
driving a single-digit display. This assumes that one of `pos_byte` bits controls the only available digit.


for a switchable signal and its common pin is typically connected to GND or VCC directly)


In this configuration first IC (first 8 bits) can be used to
turn display segments ON and OFF, thus causing various glyphs to be output. This byte is called "glyph byte",
or `gbyte`. 

for common anode inverted (in a sense that `cpbyte` outputs must connect common pins to the positive rail, not to the ground).


(`cpbyte`) (`gbyte`)





## API usage

```cpp
#define DATA_PIN  16
#define LATCH_PIN 17
#define CLOCK_PIN 18

#define POS_1_BIT 7
#define POS_2_BIT 5
#define POS_3_BIT 3
#define POS_4_BIT 1

drv_7seg_2x595.init_bb(DRV7SEG2X595_POS_BYTE_FIRST, // Other option is DRV7SEG2X595_SEG_BYTE_FIRST. 
                       DRV7SEG2X595_COMMON_CATHODE, // Other option is DRV7SEG2X595_COMMON_ANODE.
                       DRV7SEG2X595_POLARITY_N,     // Other option is DRV7SEG2X595_POLARITY_P.

                       // Signed integers, must be >= 0.
                       DATA_PIN,   // Signed integer, must be >= 0.
                       LATCH_PIN,  // Signed integer, must be >= 0.
                       CLOCK_PIN,  // Signed integer, must be >= 0.

                       /* Signed integers, may be omitted (default to a negative integer).
                        * If not omitted, must be >= 0.
                        */
                       POS_1_BIT,  // Signed integer, must be >= 0.
                       POS_2_BIT,  // Signed integer, must be >= 0, may be omitted (defaults to a negative integer).
                       POS_3_BIT,  // Signed integer, must be >= 0, may be omitted (defaults to a negative integer).
                       POS_4_BIT,  // Signed integer, must be >= 0, may be omitted (defaults to a negative integer).
                      );
}
```

## Glyphs and byte mapping

Typically, outputting a glyph (a character representation) to a 7-segment display involves custom-forming a byte
whose combination of bit states (set or cleared) corresponds to a pattern in which the segments must be turned
ON and OFF to form a recognizable symbol. Finding the proper correspondence between the bit states and the segment
pattern is called **mapping**.

The **mapped bytes** (sometimes called **bit masks**) can be formed in advance and hard-coded into a program.
Although it may be perfectly acceptable, it may become troublesome if the program needs to be adapted to a circuit
with a different wiring order between the device's outputs and the display's control pins. To simplify and automate
the task, you may want to try [SegMap595](https://github.com/ErlingSigurdson/SegMap595) library that provides
a simple API for byte mapping and retrieving the necessary bytes.

## Compatibility

The library works with any Arduino-compatible MCU capable of bit-banging or SPI data transfer.

## Dependencies

* `SPI.h` library implementation for the Arduino core you're using.

`SPI.h` is commonly available for all Arduino cores, although it's not guaranteed.
In an unlikely case where it is not implemented for your core you can still use **Drv7Seg2x595**
in bit-banging mode, but in order to avoid compilation errors you'll have to manually uncomment
the `#define DRV7SEG2X595_SPI_NOT_IMPLEMENTED` preprocessor directive in `Drv7Seg2x595.h`.

* **SegMap595** C++ library (see links below) is used in the example sketch to
simplify proper glyph output, but otherwise isn't necessary at all. 

TODO: micros provided

## Links

### This library
* [Primary repository on GitHub](https://github.com/ErlingSigurdson/Drv7Seg2x595)
* [Backup repository on GitFlic](https://gitflic.ru/project/efimov-d-v/drv7seg2x595)

### Related library — **SegMap595**
* [Primary repository on GitHub](https://github.com/ErlingSigurdson/SegMap595)
* [Backup repository on GitFlic](https://gitflic.ru/project/efimov-d-v/segmap595)

## Contact details

**Maintainer** — Dmitriy Efimov aka Erling Sigurdson
* <efimov-d-v@yandex.ru>
* <erlingsigurdson1@gmail.com>
* Telegram: @erlingsigurdson

Your feedback and pull requests are welcome.



* * * * * * *

 *           TODO dependencies (SPI.h).
 *           TODO what if not first digit on a physical display?
 * TODO: note on not using a blocking timer.

"Using two 74HC595 for driving a 7-segment display may seem crude since there are specialized solutions available, but I think it bears a certain appeal: it's very transparent, which suits DIY approach perfectly. Basically you control a register, and low-level register control is a big thing in microcontroller world."

In Drv7Seg2x595 README: You still **can** drive a single-digit display using two 595s and this library. If your display is powered via a switch controlled by a pos_byte bit, just make sure that respective bit will always be ON. If your display is powered independently, use only seg_byte (pos_byte still must be shifted to the register, but its value is irrelevant).

PCB (all KiCAD stuff) license note

MCU explaining meaning early in the README text
