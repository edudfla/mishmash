#!/usr/bin/env bash
password_length=20
bit_length=$(echo "${password_length} * l(95) / l(2) + 1" | bc -l | grep -E -o --color=none -e '^[[:digit:]]+')
bytes_length=$((bit_length / 8))
((bit_length % 8 > 0)) && ((bytes_length++))
hex_password=$(dd if=/dev/urandom ibs=${bytes_length} count=1 2>/dev/null | hexdump -v -e '1/1 "%02X"')
echo ${hex_password} >&2
echo ${#hex_password} >&2
base96_password=$(dc -e "16 i ${hex_password}  A i 95 o p")
ascii_password=''
base96_fragments=(${base96_password})
echo ${#base96_fragments[@]} >&2
for base96_fragment in ${base96_fragments[@]: -$password_length}
do
	ascii_password+=$(printf '\\x%02X'  $((32 + 10#$base96_fragment)))
done
echo -e ">>>>${ascii_password}<<<<"
