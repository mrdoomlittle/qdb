extern crate getdigit;
use std::env;
fn main() {
    let args: Vec<_> = env::args().collect();
    if args.len() < 3 {
        println!("need's 2 arguments to work.");	
        return;
    }

    let num = args[1].parse::<getdigit::uint_t>().unwrap();
    let unit = args[2].parse::<usize>().unwrap();

    let digit;
    unsafe {
        digit = getdigit::getdigit(num,unit);
    }

    println!("{}", digit)
}
