/*************** FILE DESCRIPTION ***************/

/**
 * Filename: Drv7Seg2x595.h
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Purpose:  A class for driving a multiplexed 7-segment display using
 *           two daisy-chained 74HC595 shift register ICs.
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Notes:    Intended for displays with 1 to 4 character positions (digits).
 */


/************ PREPROCESSOR DIRECTIVES ***********/

// Include guards.
#ifndef DRV7SEG2X595_H
#define DRV7SEG2X595_H


/*--- Includes ---*/

// Arduino core header file.
#include <Arduino.h>


/*--- Misc ---*/

#define DRV7SEG2X595_MSB 7
#define DRV7SEG2X595_LSB 0

#define DRV7SEG2X595_POS_MIN 1
#define DRV7SEG2X595_POS_MAX 4

#define DRV7SEG2X595_ALL_BITS_CLEARED_MASK 0x00
#define DRV7SEG2X595_ALL_BITS_SET_MASK     0xFF

/* Duration (in microseconds) of a tiny period during which a currently output glyph
 * is retained on a respective character position. Intended for a glyph ghosting prevention.
 */
#define DRV7SEG2X595_ANTI_GHOSTING_DEFAULT_RETENTION_DURATION_US 2000

// Driver configuration status codes. Double as return codes for some methods.
#define DRV7SEG2X595_STATUS_INITIAL                     -1
#define DRV7SEG2X595_STATUS_ERR_VARIANT_NOT_SPECIFIED   -2
#define DRV7SEG2X595_STATUS_ERR_INVALID_BYTE_ORDER      -3
#define DRV7SEG2X595_STATUS_ERR_INVALID_POS_SWITCH_TYPE -4
#define DRV7SEG2X595_STATUS_ERR_INVALID_POS_BIT         -5
#define DRV7SEG2X595_STATUS_OK                           0

// output() method additional return codes.
#define DRV7SEG2X595_OUTPUT_ERR_POS_BIT_NOT_SPECIFIED_FOR_POS -6
#define DRV7SEG2X595_OUTPUT_ERR_INVALID_POS                   -7
#define DRV7SEG2X595_OUTPUT_ANTI_GHOSTING_RETENTION_RUNNING    0

// Comment out if the Arduino core you're using doesn't provide SPI.h library.
#define DRV7SEG2X595_SPI_IMPLEMENTED

// Driver configuration variant codes.
#define DRV7SEG2X595_VARIANT_INITIAL     -1
#define DRV7SEG2X595_VARIANT_BIT_BANGING  0
#ifdef DRV7SEG2X595_SPI_IMPLEMENTED
    #define DRV7SEG2X595_VARIANT_SPI      1
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
            ActiveHigh = 0,
            ActiveLow  = 1
        };

        enum class PosBit {
            PosBitInitial = -1,
            PosBit0       =  DRV7SEG2X595_LSB,  // 0
            PosBit1       =  1,
            PosBit2       =  2,
            PosBit3       =  3,
            PosBit4       =  4,
            PosBit5       =  5,
            PosBit6       =  6,
            PosBit7       =  DRV7SEG2X595_MSB   // 7
        };

        enum class Pos {
            Pos1 = DRV7SEG2X595_POS_MIN,  // 1
            Pos2 = 2,
            Pos3 = 3,
            Pos4 = DRV7SEG2X595_POS_MAX   // 4
        };


        /*--- Methods ---*/

        // Default constructor.
        Drv7Seg2x595Class();

        // TODO: comments on parameters.

        /* Configure the driver to use bit-banging.
         *
         * Returns: zero if configuration was successful, negative integer otherwise
         * (see the preprocessor macros list for possible values).
         *
         * Multiple calls to this method are valid, each call will lead to a fresh configuration.
         */
        int32_t begin_bb(ByteOrder byte_order,
                         PosSwitchType pos_switch_type,
                         uint32_t data_pin,
                         uint32_t latch_pin,
                         uint32_t clock_pin,
                         PosBit pos_1_bit,
                         PosBit pos_2_bit = PosBit::PosBitInitial,
                         PosBit pos_3_bit = PosBit::PosBitInitial,
                         PosBit pos_4_bit = PosBit::PosBitInitial
                        );

        /* Configure the driver to use SPI with default pins.
         *
         * Returns: zero if configuration was successful, negative integer otherwise
         * (see the preprocessor macros list for possible values).
         *
         * Safety of multiple calls to this method depends on hardware and SPI.h implementation.
         */
        #ifdef DRV7SEG2X595_SPI_IMPLEMENTED
        int32_t begin_spi(ByteOrder byte_order,
                          PosSwitchType pos_switch_type,
                          uint32_t latch_pin,
                          PosBit pos_1_bit,
                          PosBit pos_2_bit = PosBit::PosBitInitial,
                          PosBit pos_3_bit = PosBit::PosBitInitial,
                          PosBit pos_4_bit = PosBit::PosBitInitial
                         );
        #endif

        /* Configure the driver to use SPI with custom-assigned pins.
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
                                      uint32_t mosi_pin,
                                      uint32_t latch_pin,
                                      uint32_t sck_pin,
                                      PosBit pos_1_bit,
                                      PosBit pos_2_bit = PosBit::PosBitInitial,
                                      PosBit pos_3_bit = PosBit::PosBitInitial,
                                      PosBit pos_4_bit = PosBit::PosBitInitial
                                     );
        #endif

        /* Shift two bytes into two daisy-chained 595s and then transfer the data into the output register.
         *
         * Returns: zero if the driver configuration was successful and the specified character position is valid,
         * negative integer otherwise (see the preprocessor macros list for possible values).
         */
        int32_t output(uint8_t seg_byte,
                       Pos pos,
                       uint32_t anti_ghosting_retention_duration_us =
                           DRV7SEG2X595_ANTI_GHOSTING_DEFAULT_RETENTION_DURATION_US
                      );

    private:
        /*--- Variables ---*/

        int32_t _status  = DRV7SEG2X595_STATUS_INITIAL;
        int32_t _variant = DRV7SEG2X595_VARIANT_INITIAL;

        ByteOrder     _byte_order;
        PosSwitchType _pos_switch_type;

        int32_t _latch_pin = -1;
        
        int32_t _data_pin  = -1;
        int32_t _clock_pin = -1;

        #ifdef DRV7SEG2X595_SPI_IMPLEMENTED
        int32_t _mosi_pin  = -1;
        int32_t _sck_pin   = -1;
        #endif

        PosBit _pos_1_bit;
        PosBit _pos_2_bit;
        PosBit _pos_3_bit;
        PosBit _pos_4_bit;
        uint32_t _active_positions = 0;

        bool     _anti_ghosting_first_output_call = true;
        Pos      _anti_ghosting_retained_pos;
        bool     _anti_ghosting_timer_new_lap = true;
        uint32_t _anti_ghosting_timer_previous_micros;


        /*--- Methods ---*/

        // Helper method that assigns the values of the parameters common to all configuration variants.
        int32_t begin_helper(int32_t config_variant,
                             ByteOrder byte_order,
                             PosSwitchType pos_switch_type,

                             // Latch pin is used in all variants.
                             uint32_t latch_pin,

                             /* These parameters always have values, even if they were omitted
                              * in the begin_*() method call (in this case default values are assigned).
                              */
                             PosBit pos_1_bit,
                             PosBit pos_2_bit,
                             PosBit pos_3_bit,
                             PosBit pos_4_bit
                            );

        bool anti_ghosting_timer(uint32_t anti_ghosting_retention_duration_us);
        Pos  anti_ghosting_next_pos_to_output();
};

// Class-related aliases.
constexpr Drv7Seg2x595Class::ByteOrder Drv7SegPosByteFirst =
          Drv7Seg2x595Class::ByteOrder::PosByteFirst;
constexpr Drv7Seg2x595Class::ByteOrder Drv7SegSegByteFirst =
          Drv7Seg2x595Class::ByteOrder::SegByteFirst;

constexpr Drv7Seg2x595Class::PosSwitchType Drv7SegActiveHigh =
          Drv7Seg2x595Class::PosSwitchType::ActiveHigh;
constexpr Drv7Seg2x595Class::PosSwitchType Drv7SegActiveLow =
          Drv7Seg2x595Class::PosSwitchType::ActiveLow;

constexpr Drv7Seg2x595Class::PosBit Drv7SegPosBitInitial =
          Drv7Seg2x595Class::PosBit::PosBitInitial;
constexpr Drv7Seg2x595Class::PosBit Drv7SegPosBit0 =
          Drv7Seg2x595Class::PosBit::PosBit0;
constexpr Drv7Seg2x595Class::PosBit Drv7SegPosBit1 =
          Drv7Seg2x595Class::PosBit::PosBit1;
constexpr Drv7Seg2x595Class::PosBit Drv7SegPosBit2 =
          Drv7Seg2x595Class::PosBit::PosBit2;
constexpr Drv7Seg2x595Class::PosBit Drv7SegPosBit3 =
          Drv7Seg2x595Class::PosBit::PosBit3;
constexpr Drv7Seg2x595Class::PosBit Drv7SegPosBit4 =
          Drv7Seg2x595Class::PosBit::PosBit4;
constexpr Drv7Seg2x595Class::PosBit Drv7SegPosBit5 =
          Drv7Seg2x595Class::PosBit::PosBit5;
constexpr Drv7Seg2x595Class::PosBit Drv7SegPosBit6 =
          Drv7Seg2x595Class::PosBit::PosBit6;
constexpr Drv7Seg2x595Class::PosBit Drv7SegPosBit7 =
          Drv7Seg2x595Class::PosBit::PosBit7;

constexpr Drv7Seg2x595Class::Pos Drv7SegPos1 =
          Drv7Seg2x595Class::Pos::Pos1;
constexpr Drv7Seg2x595Class::Pos Drv7SegPos2 =
          Drv7Seg2x595Class::Pos::Pos2;
constexpr Drv7Seg2x595Class::Pos Drv7SegPos3 =
          Drv7Seg2x595Class::Pos::Pos3;
constexpr Drv7Seg2x595Class::Pos Drv7SegPos4 =
          Drv7Seg2x595Class::Pos::Pos4;


/*************** GLOBAL VARIABLES ***************/

// An Arduino-style singleton object.
extern Drv7Seg2x595Class Drv7Seg;


#endif  // Include guards.
