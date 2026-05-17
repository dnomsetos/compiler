; ModuleID = 'tests/test_files/simple_references/code.txt'
source_filename = "tests/test_files/simple_references/code.txt"

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

define i32 @add_one(ptr %0) {
entry:
  %x = alloca ptr, align 8
  store ptr %0, ptr %x, align 8
  %1 = load ptr, ptr %x, align 8
  %2 = load i32, ptr %1, align 4
  %3 = add i32 %2, 1
  ret i32 %3
}

define i32 @add_two(ptr %0) {
entry:
  %x = alloca ptr, align 8
  store ptr %0, ptr %x, align 8
  %y = alloca i32, align 4
  %1 = load ptr, ptr %x, align 8
  %2 = call i32 @add_one(ptr %1)
  store i32 %2, ptr %y, align 4
  %3 = load i32, ptr %y, align 4
  %4 = add i32 %3, 1
  ret i32 %4
}

define i32 @sum_three(ptr %0, ptr %1, ptr %2) {
entry:
  %a = alloca ptr, align 8
  store ptr %0, ptr %a, align 8
  %b = alloca ptr, align 8
  store ptr %1, ptr %b, align 8
  %c = alloca ptr, align 8
  store ptr %2, ptr %c, align 8
  %3 = load ptr, ptr %a, align 8
  %4 = load i32, ptr %3, align 4
  %5 = load ptr, ptr %b, align 8
  %6 = load i32, ptr %5, align 4
  %7 = add i32 %4, %6
  %8 = load ptr, ptr %c, align 8
  %9 = load i32, ptr %8, align 4
  %10 = add i32 %7, %9
  ret i32 %10
}

define {} @main() {
entry:
  %a = alloca i32, align 4
  store i32 10, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 20, ptr %b, align 4
  %c = alloca i32, align 4
  store i32 30, ptr %c, align 4
  %ra = alloca ptr, align 8
  store ptr %a, ptr %ra, align 8
  %rb = alloca ptr, align 8
  store ptr %b, ptr %rb, align 8
  %rc = alloca ptr, align 8
  store ptr %c, ptr %rc, align 8
  %s1 = alloca i32, align 4
  %0 = load ptr, ptr %ra, align 8
  %1 = load ptr, ptr %rb, align 8
  %2 = load ptr, ptr %rc, align 8
  %3 = call i32 @sum_three(ptr %0, ptr %1, ptr %2)
  store i32 %3, ptr %s1, align 4
  %4 = load i32, ptr %s1, align 4
  call void @print_i32(i32 %4)
  %s2 = alloca i32, align 4
  %5 = load ptr, ptr %ra, align 8
  %6 = call i32 @add_two(ptr %5)
  store i32 %6, ptr %s2, align 4
  %7 = load i32, ptr %s2, align 4
  call void @print_i32(i32 %7)
  %inner = alloca ptr, align 8
  store ptr %a, ptr %inner, align 8
  %v = alloca i32, align 4
  %8 = load ptr, ptr %inner, align 8
  %9 = load i32, ptr %8, align 4
  %10 = add i32 %9, 5
  store i32 %10, ptr %v, align 4
  %11 = load i32, ptr %v, align 4
  call void @print_i32(i32 %11)
  %again = alloca i32, align 4
  %12 = call i32 @add_one(ptr %b)
  store i32 %12, ptr %again, align 4
  %13 = load i32, ptr %again, align 4
  %14 = icmp eq i32 %13, 21
  call void @print_bool(i1 %14)
  %chain1 = alloca i32, align 4
  %15 = call i32 @add_two(ptr %c)
  store i32 %15, ptr %chain1, align 4
  %chain2 = alloca i32, align 4
  %16 = call i32 @add_one(ptr %chain1)
  store i32 %16, ptr %chain2, align 4
  %17 = load i32, ptr %chain2, align 4
  %18 = icmp eq i32 %17, 33
  call void @print_bool(i1 %18)
  ret {} undef
}
