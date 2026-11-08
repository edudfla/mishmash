#pragma once
class lib_a_cpp_class_a {
    public:
        virtual ~lib_a_cpp_class_a() = default;
        virtual unsigned long usec_sleep_inline(unsigned long usec_sleep_interval) const = 0;
        virtual unsigned long usec_sleep_reentrant(unsigned long usec_sleep_interval) const = 0;
};
