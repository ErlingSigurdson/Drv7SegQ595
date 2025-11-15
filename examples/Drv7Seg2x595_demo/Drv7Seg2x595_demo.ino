/*************** FILE DESCRIPTION ***************/

/**
 * Filename: Drv7Seg2x595_demo.ino
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Purpose:  An example sketch demonstrating basic usage of the Drv7Seg2x595
 *           library.
 *
 *           Works as a simple electronic clock that counts seconds and minutes
 *           and outputs current values on a 7-segment 4-digit display.
 *           Additionally, prints diagnostic information via UART.
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Notes:    Refer to the README for a general library overview.
 *           Refer to Drv7Seg2x595.h for a detailed API description.
 */


/************ PREPROCESSOR DIRECTIVES ***********/

/*--- Includes ---*/

#include <Drv7Seg2x595.h>

/* Helper library.
 * Provides the user-friendly API for mapping the parallel outputs of the 74HC595 shift register IC
 * (the one that corresponds to seg_byte) to the segment control pins of your 7-segment display.
 */
#include <SegMap595.h>


/*--- Drv7Seg2x595 library API parameters ---*/

/* Specify the data transfer approach. Use one variant, comment out or delete the others.
 *
 * SPI is available for the Arduino cores that provide SPI.h
 * (almost every core does, but it's not guaranteed).
 *
 * SPI with custom pins is only available for the Arduino cores
 * that provide a suitable SPI.h version (as of the last library
 * update those are ESP32 and STM32 cores).
 */
#define USE_BIT_BANGING
//#define USE_SPI_DEFAULT_PINS
//#define USE_SPI_CUSTOM_PINS

/* Specify the order in which seg_byte and pos_byte are placed within
 * the shift register. Use one variant, comment out or delete the other.
 */
Drv7Seg2x595Class::ByteOrder byte_order = Drv7SegPosByteFirst;
//Drv7Seg2x595Class::ByteOrder byte_order = Drv7SegSegByteFirst;

/* Specify the signal level that turns on the character positions of your display.
 * Use one variant, comment out or delete the other.
 */
Drv7Seg2x595Class::PosSwitchType pos_switch_type = Drv7SegActiveHigh;
//Drv7Seg2x595Class::PosSwitchType pos_switch_type = Drv7SegActiveLow;

// Specify appropriately based on your wiring. Variant for bit-banging.
#ifdef USE_BIT_BANGING
    #define DATA_PIN  16
    #define LATCH_PIN 17
    #define CLOCK_PIN 18
#endif

// Specify appropriately based on your wiring. Variant for SPI with default pins.
#ifdef USE_SPI_DEFAULT_PINS
    #ifdef LATCH_PIN
        #undef LATCH_PIN
        #define LATCH_PIN 17
    #endif
#endif

// Specify appropriately based on your wiring. Variant for SPI with custom-assigned pins.
#ifdef USE_SPI_CUSTOM_PINS
    #define MOSI_PIN      16  // TODO sensible default
    #ifdef LATCH_PIN
        #undef LATCH_PIN
        #define LATCH_PIN 17
    #endif
    #define SCK_PIN       18  // TODO sensible default
#endif

/* Specify appropriately based on which pos_byte bits
 * control the character positions of your 7-segment display.
 *
 * Valid value syntax is as follows: Drv7SegPosBitX, where X must be from 0 to 7.
 *
 * POS_1_BIT means the leftmost character position (often referred to as 'D1' in 7-segment display pinout diagrams).
 * POS_4_BIT means the rightmost character position (often referred to as 'D4' in 7-segment display pinout diagrams).
 */
#define POS_1_BIT Drv7SegPosBit7
#define POS_2_BIT Drv7SegPosBit5
#define POS_3_BIT Drv7SegPosBit3
#define POS_4_BIT Drv7SegPosBit1


/*--- SegMap595 library API parameters ---*/

/* Map string.
 *
 * This string must reflect the actual (physical) order of connections made between
 * the parallel outputs of your 74HC595 and the segment control pins of your 7-segment display.
 *
 * The map string must consist of exactly 8 ASCII characters: @, A, B, C, D, E, F and G.
 * Every character corresponds to a single segment (@ stands for a dot).
 *
 * The first (leftmost) character in the map string corresponds to the 7th (most significant)
 * bit of the IC's parallel outputs (Q7 output), the last (rightmost) character corresponds to
 * the 0th (least significant) bit (Q0 output).
 *
 * Uppercase characters may be replaced with their lowercase counterparts. Any other characters
 * are invalid. Duplicating characters is invalid as well.
 */
#define MAP_STR "ED@CGAFB"

// Specify your display type based on its common pin. Use one variant, comment out or delete the other.
SegMap595Class::DisplayType display_common_pin = SegMap595CommonCathode;
//SegMap595Class::DisplayType display_common_pin = SegMap595CommonAnode;

// Select a glyph set. Use one variant, comment out or delete the other.
SegMap595Class::GlyphSetId glyph_set_id = SegMap595GlyphSet1;
//SegMap595Class::GlyphSetId glyph_set_id = SegMap595GlyphSet2;


/*--- Misc ---*/

// Set appropriately based on the baud rate you use.
#define BAUD_RATE 115200

// Output interval ("once every X milliseconds").
#define INTERVAL 1000

#define MAX_COUNT_MINUTES 60
#define MAX_COUNT_SECONDS 60

#define SERIAL_OUTPUT_VALUES


/******************* FUNCTIONS ******************/

void setup()
{
    Serial.begin(BAUD_RATE);


    /*--- Byte mapping ---*/

    SegMap595.init(MAP_STR, display_common_pin, glyph_set_id);

    /* Mapping status check.
     * You can also check the value returned by init() instead of calling get_status().
     */
    int32_t mapping_status = SegMap595.get_status();

    // Loop the error output if the mapping was unsuccessful.
    if (mapping_status < 0) {  // If an error is detected.
        while(true) {
            Serial.print("Error: mapping failed, error code ");
            Serial.println(mapping_status);
            delay(INTERVAL);
        }
    }


    /*--- Driver object configuration ---*/

    int32_t drv_config_status = -1;  // Initial value equals an idiomatic error indicator.

    #ifdef USE_BIT_BANGING
    drv_config_status = Drv7Seg.begin_bb(byte_order, pos_switch_type,
                                         DATA_PIN, LATCH_PIN, CLOCK_PIN,
                                         POS_1_BIT, POS_2_BIT, POS_3_BIT, POS_4_BIT
                                        );
    #endif

    #ifdef USE_SPI_DEFAULT_PINS
    drv_config_status = Drv7Seg.begin_spi(byte_order, pos_switch_type,
                                          LATCH_PIN,
                                          POS_1_BIT, POS_2_BIT, POS_3_BIT, POS_4_BIT
                                         );
    #endif

    #ifdef USE_SPI_CUSTOM_PINS
    drv_config_status = Drv7Seg.begin_spi_custom_pins(byte_order, pos_switch_type,
                                                      MOSI_PIN, LATCH_PIN, SCK_PIN,
                                                      POS_1_BIT, POS_2_BIT, POS_3_BIT, POS_4_BIT
                                                     );
    #endif

    // Loop the error output if the driver configuration was unsuccessful.
    if (drv_config_status < 0) {  // If an error is detected.
        while(true) {
            Serial.print("Error: driver configuration failed, error code ");
            Serial.println(drv_config_status);
            delay(INTERVAL);
        }
    }
}

void loop()
{
    /*--- Counter and value update trigger ---*/

    uint64_t current_millis = millis();
    static uint64_t previous_millis = current_millis;

    static size_t counter_seconds = 0;
    static size_t counter_minutes = 0;
    
    if (counter_seconds >= MAX_COUNT_SECONDS) {
        ++counter_minutes;
        counter_seconds = 0;
    }

    if (counter_minutes >= MAX_COUNT_MINUTES) {
        counter_minutes = 0;
    }

    static uint8_t seg_byte_minutes_tens;
    static uint8_t seg_byte_minutes_ones;
    static uint8_t seg_byte_seconds_tens;
    static uint8_t seg_byte_seconds_ones;

    // Value update trigger.
    static bool update_due = true;


    /*--- Demo output ---*/

    if (update_due) {
        seg_byte_minutes_tens = SegMap595.get_mapped_byte(counter_minutes / 10);
        seg_byte_minutes_ones = SegMap595.get_mapped_byte(counter_minutes % 10);
        seg_byte_seconds_tens = SegMap595.get_mapped_byte(counter_seconds / 10);
        seg_byte_seconds_ones = SegMap595.get_mapped_byte(counter_seconds % 10);

        // Dot-segment blink.
        if (counter_seconds % 2) {
            /* Normally you should check the returned value for being negative (error status indicator),
             * since shifting for a negative count leads to undefined behavior. But in this sketch
             * it's safe to assume a positive value because the status has already been checked.
             */
            static int32_t dot_bit_pos = SegMap595.get_dot_bit_pos();
            static uint8_t mask = static_cast<uint8_t>(1u << dot_bit_pos);
            seg_byte_minutes_ones ^= mask;
        }

        #ifdef SERIAL_OUTPUT_VALUES
            Serial.print("Current values are: ");
            Serial.print(counter_minutes / 10);
            Serial.print(counter_minutes % 10);
            Serial.print(":");
            Serial.print(counter_seconds / 10);
            Serial.println(counter_seconds % 10);
        #endif

        update_due = false;
    }

    // Display output.
    Drv7Seg.output(seg_byte_minutes_tens, Drv7SegPos1);
    Drv7Seg.output(seg_byte_minutes_ones, Drv7SegPos2);
    Drv7Seg.output(seg_byte_seconds_tens, Drv7SegPos3);
    Drv7Seg.output(seg_byte_seconds_ones, Drv7SegPos4);


    /*--- Counter and value update trigger, continued ---*/

    if (current_millis - previous_millis >= INTERVAL) {
        ++counter_seconds;
        update_due = true;
        previous_millis = current_millis;
    }
}
