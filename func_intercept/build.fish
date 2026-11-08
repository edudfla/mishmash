#!/usr/bin/env fish

pushd (path dirname (readlink -f (status filename)))
set dest_dir ~/bin/lib_a_bin
test -d $dest_dir
    or mkdir $dest_dir
set -l lib_flags -O0 -ggdb -fPIC -shared
set -l exe_flags -O0 -ggdb
# set -l lib_flags -fPIC -shared
# set -l exe_flags 
echo A
gcc $lib_flags lib_a.c -o $dest_dir/liblib_a.so
echo B
gcc $exe_flags main.c -o $dest_dir/main -llib_a -L$dest_dir
echo C
gcc $lib_flags lib_a_ld_preload.c -o $dest_dir/liblib_a_ld_preload.so
echo D
g++ $lib_flags lib_b.cpp -o $dest_dir/liblib_a_cpp.so -llib_a -L$dest_dir
echo E
g++ $exe_flags main.cpp -o $dest_dir/main_cpp -llib_a -llib_a_cpp -L$dest_dir
echo F
popd
