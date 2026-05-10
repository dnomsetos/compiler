; ModuleID = '/code.txt'
source_filename = "/code.txt"

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

define i32 @test(i32 %0) {
entry:
  %a = alloca i32, align 4
  store i32 %0, ptr %a, align 4
  %"0y" = alloca i32, align 4
  %if.result = alloca i32, align 4
  %1 = load i32, ptr %a, align 4
  %2 = icmp sgt i32 %1, 0
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %"0x" = alloca i32, align 4
  %3 = load i32, ptr %a, align 4
  store i32 %3, ptr %"0x", align 4
  %4 = load i32, ptr %"0x", align 4
  store i32 %4, ptr %if.result, align 4
  br label %if.merge

if.else:                                          ; preds = %entry
  %"1x" = alloca i32, align 4
  %5 = load i32, ptr %a, align 4
  %6 = sub i32 0, %5
  store i32 %6, ptr %"1x", align 4
  %7 = load i32, ptr %"1x", align 4
  store i32 %7, ptr %if.result, align 4
  br label %if.merge

if.merge:                                         ; preds = %if.else, %if.then
  %8 = load i32, ptr %if.result, align 4
  store i32 %8, ptr %"0y", align 4
  %9 = load i32, ptr %"0y", align 4
  %10 = sub i32 %9, 43
  call void @print_i32(i32 %10)
  %11 = load i32, ptr %"0y", align 4
  ret i32 %11
}

define {} @main() {
entry:
  %0 = call i32 @test(i32 -42)
  %1 = zext i32 %0 to i64
  call void @print_u64(i64 %1)
  ret {} undef
}
