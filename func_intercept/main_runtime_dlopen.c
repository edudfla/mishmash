#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lib_c.h"
int main(int argc, const char *argv[])
{
    unsigned long count_down = 1;
    if (2 == argc) {
        char *s;
        unsigned long i = strtoul(argv[1], &s, 0);
        if (('\0' == *s) && (i > 0)) {
            count_down = i;
        }
    }
    //
    // Tries to load the version from the LD_PRELOADed library.
    //
    lib_c_func_a_ptr lib_c_func_a = dlsym(RTLD_NEXT, lib_c_func_a_name);
    if (NULL != lib_c_func_a) {
        fprintf(stderr, "Succeeded loading the LD_PRELOADed version of `%s`.\n", lib_c_func_a_name);
    }
    //
    // Loads the library at runtime.
    // Without the following line it fails with SIGSEGV.
    //
    void *lib_c_handle = dlopen(lib_c_lib_name, RTLD_LAZY);
    if (NULL != lib_c_handle) {
        fprintf(stderr, "Succeeded loading `%s`.\n", lib_c_lib_name);
    }
    struct timespec t_i, t_f;
    while (count_down > 0) {
        clock_gettime(CLOCK_REALTIME, &t_i);
        lib_c_func_a(10000);
        clock_gettime(CLOCK_REALTIME, &t_f);
        printf("%lu\n", ((t_f.tv_sec - t_i.tv_sec) * 1000000000 + (t_f.tv_nsec - t_i.tv_nsec)));
        -- count_down;
    }
}
