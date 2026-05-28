; ModuleID = 'tests/test_files/loop_labels/code.rs'
source_filename = "tests/test_files/loop_labels/code.rs"

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

define i32 @labeled_loop_block_value(i32 %0, i32 %1) {
entry:
  %a = alloca i32, align 4
  store i32 %0, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 %1, ptr %b, align 4
  %result = alloca i32, align 4
  %result1 = alloca i32, align 4
  br label %loop.body

loop.body:                                        ; preds = %entry
  %x = alloca i32, align 4
  %2 = load i32, ptr %a, align 4
  %3 = load i32, ptr %b, align 4
  %4 = add i32 %2, %3
  store i32 %4, ptr %x, align 4
  %y = alloca i32, align 4
  %t = alloca i32, align 4
  %5 = load i32, ptr %x, align 4
  %6 = mul i32 %5, 2
  store i32 %6, ptr %t, align 4
  %7 = load i32, ptr %t, align 4
  %8 = add i32 %7, 1
  store i32 %8, ptr %y, align 4
  %if.result = alloca {}, align 1
  %9 = load i32, ptr %y, align 4
  %10 = icmp sgt i32 %9, 10
  br i1 %10, label %if.then, label %if.else

loop.finish:                                      ; preds = %loop.finish4, %if.then6, %if.then
  %11 = load i32, ptr %result1, align 4
  store i32 %11, ptr %result, align 4
  %12 = load i32, ptr %result, align 4
  ret i32 %12

if.then:                                          ; preds = %loop.body
  %r = alloca i32, align 4
  %13 = load i32, ptr %y, align 4
  %14 = sub i32 %13, 3
  store i32 %14, ptr %r, align 4
  %15 = load i32, ptr %r, align 4
  store i32 %15, ptr %result1, align 4
  br label %loop.finish

if.else:                                          ; preds = %loop.body
  store {} zeroinitializer, ptr %if.result, align 1
  br label %if.merge

if.merge:                                         ; preds = %if.else
  %16 = load {}, ptr %if.result, align 1
  %z = alloca i32, align 4
  %result2 = alloca i32, align 4
  br label %loop.body3

loop.body3:                                       ; preds = %if.merge
  %if.result5 = alloca {}, align 1
  %17 = load i32, ptr %x, align 4
  %18 = icmp sgt i32 %17, 0
  br i1 %18, label %if.then6, label %if.else7

loop.finish4:                                     ; preds = %if.merge8
  %19 = load i32, ptr %result2, align 4
  store i32 %19, ptr %z, align 4
  %fallback = alloca i32, align 4
  store i32 0, ptr %fallback, align 4
  %20 = load i32, ptr %fallback, align 4
  store i32 %20, ptr %result1, align 4
  br label %loop.finish

if.then6:                                         ; preds = %loop.body3
  %s = alloca i32, align 4
  %21 = load i32, ptr %x, align 4
  %22 = load i32, ptr %y, align 4
  %23 = add i32 %21, %22
  store i32 %23, ptr %s, align 4
  %q = alloca i32, align 4
  %24 = load i32, ptr %s, align 4
  %25 = mul i32 %24, 2
  store i32 %25, ptr %q, align 4
  %26 = load i32, ptr %q, align 4
  store i32 %26, ptr %result1, align 4
  br label %loop.finish

if.else7:                                         ; preds = %loop.body3
  store {} zeroinitializer, ptr %if.result5, align 1
  br label %if.merge8

if.merge8:                                        ; preds = %if.else7
  %27 = load {}, ptr %if.result5, align 1
  %k = alloca i32, align 4
  store i32 1, ptr %k, align 4
  %28 = load i32, ptr %k, align 4
  store i32 %28, ptr %result2, align 4
  br label %loop.finish4
}

define {} @main() {
entry:
  %0 = call i32 @labeled_loop_block_value(i32 42, i32 239)
  call void @print_i32(i32 %0)
  ret {} undef
}
