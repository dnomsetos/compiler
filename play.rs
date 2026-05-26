fn add(x: &mut i32, y: &i32) -> i32 {
    *x + *y
}

fn main() {
    let mut x = 42;
    let r = &mut x;
    if *r > 10 {
        *r = 239;
    } else {
        x = 67;
        *r = 34;
    }
    print_i32(x);
}
