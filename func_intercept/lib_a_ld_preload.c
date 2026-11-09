#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#if 0
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
void lib_a_func_d()
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
//
// Possible regex to list the C++ functions:
// > objdump -T ~/bin/lib_a_bin/liblib_a_cpp.so | rg -e '(?-u) g    DF .text\t'
//
void _ZNK17lib_a_cpp_class_b10usec_sleepEm()
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
        lib_a_func_d_target = dlsym(RTLD_NEXT, "_ZNK17lib_a_cpp_class_b10usec_sleepEm");
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
#endif
//------------------------------------------------------------------------------
typedef struct {
    int function_index;
    void* function_target;
    void* actual_return_address;
    struct timespec last_entry_ts;
    struct timespec last_exit_ts;
    const char * function_name;
} function_profile_intercept_t;
//------------------------------------------------------------------------------
//
// the static variables intercepted_functions, active_fpi, and other_fpi could
// be moved to the scope of function_interceptor.
//
static function_profile_intercept_t intercepted_functions[] = {
    {0, NULL, NULL, {}, {}, "lib_a_func_a"},
    {1, NULL, NULL, {}, {}, "lib_a_func_b"},
    {2, NULL, NULL, {}, {}, "lib_a_func_c"},
    {3, NULL, NULL, {}, {}, "lib_a_func_d"},
    {4, NULL, NULL, {}, {}, "_ZNK17lib_a_cpp_class_b17usec_sleep_inlineEm"},
    {5, NULL, NULL, {}, {}, "_ZNK17lib_a_cpp_class_b20usec_sleep_reentrantEm"},
    {-1, NULL, NULL, {}, {}, ""}
};
//
// this code **IS NOT** thread safe
//
static function_profile_intercept_t* active_fpi = NULL;
static function_profile_intercept_t* other_fpi = NULL;
static void* jump_or_return_address_scratch_pad;
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
//
// keep it always even!!!
//
#define ISR_LIKE_STATE_LENGTH (10 * 8)
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
        : "i" (ISR_LIKE_STATE_LENGTH) \
    )
//------------------------------------------------------------------------------
//
// Possible regex to list the C++ functions:
// > objdump -T ~/bin/lib_a_bin/liblib_a_cpp.so | rg -e '(?-u) g    DF .text\t'
//
// function_interceptor is never called directly.
// instead the interceptors jump to it.
// the context is preserved with the return addres and the frame pointer.
// the stack is kept untouched to allow the transparent interception.
//
void function_interceptor()
{
    register uint64_t function_index;
    asm volatile (
        "\tmov %%rbx,%0\n"
        : "=r" (function_index)
    );
    other_fpi = &(intercepted_functions[function_index]);
    if (NULL == other_fpi->function_target) {
        //
        // ensures that rsp is 16 byte aligned.
        // this is to avoid SIGSEGV in dlsym
        //
        asm volatile (
            "\tmov $0x0,%%rax\n"
            "\tmov $0x8,%%rbx\n"
            "\ttest $0xf,%%rsp\n"
            "\tcmovnz %%rbx,%%rax\n"
            "\tsub %%rax,%%rsp\n"
            :
        );
        other_fpi->function_target = dlsym(RTLD_NEXT, other_fpi->function_name);
    }
    if (NULL == other_fpi->function_target) {
        //
        // irrecoverable failure
        //
        return;
    }
    //
    // check if there is no other active call
    //
    if (NULL != active_fpi) {
        //
        // reentrant call
        //
        //
        // needs to discard the dangling frame pointer because arrived here through jump.
        //
        asm volatile (
            "\tmov %%rbp,%%rsp\n"
            "\tpop %%rbp\n"
            :
        );
        jump_or_return_address_scratch_pad = other_fpi->function_target;
        ISR_LIKE_RESTORE_STATE;
        asm volatile (
            "\tmov %%rbp,%%rsp\n"
            "\tpop %%rbp\n"
            "\tjmp *%0\n"
            :
            : "m" (jump_or_return_address_scratch_pad)
        );
    }
    active_fpi = other_fpi;
    //
    // start the profiling.
    // the following assignment is the "lock" checked to detect reentrant calls.
    // this code supports single threaded processes only.
    //
    asm volatile (
        "\tmov %%rbp,%%rax\n"
        "\tmov (%%rax),%%rax\n"
        "\tmov 8(%%rax),%0\n" // saves the original return adddress.
        : "=p" (jump_or_return_address_scratch_pad)
    );
    active_fpi->actual_return_address = jump_or_return_address_scratch_pad;
    clock_gettime(CLOCK_MONOTONIC, &(active_fpi->last_entry_ts));
    //
    // needs to discard the dangling frame pointer because arrived here through jump.
    //
    asm volatile (
        "\tmov %%rbp,%%rsp\n"
        "\tpop %%rbp\n"
        :
    );
    ISR_LIKE_RESTORE_STATE;
    asm volatile (
        "\tmov %%rbp,%%rsp\n"
        "\tpop %%rbp\n"
        //
        // the frame pointer is not available anymore from here on.
        //
        "\tadd $8,%%rsp\n" // rewind the stack to the point where the original call was performed
        "\tcall *%0\n"     // calls the original function
        :
        : "m" (active_fpi->function_target)
    );
    ISR_LIKE_SAVE_STATE;
    clock_gettime(CLOCK_MONOTONIC, &(active_fpi->last_exit_ts));
    //
    // needs to be register to avoid changing the stack.
    //
    register unsigned long duration =
        (unsigned long)
        ((active_fpi->last_exit_ts.tv_sec * 1000000000 + active_fpi->last_exit_ts.tv_nsec) -
        (active_fpi->last_entry_ts.tv_sec * 1000000000 + active_fpi->last_entry_ts.tv_nsec));
    // printf("%d %s %lu\n",
    //    active_fpi->function_index,
    //    active_fpi->function_name,
    //    duration);
    printf("%lu", duration);
    //
    // stop the profiling
    // active_fpi needs to be set back to NULL
    //
    jump_or_return_address_scratch_pad = active_fpi->actual_return_address;
    active_fpi = NULL;
    ISR_LIKE_RESTORE_STATE;
    asm volatile (
        "\tjmp *%0\n" // terminates the interception
        :
        : "m" (jump_or_return_address_scratch_pad)  // the "m" is important.
                                                    // lib_a_func_d returns in rax.
                                                    // when using "r", register rax was being overwritten and the return value was being lost.
                                                    // the usage if static global variable jump_or_return_address_scratch_pad preservers the return value
                                                    // when the field actual_return_address was in use the register with the return value was overwritten because of the multiple indirections needed to fetch the final jump value
    );
}
//------------------------------------------------------------------------------
//
// this is screming for a macro.
// keep like that for now.
// a macro would make debuggin harder.
// in the end the code will be generated anyway.
//
void lib_a_func_a()
{
    ISR_LIKE_SAVE_STATE; // how come this is interfering with dlsym?
                         // why adding even a single push instruction here causes dlsym to SIGSEGV?
                         // changing (sub) the rsp also causes weird issues.
                         // what is going on here?
                         // issue solved.
                         // the stack pointer shall be an integer multiple of 16.
                         // the SIGSEGV was because an odd number of pushes was being done in macro ISR_LIKE_SAVE_STATE.
    asm volatile (
        "\tmov %0,%%rbx\n"
        "\tjmp *%1\n"
        :
        : "n" (0)
        , "r" (function_interceptor) // this goes to rax
    );
}
//------------------------------------------------------------------------------
void lib_a_func_b()
{
    ISR_LIKE_SAVE_STATE;
    asm volatile (
        "\tmov %0,%%rbx\n"
        "\tjmp *%1\n"
        :
        : "n" (1)
        , "r" (function_interceptor)
    );
}
//------------------------------------------------------------------------------
void lib_a_func_c()
{
    ISR_LIKE_SAVE_STATE;
    asm volatile (
        "\tmov %0,%%rbx\n"
        "\tjmp *%1\n"
        :
        : "n" (2)
        , "r" (function_interceptor)
    );
}
//------------------------------------------------------------------------------
void lib_a_func_d()
{
    ISR_LIKE_SAVE_STATE;
    asm volatile (
        "\tmov %0,%%rbx\n"
        "\tjmp *%1\n"
        :
        : "n" (3)
        , "r" (function_interceptor)
    );
}
//------------------------------------------------------------------------------
void _ZNK17lib_a_cpp_class_b17usec_sleep_inlineEm()
{
    ISR_LIKE_SAVE_STATE;
    asm volatile (
        "\tmov %0,%%rbx\n"
        "\tjmp *%1\n"
        :
        : "n" (4)
        , "r" (function_interceptor)
    );
}
//------------------------------------------------------------------------------
void _ZNK17lib_a_cpp_class_b20usec_sleep_reentrantEm()
{
    ISR_LIKE_SAVE_STATE;
    asm volatile (
        "\tmov %0,%%rbx\n"
        "\tjmp *%1\n"
        :
        : "n" (5)
        , "r" (function_interceptor)
    );
}
//------------------------------------------------------------------------------
