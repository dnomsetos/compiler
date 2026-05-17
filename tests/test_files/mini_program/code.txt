fn func(a: i32, b: i32) -> u64 {
    let sum = (a + b) as u64;
    let mut scaled = sum * (b as u64);

    if scaled > 100 as u64 {
        scaled = 100 as u64;
    } else {
        loop {
            let tmp = (scaled as i64) - 1;
            break;
        }
    }

    sum + scaled as u64
}

fn main() {
    print_u64(func(42, 239));
}
