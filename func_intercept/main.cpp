#include <iostream>
#include "lib_a.h"
#include "lib_a.hpp"
#include "lib_b.hpp"
int main()
{
#if 0
    lib_a_func_a();
    lib_a_func_b();
    unsigned long ns_duration = lib_a_func_c(1000000);
    printf("lib_a_func_c returned %lu (ns_duration)\n", ns_duration);
    unsigned long ns_duration_d = lib_a_func_d(1000000);
    printf("lib_a_func_d returned %lu (ns_duration)\n", ns_duration_d);
    lib_a_cpp_class_a* object_a = new lib_a_cpp_class_b();
    unsigned long sleep_time = 1000000;

    auto ns_duration_cpp_inline = object_a->usec_sleep_inline(sleep_time);
    std::cout << "lib_a_cpp_class_a->usec_sleep_inline(" << sleep_time << ") returned " << ns_duration_cpp_inline << " nanoseconds." << std::endl;

    auto ns_duration_cpp_reentrant = object_a->usec_sleep_reentrant(sleep_time);
    std::cout << "lib_a_cpp_class_a->usec_sleep_reentrant(" << sleep_time << ") returned " << ns_duration_cpp_reentrant << " nanoseconds." << std::endl;
#endif
    lib_a_cpp_class_a* object_a = new lib_a_cpp_class_b();
    unsigned long sleep_time = 1000000;
    int i;
    for (i = 0; i < 100; ++ i) {
        std::cout << i << " ";
        auto ns_duration_cpp_inline = object_a->usec_sleep_inline(sleep_time);
        std::cout << " " << ns_duration_cpp_inline << std::endl;
    }
    //
    // Assessing the overhead (values in nanoseconds):
    //
    // > ./run.fish | tee /tmp/tmp.txt
    // > ./run.fish | tee /tmp/tmp2.txt
    // > ./run.fish | tee /tmp/tmp3.txt
    // > ./run.fish | tee /tmp/tmp4.txt
    // > for f in /tmp/tmp{,2,3,4}.txt; set -l n; set -l t 0; while read i d s; set t (math $t + $d - $s); set n $i; end < $f; math $t / \($n + 1\); end
    // 870.71
    // 942.71
    // 967.02
    // 891.91
    //
    delete object_a;

    return 0;
}
