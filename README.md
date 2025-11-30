# Overview

**Drv7Seg2x595** is a single-class Arduino library for driving a multiplexed 7-segment display using
two daisy-chained 74HC595 shift register ICs.

## Concept

7-segment displays are simple, cheap and reliable output devices. However, interfacing them with a microcontroller
unit (MCU) requires an appropriate software and hardware driver.

Typically, 7-segment displays come in models with 1 to 4 character positions (digits). Number of input pins for
any given model equals 8 + number of positions, thus 9 is the minimum and 12 is the maximum (duplicated pins count
as one).

Driving such a display requires a number of individual signals equal to the number of input pins:
* 8 signals to turn ON and OFF individual segments (including a dot segment, also known as a decimal point or DP).
* 1 to 4 signals to turn ON and OFF whole positions.

Due to significant number of required signals, output-extending devices, such as output shift registers, are commonly
used.

**74HC595**, sometimes simply called **595**, is a widely used 8-bit serial-in, parallel-out (SIPO) shift register
integrated circuit (IC) commonly employed to drive 7-segment displays. Despite being a SIPO register primarily,
it also features an auxiliary serial output that allows for **daisy-chaining**: connecting multiple 595s in such a way
that the serial output of the previous IC goes to the serial input of the next one and all ICs in the chain share
the same **clock** / **SCK** and **latch** signals. Two daisy-chained 595s form a 16-bit shift register, which is
sufficient for controlling any typical 7-segment display.

## Control bytes

This library assumes that the 16-bit register consists of two bytes with distinct roles:
* **segment byte** (**`seg_byte`**) controls individual display segments to form recognizable symbols.
* **position byte** (**`pos_byte`**) controls whole character positions and thus determines the character position
on which the segment pattern (defined by `seg_byte`) will be output. 1 to 4 bits out of 8 are used, the rest are
NC (not connected).

The library API allows for any order of `seg_byte` and `pos_byte` placement within the register, that is, 
any of these bytes may be either upper or lower byte.

`pos_byte` bits switch character positions by electrically connecting and disconnecting the display's common pin to
the ground (GND, for a common-cathode display) or to the positive rail (VCC, for a common-anode display). Usually it
is done via a switching device (most commonly a transistor), since 595's ability to source/sink current by itself for
a whole set of 7 LEDs gets close to exceeds its electrical limitations.

## Multiplexing

This library assumes a hardware driver built for **multiplexing**: a way of driving a multi-digit 7-segment display by
turning on only one character position at a time in quick succession, in a cycle. This approach greatly simplifies
the circuit because it doesn't rely on a separate set of segment-control outputs for every character position and
instead utilizes the single set of segment-control outputs shared by **all** positions (the defined segment pattern
shows up only on the intended position anyway, because only that position will be turned on at the appropriate moment).

### Ghosting and countermeasures

Because multiplexing cycle run faster than the eye can follow, all digits appear to be lit continuously, even though
only one is actually turned on at a given moment. However, this comes with an unwanted side effect called **ghosting**:
segments can seem to have a faint, "ghost-like" glow on positions that are supposed to be off. To avoid this effect,
the library relies on the anti-ghosting techniques, some of them may be fine-tuned by the library user via the API.

## Reference wiring

Here's a typical circuit diagram for the described arrangement (assumes a common-cathode display):
![Circuit diagram (schematic)](extras/images/circuit_diagram_(schematic).png)

Wiring for a common-anode display is almost identical, the only difference being that the transistors' emitters
should connect to the display's common pins and their collectors should connect to the circuit's positive rail (VCC).

For convenience, the example sketch default constant values corresponds to the circuit diagram provided here.

The wiring in your circuit **may** differ from the provided schematic to some degree, and the library will still be
applicable as long as your circuit complies with the premise of the `seg_byte` and `pos_byte` distinct roles.

## API usage

Include the library:
```cpp
#include <Drv7Seg2x595.h>    // Arduino style.
//#include "Drv7Seg2x595.h"  // Generic embedded programming style.
```

### Driver configuration

Choose **one** variant.

Bit-banging:
```cpp
// Prototype.
int32_t begin_bb(ByteOrder byte_order,           // Whether seg_byte or pos_byte will be an upper byte.
                 PosSwitchType pos_switch_type,  // Logical level that turns the position on (high or low).
                 uint32_t data_pin,              // Pin that provides the next bit to be shifted.
                 uint32_t latch_pin,             // Pin that moves the shifted data to the output register.
                 uint32_t clock_pin,             // Pin that commands the next bit to be shifted.
                 PosBit pos_1_bit,               // Number of the pos_byte bit that will control the 1st position.
                 PosBit pos_2_bit                // Number of the pos_byte bit that will control the 2nd position.
                 PosBit pos_3_bit                // Number of the pos_byte bit that will control the 3rd position.
                 PosBit pos_4_bit                // Number of the pos_byte bit that will control the 4th position.
                );

// Example call.
int32_t begin_bb(Drv7SegPosByteFirst,  // Other option is Drv7SegSegByteFirst.
                 Drv7SegActiveHigh,    // Other option is Drv7SegActiveLow.

                 // Arguments must be unsigned integers and correspond to the Arduino pin numbering.
                 DATA_PIN,
                 LATCH_PIN,
                 CLOCK_PIN,

                 /* Valid arguments are PosBitN, where N is in the 0..7 range (MSB to LSB of pos_byte).
                  *
                  * First of these parameters (bit that controls the 1st position) is required.
                  * Subsequent parameters are omittable. The driver will be configured to control
                  * the number of character positions equal to the number of parameters that weren't omitted.
                  */
                 PosBit7,
                 PosBit5,
                 PosBit3,
                 PosBit1
                );
```

SPI with default pins:
```cpp
/* Mostly identical to the bit-banging variant, but only the latch pin needs to be specified
 * (default MOSI pin must be used as a data pin and default SCK pin must be used as a clock pin).
 */

// Example call.
int32_t begin_spi(...
                  LATCH_PIN,
                  ...
                 );
```

SPI with custom pins:
```cpp
/* Mostly identical to the bit-banging variant, but the data pin role goes to
 * the specified MOSI pin and the clock pin role goes to the specified SCK pin.
 */

// Example call.
int32_t begin_spi_custom_pins(...
                              MOSI_PIN,
                              LATCH_PIN,
                              SCK_PIN
                              ...
                             );
```

### Status check


### Output

Commence the actual output:

## Glyphs and byte mapping

Typically, outputting a glyph (a character representation) on a 7-segment display involves composing a byte
whose combination of bit states (set or cleared) corresponds to a pattern in which the segments must be turned
ON and OFF to form a recognizable symbol. Finding the proper correspondence between the bit states and the segment
pattern is called **mapping**.

**Mapped bytes** (sometimes called **bit masks**) can be precomputed and hard-coded into a program
run by a microcontroller unit (MCU) or a similar device that drives a display. Although it may be perfectly
acceptable, it may become troublesome if the program needs to be adapted to a circuit with a different wiring
order between the device's outputs and the display's control pins. To simplify and automate the task, you may want to
try [SegMap595](https://github.com/ErlingSigurdson/SegMap595) library that provides a simple API for byte mapping and
retrieving the necessary bytes.

## Compatibility

The library works with any Arduino-compatible MCU capable of bit-banging or SPI data transfer.

## Dependencies

* `SPI.h` library implementation for the Arduino core you're using. It is commonly available for all Arduino cores,
although it's not guaranteed. In an unlikely case where it is not implemented for your core, you can still use
**Drv7Seg2x595** in a bit-banging mode, but in order to avoid compilation errors you'll have to manually uncomment
the `#define DRV7SEG2X595_SPI_PROVIDED_ASSUMED` preprocessor directive in `Drv7Seg2x595.h`.

TODO: note on custom pins

* **SegMap595** C++ library (see links below) is used in the example sketch to
simplify proper glyph output, but otherwise isn't necessary at all. 

TODO: micros provided

## Miscellaneous

No blocking, `delay()`-based timers are used. Instead, the library relies on non-blocking, `micros()`-based timers.

## Links

### This library
* [Primary repository on GitHub](https://github.com/ErlingSigurdson/Drv7Seg2x595)
* [Backup repository on GitFlic](https://gitflic.ru/project/efimov-d-v/drv7seg2x595)

### A library used in the example sketch — **SegMap595**
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

TODO: only one begin_*() method to choose

TODO re how to use output() method:
     * Call output() method in quick succession for all character positions
     * controlled by pos_byte (for every position to which a position bit was
     * assigned at begin_*() method call).
     */

Refer to `Drv7Seg2x595.h` for more API details.

TODO: single-position displays

TODO: single-IC arrangements

TODO: ghosting definition and explanation

TODO: position switch type (N-type, P-type)


## Single-digit displays

TODO: rename "Edge cases", move below

With a single-digit display there's usually no purpose in a switchable signal that turns the only digit ON and OFF
wholly. Instead, the common pin would go directly to GND (for common cathode) or VCC (for common anode). With such
configuration using daisy-chained 595s would be commonly regarded as redundant. However, the library still supports
driving a single-digit display. This assumes that one of `pos_byte` bits controls the only available digit.

for a switchable signal and its common pin is typically connected to GND or VCC directly)
