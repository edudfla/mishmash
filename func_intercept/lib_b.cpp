#include <iostream>
#include <unistd.h>
#include "lib_b.hpp"
#include "lib_a.h"
unsigned long lib_a_cpp_class_b::usec_sleep_inline(unsigned long usec_sleep_interval) const
{
    struct timespec a;
    clock_gettime(CLOCK_MONOTONIC, &a);
    usleep((useconds_t)usec_sleep_interval);
    struct timespec b;
    clock_gettime(CLOCK_MONOTONIC, &b);
    unsigned long result = (unsigned long)((b.tv_sec * 1000000000 + b.tv_nsec) - (a.tv_sec * 1000000000 + a.tv_nsec));
    // printf("lib_a_cpp_class_b::usec_sleep_inline is about to return %lu.\n", result);
    return result;
}
unsigned long lib_a_cpp_class_b::usec_sleep_reentrant(unsigned long usec_sleep_interval) const
{
    auto result = lib_a_func_d(usec_sleep_interval);
    // std::cout << "lib_a_cpp_class_b::usec_sleep_reentrant is about to return " << result << std::endl;
    return result;
}
