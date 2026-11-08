#!/usr/bin/env fish

pushd (path dirname (readlink -f (status filename)))
set dest_dir ~/bin/lib_a_bin
test -d $dest_dir
    or mkdir $dest_dir
echo A
gcc -O0 -ggdb -fPIC -shared lib_a.c -o $dest_dir/liblib_a.so
echo B
gcc -O0 -ggdb main.c -o $dest_dir/main -llib_a -L$dest_dir
echo C
gcc -O0 -ggdb -fPIC -shared lib_a_ld_preload.c -o $dest_dir/liblib_a_ld_preload.so
echo D
popd
