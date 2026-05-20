fn find_positive(a: i32, b: i32, c: i32) -> i32 {
    let i = 0;

    let result = loop {
        if i == 0 {
            if a > 0 {
                break a;
            }
        }

        if i == 1 {
            if b > 0 {
                break b;
            }
        }

        if i == 2 {
            if c > 0 {
                break c;
            }
        }

        break 0;
    };

    result
}

fn main() {
    print_i32(find_positive(-42, 146 - 239, 0));
}
