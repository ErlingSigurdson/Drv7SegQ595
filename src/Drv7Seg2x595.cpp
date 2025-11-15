/*************** FILE DESCRIPTION ***************/

/**
 * Filename: Drv7Seg2x595.cpp
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Purpose:  A class for driving a multiplexed 7-segment display using
 *           two daisy-chained 74HC595 shift register ICs.
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Notes:    Works with displays that have from 1 to 4
 *           character positions (digits).
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
                                    uint32_t data_pin,
                                    uint32_t latch_pin,
                                    uint32_t clock_pin,
                                    PosBit pos_1_bit,
                                    PosBit pos_2_bit,
                                    PosBit pos_3_bit,
                                    PosBit pos_4_bit
                                   )
{
    _status = begin_helper(DRV7SEG2X595_VARIANT_BIT_BANGING,
                           byte_order,
                           pos_switch_type,
                           latch_pin,
                           pos_1_bit,
                           pos_2_bit,
                           pos_3_bit,
                           pos_4_bit
                          );

    if (_status < 0) {
        return _status;
    }
    
    _data_pin  = data_pin;
    _clock_pin = clock_pin;
    pinMode(_data_pin,  OUTPUT);
    pinMode(_clock_pin, OUTPUT);

    return _status;
}

#ifdef DRV7SEG2X595_SPI_IMPLEMENTED
int32_t Drv7Seg2x595Class::begin_spi(ByteOrder byte_order,
                                     PosSwitchType pos_switch_type,
                                     uint32_t latch_pin,
                                     PosBit pos_1_bit,
                                     PosBit pos_2_bit,
                                     PosBit pos_3_bit,
                                     PosBit pos_4_bit
                                    )
{
    _status = begin_helper(DRV7SEG2X595_VARIANT_SPI,
                           byte_order,
                           pos_switch_type,
                           latch_pin,
                           pos_1_bit,
                           pos_2_bit,
                           pos_3_bit,
                           pos_4_bit
                          );

    if (_status < 0) {
        return _status;
    }
    
    SPI.begin();

    return _status;
}
#endif

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_STM32)
int32_t Drv7Seg2x595Class::begin_spi_custom_pins(ByteOrder byte_order,
                                                 PosSwitchType pos_switch_type,
                                                 uint32_t mosi_pin,
                                                 uint32_t latch_pin,
                                                 uint32_t sck_pin,
                                                 PosBit pos_1_bit,
                                                 PosBit pos_2_bit,
                                                 PosBit pos_3_bit,
                                                 PosBit pos_4_bit
                                                )
{
    _status = begin_helper(DRV7SEG2X595_VARIANT_SPI,
                           byte_order,
                           pos_switch_type,
                           latch_pin,
                           pos_1_bit,
                           pos_2_bit,
                           pos_3_bit,
                           pos_4_bit
                          );

    if (_status < 0) {
        return _status;
    }
    
    _mosi_pin = mosi_pin;
    _sck_pin  = sck_pin;
    SPI.begin(_sck_pin, -1, _mosi_pin, -1);
    
    return _status;
}
#endif

int32_t Drv7Seg2x595Class::output(uint8_t seg_byte,
                                  Pos pos,
                                  uint32_t anti_ghosting_retention_duration_us
                                 )
{
    /*--- Configuration status check ---*/

    if (_status < 0) {
        return _status;
    }

    //Serial.println("DEBUG: output func call status check passed.");


    /*--- Anti-ghosting retention ---*/

    if (_anti_ghosting_first_output_call == false) {
        //Serial.println("DEBUG: _anti_ghosting_first_output_call is __FALSE__");
    } else {
        //Serial.println("DEBUG: _anti_ghosting_first_output_call is __TRUE__");
    }

    if (_anti_ghosting_first_output_call == false) {

        //Serial.println("DEBUG: after first retention has started.");
        //Serial.print("DEBUG: retained pos is ");
        //Serial.println(static_cast<uint32_t>(_anti_ghosting_retained_pos));

        /* If this method has been called not for the character position
         * that must be turned on next, return and continue the retention.
         */
        if (pos != anti_ghosting_next_pos_to_output(_anti_ghosting_retained_pos)) {
            return DRV7SEG2X595_OUTPUT_ANTI_GHOSTING_RETENTION_RUNNING;
        }

        //Serial.println("DEBUG: next pos to output reached.");

        // If the retention timer hasn't elapsed, return and continue the retention.
        if (anti_ghosting_timer(anti_ghosting_retention_duration_us) == false) {
            return DRV7SEG2X595_OUTPUT_ANTI_GHOSTING_RETENTION_RUNNING;
        }

        //Serial.println("DEBUG: anti-ghosting timer lapsing detected.");
    } else {
        _anti_ghosting_first_output_call = false;
    }

    if (_anti_ghosting_first_output_call == false) {
        //Serial.println("DEBUG: _anti_ghosting_first_output_call is now FALSE.");
    }


    /*--- Composing pos_byte ---*/

    uint8_t pos_byte = DRV7SEG2X595_ALL_BITS_CLEARED_MASK;
    //Serial.println("DEBUG: composing pos_byte started.");

    switch (pos) {
        case Drv7SegPos1:
            if (_pos_1_bit == Drv7SegPosBitInitial) {
                return DRV7SEG2X595_OUTPUT_ERR_POS_BIT_NOT_SPECIFIED_FOR_POS;
            } else {
                pos_byte |= 1u << static_cast<uint32_t>(_pos_1_bit);   
            }
            break;

        case Drv7SegPos2:
            if (_pos_2_bit == Drv7SegPosBitInitial) {
                return DRV7SEG2X595_OUTPUT_ERR_POS_BIT_NOT_SPECIFIED_FOR_POS;
            } else {
                pos_byte |= 1u << static_cast<uint32_t>(_pos_2_bit);   
            }
            break;

        case Drv7SegPos3:
            if (_pos_3_bit == Drv7SegPosBitInitial) {
                return DRV7SEG2X595_OUTPUT_ERR_POS_BIT_NOT_SPECIFIED_FOR_POS;
            } else {
                pos_byte |= 1u << static_cast<uint32_t>(_pos_3_bit);   
            }
            break;

        case Drv7SegPos4:
            if (_pos_4_bit == Drv7SegPosBitInitial) {
                return DRV7SEG2X595_OUTPUT_ERR_POS_BIT_NOT_SPECIFIED_FOR_POS;
            } else {
                pos_byte |= 1u << static_cast<uint32_t>(_pos_4_bit);   
            }
            break;

        default:
            // Protection from tricky casts. 
            return DRV7SEG2X595_OUTPUT_ERR_INVALID_POS;
    }


    /*--- Account for character position switch type ---*/

    if (_pos_switch_type == Drv7SegActiveLow) {
        pos_byte ^= static_cast<uint8_t>(DRV7SEG2X595_ALL_BITS_SET_MASK);
    }


    /*--- Account for byte order ---*/
    
    uint8_t upper_byte;
    uint8_t lower_byte;
    
    if (_byte_order == Drv7SegPosByteFirst) {
        upper_byte = pos_byte;
        lower_byte = seg_byte;
    } else {
        upper_byte = seg_byte;
        lower_byte = pos_byte;
    }


    /*--- Shift data ---*/
    
    switch (_variant) {
        case DRV7SEG2X595_VARIANT_BIT_BANGING:
            digitalWrite(_latch_pin, LOW);
            /* In theory, shifting a single zeroed byte is enough to produce a blank output,
             * and byte order is irrelevant, because both seg_byte and pos_byte, being zeroed,
             * guarantee that either all segments will be turned off individually or the whole
             * character position will be turned off.
             *
             * However, in practice in can lead to artifacts due to imperfectness of shift
             * register ICs and switching devices. Therefore two bytes are shifted.
             *
             * The same is relevant to the SPI variant.
             */
            shiftOut(_data_pin, _clock_pin, MSBFIRST, DRV7SEG2X595_ALL_BITS_CLEARED_MASK);
            shiftOut(_data_pin, _clock_pin, MSBFIRST, DRV7SEG2X595_ALL_BITS_CLEARED_MASK);
            digitalWrite(_latch_pin, HIGH);

            digitalWrite(_latch_pin, LOW);
            shiftOut(_data_pin, _clock_pin, MSBFIRST, upper_byte);
            shiftOut(_data_pin, _clock_pin, MSBFIRST, lower_byte);
            digitalWrite(_latch_pin, HIGH);
            break;

        #ifdef DRV7SEG2X595_SPI_IMPLEMENTED
        case DRV7SEG2X595_VARIANT_SPI:
            digitalWrite(_latch_pin, LOW);
            SPI.transfer(DRV7SEG2X595_ALL_BITS_CLEARED_MASK);
            SPI.transfer(DRV7SEG2X595_ALL_BITS_CLEARED_MASK);
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
    //Serial.println("DEBUG: retention started.");

    return DRV7SEG2X595_OUTPUT_ANTI_GHOSTING_RETENTION_RUNNING;
}


/*--- Private methods ---*/

int32_t Drv7Seg2x595Class::begin_helper(int32_t variant,
                                        ByteOrder byte_order,
                                        PosSwitchType pos_switch_type,

                                        // Latch pin is used in all variants.
                                        int32_t latch_pin,

                                        /* These parameters always have values, even if
                                         * they were omitted in the begin_*() method call
                                         * (in this case default values are assigned).
                                         */
                                        PosBit pos_1_bit,
                                        PosBit pos_2_bit,
                                        PosBit pos_3_bit,
                                        PosBit pos_4_bit
                                       )
{
    /* Highly unlikely to occur without messing with the code,
     * but still preserved as a redundant safety measure.
     */
    if (variant < 0) {
        return DRV7SEG2X595_STATUS_ERR_VARIANT_NOT_SPECIFIED;
    }

    // Byte order validity check.
    if (byte_order != Drv7SegPosByteFirst && byte_order != Drv7SegSegByteFirst) {
        return DRV7SEG2X595_STATUS_ERR_INVALID_BYTE_ORDER;
    }

    // Position switch type validity check.
    if (pos_switch_type != Drv7SegActiveHigh && pos_switch_type != Drv7SegActiveLow) {
        return DRV7SEG2X595_STATUS_ERR_INVALID_POS_SWITCH_TYPE;
    }

    // Position bits validity check.
    PosBit pos_bit_arr[DRV7SEG2X595_POS_MAX] = {pos_1_bit, pos_2_bit, pos_3_bit, pos_4_bit};
    for (uint32_t i = 0; i < DRV7SEG2X595_POS_MAX; ++i) {

        // First position bit (array index zero) must belong to the 0..7 range (from LSB to MSB).
        if (i == 0) {
            if (pos_bit_arr[i] < Drv7SegPosBit0 || pos_bit_arr[i] > Drv7SegPosBit7) {
                return DRV7SEG2X595_STATUS_ERR_INVALID_POS_BIT;
            } else {
                ++_active_positions;  // Active positions are counted for anti-ghosting purposes.
            }
        }

        // Other position bits must belong to the -1..7 range (from the initial value to MSB).
        if (i > 0) {
            if (pos_bit_arr[i] < Drv7SegPosBitInitial || pos_bit_arr[i] > Drv7SegPosBit7) {
                return DRV7SEG2X595_STATUS_ERR_INVALID_POS_BIT;
            } else if (pos_bit_arr[i] > Drv7SegPosBitInitial) {
                ++_active_positions;  // Active positions are counted for anti-ghosting purposes.
            }
        }
    }

    _variant         = variant;
    _byte_order      = byte_order;
    _pos_switch_type = pos_switch_type;
    _latch_pin       = latch_pin;
    _pos_1_bit       = pos_1_bit;
    _pos_2_bit       = pos_2_bit;
    _pos_3_bit       = pos_3_bit;
    _pos_4_bit       = pos_4_bit;

    pinMode(_latch_pin, OUTPUT);

    return DRV7SEG2X595_STATUS_OK;
}

bool Drv7Seg2x595Class::anti_ghosting_timer(uint32_t anti_ghosting_retention_duration_us)
{
    uint64_t current_micros = micros();

    if (_anti_ghosting_timer_new_lap == true) {
        _anti_ghosting_timer_previous_micros = current_micros;
        _anti_ghosting_timer_new_lap = false;
    }

    if (current_micros - _anti_ghosting_timer_previous_micros >= anti_ghosting_retention_duration_us) {
        _anti_ghosting_timer_new_lap = true;
        return true;
    } else {
        return false;
    }
}

Drv7Seg2x595Class::Pos Drv7Seg2x595Class::anti_ghosting_next_pos_to_output(Drv7Seg2x595Class::Pos retained_pos)
{
    // If the fourth position is retained, the next position to light up is the first one.
    if (retained_pos == static_cast<Pos>(_active_positions)) {
        return Drv7SegPos1;
    // Otherwise the next position to light up = currently retained position + 1.
    } else {
        uint32_t next_pos_as_int = static_cast<uint32_t>(retained_pos) + 1;
        return static_cast<Pos>(next_pos_as_int);        
    }
}

