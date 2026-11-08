#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
//------------------------------------------------------------------------------
void lib_a_func_a()
{
    static void* lib_a_func_a_target;
    if (NULL == lib_a_func_a_target) {
        lib_a_func_a_target = dlsym(RTLD_NEXT, "lib_a_func_a");
    }
    //
    // This already works.
    // The "pop %%rbp" is needed when frame pointer is enabled (the case when
    // optimization is disabled).
    //
    if (NULL != lib_a_func_a_target) {
        asm (
            "\tpop %%rbp\n"
            "\tjmp *%0\n"
            :
            :
            "r" (lib_a_func_a_target)
        );
    }
    //
    // Next step is to capture the return.
    //
}
//------------------------------------------------------------------------------
void lib_a_func_b_exit();
static void* actual_return_address;
void lib_a_func_b()
{
    static void* lib_a_func_b_target;
    if (NULL == lib_a_func_b_target) {
        lib_a_func_b_target = dlsym(RTLD_NEXT, "lib_a_func_b");
    }
    if (NULL != lib_a_func_b_target) {
        asm volatile (
            "\tmov  8(%%rbp),%0\n"  // this code shall always be compiled with "-fno-omit-frame-pointer -mno-omit-leaf-frame-pointer"
                                    // saves the original return address
                                    // rbp contains the rsp when entering the function
                                    // *(rbp + 0) contains the original content of rbp on function entry
                                    // *(rbp + 8) contains the return address
                                    // rsp growis downward
                                    // rsp is the 64 bit stack pointer
                                    // the content of the top of the stack minus one item is the actual return address
            "\tmov  %1,8(%%rbp)\n"  // injects the intercept return address
            : "=a" (actual_return_address) // %0
            : "b"  (lib_a_func_b_exit)     // %1
        );
        printf("actual_return_address = %p\n", actual_return_address);
        //
        // In tests, the "jmp" instruction was preceding the capture of
        // "actual_return_address".
        // This was figured out by inspection of the disassembled code.
        // Because of that, the capture of "actual_return_address" is
        // explicitly moved for before.
        //
        asm volatile (
            "\tmov  %%rbp,%%rsp\n"  // this code shall always be compiled with "-fno-omit-frame-pointer -mno-omit-leaf-frame-pointer"
            "\tpop        %%rbp\n"
            "\tjmp          *%0\n"  // jumps to the original function
            :
            : "a"  (lib_a_func_b_target)   // %0
        );
    }
}
void lib_a_func_b_exit()
{
    asm volatile (
        "\tpop %%rbp\n"
        "\tjmp *%0"
        :
        : "r" (actual_return_address)
    );
}
//------------------------------------------------------------------------------
void lib_a_func_c_exit();
static void* actual_return_address;
struct timespec lib_a_func_c_entry_timespec;
struct timespec lib_a_func_c_exit_timespec;
void lib_a_func_c()
{
    static void* lib_a_func_c_target;
    if (NULL == lib_a_func_c_target) {
        lib_a_func_c_target = dlsym(RTLD_NEXT, "lib_a_func_c");
    }
    if (NULL != lib_a_func_c_target) {
        asm volatile (
            "\tmov  8(%%rbp),%0\n"  // this code shall always be compiled with "-fno-omit-frame-pointer -mno-omit-leaf-frame-pointer"
                                    // saves the original return address
                                    // rbp contains the rsp when entering the function
                                    // *(rbp + 0) contains the original content of rbp on function entry
                                    // *(rbp + 8) contains the return address
                                    // rsp growis downward
                  // rsp is the 64 bit stack pointer
                                    // the content of the top of the stack minus one item is the actual return address
            "\tmov  %1,8(%%rbp)\n"  // injects the intercept return address
            : "=a" (actual_return_address) // %0
            : "b"  (lib_a_func_c_exit)     // %1
        );
        //
        // In tests, the "jmp" instruction was preceding the capture of
        // "actual_return_address".
        // This was figured out by inspection of the disassembled code.
        // Because of that, the capture of "actual_return_address" is
        // explicitly moved for before.
        //
        clock_gettime(CLOCK_MONOTONIC, &lib_a_func_c_entry_timespec);
        asm volatile (
            "\tmov  %%rbp,%%rsp\n"  // this code shall always be compiled with "-fno-omit-frame-pointer -mno-omit-leaf-frame-pointer"
            "\tpop        %%rbp\n"
            "\tjmp          *%0\n"  // jumps to the original function
            :
            : "a"  (lib_a_func_c_target)   // %0
        );
    }
}
void lib_a_func_c_exit()
{
    clock_gettime(CLOCK_MONOTONIC, &lib_a_func_c_exit_timespec);
    asm volatile (
        "\tpop %%rbp\n"
        "\tjmp *%0"
        :
        : "r" (actual_return_address)
    );
}
unsigned long lib_a_func_c_duration_nsec()
{
    return
        (lib_a_func_c_exit_timespec.tv_sec * 1000000000 + lib_a_func_c_exit_timespec.tv_nsec) -
        (lib_a_func_c_entry_timespec.tv_sec * 1000000000 + lib_a_func_c_entry_timespec.tv_nsec);
}
//------------------------------------------------------------------------------
//
// During tests it was seen the exception like:
// Program received signal SIGSEGV, Segmentation fault.
// 0x00007ffff7fc24ba in __GI__dl_catch_exception (exception=exception@entry=0x7fffffffd778, 
//     operate=0x7ffff7e64995 <dlopen_doit>, args=0x7fffffffd818) at ./elf/dl-catch.c:233
// The reason seems to be that the stack pointer gets misaligned.
// For the exception disappear an extra push has to be added.
// Apparently the stack pointer has to be an integer multiple of 16.
// Then rdx was pushed.
//
// Inspired in SAVE_ARGS_IRQ
#define ISR_LIKE_SAVE_STATE \
    asm volatile ( \
        "\tpush %%rdi\n" \
        "\tpush %%rsi\n" \
        "\tpush %%rdx\n" \
        "\tpush %%rcx\n" \
        "\tpush %%rbx\n" \
        "\tpush %%rax\n" \
        "\tpush %%r8\n" \
        "\tpush %%r9\n" \
        "\tpush %%r10\n" \
        "\tpush %%r11\n" \
        : \
    )
#define ISR_LIKE_RESTORE_STATE \
    asm volatile ( \
        "\tpop %%r11\n" \
        "\tpop %%r10\n" \
        "\tpop %%r9\n" \
        "\tpop %%r8\n" \
        "\tpop %%rax\n" \
        "\tpop %%rbx\n" \
        "\tpop %%rcx\n" \
        "\tpop %%rdx\n" \
        "\tpop %%rsi\n" \
        "\tpop %%rdi\n" \
        : \
    )
unsigned long lib_a_func_d()
{
    static void* actual_return_address_func_d = NULL;
    static void* lib_a_func_d_target = NULL;
    static struct timespec lib_a_func_d_entry_timespec = {};
    static struct timespec lib_a_func_d_exit_timespec = {};
    ISR_LIKE_SAVE_STATE; // how come this is interfering with dlsym?
                         // why adding even a single push instruction here causes dlsym to SIGSEGV?
                         // changing (sub) the rsp also causes weird issues.
                         // what is going on here?
                         // issue solved.
                         // the stack pointer shall be an integer multiple of 16.
                         // the SIGSEGV was because an odd number of pushes was being done in macro ISR_LIKE_SAVE_STATE.
    if (NULL == lib_a_func_d_target) {
        lib_a_func_d_target = dlsym(RTLD_NEXT, "lib_a_func_d");
    }
    if (NULL != lib_a_func_d_target) {
        asm volatile (
            "\tmov 8(%%rbp),%0\n" // saves the original return adddress.
            : "=r" (actual_return_address_func_d)
        );
        clock_gettime(CLOCK_MONOTONIC, &lib_a_func_d_entry_timespec);
        ISR_LIKE_RESTORE_STATE;
        asm volatile (
            "\tpop %%rbp\n"
            "\tadd $8,%%rsp\n" // rewind the stack to the point where the original call was performed
            "\tcall *%0\n"     // calls the original function
            :
            : "r" (lib_a_func_d_target)
        );
        ISR_LIKE_SAVE_STATE;
        clock_gettime(CLOCK_MONOTONIC, &lib_a_func_d_exit_timespec);
        printf("%lu\n",
            (lib_a_func_d_exit_timespec.tv_sec * 1000000000 + lib_a_func_d_exit_timespec.tv_nsec) -
            (lib_a_func_d_entry_timespec.tv_sec * 1000000000 + lib_a_func_d_entry_timespec.tv_nsec));
        ISR_LIKE_RESTORE_STATE;
        asm volatile (
            "\tjmp *%0\n" // terminates the interception
            :
            : "m" (actual_return_address_func_d) // the "m" is important.
                                                 // lib_a_func_d returns in rax.
                                                 // when using "r", register rax was being overwritten and the return value was being lost.
        );
    } else {
        // What to do if the intercepted original function is not found?
    }
}
//------------------------------------------------------------------------------

