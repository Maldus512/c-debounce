#include "keypad.h"
#include "_timecheck.h"

#define NTH(x, i) ((x >> i) & 0x1)

unsigned char keypad_get_key_state(keypad_key_t *key) {
    return key->_state.value;
}

keypad_event_t keypad_routine(keypad_key_t *keys, unsigned long click, unsigned long longclick,
                              unsigned long press_period, unsigned long timestamp, unsigned long bitvalue) {
    int            i = 0, found = -1;
    keypad_key_t  *key;
    keypad_event_t event = {0};

    while (keys[i].bitvalue) {
        // If a key is found
        if (keys[i].bitvalue == bitvalue) {
            keys[i]._state.value = 1;
            found                = i;
            event.code           = keys[i].code;
            // break;
        }
        // If the key was pressed but it's not anymore
        else if (keys[i]._state.oldvalue && keys[i].bitvalue != bitvalue) {
            keys[i]._state.value = 0;
            found                = i;
            event.code           = keys[i].code;
            break;
        }
        // If the last event was a click and the key is not pressed anymore
        else if (keys[i]._state.lastevent != KEYPAD_EVENT_TAG_RELEASE &&
                 keys[i]._state.lastevent != KEYPAD_EVENT_TAG_NOTHING) {
            found      = i;
            event.code = keys[i].code;
            break;
        }
        i++;
    }

    if (found == -1)
        return event;

    key = &keys[found];

    if (key->_state.value == key->_state.oldvalue && !key->_state.ignore) {
        if (key->_state.lastevent == KEYPAD_EVENT_TAG_PRESS && key->_state.value == 1 &&
            is_strictly_expired(key->_state.time_state, timestamp, click)) {
            event.tag               = KEYPAD_EVENT_TAG_CLICK;
            key->_state.time_period = timestamp;
            key->_state.lastevent   = event.tag;
        } else if ((key->_state.lastevent != KEYPAD_EVENT_TAG_RELEASE ||
                    key->_state.lastevent == KEYPAD_EVENT_TAG_NOTHING) &&
                   key->_state.value == 0 && is_strictly_expired(key->_state.time_state, timestamp, click)) {
            event.tag             = KEYPAD_EVENT_TAG_RELEASE;
            key->_state.lastevent = event.tag;
        } else if ((key->_state.lastevent == KEYPAD_EVENT_TAG_CLICK ||
                    key->_state.lastevent == KEYPAD_EVENT_TAG_PRESS) &&
                   is_strictly_expired(key->_state.time_state, timestamp, longclick)) {
            event.tag               = KEYPAD_EVENT_TAG_LONGCLICK;
            key->_state.time_state  = timestamp;
            key->_state.time_period = timestamp;
            key->_state.lastevent   = event.tag;
        } else if (key->_state.lastevent == KEYPAD_EVENT_TAG_LONGCLICK && (key->_state.value == 1) &&
                   is_strictly_expired(key->_state.time_period, timestamp, press_period)) {
            event.tag               = KEYPAD_EVENT_TAG_LONGPRESSING;
            key->_state.time_period = timestamp;
        } else if (key->_state.lastevent == KEYPAD_EVENT_TAG_CLICK && (key->_state.value == 1) &&
                   is_strictly_expired(key->_state.time_period, timestamp, press_period)) {
            event.tag               = KEYPAD_EVENT_TAG_PRESSING;
            key->_state.time_period = timestamp;
        }
    } else if (key->_state.value != key->_state.oldvalue) {
        key->_state.ignore     = 0;
        key->_state.oldvalue   = key->_state.value;
        key->_state.time_state = timestamp;

        if (key->_state.value) {
            event.tag             = KEYPAD_EVENT_TAG_PRESS;
            key->_state.lastevent = event.tag;
        } else if (key->_state.lastevent == KEYPAD_EVENT_TAG_PRESS) {
            event.tag             = KEYPAD_EVENT_TAG_RELEASE;
            key->_state.lastevent = event.tag;
        }
    }

    return event;
}

void keypad_reset_keys(keypad_key_t *keys) {
    int i = 0;
    while (keys[i].bitvalue) {
        keys[i]._state.time_state  = 0;
        keys[i]._state.time_period = 0;
        keys[i]._state.lastevent   = KEYPAD_EVENT_TAG_NOTHING;
        keys[i]._state.oldvalue    = 0;
        keys[i]._state.value       = 0;
        keys[i]._state.ignore      = 1;

        i++;
    }
}
