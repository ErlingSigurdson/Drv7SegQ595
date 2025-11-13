/*************** FILE DESCRIPTION ***************/

/**
 * Filename: Drv7Seg2x595.cpp
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Purpose:  A class for driving a multiplexed 7-segment display using
 *           two daisy-chained 74HC595 shift register ICs.
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Notes:
 */


/************ PREPROCESSOR DIRECTIVES ***********/

/*--- Includes ---*/

// This source file's own header file.
#include "Drv7Seg2x595.h"

// Additional Arduino libraries.
#ifdef DRV7SEG2X595_SPI_IMPLEMENTED
    #include <SPI.h>
#endif


/*************** GLOBAL VARIABLES ***************/

Drv7Seg2x595Class Drv7Seg;


/******************* FUNCTIONS ******************/

/*--- Constructors ---*/

Drv7Seg2x595Class::Drv7Seg2x595Class() {}


/*--- Public methods ---*/

int32_t Drv7Seg2x595Class::begin_bb(ByteOrder byte_order,
                                    PosSwitchType pos_switch_type,
                                    int32_t data_pin,
                                    int32_t latch_pin,
                                    int32_t clock_pin,
                                    int32_t pos_bit_1,
                                    int32_t pos_bit_2,
                                    int32_t pos_bit_3,
                                    int32_t pos_bit_4
                                   )
{
    _variant = DRV7SEG2X595_CONFIG_VARIANT_BIT_BANGING;

    _byte_order         = byte_order;
    _pos_switch_type    = pos_switch_type;

    _data_pin  = data_pin;
    _latch_pin = latch_pin;
    _clock_pin = clock_pin;
    pinMode(_data_pin,  OUTPUT);
    pinMode(_latch_pin, OUTPUT);
    pinMode(_clock_pin, OUTPUT);

    _pos_bit_1 = pos_bit_1;
    _pos_bit_2 = pos_bit_2;
    _pos_bit_3 = pos_bit_3;
    _pos_bit_4 = pos_bit_4;

    return _status = DRV7SEG2X595_CONFIG_STATUS_OK;
}

#ifdef DRV7SEG2X595_SPI_IMPLEMENTED
int32_t Drv7Seg2x595Class::begin_spi(ByteOrder byte_order,
                                     PosSwitchType pos_switch_type,
                                     int32_t latch_pin,
                                     int32_t pos_bit_1,
                                     int32_t pos_bit_2,
                                     int32_t pos_bit_3,
                                     int32_t pos_bit_4
                                    )
{
    _variant = DRV7SEG2X595_CONFIG_VARIANT_SPI;

    _byte_order         = byte_order;
    _pos_switch_type    = pos_switch_type;

    _latch_pin = latch_pin;
    pinMode(_latch_pin, OUTPUT);

    SPI.begin();

    _pos_bit_1 = pos_bit_1;
    _pos_bit_2 = pos_bit_2;
    _pos_bit_3 = pos_bit_3;
    _pos_bit_4 = pos_bit_4;

    return _status = DRV7SEG2X595_CONFIG_STATUS_OK;
}
#endif

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_STM32)
int32_t Drv7Seg2x595Class::begin_spi_custom_pins(ByteOrder byte_order,
                                                 PosSwitchType pos_switch_type,
                                                 int32_t mosi_pin,
                                                 int32_t latch_pin,
                                                 int32_t sck_pin,
                                                 int32_t pos_bit_1,
                                                 int32_t pos_bit_2,
                                                 int32_t pos_bit_3,
                                                 int32_t pos_bit_4
                                                )
{
    _variant = DRV7SEG2X595_CONFIG_VARIANT_SPI;

    _byte_order         = byte_order;
    _pos_switch_type    = pos_switch_type;

    _mosi_pin  = mosi_pin;
    _latch_pin = latch_pin;
    _sck_pin   = sck_pin;
    pinMode(_mosi_pin,  OUTPUT);
    pinMode(_latch_pin, OUTPUT);
    pinMode(_sck_pin,   OUTPUT);

    SPI.begin(_sck_pin, -1, mosi_pin, -1);

    _pos_bit_1 = pos_bit_1;
    _pos_bit_2 = pos_bit_2;
    _pos_bit_3 = pos_bit_3;
    _pos_bit_4 = pos_bit_4;

    return _status = DRV7SEG2X595_CONFIG_STATUS_OK;
}
#endif

int32_t Drv7Seg2x595Class::output(uint8_t  seg_byte,
                                  uint32_t pos,
                                  uint32_t anti_ghosting_retention_duration_us
                                 )
{
    /*--- Configuration status check ---*/

    if (_status < 0) {
        return _status;
    }


    /*--- Anti-ghosting retention ---*/

    static bool first_call = true;

    if (first_call == false) {  /* On the first call to this method the anti-ghosting retention is not in effect yet,
                                 * therefore the following condition checks take place only on subsequent calls.

        /* If this method has been called not for the character position
         * that must be turned on next, return and continue the retention.
         */
        if (pos != anti_ghosting_retention_next_pos_to_output(_anti_ghosting_retained_pos)) {
            return DRV7SEG2X595_OUTPUT_ANTI_GHOSTING_RETENTION_RUNNING;
        }

        // If the retention timer hasn't elapsed, return and continue the retention.
        if (anti_ghosting_retention_timer(anti_ghosting_retention_duration_us) == false) {
            return DRV7SEG2X595_OUTPUT_ANTI_GHOSTING_RETENTION_RUNNING;
        }
    } else {
        first_call == false;
    }


    /*--- Composing pos_byte ---*/

    uint8_t pos_byte = DRV7SEG2X595_BLANK_GLYPH;

    switch (pos) {
        case 1:
            if (_pos_bit_1 < 0) return DRV7SEG2X595_OUTPUT_ERR_NEGATIVE_POS_BIT;
            pos_byte |= 1 << _pos_bit_1;
            break;

        case 2:
            if (_pos_bit_2 < 0) return DRV7SEG2X595_OUTPUT_ERR_NEGATIVE_POS_BIT;
            pos_byte |= 1 << _pos_bit_2;
            break;

        case 3:
            if (_pos_bit_3 < 0) return DRV7SEG2X595_OUTPUT_ERR_NEGATIVE_POS_BIT;
            pos_byte |= 1 << _pos_bit_3;
            break;

        case 4:
            if (_pos_bit_4 < 0) return DRV7SEG2X595_OUTPUT_ERR_NEGATIVE_POS_BIT;
            pos_byte |= 1 << _pos_bit_4;
            break;

        default:
            return DRV7SEG2X595_OUTPUT_ERR_INVALID_POS;
    }


    /*--- Account for character position switch type ---*/

    if (_pos_switch_type == Drv7Seg2x595ActiveLow) {
        pos_byte ^= static_cast<uint8_t>(DRV7SEG2X595_ALL_BITS_SET_MASK);
    }


    /*--- Account for byte order ---*/
    
    uint8_t upper_byte;
    uint8_t lower_byte;
    
    if (_byte_order == Drv7Seg2x595PosByteFirst) {
        upper_byte = pos_byte;
        lower_byte = seg_byte;
    } else {
        upper_byte = seg_byte;
        lower_byte = pos_byte;
    }


    /*--- Shift data ---*/
    
    switch (_variant) {
        case DRV7SEG2X595_CONFIG_VARIANT_BIT_BANGING:
            digitalWrite(_latch_pin, LOW);
            /* Shifting a single zeroed byte is enough to produce a blank output,
             * and byte order is irrelevant, because both seg_byte and pos_byte,
             * being zeroed, guarantee that either all segments will be turned off
             * individually or the whole character position will be turned off.
             */
            shiftOut(_data_pin, _clock_pin, MSBFIRST, DRV7SEG2X595_BLANK_GLYPH);
            digitalWrite(_latch_pin, HIGH);

            digitalWrite(_latch_pin, LOW);
            shiftOut(_data_pin, _clock_pin, MSBFIRST, upper_byte);
            shiftOut(_data_pin, _clock_pin, MSBFIRST, lower_byte);
            digitalWrite(_latch_pin, HIGH);
            break;

        #ifdef DRV7SEG2X595_SPI_IMPLEMENTED
        case DRV7SEG2X595_CONFIG_VARIANT_SPI:
            digitalWrite(_latch_pin, LOW);
            /* Shifting a single zeroed byte is enough to produce a blank output,
             * and byte order is irrelevant, because both seg_byte and pos_byte,
             * being zeroed, guarantee that either all segments will be turned off
             * individually or the whole character position will be turned off.
             */
            SPI.transfer(DRV7SEG2X595_BLANK_GLYPH);
            digitalWrite(_latch_pin, HIGH);

            digitalWrite(_latch_pin, LOW);
            SPI.transfer(upper_byte);
            SPI.transfer(lower_byte);
            digitalWrite(_latch_pin, HIGH);
            break;
        #endif

        default:
            break;  // Do nothing and hail MISRA.
    }

    _anti_ghosting_retained_pos = pos;

    return DRV7SEG2X595_OUTPUT_OK;
}


/*--- Private methods ---*/

bool Drv7Seg2x595Class::anti_ghosting_retention_timer(uint32_t anti_ghosting_retention_duration_us)
{
    uint64_t current_micros = micros();
    static uint64_t previous_micros = current_micros;

    static bool new_lap = true;
    if (new_lap == true) {
        previous_micros = current_micros;
        new_lap = false;
    }

    if (current_micros - previous_micros >= anti_ghosting_retention_duration_us) {
        new_lap = true;
        return true;
    } else {
        return false;
    }
}

uint32_t Drv7Seg2x595Class::anti_ghosting_next_pos(uint32_t retained_pos)
{
    /* When a character position is retained (output glyph is)
     *
     */


    if (retained_pos != 4) {
        return retained_pos + 1;  // Next position to output glyph on is the 
    } else {
        return 1;                 /* If 4th position is retained,
                                   * next position to output glyph on is 1st.
                                   */ 
    }
}

