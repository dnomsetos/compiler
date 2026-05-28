; ModuleID = 'tests/test_files/mini_program/code.rs'
source_filename = "tests/test_files/mini_program/code.rs"

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

define i64 @func(i32 %0, i32 %1) {
entry:
  %a = alloca i32, align 4
  store i32 %0, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 %1, ptr %b, align 4
  %sum = alloca i64, align 4
  %2 = load i32, ptr %a, align 4
  %3 = load i32, ptr %b, align 4
  %4 = add i32 %2, %3
  %5 = sext i32 %4 to i64
  store i64 %5, ptr %sum, align 4
  %scaled = alloca i64, align 4
  %6 = load i64, ptr %sum, align 4
  %7 = load i32, ptr %b, align 4
  %8 = sext i32 %7 to i64
  %9 = mul i64 %6, %8
  store i64 %9, ptr %scaled, align 4
  %if.result = alloca {}, align 1
  %10 = load i64, ptr %scaled, align 4
  %11 = icmp ugt i64 %10, 100
  br i1 %11, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i64 100, ptr %scaled, align 4
  store {} zeroinitializer, ptr %if.result, align 1
  br label %if.merge

if.else:                                          ; preds = %entry
  %result = alloca {}, align 1
  br label %loop.body

if.merge:                                         ; preds = %loop.finish, %if.then
  %12 = load {}, ptr %if.result, align 1
  %13 = load i64, ptr %sum, align 4
  %14 = load i64, ptr %scaled, align 4
  %15 = add i64 %13, %14
  ret i64 %15

loop.body:                                        ; preds = %if.else
  %tmp = alloca i64, align 4
  %16 = load i64, ptr %scaled, align 4
  %17 = sub i64 %16, 1
  store i64 %17, ptr %tmp, align 4
  store {} zeroinitializer, ptr %result, align 1
  br label %loop.finish

loop.finish:                                      ; preds = %loop.body
  %18 = load {}, ptr %result, align 1
  store {} %18, ptr %if.result, align 1
  br label %if.merge
}

define {} @main() {
entry:
  %0 = call i64 @func(i32 42, i32 239)
  call void @print_u64(i64 %0)
  ret {} undef
}
