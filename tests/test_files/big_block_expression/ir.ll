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

define i32 @helper(i32 %0, i32 %1) {
entry:
  %a = alloca i32, align 4
  store i32 %0, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 %1, ptr %b, align 4
  %"0result" = alloca i32, align 4
  %"0x" = alloca i32, align 4
  %2 = load i32, ptr %a, align 4
  %3 = load i32, ptr %b, align 4
  %4 = add i32 %2, %3
  store i32 %4, ptr %"0x", align 4
  %"0y" = alloca i32, align 4
  %"0z" = alloca i32, align 4
  %5 = load i32, ptr %"0x", align 4
  %6 = mul i32 %5, 3
  store i32 %6, ptr %"0z", align 4
  %result = alloca i32, align 4
  br label %loop.body

loop.body:                                        ; preds = %entry
  %if.result = alloca {}, align 1
  %7 = load i32, ptr %"0z", align 4
  %8 = icmp sgt i32 %7, 50
  br i1 %8, label %if.then, label %if.else

loop.finish:                                      ; preds = %if.merge, %if.then
  %9 = load i32, ptr %result, align 4
  store i32 %9, ptr %"0y", align 4
  %"0final" = alloca i32, align 4
  %10 = load i32, ptr %"0y", align 4
  %11 = mul i32 %10, 2
  store i32 %11, ptr %"0final", align 4
  %if.result1 = alloca i32, align 4
  %12 = load i32, ptr %"0final", align 4
  %13 = icmp sgt i32 %12, 100
  br i1 %13, label %if.then2, label %if.else3

if.then:                                          ; preds = %loop.body
  %14 = load i32, ptr %"0z", align 4
  %15 = sub i32 %14, 10
  store i32 %15, ptr %result, align 4
  br label %loop.finish

if.else:                                          ; preds = %loop.body
  store {} zeroinitializer, ptr %if.result, align 1
  br label %if.merge

if.merge:                                         ; preds = %if.else
  %16 = load {}, ptr %if.result, align 1
  %17 = load i32, ptr %"0z", align 4
  %18 = add i32 %17, 10
  store i32 %18, ptr %result, align 4
  br label %loop.finish

if.then2:                                         ; preds = %loop.finish
  %19 = load i32, ptr %"0final", align 4
  store i32 %19, ptr %if.result1, align 4
  br label %if.merge4

if.else3:                                         ; preds = %loop.finish
  %20 = load i32, ptr %"0final", align 4
  %21 = add i32 %20, 1
  store i32 %21, ptr %if.result1, align 4
  br label %if.merge4

if.merge4:                                        ; preds = %if.else3, %if.then2
  %22 = load i32, ptr %if.result1, align 4
  store i32 %22, ptr %"0result", align 4
  %23 = load i32, ptr %"0result", align 4
  ret i32 %23
}

define {} @main() {
entry:
  %0 = call i32 @helper(i32 146, i32 67)
  %1 = sext i32 %0 to i64
  call void @print_u64(i64 %1)
  ret {} undef
}
