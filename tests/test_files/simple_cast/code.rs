fn add(a: i64, b: f64) -> f64 {
  a as f64 * 42.42 + b / 239.0
}

fn compute(a: i32, b: f32) -> f32 {
  add(a as i64, b as f64) as f32 + 160.0
}

fn strange_func() -> f32 {
  compute(42, add(239, 146.67) as f32)
}

fn main() {
  print_i32(add(5, 3.0) as i32);
  print_u64(strange_func() as u64);
}
