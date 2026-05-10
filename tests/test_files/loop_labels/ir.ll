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

define i32 @labeled_loop_block_value(i32 %0, i32 %1) {
entry:
  %a = alloca i32, align 4
  store i32 %0, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 %1, ptr %b, align 4
  %"0result" = alloca i32, align 4
  %result = alloca i32, align 4
  br label %loop.body

loop.body:                                        ; preds = %entry
  %"0x" = alloca i32, align 4
  %2 = load i32, ptr %a, align 4
  %3 = load i32, ptr %b, align 4
  %4 = add i32 %2, %3
  store i32 %4, ptr %"0x", align 4
  %"0y" = alloca i32, align 4
  %"0t" = alloca i32, align 4
  %5 = load i32, ptr %"0x", align 4
  %6 = mul i32 %5, 2
  store i32 %6, ptr %"0t", align 4
  %7 = load i32, ptr %"0t", align 4
  %8 = add i32 %7, 1
  store i32 %8, ptr %"0y", align 4
  %if.result = alloca {}, align 1
  %9 = load i32, ptr %"0y", align 4
  %10 = icmp sgt i32 %9, 10
  br i1 %10, label %if.then, label %if.else

loop.finish:                                      ; preds = %loop.finish3, %if.then5, %if.then
  %11 = load i32, ptr %result, align 4
  store i32 %11, ptr %"0result", align 4
  %12 = load i32, ptr %"0result", align 4
  ret i32 %12

if.then:                                          ; preds = %loop.body
  %"0r" = alloca i32, align 4
  %13 = load i32, ptr %"0y", align 4
  %14 = sub i32 %13, 3
  store i32 %14, ptr %"0r", align 4
  %15 = load i32, ptr %"0r", align 4
  store i32 %15, ptr %result, align 4
  br label %loop.finish

if.else:                                          ; preds = %loop.body
  store {} zeroinitializer, ptr %if.result, align 1
  br label %if.merge

if.merge:                                         ; preds = %if.else
  %16 = load {}, ptr %if.result, align 1
  %"0z" = alloca i32, align 4
  %result1 = alloca i32, align 4
  br label %loop.body2

loop.body2:                                       ; preds = %if.merge
  %if.result4 = alloca {}, align 1
  %17 = load i32, ptr %"0x", align 4
  %18 = icmp sgt i32 %17, 0
  br i1 %18, label %if.then5, label %if.else6

loop.finish3:                                     ; preds = %if.merge7
  %19 = load i32, ptr %result1, align 4
  store i32 %19, ptr %"0z", align 4
  %"0fallback" = alloca i32, align 4
  store i32 0, ptr %"0fallback", align 4
  %20 = load i32, ptr %"0fallback", align 4
  store i32 %20, ptr %result, align 4
  br label %loop.finish

if.then5:                                         ; preds = %loop.body2
  %"0s" = alloca i32, align 4
  %21 = load i32, ptr %"0x", align 4
  %22 = load i32, ptr %"0y", align 4
  %23 = add i32 %21, %22
  store i32 %23, ptr %"0s", align 4
  %"0q" = alloca i32, align 4
  %24 = load i32, ptr %"0s", align 4
  %25 = mul i32 %24, 2
  store i32 %25, ptr %"0q", align 4
  %26 = load i32, ptr %"0q", align 4
  store i32 %26, ptr %result, align 4
  br label %loop.finish

if.else6:                                         ; preds = %loop.body2
  store {} zeroinitializer, ptr %if.result4, align 1
  br label %if.merge7

if.merge7:                                        ; preds = %if.else6
  %27 = load {}, ptr %if.result4, align 1
  %"0k" = alloca i32, align 4
  store i32 1, ptr %"0k", align 4
  %28 = load i32, ptr %"0k", align 4
  store i32 %28, ptr %result1, align 4
  br label %loop.finish3
}

define {} @main() {
entry:
  %0 = call i32 @labeled_loop_block_value(i32 42, i32 239)
  call void @print_i32(i32 %0)
  ret {} undef
}
