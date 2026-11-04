# mishmash
## generate_password.bash

Simple `bash` script to generate passwords using `dd`, `/dev/urandom`, `dc`, `printf`, `echo -e`, `bc`, `hexdump` and some other tricks.
The core principle is to convert a random binary number to representation in base 96, then adding 32 (the ASCII value of white space) and then finally converting back the the whole range between ASCII 32 to ASCII 127 to reach the final password.
