#include <iostream>
#include "lib_a.h"
#include "lib_a.hpp"
#include "lib_b.hpp"
#include <cstdint>
#include <time.h>

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
    uint64_t v_0[64];
    for (uint64_t i = 0; i < 64; ++ i) {
        if (0 == (i & 1)) {
            v_0[i] = 0xAAAAAAAAAAAAAAAA + (1 << i);
        } else {
            v_0[i] = 0x5555555555555555 + (1 << i);
        }
    }
    uint64_t v_1 = 0x8000000000000001;
    uint64_t v_2 = 0x8000000000000002;
    uint64_t v_3 = 0x8000000000000003;
    uint64_t v_4 = 0x8000000000000004;
    uint64_t v_5 = 0x8000000000000005;
    uint64_t v_6 = 0x8000000000000006;
    uint64_t v_7 = 0x8000000000000007;
    uint64_t v_8 = 0x8000000000000008;
    struct timespec t_i, t_f;
    void *rsp_i;
    void *rbp_i;
    void *rsp_f;
    void *rbp_f;
    asm (
        "\tmov %%rsp,%0\n"
        "\tmov %%rbp,%1\n"
        : "=m" (rsp_i)
        , "=m" (rbp_i)
    );
    for (i = 0; i < 100; ++ i) {
        std::cout << i;
        clock_gettime(CLOCK_REALTIME, &t_i);
        auto ns_duration_cpp_inline = object_a->usec_sleep_inline(sleep_time);
        clock_gettime(CLOCK_REALTIME, &t_f);
        std::cout << "," << ((t_f.tv_sec - t_i.tv_sec) * 1000000000 + (t_f.tv_nsec - t_i.tv_nsec));
        // std::cout << "," << ns_duration_cpp_inline;
        clock_gettime(CLOCK_REALTIME, &t_i);
        auto ns_duration_cpp_reentrant = object_a->usec_sleep_reentrant(sleep_time);
        clock_gettime(CLOCK_REALTIME, &t_f);
        std::cout << "," << ((t_f.tv_sec - t_i.tv_sec) * 1000000000 + (t_f.tv_nsec - t_i.tv_nsec));
        // std::cout << "," << ns_duration_cpp_reentrant;
        std::cout << std::endl;
    }
    //
    // Check if the initially fixed values still hold the original values.
    //
    asm (
        "\tmov %%rsp,%0\n"
        "\tmov %%rbp,%1\n"
        : "=m" (rsp_f)
        , "=m" (rbp_f)
    );
    int failures = 0;
    if (rsp_i != rsp_f) {
        failures ++;
    }
    if (rbp_i != rbp_f) {
        failures ++;
    }
    for (int i = 0; i < 64; ++ i) {
        if (0 == (i & 1)) {
            if (v_0[i] != 0xAAAAAAAAAAAAAAAA + (1 << i)) {
                failures ++;
            }
        } else {
            if (v_0[i] != 0x5555555555555555 + (1 << i)) {
                failures ++;
            }
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
    // > begin; ./build.fish; set -l f (mktemp); for i in (seq 10); set -l t 0 0; ./run.fish > $f; set -l r $status; set -l g (ls -t /tmp/lib_a_ld_preload-*-*-*.csv)[1]; while read -d , -a a; set t[1] (math $t[1] + $a[2] - \($a[6] - $a[5]\)); set t[2] (math $t[2] + $a[3] - \($a[9] - $a[8]\)); end < (paste -d , $f (cat $g | paste -s -d ',\n' | psub) | psub); set -l n (cat $f | wc -l); echo $r (math $t[1] / $n) (math $t[2] / $n); end; end
    // 0 8964.68 9659.47
    // 0 9777.38 10680.12
    // 0 9548.39 10141.81
    // 0 9923.15 10679.56
    // 0 9936.51 10364.07
    // 0 9109.62 9106.59
    // 0 9258.2 9627.69
    // 0 9198.35 9646.03
    // 0 10243.46 9929.66
    // 0 9138.25 10037.73
    //
    delete object_a;

    return result;
}
