fn outer() {
  fn inner() {
    let x = 42;
    print_i32(x);
  }

  inner();
}
fn main() {
  outer();
}
