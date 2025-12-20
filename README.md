# Overview

**Drv7SegQ595** is a single-class Arduino library for driving a multiplexed 7-segment display using
a single 74HC595 shift register and a set of transistors.

## Parent project

This library is a derivative (a fork) of the [Drv7Seg2x595](https://github.com/ErlingSigurdson/Drv7Seg2x595) library.
The main difference is that the parent project uses two daisy-chained 74HC595 shift register ICs, while this library
uses a single shift register IC and a set of 1 to 4 GPIO-driven transistors instead.

## Concept

7-segment displays are simple, cheap and reliable data output devices. However, interfacing them with
a microcontroller (MC) requires appropriate software and, most commonly, additional hardware.

Typically, 7-segment displays come in models with 1 to 4 character positions (digits). Number of input pins for
any given model equals 8 + number of positions, thus 9 is the minimum and 12 is the maximum (duplicated pins count
as one). Driving such a display requires a number of individual signals equal to the number of the input pins:
* 8 signals to turn ON and OFF individual segments (including a dot segment, also known as a decimal point or DP).
* 1 to 4 signals to turn ON and OFF whole positions.

WORKING HERE

Due to significant number of required signals, output-extending devices, such as output shift registers, are commonly
used. **74HC595**, sometimes simply called **595**, is a widely used 8-bit serial-in, parallel-out (SIPO) shift register
integrated circuit (IC) commonly employed to drive 7-segment displays. A single 595 provies an 8-bit shift register,
which is sufficient for controlling any typical 7-segment display.

The API provided by this library allows for control over 1 to 4 character positions. The number of positions to be
used must be specified during the driver configuration.

DIDN'T GO FURTHER THAN HERE
TODO notice there's no PCB design

## License

This library is licensed under the **MIT License** (see `LICENSE` [here](LICENSE)).

## Links

### This library
* [Primary repository on GitHub](https://github.com/ErlingSigurdson/Drv7SegQ595)
* [Backup repository on GitFlic](https://gitflic.ru/project/efimov-d-v/drv7segq595)
* [Backup repository on Codeberg](https://codeberg.org/ErlingSigurdson/Drv7SegQ595)

### SegMap595
* [Primary repository on GitHub](https://github.com/ErlingSigurdson/SegMap595)
* [Backup repository on GitFlic](https://gitflic.ru/project/efimov-d-v/segmap595)
* [Backup repository on Codeberg](https://codeberg.org/ErlingSigurdson/SegMap595)

### Drv7Seg2x595 (parent project)
* [Primary repository on GitHub](https://github.com/ErlingSigurdson/Drv7Seg2x595)
* [Backup repository on GitFlic](https://gitflic.ru/project/efimov-d-v/drv7seg2x595)
* [Backup repository on Codeberg](https://codeberg.org/ErlingSigurdson/Drv7Seg2x595)

## Contact details

**Maintainer** — Dmitriy Efimov aka Erling Sigurdson
* <efimov-d-v@yandex.ru>
* <erlingsigurdson1@gmail.com>
* Telegram: @erlingsigurdson

Your feedback and pull requests are welcome.
