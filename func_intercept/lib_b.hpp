#pragma once
#include "lib_a.hpp"
class lib_a_cpp_class_b : public lib_a_cpp_class_a {
    public:
        lib_a_cpp_class_b() = default;
        ~lib_a_cpp_class_b() override = default;
        unsigned long usec_sleep_inline(unsigned long usec_sleep_interval) const override;
        unsigned long usec_sleep_reentrant(unsigned long usec_sleep_interval) const override;
};
