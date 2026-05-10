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

define i32 @find_positive(i32 %0, i32 %1, i32 %2) {
entry:
  %a = alloca i32, align 4
  store i32 %0, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 %1, ptr %b, align 4
  %c = alloca i32, align 4
  store i32 %2, ptr %c, align 4
  %"0i" = alloca i32, align 4
  store i32 0, ptr %"0i", align 4
  %"0result" = alloca i32, align 4
  %result = alloca i32, align 4
  br label %loop.body

loop.body:                                        ; preds = %entry
  %if.result = alloca {}, align 1
  %3 = load i32, ptr %"0i", align 4
  %4 = icmp eq i32 %3, 0
  br i1 %4, label %if.then, label %if.else

loop.finish:                                      ; preds = %if.merge16, %if.then18, %if.then10, %if.then2
  %5 = load i32, ptr %result, align 4
  store i32 %5, ptr %"0result", align 4
  %6 = load i32, ptr %"0result", align 4
  ret i32 %6

if.then:                                          ; preds = %loop.body
  %if.result1 = alloca {}, align 1
  %7 = load i32, ptr %a, align 4
  %8 = icmp sgt i32 %7, 0
  br i1 %8, label %if.then2, label %if.else3

if.else:                                          ; preds = %loop.body
  store {} zeroinitializer, ptr %if.result, align 1
  br label %if.merge

if.merge:                                         ; preds = %if.else, %if.merge4
  %9 = load {}, ptr %if.result, align 1
  %if.result5 = alloca {}, align 1
  %10 = load i32, ptr %"0i", align 4
  %11 = icmp eq i32 %10, 1
  br i1 %11, label %if.then6, label %if.else7

if.then2:                                         ; preds = %if.then
  %12 = load i32, ptr %a, align 4
  store i32 %12, ptr %result, align 4
  br label %loop.finish

if.else3:                                         ; preds = %if.then
  store {} zeroinitializer, ptr %if.result1, align 1
  br label %if.merge4

if.merge4:                                        ; preds = %if.else3
  %13 = load {}, ptr %if.result1, align 1
  store {} %13, ptr %if.result, align 1
  br label %if.merge

if.then6:                                         ; preds = %if.merge
  %if.result9 = alloca {}, align 1
  %14 = load i32, ptr %b, align 4
  %15 = icmp sgt i32 %14, 0
  br i1 %15, label %if.then10, label %if.else11

if.else7:                                         ; preds = %if.merge
  store {} zeroinitializer, ptr %if.result5, align 1
  br label %if.merge8

if.merge8:                                        ; preds = %if.else7, %if.merge12
  %16 = load {}, ptr %if.result5, align 1
  %if.result13 = alloca {}, align 1
  %17 = load i32, ptr %"0i", align 4
  %18 = icmp eq i32 %17, 2
  br i1 %18, label %if.then14, label %if.else15

if.then10:                                        ; preds = %if.then6
  %19 = load i32, ptr %b, align 4
  store i32 %19, ptr %result, align 4
  br label %loop.finish

if.else11:                                        ; preds = %if.then6
  store {} zeroinitializer, ptr %if.result9, align 1
  br label %if.merge12

if.merge12:                                       ; preds = %if.else11
  %20 = load {}, ptr %if.result9, align 1
  store {} %20, ptr %if.result5, align 1
  br label %if.merge8

if.then14:                                        ; preds = %if.merge8
  %if.result17 = alloca {}, align 1
  %21 = load i32, ptr %c, align 4
  %22 = icmp sgt i32 %21, 0
  br i1 %22, label %if.then18, label %if.else19

if.else15:                                        ; preds = %if.merge8
  store {} zeroinitializer, ptr %if.result13, align 1
  br label %if.merge16

if.merge16:                                       ; preds = %if.else15, %if.merge20
  %23 = load {}, ptr %if.result13, align 1
  store i32 0, ptr %result, align 4
  br label %loop.finish

if.then18:                                        ; preds = %if.then14
  %24 = load i32, ptr %c, align 4
  store i32 %24, ptr %result, align 4
  br label %loop.finish

if.else19:                                        ; preds = %if.then14
  store {} zeroinitializer, ptr %if.result17, align 1
  br label %if.merge20

if.merge20:                                       ; preds = %if.else19
  %25 = load {}, ptr %if.result17, align 1
  store {} %25, ptr %if.result13, align 1
  br label %if.merge16
}

define {} @main() {
entry:
  %0 = call i32 @find_positive(i32 -42, i32 -93, i32 0)
  call void @print_i32(i32 %0)
  ret {} undef
}
