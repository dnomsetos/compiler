fn compute(a: i32, b: i32) -> i32 {
    let result = {
        let x = a + b;
        let y = x * 2;

        if y > 10 {
            y + 1
        } else {
            y - 1
        }
    };

    result
}

fn main() {
  print_u32(compute(42, 239) as u32);
}
