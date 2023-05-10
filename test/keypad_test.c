#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "keypad.h"


#define ONEFIVEBMP    0x11
#define ONETWOFIVEBMP 0x13


enum {
    P_ONE = 0,
    P_TWO,
    P_THREE,
    P_FOUR,
    P_FIVE,
    P_ONEFIVE,
    P_ONETWOFIVE,
    P_NULL,
    P_NUM,
};


static keypad_key_t keys[P_NUM] = {
    {.bitvalue = ONETWOFIVEBMP, .code = P_ONETWOFIVE},
    {.bitvalue = ONEFIVEBMP, .code = P_ONEFIVE},
    {.bitvalue = 0x01, .code = P_ONE},
    {.bitvalue = 0x02, .code = P_TWO},
    {.bitvalue = 0x04, .code = P_THREE},
    {.bitvalue = 0x08, .code = P_FOUR},
    {.bitvalue = 0x10, .code = P_FIVE},
    KEYPAD_NULL_KEY,
};


static int setup(void **state) {
    (void)state;

    keypad_reset_keys(keys);

    return 0;
}


static int teardown(void **state) {
    (void)state;
    return 0;
}


void click_release(unsigned long bitmap, int code, int period) {
    keypad_update_t event;
    unsigned long   longclick = period * 10;

    event = keypad_routine(keys, period, longclick, 1, 0, bitmap);
    assert_int_equal(KEY_PRESS, event.event);

    event = keypad_routine(keys, period, longclick, 1, period, bitmap);
    assert_int_equal(KEY_NOTHING, event.event);

    event = keypad_routine(keys, period, longclick, 1, period + 1, bitmap);
    assert_int_equal(KEY_CLICK, event.event);
    assert_int_equal(code, event.code);

    event = keypad_routine(keys, period, longclick, 1, period + 2, 0);
    assert_int_equal(KEY_NOTHING, event.event);

    event = keypad_routine(keys, period, longclick, 1, period * 2 + 2, 0);
    assert_int_equal(KEY_NOTHING, event.event);

    event = keypad_routine(keys, period, longclick, 1, period * 2 + 3, 0);
    assert_int_equal(KEY_RELEASE, event.event);
    assert_int_equal(code, event.code);
}

void test_click_release(void **state) {
    keypad_reset_keys(keys);

    (void)state;
    int i, j;

    for (j = 40; j <= 4000; j += 40) {
        for (i = P_ONE; i < P_NUM - 1; i++) {
            click_release(keys[i].bitvalue, keys[i].code, j);
        }
    }
}


void test_event_chain(void **state) {
    keypad_reset_keys(keys);

    (void)state;
    keypad_update_t event;

    event = keypad_routine(keys, 40, 2000, 100, 0, 0);
    assert_int_equal(KEY_NOTHING, event.event);

    event = keypad_routine(keys, 40, 2000, 100, 1, 0x01);
    assert_int_equal(KEY_PRESS, event.event);

    event = keypad_routine(keys, 40, 2000, 100, 20, 0x01);
    assert_int_equal(KEY_NOTHING, event.event);

    event = keypad_routine(keys, 40, 2000, 100, 42, 0x01);
    assert_int_equal(KEY_CLICK, event.event);

    event = keypad_routine(keys, 40, 2000, 100, 42 + 50, 0x01);
    assert_int_equal(KEY_NOTHING, event.event);

    event = keypad_routine(keys, 40, 2000, 100, 42 + 101, 0x01);
    assert_int_equal(KEY_PRESSING, event.event);

    event = keypad_routine(keys, 40, 2000, 100, 42 + 101 * 2, 0x01);
    assert_int_equal(KEY_PRESSING, event.event);

    event = keypad_routine(keys, 40, 2000, 100, 42 + 2001, 0x01);
    assert_int_equal(KEY_LONGCLICK, event.event);

    event = keypad_routine(keys, 40, 2000, 100, 42 + 2001 + 101, 0x01);
    assert_int_equal(KEY_LONGPRESSING, event.event);

    event = keypad_routine(keys, 40, 2000, 100, 42 + 2001 + 102, 0x01);
    assert_int_equal(KEY_NOTHING, event.event);

    event = keypad_routine(keys, 40, 2000, 100, 42 + 2001 + 101 * 2, 0x01);
    assert_int_equal(KEY_LONGPRESSING, event.event);

    event = keypad_routine(keys, 40, 2000, 100, 4000, 0x01);
    assert_int_equal(KEY_LONGPRESSING, event.event);

    event = keypad_routine(keys, 40, 2000, 100, 4001, 0x0);
    assert_int_equal(KEY_NOTHING, event.event);

    event = keypad_routine(keys, 40, 2000, 100, 4042, 0x0);
    assert_int_equal(KEY_RELEASE, event.event);
}


void test_longclick(void **state) {
    keypad_reset_keys(keys);

    (void)state;
    keypad_update_t event;

    event = keypad_routine(keys, 40, 2000, 5, 0, 0x01);
    assert_int_equal(KEY_PRESS, event.event);

    event = keypad_routine(keys, 40, 2000, 5, 2001, 0x01);
    assert_int_equal(KEY_CLICK, event.event);

    event = keypad_routine(keys, 40, 2000, 5, 2001, 0x01);
    assert_int_equal(KEY_LONGCLICK, event.event);

    event = keypad_routine(keys, 40, 2000, 5, 2007, 0x01);
    assert_int_equal(KEY_LONGPRESSING, event.event);

    event = keypad_routine(keys, 40, 2000, 1, 2004, 0);
    assert_int_equal(KEY_NOTHING, event.event);

    event = keypad_routine(keys, 40, 2000, 1, 2043, 0);
    assert_int_equal(KEY_NOTHING, event.event);

    event = keypad_routine(keys, 40, 2000, 1, 2045, 0);
    assert_int_equal(KEY_RELEASE, event.event);
}


void test_multiclick(void **state) {
    keypad_reset_keys(keys);

    (void)state;
    keypad_update_t event;

    event = keypad_routine(keys, 40, 2000, 1, 0, ONEFIVEBMP);
    assert_int_equal(KEY_PRESS, event.event);

    event = keypad_routine(keys, 40, 2000, 1, 50, ONEFIVEBMP);
    assert_int_equal(KEY_CLICK, event.event);
    assert_int_equal(P_ONEFIVE, event.code);

    event = keypad_routine(keys, 40, 2000, 1, 60, 0x1);
    assert_int_equal(KEY_NOTHING, event.event);

    event = keypad_routine(keys, 40, 2000, 1, 120, 0x1);
    assert_int_equal(KEY_RELEASE, event.event);
    assert_int_equal(P_ONEFIVE, event.code);

    event = keypad_routine(keys, 40, 2000, 1, 120, 0x1);
    assert_int_equal(KEY_PRESS, event.event);

    event = keypad_routine(keys, 40, 2000, 1, 180, 0x1);
    assert_int_equal(KEY_CLICK, event.event);

    event = keypad_routine(keys, 40, 2000, 1, 180, 0);
    assert_int_equal(KEY_NOTHING, event.event);

    event = keypad_routine(keys, 40, 2000, 1, 250, 0);
    assert_int_equal(KEY_RELEASE, event.event);

    event = keypad_routine(keys, 40, 2000, 1, 300, 0x1);
    assert_int_equal(KEY_PRESS, event.event);

    event = keypad_routine(keys, 40, 2000, 1, 350, 0x1);
    assert_int_equal(KEY_CLICK, event.event);
    assert_int_equal(P_ONE, event.code);
}


void test_comboclick(void **state) {
    keypad_reset_keys(keys);

    (void)state;
    keypad_update_t event;

    keypad_routine(keys, 40, 2000, 1, 0, ONEFIVEBMP);
    event = keypad_routine(keys, 40, 2000, 1, 41, ONEFIVEBMP);
    assert_int_equal(KEY_CLICK, event.event);
    assert_int_equal(P_ONEFIVE, event.code);

    event = keypad_routine(keys, 40, 2000, 1, 41, ONETWOFIVEBMP);
    assert_int_equal(KEY_NOTHING, event.event);
    event = keypad_routine(keys, 40, 2000, 1, 82, ONETWOFIVEBMP);
    assert_int_equal(KEY_RELEASE, event.event);
    assert_int_equal(P_ONEFIVE, event.code);

    event = keypad_routine(keys, 40, 2000, 1, 82, ONEFIVEBMP);
    assert_int_equal(KEY_PRESS, event.event);
    assert_int_equal(P_ONEFIVE, event.code);
    event = keypad_routine(keys, 40, 2000, 1, 123, ONEFIVEBMP);
    assert_int_equal(KEY_CLICK, event.event);
    assert_int_equal(P_ONEFIVE, event.code);

    event = keypad_routine(keys, 40, 2000, 1, 123, ONETWOFIVEBMP);
    assert_int_equal(KEY_NOTHING, event.event);
    event = keypad_routine(keys, 40, 2000, 1, 164, ONETWOFIVEBMP);
    assert_int_equal(KEY_RELEASE, event.event);
    assert_int_equal(P_ONEFIVE, event.code);
    event = keypad_routine(keys, 40, 2000, 1, 165, ONETWOFIVEBMP);
    assert_int_equal(KEY_PRESS, event.event);
    assert_int_equal(P_ONETWOFIVE, event.code);
    event = keypad_routine(keys, 40, 2000, 1, 206, ONETWOFIVEBMP);
    assert_int_equal(KEY_CLICK, event.event);
    assert_int_equal(P_ONETWOFIVE, event.code);
}


int keypad_test_run(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_comboclick),
        cmocka_unit_test(test_click_release),
        cmocka_unit_test(test_longclick),
        cmocka_unit_test(test_event_chain),
        cmocka_unit_test(test_multiclick),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */
    int count_fail_tests = cmocka_run_group_tests(tests, setup, teardown);

    return count_fail_tests;
}
