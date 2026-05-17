; ModuleID = 'tests/test_files/simple/code.rs'
source_filename = "tests/test_files/simple/code.rs"

@x = global i64 7

declare void @print_i8(i8)

declare void @print_i16(i16)

declare void @print_i32(i32)

declare void @print_i64(i64)

declare void @print_f32(float)

declare void @print_f64(double)

declare void @print_bool(i1)

declare void @print_char(i8)

declare void @print_u8(i8)

declare void @print_u16(i16)

declare void @print_u32(i32)

declare void @print_u64(i64)

declare void @print_void()

define {} @main() {
entry:
  %0 = load i64, ptr @x, align 4
  call void @print_i64(i64 %0)
  ret {} undef
}
