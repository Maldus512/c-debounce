#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "debounce.h"


debounce_filter_t filter;


/* These functions will be used to initialize
   and clean resources up after each test run */
int setup(void **state) {
    (void)state;

    int i;
    debounce_filter_init(&filter);
    debounce_filter_set(&filter, 0);

    for (i = 0; i < 16; i++) {
        assert_int_equal(0, debounce_read(&filter, i));
    }

    return 0;
}

int teardown(void **state) {
    (void)state;
    return 0;
}


static void do_100_changes(int debounce) {
    int i, j, z;
    int res;
    debounce_filter_init(&filter);
    debounce_filter_set(&filter, 0);

    for (i = 0; i < 16; i++) {
        for (j = 0; j < 100; j++) {
            int value = (!(j % 2)) > 0;

            for (z = 0; z < debounce + 1; z++)
                res = debounce_filter(&filter, value << i, debounce);

            assert_true(res);
            assert_int_equal(value, debounce_read(&filter, i));
        }
    }
}

void test_no_debounce(void **state) {
    (void)state;
    do_100_changes(0);
}

void test_1_debounce(void **state) {
    (void)state;
    do_100_changes(1);
}

void test_100_debounce(void **state) {
    (void)state;
    do_100_changes(100);
}


void test_interrupt_change(void **state) {
    (void)state;
    debounce_filter_init(&filter);
    debounce_filter_set(&filter, 0);

    assert_true(!debounce_filter(&filter, 1, 4));
    assert_true(!debounce_filter(&filter, 1, 4));
    assert_true(!debounce_filter(&filter, 1, 4));

    assert_true(!debounce_filter(&filter, 0, 4));

    assert_true(!debounce_filter(&filter, 1, 4));
    assert_true(!debounce_filter(&filter, 1, 4));
    assert_true(!debounce_filter(&filter, 1, 4));
    assert_true(!debounce_filter(&filter, 1, 4));
    assert_true(debounce_filter(&filter, 1, 4));
}


int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_no_debounce),
        cmocka_unit_test(test_1_debounce),
        cmocka_unit_test(test_100_debounce),
        cmocka_unit_test(test_interrupt_change),
    };

    /* If setup and teardown functions are not
       needed, then NULL may be passed instead */
    int count_fail_tests = cmocka_run_group_tests(tests, setup, teardown);

    return count_fail_tests;
}
