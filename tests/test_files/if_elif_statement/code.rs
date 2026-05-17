fn func(x: mut u64) -> u64 {
  if x == 0 {
    x = 10;
  } else {
    x = x - 42;
  }

  if x > 10 {
    let a = x;
    a
  } else if x > 5 {
    let b = x * 2;
    b
  } else {
    let c = x - 1;
    print_u64(c);
    c
  }
}

fn main() {
  print_i32(func(42) as i32);
}
