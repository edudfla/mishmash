#!/usr/bin/env sh
x=$(mktemp) || { echo "Failed to create temporary file." >& 2; exit 1; }
trap "rm -f $x" EXIT INT QUIT TERM || { echo "Failed to trap the removal of temporary file." >& 2; rm -f $x; exit 1; }
s=$(dirname $(readlink -f "${0}"))/ascii_table.rs || { echo "Failed to calculate the path to ascii_table.rs." >& 2; exit 1; }
test -f $s -a -r $f || { echo "\"$s\" is not a file, does not exist or is not accessible." >& 2; exit 1; }
rustc -o $x $(dirname $(readlink -f "${0}"))/ascii_table.rs || { echo "Failed to build ascii_table.rs." >& 2; exit 1; }
$x
