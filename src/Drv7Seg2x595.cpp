/*************** FILE DESCRIPTION ***************/

/**
 * Filename: Drv7Seg2x595.cpp
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Purpose:  A class for shifting 2-byte data into 2 daisy-chained 74HC595 ICs.
 *           Usually used to drive a multiplexed 4-digit 7-segment display.
 *           Intended for use with the ESP32 or ESP8266 Arduino core.
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Notes:
 */


/************ PREPROCESSOR DIRECTIVES ***********/

/*--- Includes ---*/

// This source file's own header file.
#include "Drv7Seg2x595.h"

// Additional Arduino libraries.
#include <SPI.h>


/*************** GLOBAL VARIABLES ***************/

Drv7Seg2x595 drv_7seg_2x595;


/******************* FUNCTIONS ******************/

/*--- Constructor ---*/

Drv7Seg2x595::Drv7Seg2x595() {}


/*--- Misc functions ---*/

int32_t Drv7Seg2x595::init_bb(int32_t byte_order, int32_t display_common_pin, int32_t switch_polarity,
                              int32_t data_pin, int32_t latch_pin, int32_t clock_pin,
                              int32_t pos_bit_1, int32_t pos_bit_2, int32_t pos_bit_3, int32_t pos_bit_4
                             )
{
    _variant = DRV7SEG2X595_VARIANT_BIT_BANGING;

    _byte_order = byte_order;
    _display_common_pin = display_common_pin;
    _switch_polarity = switch_polarity.


    /*--- Bit-banging pins ---*/
    
    if (data_pin < 0 || latch_pin < 0 || clock_pin < 0) {
        return _status = DRV7SEG2X595_STATUS_ERR_SIG_PINS;
    } else {
        _data_pin =  data_pin;
        _latch_pin = latch_pin;
        _clock_pin = clock_pin;

        pinMode(_data_pin,  OUTPUT);
        pinMode(_latch_pin, OUTPUT);
        pinMode(_clock_pin, OUTPUT);
    }


    /*--- Character position bits ---*/

    if (pos_bit_1 < 0 && pos_bit_2 < 0 && pos_bit_3 < 0 && pos_bit_4 < 0) {
        return _status = DRV7SEG2X595_STATUS_ERR_POS_BITS;
    } else {
        _pos_bit_1 = pos_bit_1;
        _pos_bit_2 = pos_bit_2;
        _pos_bit_3 = pos_bit_3;
        _pos_bit_4 = pos_bit_4;
    }

    return _status = DRV7SEG2X595_STATUS_OK;
}

/*
void Drv7Seg2x595::init_spi(uint32_t latch_pin, uint32_t ghosting_prevention_delay)
{
    _variant = Drv7Seg2x595_VARIANT_SPI;
    _latch_pin = latch_pin;
    _ghosting_prevention_delay = ghosting_prevention_delay;

    pinMode(latch_pin, OUTPUT);
    SPI.begin();
}
*/

/*
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_STM32)
void Drv7Seg2x595::init_spi(uint32_t mosi_pin, uint32_t latch_pin, uint32_t sck_pin,
                                  uint32_t ghosting_prevention_delay)
{
    _variant = Drv7Seg2x595_VARIANT_SPI;
    _latch_pin = latch_pin;
    _ghosting_prevention_delay = ghosting_prevention_delay;

    pinMode(_latch_pin, OUTPUT);
    SPI.begin(sck_pin, -1, mosi_pin, -1);
}
#endif
*/

int32_t Drv7Seg2x595::output(uint8_t seg_byte, uint32_t pos, uint32_t ghosting_prevention_delay)
{
    if (_status < 0) {
        return _status;
    }


    /*--- Composing of character position byte ---*/

    if (pos > DRV7SEG2X595_MAX_POS) {
        return DRV7SEG2X595_ERR_MAX_POS;
    }

    _pos_byte = DRV7SEG2X595_BLANK_GLYPH;

    switch (pos) {
        case 1:
            _pos_byte |= 1 << _pos_bit_1;
            break;

        case 2:
            _pos_byte |= 1 << _pos_bit_2;
            break;

        case 3:
            _pos_byte |= 1 << _pos_bit_3;
            break;

        case 4:
            _pos_byte |= 1 << _pos_bit_4;
            break;

        default:
            break;  // Do nothing and hail MISRA.
    }


    /*--- Account for display type (its common pin) ---*/

    if (_display_common_pin != 0) {
        _pos_byte ^= static_cast<uint8_t>(DRV7SEG2X595_ALL_BITS_SET_MASK);
    }


    /*--- Account for byte order ---*/
    
    uint8_t upper_byte;
    uint8_t lower_byte;
    
    if (_byte_order == DRV7SEG2X595_POS_BYTE_FIRST) {
        upper_byte = _pos_byte;
        lower_byte = seg_byte;
    } else {
        upper_byte = seg_byte;
        lower_byte = _pos_byte;       
    }


    /*--- Shift data ---*/
    
    switch (_variant) {
        case DRV7SEG2X595_VARIANT_INITIAL:
            return DRV7SEG2X595_ERR_VARIANT_NOT_SET;

        case DRV7SEG2X595_VARIANT_BIT_BANGING:
            digitalWrite(_latch_pin, LOW);
            shiftOut(_data_pin, _clock_pin, MSBFIRST, upper_byte);
            shiftOut(_data_pin, _clock_pin, MSBFIRST, lower_byte);
            digitalWrite(_latch_pin, HIGH);
            delayMicroseconds(ghosting_prevention_delay);
            
            digitalWrite(_latch_pin, LOW);
            // Single byte is enough in this case since it's guaranteed to produce a blank output.
            shiftOut(_data_pin, _clock_pin, MSBFIRST, DRV7SEG2X595_BLANK_GLYPH);
            digitalWrite(_latch_pin, HIGH);
            break;

        case DRV7SEG2X595_VARIANT_SPI:
            digitalWrite(_latch_pin, LOW);
            SPI.transfer(upper_byte);
            SPI.transfer(lower_byte);
            digitalWrite(_latch_pin, HIGH);
            delayMicroseconds(ghosting_prevention_delay);
            
            digitalWrite(_latch_pin, LOW);
            // Single byte is enough in this case since it's guaranteed to produce a blank output.
            SPI.transfer(DRV7SEG2X595_BLANK_GLYPH);
            digitalWrite(_latch_pin, HIGH);
            break;

        default:
            break;  // Do nothing and hail MISRA.
    }

    return DRV7SEG2X595_STATUS_OK;
}
