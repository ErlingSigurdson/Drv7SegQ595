# Overview

**Drv7Seg2x595** is a single-class Arduino library for driving a multiplexed 7-segment display using
two daisy-chained 74HC595 shift register ICs.

## Concept

7-segment displays are simple, cheap and reliable data output devices. However, interfacing them with
a microcontroller (MC) requires an appropriate software and, most commonly, an additional hardware.

Typically, 7-segment displays come in models with 1 to 4 character positions (digits). Number of input pins for
any given model equals 8 + number of positions, thus 9 is the minimum and 12 is the maximum (duplicated pins count
as one). Driving such a display requires a number of individual signals equal to the number of the input pins:
* 8 signals to turn ON and OFF individual segments (including a dot segment, also known as a decimal point or DP).
* 1 to 4 signals to turn ON and OFF whole positions.

Due to significant number of required signals, output-extending devices, such as output shift registers, are commonly
used. **74HC595**, sometimes simply called **595**, is a widely used 8-bit serial-in, parallel-out (SIPO) shift register
integrated circuit (IC) commonly employed to drive 7-segment displays. Despite being a SIPO register primarily, it also
features an auxiliary serial output that allows for **daisy-chaining**: connecting multiple 595s in such a way that
the serial output of the previous IC goes to the serial input of the next one and all ICs in the chain share the same
**clock** and **latch** signals. Two daisy-chained 595s form a 16-bit shift register, which is sufficient for
controlling any typical 7-segment display.

## Control bytes

This library assumes that the 16-bit register consists of two bytes with distinct roles:
* **segment byte** (**`seg_byte`**) controls individual display segments to form recognizable symbols.
* **position byte** (**`pos_byte`**) controls whole character positions and thus determines where the next segment
pattern (defined by `seg_byte`) will be output. 1 to 4 bits out of 8 are used, the rest are NC (not connected).

Provided API allows for any order of `seg_byte` and `pos_byte` placement within the register, that is, any of these
bytes may be either upper or lower byte. The byte order must be specified during the driver configuration.

### Position switching

Every position is powered (turned on) by either setting or clearing the `pos_byte` bit specified during the driver
configuration. It can be done because the bit value determines the logical output level (either high or low) of
the corresponding parallel output pin, which makes it possible to source/sink current from/to that pin.
The resulting current may be used to power the character position directly or via a switching device, such as
a transistor. The latter approach is far more common, since 595's capacity for sourcing/sinking current for
a whole set of 8 LEDs gets close to or exceeds its electrical limitations.

Depending on the display type (its common pin polarity) and the switching device polarity (whether it's active-high,
like an NPN transistor, or active-low, like a PNP transistor), character positiong will be turned on either by high
or low logical output level. Provided allows API allows for both variants. The switching type (active-high or
active-low) must be specified during the driver configuration.

## Multiplexing

This library relies on **multiplexing**: a way of driving a multi-digit 7-segment display by
turning on only one character position at a time in quick succession, in a cycle. This approach greatly simplifies
the circuit because it doesn't rely on a separate set of segment-control outputs for every character position and
instead utilizes the single set of segment-control outputs shared by **all** positions (the defined segment pattern
shows up only on the intended position anyway, because only that position will be turned on at the appropriate moment).

### Ghosting and countermeasures

Because multiplexing cycle run faster than the eye can follow, all digits appear to be lit continuously, even though
only one is actually turned on at a given moment. However, this comes with an unwanted side effect called **ghosting**:
segments can seem to have a faint, "ghost-like" glow on positions that are supposed to be off. To avoid this effect,
the library utilizes the anti-ghosting techniques. Some of them may be fine-tuned by the library user via the API.

Library's anti-ghosting logic is based on a non-blocking, `micros()`-based timer. No blocking, `delay()`-based timers
are used.

## Reference wiring

Here's a typical circuit diagram for the described arrangement (assumes a common-cathode display):

![circuit_diagram_(schematic).png](extras/images/circuit_diagram_(schematic).png)

For convenience, the example sketch default constant values corresponds to the circuit diagram provided here.

Wiring for a common-anode display is almost identical, the only difference being that the transistors' emitters
should connect to the display's common pins and their collectors should connect to the circuit's positive rail (VCC).

The wiring in your circuit **may** differ from the provided schematic to some degree, and the library will still be
applicable as long as your circuit complies with the premise of the `seg_byte` and `pos_byte` distinct roles.

## API usage

Include the library:
```cpp
#include <Drv7Seg2x595.h>    // Arduino style.
//#include "Drv7Seg2x595.h"  // Generic embedded programming style.
```

### Driver configuration

Choose **one** of the following variants.

Bit-banging:
```cpp
// Prototype.
int32_t begin_bb(ByteOrder byte_order,           // Whether seg_byte or pos_byte will be an upper byte.
                 PosSwitchType pos_switch_type,  // Logical level that turns on a character position on (high or low).
                 uint32_t data_pin,              // Pin that provides the next bit to be shifted.
                 uint32_t latch_pin,             // Pin that propagates the shifted data to the output register.
                 uint32_t clock_pin,             // Pin that commands the next bit to be shifted.
                 PosBit pos_1_bit,               // Number of the pos_byte bit that will control the 1st position.
                 PosBit pos_2_bit                // Number of the pos_byte bit that will control the 2nd position.
                 PosBit pos_3_bit                // Number of the pos_byte bit that will control the 3rd position.
                 PosBit pos_4_bit                // Number of the pos_byte bit that will control the 4th position.
                );

// Example call.
int32_t begin_bb(Drv7SegPosByteFirst,  // Other option is Drv7SegSegByteFirst.
                 Drv7SegActiveHigh,    // Other option is Drv7SegActiveLow.

                 /* Arguments must be unsigned integers corresponding to
                  * the Arduino digital output pins numbering.
                 16,
                 17,
                 18,

                 /* Valid arguments are PosBitN, where N is in the 0..7 range (MSB to LSB of pos_byte).
                  *
                  * First of these parameters (1st position bit) is required.
                  * Subsequent parameters are optional. The driver will be configured to control
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

Check if the driver was configured successfully:
```cpp
int32_t drv_config_status = Drv7Seg.get_status();

// Loop the error output if the driver configuration was unsuccessful.
if (drv_config_status < 0) {  // If an error is detected.
    while(true) {
        Serial.print("Error: driver configuration failed, error code ");
        Serial.println(drv_config_status);
        delay(INTERVAL);
    }
}
```

### Output

Commence the actual output:
```cpp
// Prototype.
Drv7Seg.output(uint8_t seg_byte,  // A byte that corresponds to the glyph to be output.
               Pos pos,           // Valid arguments are Drv7SegPosN, where N is in the 1..4 range.

               /* Duration (in microseconds) of a short period during which a currently output glyph
                * is retained on a respective character position. This parameter is optional,
                * the default value is 1000 microseconds.
                */
               uint32_t anti_ghosting_retention_duration_us
              )

/* Example calls.
 * Call output() method in quick succession for all character positions
 * controlled by pos_byte (for every position to which a position bit was
 * assigned at begin_*() method call).
 */
Drv7Seg.output(minutes_tens, Drv7SegPos1);
Drv7Seg.output(minutes_ones, Drv7SegPos2);
Drv7Seg.output(seconds_tens, Drv7SegPos3);
Drv7Seg.output(seconds_ones, Drv7SegPos4);
```

Refer to `Drv7Seg2x595.h` for more API details.

## Edge cases

* **Single-digit displays**. With a single-digit display there's usually no purpose in a switchable signal that turns
the only character position ON and OFF (all control job can be done by `seg_byte` alone), nor there's a need for
multiplexing. Still, you can use this library to control a single-digit display in a pinch. You can either assign
a single position bit at `begin_*()` method call and connect your display's common pin to the corresponding 595's
parallel output (in this case the multiplexing logic will still be applied, but your single position will always be
the one to be turned on next), or just ignore `pos_byte` completely by powering your display directly by connecting
it to GND or VCC, according to the polarity of its common pin. You will still have to pass a single position bit
argument during `begin_*()` method call to comply with the library logic, but its particular value becomes irrelevant
(you can pick randomly from `Drv7SegPosBit0` to `Drv7SegPosBit7`).

* **Not using the leftmost digit**. If you, for instance, have a 4-digit display and for some reason you want to use
only character positions 2 and 3, it's completely OK, you can do that. Pass two position bits during `begin_*()` method
call: one for the 2nd digit (it'll be treated as `Drv7SegPos1`) and another for the 3rd digit (it'll be treated as
`Drv7SegPos2`).

## Dependencies

* `SPI.h` library implementation for the Arduino core you're using. It is commonly available for all Arduino cores,
although it's not strictly guaranteed. In an unlikely case where it is not implemented for your core, you can still use
**Drv7Seg2x595** in a bit-banging mode, but in order to avoid compilation errors you'll have to manually uncomment
the `#define DRV7SEG2X595_SPI_PROVIDED_ASSUMED` preprocessor directive in `Drv7Seg2x595.h`.

* **SegMap595** C++ library (see links below) is used in the example sketch in order to simplify byte mapping, but
aside from that it's not a prerequisite for using `Drv7Seg2x595.h`.

## Compatibility

The library works with any Arduino-compatible MC capable of bit-banging or SPI data transfer.

Availability of the configuration variant that takes custom-assigned SPI pins on the capabilites
of a given MC and a corresponding `SPI.h` implementation. As of the last update to list library,
that variant is only provided for ESP32 and STM32 MC families.

### PCB design and rich circuit diagram

You may opt to use [KiCAD](https://www.kicad.org/) [files](extras/kicad/) provided with this library to build a DIY
hardware driver compliant with the library's premises and reference wiring. All hardware-related assets are licensed
under **CERN-OHL-P v2** permissive open license.

![pcb_view_w_footprints_preview.png](extras/images/pcb_view_w_footprints_preview.png)

[(Click here to view full-size image)](extras/images/pcb_view_w_footprints_full_size.png)

Using the provided design is totally **optional**! This library is built with
flexibility in mind and does **NOT** depend on a single particular wiring.

## Links

### This library
* [Primary repository on GitHub](https://github.com/ErlingSigurdson/Drv7Seg2x595)
* [Backup repository on GitFlic](https://gitflic.ru/project/efimov-d-v/drv7seg2x595)
* [Backup repository on Codeberg](https://codeberg.org/ErlingSigurdson/Drv7Seg2x595)

### SegMap595
* [Primary repository on GitHub](https://github.com/ErlingSigurdson/SegMap595)
* [Backup repository on GitFlic](https://gitflic.ru/project/efimov-d-v/segmap595)
* [Backup repository on Codeberg](https://codeberg.org/ErlingSigurdson/SegMap595)

## Contact details

**Maintainer** — Dmitriy Efimov aka Erling Sigurdson
* <efimov-d-v@yandex.ru>
* <erlingsigurdson1@gmail.com>
* Telegram: @erlingsigurdson

Your feedback and pull requests are welcome.
