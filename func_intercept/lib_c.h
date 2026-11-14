#pragma once
typedef unsigned long (*lib_c_func_a_ptr)(unsigned long usleep_duration);
const char *lib_c_lib_name = "liblib_c.so";
const char *lib_c_func_a_name = "lib_c_func_a";
unsigned long lib_c_func_a(unsigned long usleep_duration);
