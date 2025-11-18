/*************** FILE DESCRIPTION ***************/

/**
 * Filename: Drv7Seg2x595.cpp
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Purpose:  A class for driving a multiplexed 7-segment display using
 *           two daisy-chained 74HC595 shift register ICs.
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Notes:    Intended for displays with 1 to 4 character positions (digits).
 *
 *           seg_byte means a byte that turns ON and OFF individual segments.
 *
 *           pos_byte means a byte that turns ON and OFF whole character
 *           positions (digits).
 */


/************ PREPROCESSOR DIRECTIVES ***********/

/*--- Includes ---*/

// This source file's own header file.
#include "Drv7Seg2x595.h"

// Additional Arduino libraries.
#ifdef DRV7SEG2X595_SPI_PROVIDED
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

#ifdef DRV7SEG2X595_SPI_PROVIDED
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

#ifdef DRV7SEG2X595_SPI_PROVIDED_CUSTOM_PINS
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


    /*--- Anti-ghosting retention ---*/

    if (anti_ghosting_retention_duration_us > 0 && _anti_ghosting_first_output_call == false) {
        /* If this method has been called not for the character position
         * that must be turned on next, return and continue the retention.
         */
        if (pos != anti_ghosting_next_pos_to_output()) {
            return DRV7SEG2X595_OUTPUT_ANTI_GHOSTING_RETENTION_RUNNING;
        }

        // If the retention timer hasn't elapsed, return and continue the retention.
        if (anti_ghosting_timer(anti_ghosting_retention_duration_us) == false) {
            return DRV7SEG2X595_OUTPUT_ANTI_GHOSTING_RETENTION_RUNNING;
        }
    } else {
        _anti_ghosting_first_output_call = false;
    }


    /*--- Composing pos_byte ---*/

    uint8_t pos_byte = DRV7SEG2X595_ALL_BITS_CLEARED_MASK;
    switch (pos) {
        case Drv7SegPos1:
            if (_pos_1_bit == Drv7SegPosBitInitial) {
                return DRV7SEG2X595_OUTPUT_ERR_POS_BIT_NOT_SPECIFIED_FOR_POS;
            } else {
                pos_byte |= 1u << static_cast<uint8_t>(_pos_1_bit);
            }
            break;

        case Drv7SegPos2:
            if (_pos_2_bit == Drv7SegPosBitInitial) {
                return DRV7SEG2X595_OUTPUT_ERR_POS_BIT_NOT_SPECIFIED_FOR_POS;
            } else {
                pos_byte |= 1u << static_cast<uint8_t>(_pos_2_bit);
            }
            break;

        case Drv7SegPos3:
            if (_pos_3_bit == Drv7SegPosBitInitial) {
                return DRV7SEG2X595_OUTPUT_ERR_POS_BIT_NOT_SPECIFIED_FOR_POS;
            } else {
                pos_byte |= 1u << static_cast<uint8_t>(_pos_3_bit);
            }
            break;

        case Drv7SegPos4:
            if (_pos_4_bit == Drv7SegPosBitInitial) {
                return DRV7SEG2X595_OUTPUT_ERR_POS_BIT_NOT_SPECIFIED_FOR_POS;
            } else {
                pos_byte |= 1u << static_cast<uint8_t>(_pos_4_bit);
            }
            break;

        default:
            // Protection from unexpected casts.
            return DRV7SEG2X595_OUTPUT_ERR_INVALID_POS;
    }


    /*--- Account for a character position switch type ---*/

    if (_pos_switch_type == Drv7SegActiveLow) {
        pos_byte ^= static_cast<uint8_t>(DRV7SEG2X595_ALL_BITS_SET_MASK);
    }


    /*--- Account for a byte order ---*/

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
            /* In theory, shifting a single zeroed byte, whether it is seg_byte or pos_byte,
             * is enough to produce a blank output, because both seg_byte and pos_byte, being
             * zeroed, guarantee that either all segments will be turned off individually or
             * the whole character position will be turned off.
             *
             * However, in practice it can lead to artifacts due to imperfectness of
             * shift register ICs and switching devices. Therefore two bytes are shifted.
             *
             * The same is applicable to the SPI variant.
             */
            shiftOut(_data_pin, _clock_pin, MSBFIRST, DRV7SEG2X595_ALL_BITS_CLEARED_MASK);
            shiftOut(_data_pin, _clock_pin, MSBFIRST, DRV7SEG2X595_ALL_BITS_CLEARED_MASK);
            digitalWrite(_latch_pin, HIGH);

            digitalWrite(_latch_pin, LOW);
            shiftOut(_data_pin, _clock_pin, MSBFIRST, upper_byte);
            shiftOut(_data_pin, _clock_pin, MSBFIRST, lower_byte);
            digitalWrite(_latch_pin, HIGH);
            break;

        #ifdef DRV7SEG2X595_SPI_PROVIDED
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

    return DRV7SEG2X595_OUTPUT_NEXT;
}


/*--- Private methods ---*/

int32_t Drv7Seg2x595Class::begin_helper(int32_t variant,
                                        ByteOrder byte_order,
                                        PosSwitchType pos_switch_type,
                                        uint32_t latch_pin,
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
    if (pos_switch_type != Drv7SegActiveLow && pos_switch_type != Drv7SegActiveHigh) {
        return DRV7SEG2X595_STATUS_ERR_INVALID_POS_SWITCH_TYPE;
    }

    // Position bits validity check and getting the active positions count.
    PosBit pos_bit_arr[DRV7SEG2X595_POS_MAX] = {pos_1_bit, pos_2_bit, pos_3_bit, pos_4_bit};
    for (uint32_t i = 0; i < DRV7SEG2X595_POS_MAX; ++i) {

        // First position bit (array index zero) must belong to the 0..7 range (from LSB to MSB).
        if (i == 0) {
            if (pos_bit_arr[i] < Drv7SegPosBit0 || pos_bit_arr[i] > Drv7SegPosBit7) {
                return DRV7SEG2X595_STATUS_ERR_INVALID_POS_BIT;
            } else {
                ++_active_positions;  // Active positions count is used within the anti-ghosting logic.
            }
        }

        // Other position bits must belong to the -1..7 range (from the initial value to MSB).
        if (i > 0) {
            if (pos_bit_arr[i] < Drv7SegPosBitInitial || pos_bit_arr[i] > Drv7SegPosBit7) {
                return DRV7SEG2X595_STATUS_ERR_INVALID_POS_BIT;
            } else if (pos_bit_arr[i] > Drv7SegPosBitInitial) {
                ++_active_positions;  // Active positions count is used within the anti-ghosting logic.
            }
        }

        // Position bits duplication check.
        for (uint32_t j = i + 1; j < DRV7SEG2X595_POS_MAX; ++j) {
            if pos_bit_arr[i] == pos_bit_arr[j] {
                return DRV7SEG2X595_STATUS_ERR_POS_BIT_DUPLICATION;
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
    if (anti_ghosting_retention_duration_us == 0) {
        return true;  /* If the passed retention duration is zero, the timer elapses
                       * without further calculations and the function returns early.
                       */
    }

    uint32_t current_micros = micros();

    if (_anti_ghosting_timer_new_lap == true) {
        _anti_ghosting_timer_previous_micros = current_micros;
        _anti_ghosting_timer_new_lap = false;
    }

    if (current_micros - _anti_ghosting_timer_previous_micros >= anti_ghosting_retention_duration_us) {
        _anti_ghosting_timer_new_lap = true;
        return true;   // The timer has elapsed.
    } else {
        return false;  // The timer hasn't elapsed yet.
    }
}

Drv7Seg2x595Class::Pos Drv7Seg2x595Class::anti_ghosting_next_pos_to_output()
{
    uint32_t retained_pos_as_int = static_cast<uint32_t>(_anti_ghosting_retained_pos);

    /* If the currently retained position is the last one in the list of active positions,
     * the next position to be turned on is the first one (the queue wraps around the edge).
     */
    if (retained_pos_as_int >= _active_positions) {
        return Drv7SegPos1;
    /* Otherwise the number of the next position to be turned on
     * equals (current retained position number) + 1.
     */
    } else {
        return static_cast<Pos>(retained_pos_as_int + 1);
    }
}
