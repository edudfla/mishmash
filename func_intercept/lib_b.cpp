#include <iostream>
#include "lib_b.hpp"
#include "lib_a.h"
unsigned long lib_a_cpp_class_b::usec_sleep(unsigned long usec_sleep_interval) const
{
    auto result = lib_a_func_d(usec_sleep_interval);
    std::cout << "lib_a_cpp_class_b::usec_sleep is about to return " << result << std::endl;
    return result;
}
