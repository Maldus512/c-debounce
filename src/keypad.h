#ifndef __GENERIC_KEYBOARD_H__
#define __GENERIC_KEYBOARD_H__

#include <stdint.h>

#define KEYPAD_KEY(b, c)                                                                                               \
    { .bitvalue = (b), .code = (c) }
#define KEYPAD_NULL_KEY                                                                                                \
    { 0 }

typedef enum {
    KEYPAD_EVENT_TAG_NOTHING,          // Nothing has happened
    KEYPAD_EVENT_TAG_PRESS,            // The button was pressed initially. Not repeated
    KEYPAD_EVENT_TAG_PRESSING,         // the button is still pressed, repeated.
    KEYPAD_EVENT_TAG_CLICK,            // The button was pressed for "click" time. Not repeated
    KEYPAD_EVENT_TAG_LONGCLICK,        // The button was pressed for "longclick" time. Not repeated
    KEYPAD_EVENT_TAG_LONGPRESSING,     // The button was kept pressed after a longclick event
    KEYPAD_EVENT_TAG_RELEASE,          // The button was released after a click event (long or short)
} keypad_event_tag_t;


// Struct containing information about the key
typedef struct {
    unsigned long bitvalue;     // Bit map of the key
    int           code;         // Key code

    // Internal state, not to be handled outside
    struct {
        unsigned long      time_state;
        unsigned long      time_period;
        uint8_t            value;
        uint8_t            oldvalue;
        uint8_t            ignore;
        keypad_event_tag_t lastevent;
    } _state;

} keypad_key_t;


typedef struct {
    keypad_event_tag_t tag;
    int                code;
} keypad_event_t;

keypad_event_t keypad_routine(keypad_key_t *keys, unsigned long click, unsigned long longclick,
                              unsigned long press_period, unsigned long timestamp, unsigned long bitvalue);

unsigned char keypad_get_key_state(keypad_key_t *key);
void          keypad_reset_keys(keypad_key_t *keys);
unsigned long keypad_get_click_time(keypad_key_t *keys, int code, unsigned long timestamp);

#endif
