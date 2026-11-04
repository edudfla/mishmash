#!/usr/bin/env rustc
fn main() {
    print!("{:>3}", "");
    for lower_nibble in 0..16 {
        print!(" {:0>2X}", lower_nibble);
    }
    println!("");
    for upper_nibble in 2..8 {
        print!("{:>3X}", upper_nibble * 16);
        for lower_nibble in 0..16 {
            let a_char_ascii: u32 = upper_nibble * 16 + lower_nibble;
            match char::from_u32(a_char_ascii) {
                Some(a_char) => print!("{:>3}", a_char),
                None => (),
            }
        }
        println!();
    }
}
