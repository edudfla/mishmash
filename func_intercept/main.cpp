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
        std::cout << i;
        auto ns_duration_cpp_inline = object_a->usec_sleep_inline(sleep_time);
        std::cout << "," << ns_duration_cpp_inline;
        auto ns_duration_cpp_reentrant = object_a->usec_sleep_reentrant(sleep_time);
        std::cout << "," << ns_duration_cpp_reentrant;
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
    int result = 0;
    if (failures != 0 ) {
        result = 1;
    }
    //
    // Assessing the overhead (values in nanoseconds):
    //
    // > begin; ./build.fish; set -l f (mktemp); for i in (seq 10); set -l t 0 0; ./run.fish > $f; set -l r $status; set -l g (ls -t /tmp/lib_a_ld_preload-*-*-*.csv)[1]; while read -d , -a a; set t[1] (math $t[1] + \($a[6] - $a[5]\) - $a[2]); set t[2] (math $t[2] + \($a[9] - $a[8]\) - $a[3]); end < (paste -d , $f (cat $g | paste -s -d ',\n' | psub) | psub); set -l n (cat $f | wc -l); echo $r (math $t[1] / $n) (math $t[2] / $n); end; end
    // 0 1217.36 1880.09
    // 0 1335.16 2509.92
    // 0 1297.49 2280.88
    // 0 1349.65 2267.87
    // 0 1275.95 2175.58
    // 0 1301.87 1898.83
    // 0 1328.34 1983.02
    // 0 1206.96 2129.28
    // 0 1284.35 1993.6
    // 0 1217.04 2032.27
    //
    delete object_a;

    return result;
}
