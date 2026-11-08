#include <stdio.h>
#include "lib_a.h"
int main()
{
    lib_a_func_a();
    lib_a_func_b();
    unsigned long ns_duration = lib_a_func_c(1000000);
    printf("lib_a_func_c returned %lu (ns_duration)\n", ns_duration);
    unsigned long ns_duration_d = lib_a_func_d(1000000);
    printf("lib_a_func_d returned %lu (ns_duration)\n", ns_duration_d);
    return 0;
}
