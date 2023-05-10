#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>


extern int debounce_test_run(void);
extern int keypad_test_run(void);


int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    return debounce_test_run() + keypad_test_run();
}
