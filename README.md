# mishmash

## generate_password.bash

Simple `bash` script to generate passwords using `dd`, `/dev/urandom`, `dc`, `printf`, `echo -e`, `bc`, `hexdump` and some other tricks.

The core principle is to convert a random binary number to representation in base 96, then adding 32 (the ASCII value of white space) and then finally converting back the the whole range between ASCII 32 to ASCII 127 to reach the final password.


## expenses_fixed_grocery_others.fish

Process a csv data file as returned from [Rabobank](https://www.rabobank.nl/particulieren) with information about financial transactions for a regular banking acount.

The output is a table with one row for every month:
    1. month in format yyyy-mm.
    2. fixed expenses (in the csv, the records with field `Code` equals to `ei` and some other personal expenses based on certain values of column `Naam tegenpartij`).
    3. grocery expenses, based on certain values of column `Naam tegenpartij`.
    4. cash withdrawals (in the csv, the records with field `Code` equals to `ga`)
    5. the remainder expenses that did not fit the three above.
    6. the earnings
    7. the balance (all earnings minus all expenses)

Notice that the backend csv exporter is not robust and then some records containing double quotes in the `Naam tegenpartij` column had to be corrected.

During initial stages the meaning of field `Code` was unknown, and then an algorithm to guess the recurrent expenses (as a proxy for the fixed expenses) was developed.
The algorithm is still available (2 different versions) in commented code sections.

This is not a "clean code", in the sense that many of the struggles are present as commented code and/or undocumented branches (`main_0` and `main_2`).

Uses [fish shell](https://fishshell.com/) and [SQLite](https://sqlite.org/).
