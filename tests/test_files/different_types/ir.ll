; ModuleID = 'tests/test_files/different_types/code.txt'
source_filename = "tests/test_files/different_types/code.txt"

@a = global i8 1
@b = global i16 2
@c = global i32 3
@d = global i64 4
@e = global i8 5
@f = global i16 6
@g = global i32 7
@h = global i64 8
@i = global float 9.000000e+00
@j = global double 1.000000e+01
@k = global i1 true
@l = global i8 97

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

define i8 @aboba(i32 %0, i1 %1, i64 %2, i8 %3) {
entry:
  %a = alloca i32, align 4
  store i32 %0, ptr %a, align 4
  %b = alloca i1, align 1
  store i1 %1, ptr %b, align 1
  %c = alloca i64, align 4
  store i64 %2, ptr %c, align 4
  %d = alloca i8, align 1
  store i8 %3, ptr %d, align 1
  call void @print_u8(i8 42)
  %4 = load i32, ptr %a, align 4
  %5 = trunc i32 %4 to i8
  %6 = load i64, ptr %c, align 4
  %7 = trunc i64 %6 to i8
  %8 = add i8 %5, %7
  ret i8 %8
}

define {} @main() {
entry:
  call void @print_char(i8 99)
  %0 = call i8 @aboba(i32 1, i1 true, i64 2, i8 97)
  %1 = zext i8 %0 to i16
  call void @print_u16(i16 %1)
  ret {} undef
}
