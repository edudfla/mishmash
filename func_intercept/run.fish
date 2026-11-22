#!/usr/bin/env fish

set bin_dir $HOME/bin/lib_a_bin
set -lx LD_LIBRARY_PATH "$bin_dir:$LD_LIBRARY_PATH"
set -lx PATH "$bin_dir:$PATH"
# set -lx LD_PRELOAD $bin_dir/liblib_a_ld_preload.so
# main_runtime_dlopen $argv # to test: begin; set -l v_outer (./run.fish 10); set -l f (ls -t /tmp/lib_a_ld_preload-*)[1]; set -l i 1; while read -d , -a v_inner; math $v_outer[$i] - \($v_inner[3] - $v_inner[2]\); set i (math $i + 1); end < $f; rm -f -r -v /tmp/lib_a_ld_preload-*; end

set -lx LD_PRELOAD $bin_dir/libprofiler.so
main_cpp
exit

set -l gdb_commands (mktemp)
trap "rm $gdb_commands" EXIT INT QUIT TERM
echo '
set breakpoint pending on
# set disassemble-next-line on
break profiler.c:find_thread_context
' > $gdb_commands
gdb --command=$gdb_commands --args main_cpp # to test: > begin; ./build.fish; set -l f (mktemp); for i in (seq 10); set -l t 0 0; ./run.fish > $f; set -l r $status; set -l g (ls -t /tmp/lib_a_ld_preload-*-*-*.csv)[1]; while read -d , -a a; set t[1] (math $t[1] + $a[2] - \($a[6] - $a[5]\)); set t[2] (math $t[2] + $a[3] - \($a[9] - $a[8]\)); end < (paste -d , $f (cat $g | paste -s -d ',\n' | psub) | psub); set -l n (cat $f | wc -l); echo $r (math $t[1] / $n) (math $t[2] / $n); end; rm -f $f; end
exit

gdb --command=(echo "
set breakpoint pending on
break lib_a_ld_preloac.c:710
" | psub) --args main_runtime_dlopen
exit


#gdb --args main
#string repeat -n 80 "="
# main
#string repeat -n 80 "="
set -l gdb_commands (mktemp)
trap "rm $gdb_commands" EXIT INT QUIT TERM
echo '
set breakpoint pending on
set disassemble-next-line on
break main.cpp:75
# break lib_a_ld_preload.c:728
break _ZNK17lib_a_cpp_class_b17usec_sleep_inlineEm
display /x $rdi
display /x $rsi
display /x $rdx
display /x $rcx
display /x $rbx
display /x $rax
display /x $r8
display /x $r9
display /x $r10
display /x $r11
' > $gdb_commands
gdb --command=$gdb_commands --args main_cpp
#string repeat -n 80 "="
