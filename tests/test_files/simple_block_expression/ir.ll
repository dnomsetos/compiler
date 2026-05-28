; ModuleID = 'tests/test_files/simple_block_expression/code.rs'
source_filename = "tests/test_files/simple_block_expression/code.rs"

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

define i32 @compute(i32 %0, i32 %1) {
entry:
  %a = alloca i32, align 4
  store i32 %0, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 %1, ptr %b, align 4
  %result = alloca i32, align 4
  %x = alloca i32, align 4
  %2 = load i32, ptr %a, align 4
  %3 = load i32, ptr %b, align 4
  %4 = add i32 %2, %3
  store i32 %4, ptr %x, align 4
  %y = alloca i32, align 4
  %5 = load i32, ptr %x, align 4
  %6 = mul i32 %5, 2
  store i32 %6, ptr %y, align 4
  %if.result = alloca i32, align 4
  %7 = load i32, ptr %y, align 4
  %8 = icmp sgt i32 %7, 10
  br i1 %8, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %9 = load i32, ptr %y, align 4
  %10 = add i32 %9, 1
  store i32 %10, ptr %if.result, align 4
  br label %if.merge

if.else:                                          ; preds = %entry
  %11 = load i32, ptr %y, align 4
  %12 = sub i32 %11, 1
  store i32 %12, ptr %if.result, align 4
  br label %if.merge

if.merge:                                         ; preds = %if.else, %if.then
  %13 = load i32, ptr %if.result, align 4
  store i32 %13, ptr %result, align 4
  %14 = load i32, ptr %result, align 4
  ret i32 %14
}

define {} @main() {
entry:
  %0 = call i32 @compute(i32 42, i32 239)
  call void @print_u32(i32 %0)
  ret {} undef
}
