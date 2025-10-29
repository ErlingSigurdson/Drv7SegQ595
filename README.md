# Overview

**Drv7Seg2x585** is a single-class Arduino library for driving a multiplexed 7-segment display
using two daisy-chained 74HC595 shift register ICs.

## Concept

Typically, 7-segment displays come in models with 1, 2, 3 or 4 digits (character positions). Number of input pins
for any given model equals 8 + number of digits, thus 12 pins is the maximum. Driving a display requires a number
of individual signals equal to the number of input pins:
* 8 signals to turn ON and OFF individual segments (including a dot segment).
* 1 to 4 signals to turn ON and OFF whole digits (although for a single-digit display there's usually no need
for a switchable signal and its common pin is typically connected to GND or VCC directly).

Due to significant number of output signals required for driving a 7-segment display, output-extending devices,
such as output shift registers, are commonly used.

**74HC595**, sometimes simply called **595**, is a widely used 8-bit serial-in, parallel-out (SIPO) shift register
integrated circuit (IC) commonly employed to drive 7-segment displays. Despite being a SIPO register primarily, it
also features an auxiliary serial output that allows for **daisy-chaining**: connecting multiple 595s in such a way
that the serial output of a previous IC goes to the serial input of the next one and all ICs in the chain share
the same **clock/SCK** and **latch** signals. Two daisy-chained 595s form a 16-bit shift register, which is sufficient
for controlling any typical 7-segment display.

This library assumes that the 16-bit register consists of two bytes with distinct roles:
* segment byte, or **`seg_byte`**, controls individual display segments and makes it possible to output readable glyphs;
* position byte, or **`pos_byte`**, determines character positions the glyph will be output to. 1 to 4 bits out of 8 are used,
the rest are NC (not connected).

The library API allows for any order of `seg_byte` and `pos_byte` placement within the register, that is, 
any of these bytes may be either upper or lower byte. 

`pos_byte` bits switch active character positions by electrically connecting and disconnecting the display's
common pin to a ground (for a common cathode) or to a positive rail (for a common anode). Usually it is done
via a switching device (most commonly a transistor), since 595's ability to source/sink current itself
for a whole set of 7 LEDs gets close to exceeds its electrical limitations.

Here's a typical circuit diagram for the described arrangement (assumes a common cathode display):
![Circuit diagram (schematic)](assets/circuit_diagram_(schematic).png)

Wiring for a common anode display is almost identical, the only difference being that the transistors' emitters
should connect to the display's common pins and their collectors should connect to the circuit's positive rail (VCC).

The wiring in your circuit **may** differ from the schematic provided in this README, and the library will still
be applicable. The only premise that must be followed is the distinction of `seg_byte` and `pos_byte` roles.

## Multiplexing

Multiplexing is


In this configuration first IC (first 8 bits) can be used to
turn display segments ON and OFF, thus causing various glyphs to be output. This byte is called "glyph byte",
or `gbyte`. 

for common anode inverted (in a sense that `cpbyte` outputs must connect common pins to the positive rail, not to the ground).


(`cpbyte`) (`gbyte`)





## API usage


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
