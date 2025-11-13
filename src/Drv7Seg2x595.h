/*************** FILE DESCRIPTION ***************/

/**
 * Filename: Drv7Seg2x595.h
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Purpose:  A class for driving a multiplexed 7-segment display using
 *           two daisy-chained 74HC595 shift register ICs.
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Notes:
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

        /* Number of a character position where last output glyph is currently retained to prevent ghosting. 
         * Zero means that nothing is retained right now. 
         */
        uint32_t _anti_ghosting_retained_pos = 0;
        

        /*--- Methods ---*/

        bool anti_ghosting_retention_timer(uint32_t anti_ghosting_pause);
        uint32_t anti_ghosting_next_pos_to_output(uint32_t retained_pos);
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

// An Arduino-style singleton object.
extern Drv7Seg2x595Class Drv7Seg;


#endif  // Include guards.
