#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

static int lib_a_func_a_global_static;
void lib_a_func_a()
{
    static int lib_a_func_a_static;
    printf("lib_a_func_a *%p = %d, *%p = %d\n", &lib_a_func_a_global_static, lib_a_func_a_global_static, &lib_a_func_a_static, lib_a_func_a_static);
}

static int lib_a_func_b_global_static;
void lib_a_func_b()
{
    static int lib_a_func_b_static;
    printf("lib_a_func_b *%p = %d, *%p = %d\n", &lib_a_func_b_global_static, lib_a_func_b_global_static, &lib_a_func_b_static, lib_a_func_b_static);
}

unsigned long lib_a_func_c(unsigned long usleep_duration)
{
    struct timespec a;
    clock_gettime(CLOCK_MONOTONIC, &a);
    usleep((useconds_t)usleep_duration);
    struct timespec b;
    clock_gettime(CLOCK_MONOTONIC, &b);
    unsigned long result = (unsigned long)((b.tv_sec * 1000000000 + b.tv_nsec) - (a.tv_sec * 1000000000 + a.tv_nsec));
    printf("lib_a_func_c is about to return %lu.\n", result);
    return result;
}

unsigned long lib_a_func_d(unsigned long usleep_duration)
{
    struct timespec a;
    clock_gettime(CLOCK_MONOTONIC, &a);
    usleep((useconds_t)usleep_duration);
    struct timespec b;
    clock_gettime(CLOCK_MONOTONIC, &b);
    unsigned long result = (unsigned long)((b.tv_sec * 1000000000 + b.tv_nsec) - (a.tv_sec * 1000000000 + a.tv_nsec));
    printf("lib_a_func_d is about to return %lu.\n", result);
    return result;
}
