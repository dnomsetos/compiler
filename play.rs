fn add(x: &mut i32, y: &i32) -> i32 {
    *x + *y
}

fn main() {
    let mut x = 42;
    let r = &x;
    let mut r1 = r;
    let mut r2 = &mut x;
    let mut y = 239;
    r1 = &y;
    x = *r1 + *r + *r2;
    
    if x == 239 {
        r2 = &mut y;
    } else {
        r1 = &x;
    }

    x = add(r2, r);
}
