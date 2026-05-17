; ModuleID = 'tests/test_files/strange_situation/code.rs'
source_filename = "tests/test_files/strange_situation/code.rs"

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
  %x = alloca i64, align 4
  store i64 42, ptr %x, align 4
  %y = alloca i64, align 4
  store i64 146, ptr %y, align 4
  %z = alloca i64, align 4
  %0 = load i64, ptr %x, align 4
  %1 = load i64, ptr %y, align 4
  %2 = add i64 %0, %1
  store i64 %2, ptr %z, align 4
  %3 = load i64, ptr %z, align 4
  call void @print_i64(i64 %3)
  %4 = load i64, ptr %x, align 4
  call void @print_i64(i64 %4)
  ret {} undef
}
