; ModuleID = 'tests/test_files/simple_cast/code.rs'
source_filename = "tests/test_files/simple_cast/code.rs"

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

define double @add(i64 %0, double %1) {
entry:
  %a = alloca i64, align 4
  store i64 %0, ptr %a, align 4
  %b = alloca double, align 8
  store double %1, ptr %b, align 8
  %2 = load i64, ptr %a, align 4
  %3 = sitofp i64 %2 to double
  %4 = fmul double %3, 4.242000e+01
  %5 = load double, ptr %b, align 8
  %6 = fdiv double %5, 2.390000e+02
  %7 = fadd double %4, %6
  ret double %7
}

define float @compute(i32 %0, float %1) {
entry:
  %a = alloca i32, align 4
  store i32 %0, ptr %a, align 4
  %b = alloca float, align 4
  store float %1, ptr %b, align 4
  %2 = load i32, ptr %a, align 4
  %3 = sext i32 %2 to i64
  %4 = load float, ptr %b, align 4
  %5 = fpext float %4 to double
  %6 = call double @add(i64 %3, double %5)
  %7 = fptrunc double %6 to float
  %8 = fadd float %7, 1.600000e+02
  ret float %8
}

define float @strange_func() {
entry:
  %0 = call double @add(i64 239, double 1.466700e+02)
  %1 = fptrunc double %0 to float
  %2 = call float @compute(i32 42, float %1)
  ret float %2
}

define {} @main() {
entry:
  %0 = call double @add(i64 5, double 3.000000e+00)
  %1 = fptosi double %0 to i32
  call void @print_i32(i32 %1)
  %2 = call float @strange_func()
  %3 = fptoui float %2 to i64
  call void @print_u64(i64 %3)
  ret {} undef
}
