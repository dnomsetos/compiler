fn helper(a: i32, b: i32) -> i32 {
    let result = {
        let x = a + b;

        let y = {
            let z = x * 3;

            loop {
                if z > 50 {
                    break z - 10;
                }

                break z + 10;
            }
        };

        {
            let final = y * 2;

            if final > 100 {
                final
            } else {
                final + 1
            }
        }
    };

    result
}

fn main() {
    print_u64(helper(146, 67) as u64);
}
