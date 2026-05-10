; ModuleID = '/code.txt'
source_filename = "/code.txt"

@"0x" = global i64 0

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
  store i64 7, ptr @"0x", align 4
  %0 = load i64, ptr @"0x", align 4
  call void @print_i64(i64 %0)
  ret {} undef
}
