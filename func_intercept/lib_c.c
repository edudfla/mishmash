#include <time.h>
#include <unistd.h>
#include "lib_c.h"
unsigned long lib_c_func_a(unsigned long usleep_duration)
{
    struct timespec a;
    clock_gettime(CLOCK_MONOTONIC, &a);
    usleep((useconds_t)usleep_duration);
    struct timespec b;
    clock_gettime(CLOCK_MONOTONIC, &b);
    unsigned long result = (unsigned long)((b.tv_sec * 1000000000 + b.tv_nsec) - (a.tv_sec * 1000000000 + a.tv_nsec));
    return result;
}

