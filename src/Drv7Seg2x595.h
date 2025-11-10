/*************** FILE DESCRIPTION ***************/

/**
 * Filename: Drv7Seg2x595.h
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Purpose:  A class for shifting 2-byte data into 2 daisy-chained 74HC595 ICs.
 *           Usually used to drive a multiplexed 4-digit 7-segment display.
 *           Intended for use with the ESP32 or ESP8266 Arduino core.
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

#define DRV7SEG2X595_MAX_POS              4

#define DRV7SEG2X595_ALL_BITS_SET_MASK    0xFF  // TODO: is it used?
#define DRV7SEG2X595_BLANK_GLYPH          0x00

#define DRV7SEG2X595_POS_BYTE_FIRST       0
#define DRV7SEG2X595_SEG_BYTE_FIRST       1

#define DRV7SEG2X595_COMMON_CATHODE       0
#define DRV7SEG2X595_COMMON_ANODE         1

// Driver setup status codes. Double as return codes for some functions.
#define DRV7SEG2X595_STATUS_INITIAL      -1
#define DRV7SEG2X595_STATUS_ERR_SIG_PINS -2
#define DRV7SEG2X595_STATUS_ERR_POS_BITS -3
#define DRV7SEG2X595_STATUS_OK            0

// Driver setup variant codes.
#define DRV7SEG2X595_VARIANT_INITIAL     -1
#define DRV7SEG2X595_VARIANT_BIT_BANGING  0
#ifndef DRV7SEG2X595_SPI_NOT_IMPLEMENTED
    #define DRV7SEG2X595_VARIANT_SPI      1
#endif

// Function return codes.
#define DRV7SEG2X595_ERR_MAX_POS             -4
#define DRV7SEG2X595_ERR_VARIANT_NOT_SET     -5
#define DRV7SEG2X595_ANTI_GHOSTING_RETENTION  1
#define DRV7SEG2X595_OK                       0

#define DRV7SEG2X595_DEFAULT_ANTI_GHOSTING_PAUSE 2

// Uncomment if the Arduino core you're using doesn't provide SPI.h library.
//#define DRV7SEG2X595_SPI_NOT_IMPLEMENTED


/****************** DATA TYPES ******************/

class Drv7Seg2x595Class {
    public:
        /*--- Methods ---*/

        // Default constructor.
        Drv7Seg2x595Class();

        // TODO: comment about reinit

        // Set a driver object to use bit-banging.
        int32_t init_bb(int32_t  byte_order, int32_t display_common_pin, int32_t switch_polarity,
                        int32_t data_pin, int32_t latch_pin, int32_t clock_pin,
                        int32_t pos_bit_1, int32_t pos_bit_2 = -1, int32_t pos_bit_3 = -1, int32_t pos_bit_4 = -1
                       );

        // Set a driver object to use default SPI pins.
        #ifndef DRV7SEG2X595_SPI_NOT_IMPLEMENTED
        int32_t init_spi(int32_t  byte_order, int32_t display_common_pin, int32_t switch_polarity,
                         int32_t latch_pin,
                         int32_t pos_bit_1, int32_t pos_bit_2 = -1, int32_t pos_bit_3 = -1, int32_t pos_bit_4 = -1
                        );
        #endif

        /* Set a driver object to use custom-assigned SPI pins.
         *
         * Conditional compilation is used because not all Arduino cores
         * expose the SPI.begin() version that takes custom pins.
         */
        #if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_STM32)
        int32_t init_spi_custom(int32_t byte_order, int32_t display_common_pin, int32_t switch_polarity,
                                int32_t mosi_pin, int32_t latch_pin, int32_t sck_pin,
                                int32_t pos_bit_1, int32_t pos_bit_2 = -1, int32_t pos_bit_3 = -1, int32_t pos_bit_4 = -1
                               );
        #endif

        /* Shift data into two daisy-chained 595s to output a glyph on a display.
         *
         * Returns: zero if a driver setup was successful, negative integer otherwise
         * (see the preprocessor macros list for possible values).
         */
        int32_t output(uint8_t seg_byte, uint32_t pos,
                       uint32_t ghosting_prevention_delay = DRV7SEG2X595_DEFAULT_ANTI_GHOSTING_PAUSE);

    private:
        /*--- Variables ---*/

        int32_t _status = DRV7SEG2X595_STATUS_INITIAL;
        int32_t _variant = DRV7SEG2X595_STATUS_INITIAL;

        int32_t _byte_order;
        int32_t _display_common_pin;
        int32_t _switch_polarity;

        int32_t _latch_pin = -1;
        
        int32_t _data_pin  = -1;
        int32_t _clock_pin = -1;

        #ifndef DRV7SEG2X595_SPI_NOT_IMPLEMENTED
        int32_t _mosi_pin  = -1;
        int32_t _sck_pin   = -1;
        #endif

        // Default values after first one are specified in init() declaration to provide for omittability.
        int32_t _pos_bit_1 = -1;
        int32_t _pos_bit_2;
        int32_t _pos_bit_3;
        int32_t _pos_bit_4;
        
        uint8_t _pos_byte = 0x00;

        /* A duration (in milliseconds) of a tiny pause that prevents the so-called
         * "ghosting" of characters being output to a multiplexed 7-segment display.
         *
         * Usually 2 milliseconds is enough.
         *
         * TODO: move
         */
        //uint32_t _ghosting_prevention_delay;

        bool _anti_ghosting_retention  = false;


        /*--- Methods ---*/

        bool anti_ghosting_timer_elapsed(uint32_t anti_ghosting_pause);
};


/*************** GLOBAL VARIABLES ***************/

/* An Arduino-style singleton object.
 * More instances of the same class can be created if necessary.
 */
extern Drv7Seg2x595Class Drv7Seg;


#endif  // Include guards.
