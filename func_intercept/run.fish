#!/usr/bin/env fish

set bin_dir $HOME/bin/lib_a_bin
set -lx LD_LIBRARY_PATH "$bin_dir:$LD_LIBRARY_PATH"
set -lx PATH "$bin_dir:$PATH"
set -lx LD_PRELOAD $bin_dir/liblib_a_ld_preload.so
#gdb --args main
string repeat -n 80 "="
main
string repeat -n 80 "="
main_cpp
string repeat -n 80 "="
