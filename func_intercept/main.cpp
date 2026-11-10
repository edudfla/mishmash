#include <iostream>
#include "lib_a.h"
#include "lib_a.hpp"
#include "lib_b.hpp"
#include <cstdint>

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
    unsigned long sleep_time = 10000;
    int i;
    //
    // Fill some stack and registers with some fixed values.
    //
    std::uint64_t v_0[64];
    for (int i = 0; i < 64; ++ i) {
        v_0[i] = 1 << 63 + i + 1;
    }
    uint64_t v_1 = 0x8000000000000001;
    uint64_t v_2 = 0x8000000000000002;
    uint64_t v_3 = 0x8000000000000003;
    uint64_t v_4 = 0x8000000000000004;
    uint64_t v_5 = 0x8000000000000005;
    uint64_t v_6 = 0x8000000000000006;
    uint64_t v_7 = 0x8000000000000007;
    uint64_t v_8 = 0x8000000000000008;
    for (i = 0; i < 100; ++ i) {
        std::cout << i << " ";
        auto ns_duration_cpp_inline = object_a->usec_sleep_inline(sleep_time);
        std::cout << " " << ns_duration_cpp_inline;
        std::cout << " ";
        auto ns_duration_cpp_reentrant = object_a->usec_sleep_reentrant(sleep_time);
        std::cout << " " << ns_duration_cpp_reentrant;
        std::cout << std::endl;
    }
    //
    // Check if the initially fixed values still hold the original values.
    //
    int failures = 0;
    for (int i = 0; i < 64; ++ i) {
        if (v_0[i] != 1 << 63 + i + 1) {
            failures ++;
        }
    }
    if (v_1 != 0x8000000000000001) {
        failures ++;
    }
    if (v_2 != 0x8000000000000002) {
        failures ++;
    }
    if (v_3 != 0x8000000000000003) {
        failures ++;
    }
    if (v_4 != 0x8000000000000004) {
        failures ++;
    }
    if (v_5 != 0x8000000000000005) {
        failures ++;
    }
    if (v_6 != 0x8000000000000006) {
        failures ++;
    }
    if (v_7 != 0x8000000000000007) {
        failures ++;
    }
    if (v_8 != 0x8000000000000008) {
        failures ++;
    }
    if (failures != 0 ) {
        std::cerr << "FAIL";
    } else {
        std::cerr << "PASS";
    }
    std::cerr << std::endl; 
    //
    // Assessing the overhead (values in nanoseconds):
    //
    // > ./run.fish | tee /tmp/tmp.txt
    // > ./run.fish | tee /tmp/tmp2.txt
    //     // > ./run.fish | tee /tmp/tmp3.txt
    //     // > ./run.fish | tee /tmp/tmp4.txt
    //     // > for f in /tmp/tmp{,2,3,4}.txt; set -l n; set -l t 0; while read i d s; set t (math $t + $d - $s); set n $i; end < $f; math $t / \($n + 1\); end
    //     // 870.71
    //     // 942.71
    //     // 967.02
    //     // 891.91
    //     // > for i in (seq 10); ./run.fish | tee /tmp/mishmash_(printf %02d $i).txt; end
    //     // > for f in /tmp/mishmash_*.txt; set -l t 0 0; while read -a a; set t[1] (math $t[1] + $a[2] - $a[3]); set t[2] (math $t[2] + $a[4] - $a[5]); end < $f; set -l n (cat $f | wc -l); echo (math $t[1] / $n) (math $t[2] / $n); end
    //
    // Results without optimization:
    // > for f in /tmp/mishmash_*.txt; set -l t 0 0; while read -a a; set t[1] (math $t[1] + $a[2] - $a[3]); set t[2] (math $t[2] + $a[4] - $a[5]); end < $f; set -l n (cat $f | wc -l); echo (math $t[1] / $n) (math $t[2] / $n); end | nl
    //      1  1291.95 2103.21
    //      2  1606.31 2033.02
    //      3  1321.04 2224.73
    //      4  1303.76 2134.5
    //      5  1485.59 2118.55
    //      6  1278.04 2021.69
    //      7  1304.71 2129.62
    //      8  1360.17 2247.33
    //      9  1304.04 2168.71
    //     10  1351.77 2061.84
    // Results with optimization
    //      1  1342.03 2001.57
    //      2  1329.71 2409.33
    //      3  1315.25 1984.75
    //      4  1326.69 2202.75
    //      5  1338.92 2537.52
    //      6  1453.06 2348.05
    //      7  1341.78 2155.02
    //      8  1314.52 2133.63
    //      9  1808.78 2204.13
    //     10  1317.65 2415.28
    delete object_a;

    return 0;
}
