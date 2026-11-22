#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <errno.h>
#include <link.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
//------------------------------------------------------------------------------
extern char *program_invocation_short_name;
//------------------------------------------------------------------------------
#define PROFILER_DATA_DIR "/tmp"
#define PROFILE_LIST_INCREASE 10
//------------------------------------------------------------------------------
typedef struct {
    const int function_index;
    void *function_target;
    const char const*function_name;
} intercepted_function_t;
//------------------------------------------------------------------------------
static intercepted_function_t intercepted_functions[] = {
    {1,  NULL, "lib_a_func_a"},
    {2,  NULL, "lib_a_func_b"},
    {3,  NULL, "lib_a_func_c"},
    {4,  NULL, "lib_a_func_d"},
    {5,  NULL, "_ZNK17lib_a_cpp_class_b17usec_sleep_inlineEm"},
    {6,  NULL, "_ZNK17lib_a_cpp_class_b20usec_sleep_reentrantEm"},
    {7,  NULL, "lib_c_func_a"},
    {0,  NULL, ""}
};
//------------------------------------------------------------------------------
typedef struct {
    pid_t thread_id;
    pthread_t thread_self;
    int function_index;
    void *actual_return_address;
    struct timespec last_entry_ts;
    FILE *csv;
} thread_profiler_t;
//------------------------------------------------------------------------------
static size_t number_active_profiled_threads = 0;
static size_t profiled_threads_length = 0;
static thread_profiler_t *profiled_threads = NULL;
static pthread_mutex_t profiled_threads_mutex;
//------------------------------------------------------------------------------
void __attribute__ ((constructor)) setup_lib_a_ld_preload()
{
    intercepted_function_t *i_f = &(intercepted_functions[0]);
    while (i_f->function_index != 0) {
        i_f->function_target = dlsym(RTLD_NEXT, i_f->function_name);
        ++ i_f;
    }
    number_active_profiled_threads = 0;
    profiled_threads_length = PROFILE_LIST_INCREASE;
    profiled_threads = calloc(profiled_threads_length, sizeof(thread_profiler_t));
    pthread_mutex_init(&profiled_threads_mutex, NULL);
}
//------------------------------------------------------------------------------
void __attribute__((destructor)) teardown_lib_a_ld_preload()
{
    //
    // TODO: confirm if this destructor is actually invoked.
    //
    pthread_mutex_lock(&profiled_threads_mutex);
    {
        thread_profiler_t *t_p_i = &(profiled_threads[0]);
        size_t t_i = 0;
        while(t_i < number_active_profiled_threads) {
            if (NULL != t_p_i->csv) {
                fflush(t_p_i->csv);
                fclose(t_p_i->csv);
            }
            t_i ++;
            t_p_i ++;
        }
    }
    //
    // Not tearing down everything to avoid unexpected failures in cases where
    // main finishes before all thre other threads.
    //
    // free(profiled_threads);
    pthread_mutex_unlock(&profiled_threads_mutex);
    // pthread_mutex_destroy(&profiled_threads_mutex);
}
//------------------------------------------------------------------------------
static thread_profiler_t* setup_thread(pid_t thread_id, thread_profiler_t *t_p)
{
    t_p->thread_id = thread_id;
    t_p->thread_self = pthread_self();
    char csv_path[1024];
    pid_t pid = getpid();
    snprintf(csv_path, 1024, PROFILER_DATA_DIR"/profiler-%s-%d-%d-XXXXXX.csv",
        program_invocation_short_name, getpid(), thread_id);
    mkstemps(csv_path, 4);
    t_p->csv = fopen(csv_path, "w");
    return t_p;
}
//------------------------------------------------------------------------------
static void teardown_thread(thread_profiler_t *t_p)
{
    fflush(t_p->csv);
    fclose(t_p->csv);
    memset(t_p, 0, sizeof(thread_profiler_t));
}
//------------------------------------------------------------------------------
static thread_profiler_t* find_thread_context(void)
{
    pid_t thread_id = gettid();
    thread_profiler_t *t_p_i = &(profiled_threads[0]);
    size_t t_i = 0;
    while(t_i < number_active_profiled_threads) {
        if(thread_id == t_p_i->thread_id) {
            if (pthread_equal(t_p_i->thread_self, pthread_self())) {
                return t_p_i;
            } else {
                // Just in case.
                // The thread_id is out of sync with the thread_self.
                // This is not expected.
                teardown_thread(t_p_i);
                return setup_thread(thread_id, t_p_i);
            }
        }
        t_i ++;
        t_p_i ++;
    }
    if (t_i < profiled_threads_length) {
        // New thread added at the end of the list.
        number_active_profiled_threads ++;
        return setup_thread(thread_id, t_p_i);
    }
    // No free space.
    // Tries to find unused slots by activity (best guess).
    t_p_i = &(profiled_threads[0]);
    t_i = 0;
    while(t_i < number_active_profiled_threads) {
        if (EBUSY != pthread_tryjoin_np(t_p_i->thread_self, NULL)) {
            // Found a slot for a thread which is finished.
            // Tear down the old finished thread and reuse the slot.
            teardown_thread(t_p_i);
            //
            // Here would be one of the points to check for too many non
            // adjacent slots. The worst case would be a slot with index
            // 1 being reused, all slots between index 2 and index
            // (number_active_profiled_threads - 2) being idle (threads
            // finished) and the slot with index
            // (number_active_profiled_threads - 1) with an active thread.
            //
            // It could also be that by some unfortunate coincidence the
            // first slot always end up being used by an ephemeral thread
            // and each time the ephemeral thread finishes all the list is
            // scanned in search for non contiguous slots.
            // That would incur in large overhead for that ephemeral thread.
            //
            // As of now the removal of non adjacencies is considered a
            // premature optimization.
            //
            return setup_thread(thread_id, t_p_i);
        }
        t_i ++;
        t_p_i ++;
    }
    // List is full.
    // Grows the list.
    pthread_mutex_lock(&profiled_threads_mutex);
    {
        profiled_threads_length += PROFILE_LIST_INCREASE;
        profiled_threads = reallocarray(profiled_threads, profiled_threads_length, sizeof(thread_profiler_t));
    }
    pthread_mutex_unlock(&profiled_threads_mutex);
    number_active_profiled_threads ++;
    return setup_thread(thread_id, &(profiled_threads[number_active_profiled_threads - 1]));
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
//
// The one or two extra pushes of rbp have two purposes:
//   1. keeps the value in rsp as a multiple integer of 16 (so always an even
//      number of push operations). This is because many system library calls
//      require the alignment of rsp as integer multiple of 16. If the rsp is
//      not an integer multiple of 16 a SIGSEGV (Address boundary error) may
//      be issued. Those extra elements are not removed by the counterpart
//      macro ISR_LIKE_RESTORE_STATE. Those extra pushed rbp need to be
//      removed on a case by case basis.
//   2. reserve an extra slot in the stack to store the jump address. All the
//      jumps are done through instruction ret to avoid touching any register
//      after restoring the register and before doing the jump (the ret instruction
//      allows to store the destination address in the stack).
//
#define ISR_LIKE_SIZE_WITH_EXTRA (16*8)
#define ISR_LIKE_SIZE_WITHOUT_EXTRA (14*8)
#define ISR_LIKE_DISCARD_EXTRA_1 "\tadd $0x8,%%rsp\n"
#define ISR_LIKE_DISCARD_EXTRA_2 "\tadd $0x10,%%rsp\n"
#define ISR_LIKE_POP_EXTRA \
    "\tpop %%rbp\n" \
    "\tpop %%rbp\n"
#define ISR_LIKE_SAVE_STATE \
    asm volatile ( \
        "\tpush %%rbp\n" \
        "\tpush %%rbp\n" \
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
        "\tpush %%r12\n" \
        "\tpush %%r13\n" \
        "\tpush %%r14\n" \
        "\tpush %%r15\n" \
        : \
    )
//
// Undo the ISR_LIKE_SAVE_STATE plus:
//
#define ISR_LIKE_RESTORE_STATE \
    asm volatile ( \
        "\tpop %%r15\n" \
        "\tpop %%r14\n" \
        "\tpop %%r13\n" \
        "\tpop %%r12\n" \
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
typedef struct {
    intercepted_function_t *i_f;
    void *n_t_f_t; // this contains the address of the local implementation ("not the function target")
} dl_iterate_phdr_callback_data_t;
int dl_iterate_phdr_callback(struct dl_phdr_info *info, size_t size, void *data)
{
    int result = 0;
    dl_iterate_phdr_callback_data_t *d_i_p_c_d = (dl_iterate_phdr_callback_data_t *)data;
    void *candidate_lib_handler = dlopen(info->dlpi_name, RTLD_NOW);
    if (NULL != candidate_lib_handler) {
        void *candidate_func_ptr = dlsym(candidate_lib_handler, d_i_p_c_d->i_f->function_name);
        if (NULL != candidate_func_ptr) {
            if (d_i_p_c_d->n_t_f_t != candidate_func_ptr) {
                //
                // Found a runtime loaded library which implements the
                // intercepted function.
                //
                d_i_p_c_d->i_f->function_target = candidate_func_ptr;
                result = 1;
            }
        }
        dlclose(candidate_lib_handler);
    }
    return result;
}
void profiler_epilogue();
static void profiler_prologue(int function_index)
{
    //
    // This function runs in the context of the callee.
    // It has two stack frames though.
    // The first stack frame is of the function which is intercepting the
    // actual target function.
    // The second stack frame if for this function.
    // Both stack frames shall be discarded before jumping to the actual
    // target function.
    //
    thread_profiler_t *p_t = find_thread_context();
    intercepted_function_t *i_f = &(intercepted_functions[function_index - 1]);
    if(NULL == i_f->function_target) {
        //
        #ifdef ENFORCE_STACK_POINTER_ALIGNMENT
        // Ensures RSP is 16 byte aligned.
        // It was seen in practice that if the content of RSP is not an
        // integer multiple of 16 before calling dl_iterate_phdr, a "SIGSEGV"
        // (address boundary error) excetption is thrown.
        //
        int restore_rsp = 0;
        asm volatile goto (
            "\ttest $0xF,%%rsp\n"
            "\tjz %l[rsp_content_is_integer_multiple_of_16]\n"
            "\tpush $0x0\n"
            :
            :
            :
            : rsp_content_is_integer_multiple_of_16
        );
        restore_rsp = 1;
        #endif
        rsp_content_is_integer_multiple_of_16:;
        dl_iterate_phdr_callback_data_t d_i_p_c_d;
        d_i_p_c_d.i_f = i_f;
        d_i_p_c_d.n_t_f_t = dlsym(RTLD_DEFAULT, i_f->function_name);
        dl_iterate_phdr(dl_iterate_phdr_callback, &d_i_p_c_d);
        #ifdef ENFORCE_STACK_POINTER_ALIGNMENT
        if(1 == restore_rsp) {
            asm volatile (
                "\tpop %%rax\n"
                :
            );
        }
        #endif
    }
    //
    // Checks if there is no other active call.
    // 
    if (0 != p_t->function_index) {
        //
        // Reentrant call.
        // It may be that the caller doesn't care about rbp.
        // It may be that the caler does care about rbp.
        // So the original rbp must be restored before calling the actual
        // function.
        //
        void *r0 = i_f->function_target;
        asm volatile (
            "\tmov (%%rbp),%%rbx\n" // Top of the stack on intercepted function entry.
            "\tmov (%%rbx),%%rcx\n"
            "\tmov %%rcx,-8(%%rbx)\n"   // Moves the saved rbp from the caller.
            "\tmov %0,(%%rbx)\n"    // Sets the address to jump as the actuall invoked function.
                                    // This will allow the use of ret do call the original function.
                                    // Notice that the space being used here is one of the extras.
            :
            : "r" (r0)
        );
        asm volatile (
            "\tmov %%rbp,%%rsp\n"   // Unwinds the stack to the what the actual function would
                                    // see if the actuall function was invoked directly.
            "\tsub %0,%%rsp\n"      // Positions in the point where the backup of the registers are.
            :
            : "n" (ISR_LIKE_SIZE_WITH_EXTRA)
        );
        ISR_LIKE_RESTORE_STATE;     // Restore the register values
        asm volatile (
            ISR_LIKE_DISCARD_EXTRA_1
            "\tpop %%rbp\n"
            "\tret\n"               // A jmp disguised as a ret.
                                    // This goes to the original function.
            :
        );
    }
    //
    // This thread context is "locked" (reentrant call detection mechanism).
    //
    p_t->function_index = function_index;
    //
    // 1. Saves the actual return address.
    //    The saved value will later be used in profiler_epilogue.
    //    Notice that there is no need of a mutex because the context is per thread.
    // 2. Replaces the return address with profiler_epilogue.
    // 3. Jumps to the actual function using ret.
    //    The usage if ret, although misleading, allows the jump without the need
    //    to touch any register before the jump.
    //
    void *r0;
    void *r1;
    asm volatile (
        "\tmov (%%rbp),%%rbx\n"
        "\tmov 8(%%rbx),%0\n"   // Saves original call return address.
        : "=r" (r0)
    );
    p_t->actual_return_address = r0;
    r0 = profiler_epilogue;
    r1 = i_f->function_target;
    asm volatile (
        "\tmov (%%rbp),%%rbx\n"
        "\tmov %0,8(%%rbx)\n"   // Sets new return address to profiler_epilogue.
        "\tmov (%%rbx),%%rcx\n" // Moves the pushed rbp one position down.
        "\tmov %%rcx,-8(%%rbx)\n"
        "\tmov %1,(%%rbx)\n"    // Sets destination to the actuall function being intercepted
                                // (uses ret to jump to it). Overwrites originally saved rbp.
        :
        : "r" (r0)
        , "r" (r1)
    );
    clock_gettime(CLOCK_REALTIME, &(p_t->last_entry_ts));
    asm volatile (
        "\tmov (%%rbp),%%rsp\n" // Unwinds the stack to the what the actual function would
                                // see if the actual function was invoked directly.
        "\tsub %0,%%rsp\n"      // Positions in the point where the backup of the registers are.
        :
        : "n" (ISR_LIKE_SIZE_WITH_EXTRA)
    );
    ISR_LIKE_RESTORE_STATE;     // Restore the register values
    asm volatile (
        ISR_LIKE_DISCARD_EXTRA_1
        "\npop %%rbp\n"
        "\tret\n"               // A jmp disguised as a ret.
                                // This goes to the original function.
        :
    );
#if 0
    1. main calls object_a->usec_sleep_inline
        rsp -= 8;
        *rsp = next line in main
    2. _ZNK17lib_a_cpp_class_b17usec_sleep_inlineEm
        rsp -= 8;
        *rsp = rbp;
        rbp = rsp;
        rsp -= 8 x 14; // isr save state
        rsp -= 8;
        *rsp = next line in _ZNK17lib_a_cpp_class_b17usec_sleep_inlineEm
    3. profiler_prologue
        rsp -= 8;
        *rsp = rbp;
        rbp = rsp; //

rbp     *rbp
0 +0      rsp on entry in profiler_prologue
1 +8      return address to _ZNK17lib_a_cpp_class_b17usec_sleep_inlineEm
2 +16     backup r15
3 +24     backup r14
4 +32     backup r13
5 +40     backup r12
6 +48     backup r11
7 +56     backup r10
8 +64     backup r9
9 +72     backup r8
10 +80     backup rax
11 +88     backup rbx
12 +96     backup rcx
13 +104    backup rdx
14 +112    backup rsi
15 +120    backup rdi
16 +128    return address to next line in main
17 +136    rsp on entry in 

disassemble *(uint64_t*)($rbp+8) - ok (_ZNK17lib_a_cpp_class_b17usec_sleep_inlineEm)
disassemble *(uint64_t*)($rbp+128) - ok (main)
disassemble *(uint64_t*)(*(uint64_t*)($rbp)) - ok??? (main)

disassemble *(uint64_t*)(*(uint64_t*)($rbp) + 8) -ok (main)
#endif
}
//------------------------------------------------------------------------------
#define ISR_LIKE_SAVE_STATE_EPILOGUE \
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
        "\tpush %%r12\n" \
        "\tpush %%r13\n" \
        "\tpush %%r14\n" \
        "\tpush %%r15\n" \
        : \
    )
#define ISR_LIKE_RESTORE_STATE_EPILOGUE \
    asm volatile ( \
        "\tpop %%r15\n" \
        "\tpop %%r14\n" \
        "\tpop %%r13\n" \
        "\tpop %%r12\n" \
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
void profiler_epilogue()
{
    //
    // This is as if it runs in the context of the caller.
    // Reaches here through a return instruction.
    // Still has the frame pointer decoration.
    // RSP diverges from RBP because of the local variables.
    // The restore sequence differs from profiler_prologue.
    //
    ISR_LIKE_SAVE_STATE;
    struct timespec last_exit_ts;
    clock_gettime(CLOCK_REALTIME, &last_exit_ts);
    thread_profiler_t *p_t = find_thread_context();
    fprintf(p_t->csv, "%d,%lu%09lu,%lu%09lu\n",
        p_t->function_index,
        p_t->last_entry_ts.tv_sec, p_t->last_entry_ts.tv_nsec,
        last_exit_ts.tv_sec, last_exit_ts.tv_nsec);
    fflush(p_t->csv);
    auto r0 = p_t->actual_return_address;
    asm volatile (
        "\tmov %0,8(%%rbp)\n"   // Restores original call return address.
        :
        : "p" (r0)
    );
    ISR_LIKE_RESTORE_STATE;     // Restore the register values
    asm volatile (
        "\tret\n"               // A jmp disguised as a ret.
                                // This goes to the original function.
        :
    );
}

//------------------------------------------------------------------------------
#define JUMP_TO_GENERIC_INTERCEPTOR(function_index) \
    ISR_LIKE_SAVE_STATE; \
    profiler_prologue(function_index)
//------------------------------------------------------------------------------
void lib_a_func_a()
{
    JUMP_TO_GENERIC_INTERCEPTOR(1);
}
//------------------------------------------------------------------------------
void lib_a_func_b()
{
    JUMP_TO_GENERIC_INTERCEPTOR(2);
}
//------------------------------------------------------------------------------
void lib_a_func_c()
{
    JUMP_TO_GENERIC_INTERCEPTOR(3);
}
//------------------------------------------------------------------------------
void lib_a_func_d()
{
    JUMP_TO_GENERIC_INTERCEPTOR(4);
}
//------------------------------------------------------------------------------
void _ZNK17lib_a_cpp_class_b17usec_sleep_inlineEm()
{
    JUMP_TO_GENERIC_INTERCEPTOR(5);
}
//------------------------------------------------------------------------------
void _ZNK17lib_a_cpp_class_b20usec_sleep_reentrantEm()
{
    JUMP_TO_GENERIC_INTERCEPTOR(6);
}
//------------------------------------------------------------------------------
void lib_c_func_a()
{
    JUMP_TO_GENERIC_INTERCEPTOR(7);
}
//------------------------------------------------------------------------------
