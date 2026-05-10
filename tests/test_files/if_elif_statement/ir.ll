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

define i64 @func(i64 %0) {
entry:
  %x = alloca i64, align 4
  store i64 %0, ptr %x, align 4
  %if.result = alloca {}, align 1
  %1 = load i64, ptr %x, align 4
  %2 = icmp eq i64 %1, 0
  br i1 %2, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i64 10, ptr %x, align 4
  store {} zeroinitializer, ptr %if.result, align 1
  br label %if.merge

if.else:                                          ; preds = %entry
  %3 = load i64, ptr %x, align 4
  %4 = sub i64 %3, 42
  store i64 %4, ptr %x, align 4
  store {} zeroinitializer, ptr %if.result, align 1
  br label %if.merge

if.merge:                                         ; preds = %if.else, %if.then
  %5 = load {}, ptr %if.result, align 1
  %if.result1 = alloca i64, align 4
  %6 = load i64, ptr %x, align 4
  %7 = icmp ugt i64 %6, 10
  br i1 %7, label %if.then2, label %elif.cond

if.then2:                                         ; preds = %if.merge
  %"0a" = alloca i64, align 4
  %8 = load i64, ptr %x, align 4
  store i64 %8, ptr %"0a", align 4
  %9 = load i64, ptr %"0a", align 4
  store i64 %9, ptr %if.result1, align 4
  br label %if.merge4

elif.cond:                                        ; preds = %if.merge
  %10 = load i64, ptr %x, align 4
  %11 = icmp ugt i64 %10, 5
  br i1 %11, label %elif.body, label %if.else3

elif.body:                                        ; preds = %elif.cond
  %"0b" = alloca i64, align 4
  %12 = load i64, ptr %x, align 4
  %13 = mul i64 %12, 2
  store i64 %13, ptr %"0b", align 4
  %14 = load i64, ptr %"0b", align 4
  store i64 %14, ptr %if.result1, align 4
  br label %if.merge4

if.else3:                                         ; preds = %elif.cond
  %"0c" = alloca i64, align 4
  %15 = load i64, ptr %x, align 4
  %16 = sub i64 %15, 1
  store i64 %16, ptr %"0c", align 4
  %17 = load i64, ptr %"0c", align 4
  call void @print_u64(i64 %17)
  %18 = load i64, ptr %"0c", align 4
  store i64 %18, ptr %if.result1, align 4
  br label %if.merge4

if.merge4:                                        ; preds = %if.else3, %elif.body, %if.then2
  %19 = load i64, ptr %if.result1, align 4
  ret i64 %19
}

define {} @main() {
entry:
  %0 = call i64 @func(i64 42)
  %1 = trunc i64 %0 to i32
  call void @print_i32(i32 %1)
  ret {} undef
}
