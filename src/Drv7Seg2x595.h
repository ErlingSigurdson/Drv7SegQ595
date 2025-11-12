/*************** FILE DESCRIPTION ***************/

/**
 * Filename: Drv7Seg2x595.h
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Purpose:
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Notes:    One byte is supposed to have just one bit set and all other bits
 *           cleared. The set bit has to be mapped to a display pin (usually
 *           connected to a respective IC pin via a transistor or a similar
 *           switch) that corresponds to a current active digit.
 *
 *           Another byte is supposed to hold a mapped character, that is,
 *           a combination of set and cleared bits that corresponds to display
 *           segments in a pattern that will provide for an output of a desired
 *           character. This byte may also hold a fully zeroed byte, which
 *           leads to a blank output.
 *
 *           Idiomatic use example:
 *           * * *
 *           // Once (pick just one variant):
 *           driver7seg.init_bb(DRV7SEG2X595_DATA_PIN, DRV7SEG2X595_LATCH_PIN, DRV7SEG2X595_CLOCK_PIN,
 *                              DRV7SEG2X595_GHOSTING_PREVENTION_DELAY);
 *
 *           driver7seg.init_spi(DRV7SEG2X595_MOSI_PIN, DRV7SEG2X595_LATCH_PIN, DRV7SEG2X595_SCK_PIN,
 *                               DRV7SEG2X595_GHOSTING_PREVENTION_DELAY);
 *
 *           driver7seg.init_spi(DRV7SEG2X595_LATCH_PIN, DRV7SEG2X595_GHOSTING_PREVENTION_DELAY);
 *
 *
 *           // In a loop:
 *           counter.update();
 *
 *           uint32_t minutes_tens = counter.minutes / 10;
 *           uint32_t minutes_ones = counter.minutes % 10;
 *           uint32_t seconds_tens = counter.seconds / 10;
 *           uint32_t seconds_ones = counter.seconds % 10;
 *
 *           uint8_t digit_1 = mapped_characters[minutes_tens];
 *           uint8_t digit_2 = mapped_characters[minutes_ones];
 *           uint8_t digit_3 = mapped_characters[seconds_tens];
 *           uint8_t digit_4 = mapped_characters[seconds_ones];
 *
 *           if (counter.seconds % 2) {
 *               uint8_t dot_bit_pos_mask = (1 << DRV7SEG2X595_DOT_BIT_POS);
 *               digit_2 |= dot_bit_pos_mask;
 *           }
 *
 *           driver7seg.shift_out((1 << DRV7SEG2X595_D1), digit_1);
 *           driver7seg.shift_out((1 << DRV7SEG2X595_D2), digit_2);
 *           driver7seg.shift_out((1 << DRV7SEG2X595_D3), digit_3);
 *           driver7seg.shift_out((1 << DRV7SEG2X595_D4), digit_4);
 *           * * *
 *
 *           TODO dependencies (SPI.h).
 *           TODO up to 4 digits (positions)
 *           TODO what if not first digit on a physical display?
 *           TODO API method for status check?
 *           TODO delay() --> millis()-based timer.
 */


/************ PREPROCESSOR DIRECTIVES ***********/

// Include guards.
#ifndef DRV7SEG2X595_H
#define DRV7SEG2X595_H


/*--- Includes ---*/

// Arduino core header file.
#include <Arduino.h>


/*--- Misc ---*/

#define DRV7SEG2X595_BLANK_GLYPH       0x00
#define DRV7SEG2X595_ALL_BITS_SET_MASK 0xFF

// Duration (in microseconds) of a tiny pause that prevents glyph ghosting.
#define DRV7SEG2X595_DEFAULT_ANTI_GHOSTING_PAUSE 3000

// Comment out if the Arduino core you're using doesn't provide SPI.h library.
#define DRV7SEG2X595_SPI_IMPLEMENTED

// Driver object configuration status codes. Double as return codes for begin_* functions.
#define DRV7SEG2X595_CONFIG_STATUS_INITIAL      -1
#define DRV7SEG2X595_CONFIG_STATUS_ERR_SIG_PINS -2
#define DRV7SEG2X595_CONFIG_STATUS_ERR_POS_BITS -3
#define DRV7SEG2X595_CONFIG_STATUS_OK            0

// output() function return codes.
#define DRV7SEG2X595_OUTPUT_ERR_NEGATIVE_POS_BIT -4
#define DRV7SEG2X595_OUTPUT_ERR_INVALID_POS      -5
#define DRV7SEG2X595_OUTPUT_OK                    0

// Driver object configuration variant codes.
#define DRV7SEG2X595_CONFIG_VARIANT_INITIAL     -1
#define DRV7SEG2X595_CONFIG_VARIANT_BIT_BANGING  0
#ifdef DRV7SEG2X595_SPI_IMPLEMENTED
    #define DRV7SEG2X595_CONFIG_VARIANT_SPI      1
#endif


/****************** DATA TYPES ******************/

class Drv7Seg2x595Class {
    public:
        /*--- Data types ---*/

        enum class ByteOrder {
            PosByteFirst = 0,
            SegByteFirst = 1
        };

        enum class PosSwitchType {
            ActiveHigh = 1,
            ActiveLow  = 0
        };


        /*--- Methods ---*/

        // Default constructor.
        Drv7Seg2x595Class();

        // TODO: comments on parameters.

        /* Configure a driver object to use bit-banging.
         *
         * Returns: zero if configuration was successful, negative integer otherwise
         * (see the preprocessor macros list for possible values).
         *
         * Multiple calls to this method are valid, each call will lead to a fresh configuration.
         */
        int32_t begin_bb(ByteOrder byte_order,
                         PosSwitchType pos_switch_type,
                         int32_t data_pin,
                         int32_t latch_pin,
                         int32_t clock_pin,
                         int32_t pos_bit_1,
                         int32_t pos_bit_2 = -1,
                         int32_t pos_bit_3 = -1,
                         int32_t pos_bit_4 = -1
                        );

        /* Configure a driver object to use SPI with default pins.
         *
         * Returns: zero if configuration was successful, negative integer otherwise
         * (see the preprocessor macros list for possible values).
         *
         * Safety of multiple calls to this method depends on hardware and SPI.h implementation.
         */
        #ifdef DRV7SEG2X595_SPI_IMPLEMENTED
        int32_t begin_spi(ByteOrder byte_order,
                          PosSwitchType pos_switch_type,
                          int32_t latch_pin,
                          int32_t pos_bit_1,
                          int32_t pos_bit_2 = -1,
                          int32_t pos_bit_3 = -1,
                          int32_t pos_bit_4 = -1
                         );
        #endif

        /* Configure a driver object to use SPI with custom-assigned pins.
         *
         * Returns: zero if configuration was successful, negative integer otherwise
         * (see the preprocessor macros list for possible values).
         *
         * Conditional compilation is used because only certain Arduino cores (e.g., ESP32 and STM32)
         * expose the begin() method version that takes custom pins.
         *
         * Safety of multiple calls to this method depends on hardware and SPI.h implementation.
         */
        #if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_STM32)
        int32_t begin_spi_custom_pins(ByteOrder byte_order, 
                                      PosSwitchType pos_switch_type,
                                      int32_t mosi_pin,
                                      int32_t latch_pin,
                                      int32_t sck_pin,
                                      int32_t pos_bit_1,
                                      int32_t pos_bit_2 = -1,
                                      int32_t pos_bit_3 = -1,
                                      int32_t pos_bit_4 = -1
                                     );
        #endif

        /* Shift two bytes into two daisy-chained 595s and then transfer the data into the output register.
         *
         * Returns: zero if driver object configuration was successful and the specified character position is valid,
         * negative integer otherwise (see the preprocessor macros list for possible values).
         */
        int32_t output(uint8_t  seg_byte,
                       uint32_t pos,
                       uint32_t anti_ghosting_pause = DRV7SEG2X595_DEFAULT_ANTI_GHOSTING_PAUSE
                      );

    private:
        /*--- Variables ---*/

        int32_t _status  = DRV7SEG2X595_CONFIG_STATUS_INITIAL;
        int32_t _variant = DRV7SEG2X595_CONFIG_VARIANT_INITIAL;

        ByteOrder     _byte_order;
        PosSwitchType _pos_switch_type;
        
        int32_t _data_pin  = -1;
        int32_t _latch_pin = -1;
        int32_t _clock_pin = -1;

        #ifdef DRV7SEG2X595_SPI_IMPLEMENTED
        int32_t _mosi_pin  = -1;
        int32_t _sck_pin   = -1;
        #endif

        int32_t _pos_bit_1;
        int32_t _pos_bit_2;
        int32_t _pos_bit_3;
        int32_t _pos_bit_4;
        uint8_t _pos_byte = DRV7SEG2X595_BLANK_GLYPH;

        /* Number of a character position where last output glyph is currently retained to prevent ghosting. 
         * Zero means that nothing is retained right now. 
         */
        uint32_t _anti_ghosting_retention = 0;
        

        /*--- Methods ---*/

        bool anti_ghosting_pause_timer(uint32_t anti_ghosting_pause);
};

// Class-related aliases.
constexpr Drv7Seg2x595Class::ByteOrder Drv7Seg2x595PosByteFirst =
          Drv7Seg2x595Class::ByteOrder::PosByteFirst;

constexpr Drv7Seg2x595Class::ByteOrder Drv7Seg2x595SegByteFirst =
          Drv7Seg2x595Class::ByteOrder::SegByteFirst;

constexpr Drv7Seg2x595Class::PosSwitchType Drv7Seg2x595ActiveHigh =
          Drv7Seg2x595Class::PosSwitchType::ActiveHigh;

constexpr Drv7Seg2x595Class::PosSwitchType Drv7Seg2x595ActiveLow =
          Drv7Seg2x595Class::PosSwitchType::ActiveLow;


/*************** GLOBAL VARIABLES ***************/

/* An Arduino-style singleton object.
 * More instances of the same class can be created if necessary.
 */
extern Drv7Seg2x595Class Drv7Seg;


#endif  // Include guards.
