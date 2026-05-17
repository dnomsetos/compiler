static a : i8 = 1;
static b : i16 = 2;
static c : i32 = 3;
static d : i64 = 4;
static e : u8 = 5;
static f : u16 = 6;
static g : u32 = 7;
static h : u64 = 8;
static i : f32 = 9.0;
static j : f64 = 10.0;
static k : bool = true;
static l : char = 'a';

fn aboba(a: i32, b: bool, c: u64, d: char) -> u8 {
  print_u8(42);
  a as u8 + c as u8
}

fn main() {
  print_char('c');
  print_u16(aboba(1, true, 2, 'a') as u16);
}

