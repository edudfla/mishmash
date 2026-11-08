#include <iostream>
#include "lib_a.h"
#include "lib_a.hpp"
#include "lib_b.hpp"
int main()
{
    lib_a_func_a();
    lib_a_func_b();
    unsigned long ns_duration = lib_a_func_c(1000000);
    printf("lib_a_func_c returned %lu (ns_duration)\n", ns_duration);
    unsigned long ns_duration_d = lib_a_func_d(1000000);
    printf("lib_a_func_d returned %lu (ns_duration)\n", ns_duration_d);
    lib_a_cpp_class_a* object_a = new lib_a_cpp_class_b();
    unsigned long sleep_time = 1000000;
    auto ns_duration_cpp_a = object_a->usec_sleep(sleep_time);
    std::cout << "lib_a_cpp_class_a->usec_sleep(" << sleep_time << ") returned " << ns_duration_cpp_a << " nanoseconds." << std::endl;
    delete object_a;
    return 0;
}
