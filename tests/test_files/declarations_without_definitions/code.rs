fn main() {
  let mut x;
  let mut z;
  let y = 34;

  if y > 10 {
    x = 10;
  } else {
    x = y;
  }

  z = x as i8 + y;

  print_i8(z);
  print_i8(y);
  print_i8(x);
}
