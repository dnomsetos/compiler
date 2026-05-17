; ModuleID = 'tests/test_files/mutable_references/code.txt'
source_filename = "tests/test_files/mutable_references/code.txt"

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

define i32 @val(ptr %0) {
entry:
  %x = alloca ptr, align 8
  store ptr %0, ptr %x, align 8
  %1 = load ptr, ptr %x, align 8
  %2 = load i32, ptr %1, align 4
  ret i32 %2
}

define i32 @add(ptr %0, ptr %1) {
entry:
  %a = alloca ptr, align 8
  store ptr %0, ptr %a, align 8
  %b = alloca ptr, align 8
  store ptr %1, ptr %b, align 8
  %2 = load ptr, ptr %a, align 8
  %3 = load i32, ptr %2, align 4
  %4 = load ptr, ptr %b, align 8
  %5 = load i32, ptr %4, align 4
  %6 = add i32 %3, %5
  ret i32 %6
}

define i32 @mul(ptr %0, ptr %1) {
entry:
  %a = alloca ptr, align 8
  store ptr %0, ptr %a, align 8
  %b = alloca ptr, align 8
  store ptr %1, ptr %b, align 8
  %2 = load ptr, ptr %a, align 8
  %3 = load i32, ptr %2, align 4
  %4 = load ptr, ptr %b, align 8
  %5 = load i32, ptr %4, align 4
  %6 = mul i32 %3, %5
  ret i32 %6
}

define {} @main() {
entry:
  %a = alloca i32, align 4
  store i32 1, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 2, ptr %b, align 4
  %c = alloca i32, align 4
  store i32 3, ptr %c, align 4
  %d = alloca i32, align 4
  store i32 4, ptr %d, align 4
  %e = alloca i32, align 4
  store i32 5, ptr %e, align 4
  %f = alloca i32, align 4
  store i32 6, ptr %f, align 4
  %r = alloca ptr, align 8
  store ptr %a, ptr %r, align 8
  %0 = load ptr, ptr %r, align 8
  %1 = load i32, ptr %0, align 4
  call void @print_i32(i32 %1)
  store ptr %b, ptr %r, align 8
  %2 = load ptr, ptr %r, align 8
  %3 = load i32, ptr %2, align 4
  call void @print_i32(i32 %3)
  %t = alloca ptr, align 8
  store ptr %c, ptr %t, align 8
  %4 = load ptr, ptr %t, align 8
  store ptr %4, ptr %r, align 8
  %5 = load ptr, ptr %r, align 8
  %6 = load i32, ptr %5, align 4
  call void @print_i32(i32 %6)
  %if.result = alloca ptr, align 8
  br i1 true, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store ptr %d, ptr %if.result, align 8
  br label %if.merge

if.else:                                          ; preds = %entry
  store ptr %e, ptr %if.result, align 8
  br label %if.merge

if.merge:                                         ; preds = %if.else, %if.then
  %7 = load ptr, ptr %if.result, align 8
  store ptr %7, ptr %r, align 8
  %8 = load ptr, ptr %r, align 8
  %9 = load i32, ptr %8, align 4
  call void @print_i32(i32 %9)
  %t1 = alloca ptr, align 8
  store ptr %f, ptr %t1, align 8
  %10 = load ptr, ptr %t1, align 8
  store ptr %10, ptr %r, align 8
  %11 = load ptr, ptr %r, align 8
  %12 = load i32, ptr %11, align 4
  call void @print_i32(i32 %12)
  %s = alloca ptr, align 8
  store ptr %a, ptr %s, align 8
  %13 = load ptr, ptr %s, align 8
  %14 = load i32, ptr %13, align 4
  call void @print_i32(i32 %14)
  store ptr %e, ptr %s, align 8
  %15 = load ptr, ptr %s, align 8
  %16 = load i32, ptr %15, align 4
  call void @print_i32(i32 %16)
  store ptr %c, ptr %s, align 8
  %17 = load ptr, ptr %r, align 8
  %18 = load ptr, ptr %s, align 8
  %19 = call i32 @add(ptr %17, ptr %18)
  call void @print_i32(i32 %19)
  %t2 = alloca ptr, align 8
  store ptr %b, ptr %t2, align 8
  %20 = load ptr, ptr %t2, align 8
  %21 = call i32 @val(ptr %20)
  call void @print_i32(i32 %21)
  store ptr %d, ptr %t2, align 8
  %22 = load ptr, ptr %t2, align 8
  %23 = call i32 @mul(ptr %22, ptr %b)
  call void @print_i32(i32 %23)
  %k = alloca ptr, align 8
  store ptr %e, ptr %k, align 8
  %24 = load ptr, ptr %k, align 8
  store ptr %24, ptr %t2, align 8
  %25 = load ptr, ptr %t2, align 8
  %26 = call i32 @add(ptr %25, ptr %f)
  call void @print_i32(i32 %26)
  %x = alloca ptr, align 8
  store ptr %a, ptr %x, align 8
  %y = alloca ptr, align 8
  store ptr %b, ptr %y, align 8
  store ptr %c, ptr %x, align 8
  store ptr %d, ptr %y, align 8
  %27 = load ptr, ptr %x, align 8
  %28 = load ptr, ptr %y, align 8
  %29 = call i32 @add(ptr %27, ptr %28)
  call void @print_i32(i32 %29)
  %p = alloca ptr, align 8
  store ptr %e, ptr %p, align 8
  %30 = load ptr, ptr %p, align 8
  store ptr %30, ptr %x, align 8
  %q = alloca ptr, align 8
  store ptr %f, ptr %q, align 8
  %31 = load ptr, ptr %q, align 8
  store ptr %31, ptr %y, align 8
  %32 = load ptr, ptr %x, align 8
  %33 = load ptr, ptr %y, align 8
  %34 = call i32 @mul(ptr %32, ptr %33)
  call void @print_i32(i32 %34)
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  %p3 = alloca ptr, align 8
  store ptr %a, ptr %p3, align 8
  %result = alloca {}, align 1
  br label %loop.body

loop.body:                                        ; preds = %if.merge15, %if.merge
  %if.result4 = alloca ptr, align 8
  %35 = load i32, ptr %i, align 4
  %36 = icmp eq i32 %35, 0
  br i1 %36, label %if.then5, label %if.else6

loop.finish:                                      ; preds = %if.then13
  %37 = load {}, ptr %result, align 1
  %j = alloca i32, align 4
  store i32 0, ptr %j, align 4
  %q16 = alloca ptr, align 8
  store ptr %f, ptr %q16, align 8
  %result17 = alloca {}, align 1
  br label %loop.body18

if.then5:                                         ; preds = %loop.body
  store ptr %b, ptr %if.result4, align 8
  br label %if.merge7

if.else6:                                         ; preds = %loop.body
  %if.result8 = alloca ptr, align 8
  %38 = load i32, ptr %i, align 4
  %39 = icmp eq i32 %38, 1
  br i1 %39, label %if.then9, label %if.else10

if.merge7:                                        ; preds = %if.merge11, %if.then5
  %40 = load ptr, ptr %if.result4, align 8
  store ptr %40, ptr %p3, align 8
  %41 = load ptr, ptr %p3, align 8
  %42 = load i32, ptr %41, align 4
  call void @print_i32(i32 %42)
  %43 = load i32, ptr %i, align 4
  %44 = add i32 %43, 1
  store i32 %44, ptr %i, align 4
  %if.result12 = alloca {}, align 1
  %45 = load i32, ptr %i, align 4
  %46 = icmp eq i32 %45, 3
  br i1 %46, label %if.then13, label %if.else14

if.then9:                                         ; preds = %if.else6
  store ptr %c, ptr %if.result8, align 8
  br label %if.merge11

if.else10:                                        ; preds = %if.else6
  store ptr %d, ptr %if.result8, align 8
  br label %if.merge11

if.merge11:                                       ; preds = %if.else10, %if.then9
  %47 = load ptr, ptr %if.result8, align 8
  store ptr %47, ptr %if.result4, align 8
  br label %if.merge7

if.then13:                                        ; preds = %if.merge7
  store {} zeroinitializer, ptr %result, align 1
  br label %loop.finish

if.else14:                                        ; preds = %if.merge7
  store {} zeroinitializer, ptr %if.result12, align 1
  br label %if.merge15

if.merge15:                                       ; preds = %if.else14
  %48 = load {}, ptr %if.result12, align 1
  br label %loop.body

loop.body18:                                      ; preds = %if.merge31, %loop.finish
  %if.result20 = alloca ptr, align 8
  %49 = load i32, ptr %j, align 4
  %50 = icmp eq i32 %49, 0
  br i1 %50, label %if.then21, label %if.else22

loop.finish19:                                    ; preds = %if.then29
  %51 = load {}, ptr %result17, align 1
  %z = alloca ptr, align 8
  store ptr %a, ptr %z, align 8
  %52 = load ptr, ptr %z, align 8
  %53 = load i32, ptr %52, align 4
  call void @print_i32(i32 %53)
  store ptr %b, ptr %z, align 8
  %54 = load ptr, ptr %z, align 8
  %55 = load i32, ptr %54, align 4
  call void @print_i32(i32 %55)
  store ptr %c, ptr %z, align 8
  %56 = load ptr, ptr %z, align 8
  %57 = load i32, ptr %56, align 4
  call void @print_i32(i32 %57)
  store ptr %d, ptr %z, align 8
  %58 = load ptr, ptr %z, align 8
  %59 = load i32, ptr %58, align 4
  call void @print_i32(i32 %59)
  store ptr %e, ptr %z, align 8
  %60 = load ptr, ptr %z, align 8
  %61 = load i32, ptr %60, align 4
  call void @print_i32(i32 %61)
  store ptr %f, ptr %z, align 8
  %62 = load ptr, ptr %z, align 8
  %63 = load i32, ptr %62, align 4
  call void @print_i32(i32 %63)
  ret {} zeroinitializer

if.then21:                                        ; preds = %loop.body18
  store ptr %a, ptr %if.result20, align 8
  br label %if.merge23

if.else22:                                        ; preds = %loop.body18
  %if.result24 = alloca ptr, align 8
  %64 = load i32, ptr %j, align 4
  %65 = icmp eq i32 %64, 1
  br i1 %65, label %if.then25, label %if.else26

if.merge23:                                       ; preds = %if.merge27, %if.then21
  %66 = load ptr, ptr %if.result20, align 8
  store ptr %66, ptr %q16, align 8
  %67 = load ptr, ptr %q16, align 8
  %68 = load i32, ptr %67, align 4
  call void @print_i32(i32 %68)
  %69 = load i32, ptr %j, align 4
  %70 = add i32 %69, 1
  store i32 %70, ptr %j, align 4
  %if.result28 = alloca {}, align 1
  %71 = load i32, ptr %j, align 4
  %72 = icmp eq i32 %71, 3
  br i1 %72, label %if.then29, label %if.else30

if.then25:                                        ; preds = %if.else22
  store ptr %d, ptr %if.result24, align 8
  br label %if.merge27

if.else26:                                        ; preds = %if.else22
  store ptr %e, ptr %if.result24, align 8
  br label %if.merge27

if.merge27:                                       ; preds = %if.else26, %if.then25
  %73 = load ptr, ptr %if.result24, align 8
  store ptr %73, ptr %if.result20, align 8
  br label %if.merge23

if.then29:                                        ; preds = %if.merge23
  store {} zeroinitializer, ptr %result17, align 1
  br label %loop.finish19

if.else30:                                        ; preds = %if.merge23
  store {} zeroinitializer, ptr %if.result28, align 1
  br label %if.merge31

if.merge31:                                       ; preds = %if.else30
  %74 = load {}, ptr %if.result28, align 1
  br label %loop.body18
}
