fn labeled_loop_block_value(a: i32, b: i32) -> i32 {
    let result = 'outer: loop {
        let x = a + b;

        let y = {
            let t = x * 2;
            t + 1
        };
 
        if y > 10 {
            break 'outer {
                let r = y - 3;
                r
            };
        }

        let z = 'inner: loop {
            if x > 0 {
                break 'outer {
                    let s = x + y;
                    {
                        let q = s * 2;
                        q
                    }
                };
            }

            break 'inner {
                let k = 1;
                k
            };
        };

        break 'outer ({
            let fallback = 0;
            fallback
        });
    };

    result
}

fn main() {
    print_i32(labeled_loop_block_value(42, 239));
}
