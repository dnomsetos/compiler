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

define {} @main() {
entry:
  %"0x" = alloca i8, align 1
  %"0z" = alloca i8, align 1
  %"0y" = alloca i8, align 1
  store i8 34, ptr %"0y", align 1
  %if.result = alloca {}, align 1
  %0 = load i8, ptr %"0y", align 1
  %1 = icmp sgt i8 %0, 10
  br i1 %1, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i8 10, ptr %"0x", align 1
  store {} zeroinitializer, ptr %if.result, align 1
  br label %if.merge

if.else:                                          ; preds = %entry
  %2 = load i8, ptr %"0y", align 1
  store i8 %2, ptr %"0x", align 1
  store {} zeroinitializer, ptr %if.result, align 1
  br label %if.merge

if.merge:                                         ; preds = %if.else, %if.then
  %3 = load {}, ptr %if.result, align 1
  %4 = load i8, ptr %"0x", align 1
  %5 = load i8, ptr %"0y", align 1
  %6 = add i8 %4, %5
  store i8 %6, ptr %"0z", align 1
  %7 = load i8, ptr %"0z", align 1
  call void @print_i8(i8 %7)
  %8 = load i8, ptr %"0y", align 1
  call void @print_i8(i8 %8)
  %9 = load i8, ptr %"0x", align 1
  call void @print_i8(i8 %9)
  ret {} undef
}
