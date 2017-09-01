extern crate intlen;

// this is just a template
fn main() {
    let mut len_of_int = 0;

    unsafe {
        len_of_int = intlen::intlen(21299);
    }

    println!("{}", len_of_int);
}
