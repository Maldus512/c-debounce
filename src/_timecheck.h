#ifndef C_DEBOUNCE_TIMECHECK_H_INCLUDED
#define C_DEBOUNCE_TIMECHECK_H_INCLUDED

#include <stdint.h>

#define time_after(a, b)          (((long)((b) - (a)) < 0))
#define time_after_or_equal(a, b) (((long)((b) - (a)) <= 0))

static inline __attribute__((always_inline)) uint8_t is_strictly_expired(unsigned long start, unsigned long current,
                                                                unsigned long delay) {
    return time_after(current, start + delay);
}

static inline __attribute__((always_inline)) unsigned long time_interval(unsigned long a, unsigned long b) {
    if (time_after(a, b))
        return -((unsigned long)b - (unsigned long)a);
    else
        return (unsigned long)b - (unsigned long)a;
}


#endif
