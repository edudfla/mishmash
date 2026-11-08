#define _GNU_SOURCE 1
#include <dlfcn.h>

int main()
{
    asm volatile (
        "\tpush %%rax\n"
        "\tpush %%rbx\n"
        :
    );
    void* lib_handle = dlopen("/usr/lib/gcc/x86_64-linux-gnu/15/libatomic.so", RTLD_NOW);
    asm volatile (
        "\tpop %%rbx\n"
        "\tpop %%rax\n"
        :
    );
    return 0;
}
