/*************** FILE DESCRIPTION ***************/

/**
 * Filename: Drv7Seg2x595_demo.ino
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Purpose:  An example sketch demonstrating basic usage of the Drv7Seg2x595
 *           library.
 *
 *           Counts from 0 to 60 and outputs tens digit and ones digit on two
 *           character positions of a 7-segment display. Additionally, prints
 *           diagnostic information via UART.
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Notes:
 */


/************ PREPROCESSOR DIRECTIVES ***********/

/*--- Includes ---*/

#include <Drv7Seg2x595.h>
#include <SegMap595.h>     /* Helper library. Makes it easier to map the parallel outputs of
                            * your 74HC595 to the segment control pins of your 7-segment display.
                            */


/*--- Drv7Seg2x595 library API parameters ---*/

// Specify the data transfer approach. Use one variant, comment out or delete the others.
#define USE_BIT_BANGING
//#define USE_SPI_DEFAULT_PINS

/* Only suitable for the Arduino cores that support a respective SPI.h version
 * (as of the last library update those are ESP32 and STM32 cores).
 */
//#define USE_SPI_CUSTOM_PINS

/* Specify the order in which seg_byte and pos_byte are placed within
 * the shift register. Use one variant, comment out or delete the other.
 */
Drv7Seg2x595Class::ByteOrder byte_order = Drv7Seg2x595PosByteFirst;
//Drv7Seg2x595Class::ByteOrder byte_order = Drv7Seg2x595SegByteFirst;

/* Specify a signal level that turns on the character positions of your display.
 * Use one variant, comment out or delete the other.
 */
Drv7Seg2x595Class::PosSwitchType pos_switch_type = Drv7Seg2x595ActiveHigh;
//Drv7Seg2x595Class::PosSwitchType pos_switch_type = Drv7Seg2x595ActiveLow;

// Specify appropriately based on your wiring. Variant for bit-banging.
#ifdef USE_BIT_BANGING
    #define DATA_PIN  16
    #define LATCH_PIN 17
    #define CLOCK_PIN 18
#endif

// Specify appropriately based on your wiring. Variant for SPI with default pins.
#ifdef USE_SPI_DEFAULT_PINS
    #define LATCH_PIN 17
#endif

// Specify appropriately based on your wiring. Variant for SPI with custom-assigned pins.
#ifdef USE_SPI_CUSTOM_PINS
    #define MOSI_PIN  16  // TODO sensible default
    #define LATCH_PIN 17
    #define SCK_PIN   18  // TODO sensible default
#endif

/* Specify appropriately based on your wiring: which pos_byte bits
 * control the character positions of your 7-segment display?
 */
#define POS_1_BIT 7
#define POS_2_BIT 5


/*--- SegMap595 library API parameters ---*/

/* Map string.
 *
 * This string must reflect the actual (physical) order of connections made between
 * parallel outputs of your 74HC595 and segment control pins of your 7-segment display.
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

// Counting interval ("once in X milliseconds").
#define INTERVAL 500

#define MAX_COUNT 100


/******************* FUNCTIONS ******************/

void setup()
{
    Serial.begin(BAUD_RATE);


    /*--- Byte mapping ---*/

    SegMap595.init(MAP_STR, display_common_pin, glyph_set_id);

    /* Mapping status check.
     * You can also check value returned by init() instead of calling get_status().
     */
    int32_t mapping_status = SegMap595.get_status();

    // Loop error output if mapping was unsuccessful.
    if (mapping_status < 0) {  // If error is detected.
        while(true) {
            Serial.print("Error: mapping failed, error code ");
            Serial.println(mapping_status);
            delay(INTERVAL);
        }
    }


    /*--- Driver object configuration ---*/

    // TODO: same status check for Drv7Seg.

    #ifdef USE_BIT_BANGING
    Drv7Seg.begin_bb(byte_order,
                     pos_switch_type,
                     DATA_PIN, LATCH_PIN, CLOCK_PIN,
                     POS_1_BIT, POS_2_BIT
                    );
    #endif

    #ifdef USE_SPI_DEFAULT_PINS
    Drv7Seg.begin_bb(byte_order,
                     pos_switch_type,
                     LATCH_PIN,
                     POS_1_BIT, POS_2_BIT
                    );
    #endif

    #ifdef USE_SPI_CUSTOM_PINS
    Drv7Seg.begin_bb(byte_order,
                     pos_switch_type,
                     MOSI_PIN, LATCH_PIN, SCK_PIN,
                     POS_1_BIT, POS_2_BIT
                    );
    #endif
}

void loop()
{
    /*--- Counter and output update trigger ---*/

    uint64_t current_millis = millis();
    static uint64_t previous_millis = current_millis;

    static size_t counter = 0;
    static size_t counter_max = MAX_COUNT;
    if (counter >= counter_max) {
        counter = 0;
    }
    
    static bool update_due = true;


    /*--- Demo output ---*/

    static uint8_t byte_to_shift_tens = DRV7SEG2X595_BLANK_GLYPH;
    static uint8_t byte_to_shift_ones = DRV7SEG2X595_BLANK_GLYPH;

    if (update_due) {
        byte_to_shift_tens = SegMap595.get_mapped_byte(counter / 10);
        byte_to_shift_ones = SegMap595.get_mapped_byte(counter % 10);
        update_due = false;
    }

    Drv7Seg.output(byte_to_shift_tens, 1);
    Drv7Seg.output(byte_to_shift_ones, 2);


    /*--- Counter and output trigger, continued ---*/

    if (current_millis - previous_millis >= INTERVAL) {
        ++counter;
        update_due = true;
        previous_millis = current_millis;
    }
}
