/*************** FILE DESCRIPTION ***************/

/**
 * Filename: Drv7Seg2x595_demo.ino
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Purpose:  An example sketch demonstrating basic usage of the Drv7Seg2x595
 *           library.
 *
 *           Counts seconds from 0 to 60 and outputs tens digit and ones digit
 *           on two character positions of a 7-segment display. Additionally,
 *           prints diagnostic information via UART.
 * ----------------------------------------------------------------------------|---------------------------------------|
 * Notes:
 */


/************ PREPROCESSOR DIRECTIVES ***********/

/*--- Includes ---*/

#include <Drv7Seg2x595.h>
#include <SegMap595.h>


/*--- Drv7Seg2x595 library API parameters ---*/

/* Specify the order in which seg_byte and pos_byte go into the shift register.
 * Use one variant, comment out or delete the other.
 */
Drv7Seg2x595Class::ByteOrder byte_order = Drv7Seg2x595PosByteFirst;
//Drv7Seg2x595Class::ByteOrder byte_order = Drv7Seg2x595SegByteFirst;

Drv7Seg2x595Class::PosSwitchType pos_switch_type = Drv7Seg2x595PosByteFirst;
//Drv7Seg2x595Class::ByteOrder byte_order = Drv7Seg2x595SegByteFirst;


/*--- SegMap595 library API parameters ---*/

/* Specify the relevant string according to the actual
 * (physical) order of connections in your circuit.
 */
#define MAP_STR "ED@CGAFB"

// Specify your display type based on its common pin. Use one directive, comment out or delete the other.
#define DISPLAY_COMMON_PIN SEGMAP595_COMMON_CATHODE
//#define DISPLAY_COMMON_PIN SEGMAP595_COMMON_ANODE

// Select a glyph set. Use one directive, comment out or delete the other.
#define GLYPH_SET_NUM SEGMAP595_GLYPH_SET_1
//#define GLYPH_SET_NUM SEGMAP595_GLYPH_SET_2

#ifndef DISPLAY_COMMON_PIN
    #error "Error: display type (common pin) not specified."
#endif

#ifndef GLYPH_SET_NUM
    #error "Error: glyph set not specified."
#endif


/*--- Misc ---*/

// Set appropriately based on the baud rate you use.
#define BAUD_RATE 115200

// Set appropriately based on your wiring.
#define DATA_PIN  16
#define LATCH_PIN 17
#define CLOCK_PIN 18

// Output interval ("once in X milliseconds").
#define INTERVAL  1000


/******************* FUNCTIONS ******************/

void setup()
{
    Serial.begin(BAUD_RATE);

    // Pin setup.
    pinMode(DATA_PIN,  OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);

    // Byte mapping.
    SegMap595.init(MAP_STR, SegMap595CommonCathode, SegMap595GlyphSet1);

    // Mapping status check.
    int32_t mapping_status = SegMap595.get_status();
    // Loop error output if mapping was unsuccessful.
    if (mapping_status < 0) {
        while(true) {
            Serial.print("Error: mapping failed, error code ");
            Serial.println(mapping_status);
            delay(INTERVAL);
        }
    }

    Drv7Seg.begin_bb(Drv7Seg2x595PosByteFirst,
                     Drv7Seg2x595ActiveHigh,
                     DATA_PIN,
                     LATCH_PIN,
                     CLOCK_PIN,
                     7,
                     5,
                     3,
                     1
                    );

    //Drv7Seg.begin_spi
    //Drv7Seg.begin_spi_custom_pins
}

void loop()
{
    /*--- Counter and output trigger ---*/

    uint64_t current_millis = millis();
    static uint64_t previous_millis = current_millis;

    static size_t counter = 0;
    static size_t glyph_num = SegMap595.get_glyph_num();
    if (counter >= glyph_num) {
        counter = 0;
    }

    // Output trigger.
    static bool update_due = true;


    /*--- Demo output ---*/

    static uint8_t byte_to_shift = DRV7SEG2X595_BLANK_GLYPH;

    if (update_due) {
        byte_to_shift = SegMap595.get_mapped_byte(counter);
        Serial.println("DEBUG 1");
        Serial.println("DEBUG 2");
        Serial.println("DEBUG 3");
        Serial.println("DEBUG 4");
        Serial.println("DEBUG 5");
        Serial.println("DEBUG 6");
        Serial.println("DEBUG 7");
        Serial.println("DEBUG 8");
        Serial.println("DEBUG 9");
        Serial.println("DEBUG 10");

        // Dot segment blink.
        if (counter % 2) {
            static uint32_t dot_bit_pos = SegMap595.get_dot_bit_pos();
            static uint8_t mask = static_cast<uint8_t>(1u << dot_bit_pos);
            byte_to_shift ^= mask;
        }

        update_due = false;
    }

    // Output a glyph on the display.

    byte_to_shift = SegMap595.get_mapped_byte(counter);
    
    Drv7Seg.output(byte_to_shift, 1);

    byte_to_shift = SegMap595.get_mapped_byte(counter+1);
    
    Drv7Seg.output(byte_to_shift, 2);

    byte_to_shift = SegMap595.get_mapped_byte(counter+2);
    
    Drv7Seg.output(byte_to_shift, 3);

    byte_to_shift = SegMap595.get_mapped_byte(counter+3);
    
    Drv7Seg.output(byte_to_shift, 4);


    /*--- Counter and output trigger, continued ---*/

    if (current_millis - previous_millis >= INTERVAL) {
        ++counter;
        update_due = true;
        previous_millis = current_millis;
    }
}
