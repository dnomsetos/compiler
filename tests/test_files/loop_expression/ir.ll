; ModuleID = 'tests/test_files/loop_expression/code.txt'
source_filename = "tests/test_files/loop_expression/code.txt"

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
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  %result = alloca i32, align 4
  %result1 = alloca i32, align 4
  br label %loop.body

loop.body:                                        ; preds = %entry
  %if.result = alloca {}, align 1
  %3 = load i32, ptr %i, align 4
  %4 = icmp eq i32 %3, 0
  br i1 %4, label %if.then, label %if.else

loop.finish:                                      ; preds = %if.merge17, %if.then19, %if.then11, %if.then3
  %5 = load i32, ptr %result1, align 4
  store i32 %5, ptr %result, align 4
  %6 = load i32, ptr %result, align 4
  ret i32 %6

if.then:                                          ; preds = %loop.body
  %if.result2 = alloca {}, align 1
  %7 = load i32, ptr %a, align 4
  %8 = icmp sgt i32 %7, 0
  br i1 %8, label %if.then3, label %if.else4

if.else:                                          ; preds = %loop.body
  store {} zeroinitializer, ptr %if.result, align 1
  br label %if.merge

if.merge:                                         ; preds = %if.else, %if.merge5
  %9 = load {}, ptr %if.result, align 1
  %if.result6 = alloca {}, align 1
  %10 = load i32, ptr %i, align 4
  %11 = icmp eq i32 %10, 1
  br i1 %11, label %if.then7, label %if.else8

if.then3:                                         ; preds = %if.then
  %12 = load i32, ptr %a, align 4
  store i32 %12, ptr %result1, align 4
  br label %loop.finish

if.else4:                                         ; preds = %if.then
  store {} zeroinitializer, ptr %if.result2, align 1
  br label %if.merge5

if.merge5:                                        ; preds = %if.else4
  %13 = load {}, ptr %if.result2, align 1
  store {} %13, ptr %if.result, align 1
  br label %if.merge

if.then7:                                         ; preds = %if.merge
  %if.result10 = alloca {}, align 1
  %14 = load i32, ptr %b, align 4
  %15 = icmp sgt i32 %14, 0
  br i1 %15, label %if.then11, label %if.else12

if.else8:                                         ; preds = %if.merge
  store {} zeroinitializer, ptr %if.result6, align 1
  br label %if.merge9

if.merge9:                                        ; preds = %if.else8, %if.merge13
  %16 = load {}, ptr %if.result6, align 1
  %if.result14 = alloca {}, align 1
  %17 = load i32, ptr %i, align 4
  %18 = icmp eq i32 %17, 2
  br i1 %18, label %if.then15, label %if.else16

if.then11:                                        ; preds = %if.then7
  %19 = load i32, ptr %b, align 4
  store i32 %19, ptr %result1, align 4
  br label %loop.finish

if.else12:                                        ; preds = %if.then7
  store {} zeroinitializer, ptr %if.result10, align 1
  br label %if.merge13

if.merge13:                                       ; preds = %if.else12
  %20 = load {}, ptr %if.result10, align 1
  store {} %20, ptr %if.result6, align 1
  br label %if.merge9

if.then15:                                        ; preds = %if.merge9
  %if.result18 = alloca {}, align 1
  %21 = load i32, ptr %c, align 4
  %22 = icmp sgt i32 %21, 0
  br i1 %22, label %if.then19, label %if.else20

if.else16:                                        ; preds = %if.merge9
  store {} zeroinitializer, ptr %if.result14, align 1
  br label %if.merge17

if.merge17:                                       ; preds = %if.else16, %if.merge21
  %23 = load {}, ptr %if.result14, align 1
  store i32 0, ptr %result1, align 4
  br label %loop.finish

if.then19:                                        ; preds = %if.then15
  %24 = load i32, ptr %c, align 4
  store i32 %24, ptr %result1, align 4
  br label %loop.finish

if.else20:                                        ; preds = %if.then15
  store {} zeroinitializer, ptr %if.result18, align 1
  br label %if.merge21

if.merge21:                                       ; preds = %if.else20
  %25 = load {}, ptr %if.result18, align 1
  store {} %25, ptr %if.result14, align 1
  br label %if.merge17
}

define {} @main() {
entry:
  %0 = call i32 @find_positive(i32 -42, i32 -93, i32 0)
  call void @print_i32(i32 %0)
  ret {} undef
}
