fn test(a: i32) -> u32 {
    let y = if a > 0 {
        let x = a;
        x
    } else {
        let x = -a;
        x
    };
    print_i32(y - 43);
    y as u32
}

fn main() {
    print_u64(test(-42) as u64);
}
