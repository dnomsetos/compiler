; ModuleID = 'tests/test_files/hard_references/code.txt'
source_filename = "tests/test_files/hard_references/code.txt"

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

define {} @add(ptr %0, i32 %1) {
entry:
  %x = alloca ptr, align 8
  store ptr %0, ptr %x, align 8
  %n = alloca i32, align 4
  store i32 %1, ptr %n, align 4
  %2 = load ptr, ptr %x, align 8
  %3 = load i32, ptr %2, align 4
  %4 = load i32, ptr %n, align 4
  %5 = add i32 %3, %4
  %6 = load ptr, ptr %x, align 8
  store i32 %5, ptr %6, align 4
  ret {} undef
}

define {} @mul(ptr %0, i32 %1) {
entry:
  %x = alloca ptr, align 8
  store ptr %0, ptr %x, align 8
  %n = alloca i32, align 4
  store i32 %1, ptr %n, align 4
  %2 = load ptr, ptr %x, align 8
  %3 = load i32, ptr %2, align 4
  %4 = load i32, ptr %n, align 4
  %5 = mul i32 %3, %4
  %6 = load ptr, ptr %x, align 8
  store i32 %5, ptr %6, align 4
  ret {} undef
}

define i32 @read(ptr %0) {
entry:
  %x = alloca ptr, align 8
  store ptr %0, ptr %x, align 8
  %1 = load ptr, ptr %x, align 8
  %2 = load i32, ptr %1, align 4
  ret i32 %2
}

define i32 @combine(ptr %0, ptr %1) {
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
  %ra = alloca ptr, align 8
  store ptr %a, ptr %ra, align 8
  %rb = alloca ptr, align 8
  store ptr %b, ptr %rb, align 8
  %s = alloca i32, align 4
  %0 = load ptr, ptr %ra, align 8
  %1 = load ptr, ptr %rb, align 8
  %2 = call i32 @combine(ptr %0, ptr %1)
  store i32 %2, ptr %s, align 4
  %3 = load i32, ptr %s, align 4
  call void @print_i32(i32 %3)
  %x = alloca ptr, align 8
  store ptr %a, ptr %x, align 8
  %4 = load ptr, ptr %x, align 8
  %5 = call {} @add(ptr %4, i32 10)
  %6 = load ptr, ptr %x, align 8
  %7 = call {} @mul(ptr %6, i32 2)
  %8 = load ptr, ptr %x, align 8
  %9 = load i32, ptr %8, align 4
  call void @print_i32(i32 %9)
  %y = alloca ptr, align 8
  store ptr %b, ptr %y, align 8
  %10 = load ptr, ptr %y, align 8
  %11 = call {} @add(ptr %10, i32 5)
  %12 = load ptr, ptr %y, align 8
  %13 = call {} @mul(ptr %12, i32 3)
  %14 = load ptr, ptr %y, align 8
  %15 = load i32, ptr %14, align 4
  call void @print_i32(i32 %15)
  %16 = load i32, ptr %a, align 4
  call void @print_i32(i32 %16)
  %17 = load i32, ptr %b, align 4
  call void @print_i32(i32 %17)
  %result = alloca {}, align 1
  br label %loop.body

loop.body:                                        ; preds = %if.merge, %entry
  %rc = alloca ptr, align 8
  store ptr %c, ptr %rc, align 8
  %rd = alloca ptr, align 8
  store ptr %d, ptr %rd, align 8
  %s1 = alloca i32, align 4
  %18 = load ptr, ptr %rc, align 8
  %19 = call i32 @read(ptr %18)
  store i32 %19, ptr %s1, align 4
  %s2 = alloca i32, align 4
  %20 = load ptr, ptr %rd, align 8
  %21 = call i32 @read(ptr %20)
  store i32 %21, ptr %s2, align 4
  %22 = load i32, ptr %s1, align 4
  call void @print_i32(i32 %22)
  %23 = load i32, ptr %s2, align 4
  call void @print_i32(i32 %23)
  %if.result = alloca {}, align 1
  %24 = load i32, ptr %s1, align 4
  %25 = icmp eq i32 %24, 3
  br i1 %25, label %if.then, label %if.else

loop.finish:                                      ; preds = %if.then2
  %26 = load {}, ptr %result, align 1
  %result5 = alloca {}, align 1
  br label %loop.body6

if.then:                                          ; preds = %loop.body
  %if.result1 = alloca {}, align 1
  %27 = load i32, ptr %s2, align 4
  %28 = icmp eq i32 %27, 4
  br i1 %28, label %if.then2, label %if.else3

if.else:                                          ; preds = %loop.body
  store {} zeroinitializer, ptr %if.result, align 1
  br label %if.merge

if.merge:                                         ; preds = %if.else, %if.merge4
  %29 = load {}, ptr %if.result, align 1
  br label %loop.body

if.then2:                                         ; preds = %if.then
  store {} zeroinitializer, ptr %result, align 1
  br label %loop.finish

if.else3:                                         ; preds = %if.then
  store {} zeroinitializer, ptr %if.result1, align 1
  br label %if.merge4

if.merge4:                                        ; preds = %if.else3
  %30 = load {}, ptr %if.result1, align 1
  store {} %30, ptr %if.result, align 1
  br label %if.merge

loop.body6:                                       ; preds = %if.merge11, %loop.finish
  %mc = alloca ptr, align 8
  store ptr %c, ptr %mc, align 8
  %31 = load ptr, ptr %mc, align 8
  %32 = call {} @add(ptr %31, i32 7)
  %33 = load ptr, ptr %mc, align 8
  %34 = load i32, ptr %33, align 4
  call void @print_i32(i32 %34)
  %if.result8 = alloca {}, align 1
  %35 = load ptr, ptr %mc, align 8
  %36 = load i32, ptr %35, align 4
  %37 = icmp eq i32 %36, 10
  br i1 %37, label %if.then9, label %if.else10

loop.finish7:                                     ; preds = %if.then9
  %38 = load {}, ptr %result5, align 1
  %39 = load i32, ptr %c, align 4
  call void @print_i32(i32 %39)
  %result12 = alloca {}, align 1
  br label %loop.body13

if.then9:                                         ; preds = %loop.body6
  store {} zeroinitializer, ptr %result5, align 1
  br label %loop.finish7

if.else10:                                        ; preds = %loop.body6
  store {} zeroinitializer, ptr %if.result8, align 1
  br label %if.merge11

if.merge11:                                       ; preds = %if.else10
  %40 = load {}, ptr %if.result8, align 1
  br label %loop.body6

loop.body13:                                      ; preds = %if.merge18, %loop.finish7
  %md = alloca ptr, align 8
  store ptr %d, ptr %md, align 8
  %41 = load ptr, ptr %md, align 8
  %42 = call {} @mul(ptr %41, i32 5)
  %43 = load ptr, ptr %md, align 8
  %44 = load i32, ptr %43, align 4
  call void @print_i32(i32 %44)
  %if.result15 = alloca {}, align 1
  %45 = load ptr, ptr %md, align 8
  %46 = load i32, ptr %45, align 4
  %47 = icmp eq i32 %46, 20
  br i1 %47, label %if.then16, label %if.else17

loop.finish14:                                    ; preds = %if.then16
  %48 = load {}, ptr %result12, align 1
  %ra19 = alloca ptr, align 8
  store ptr %a, ptr %ra19, align 8
  %rb20 = alloca ptr, align 8
  store ptr %b, ptr %rb20, align 8
  %rc21 = alloca ptr, align 8
  store ptr %c, ptr %rc21, align 8
  %rd22 = alloca ptr, align 8
  store ptr %d, ptr %rd22, align 8
  %total1 = alloca i32, align 4
  %49 = load ptr, ptr %ra19, align 8
  %50 = load ptr, ptr %rb20, align 8
  %51 = call i32 @combine(ptr %49, ptr %50)
  store i32 %51, ptr %total1, align 4
  %total2 = alloca i32, align 4
  %52 = load ptr, ptr %rc21, align 8
  %53 = load ptr, ptr %rd22, align 8
  %54 = call i32 @combine(ptr %52, ptr %53)
  store i32 %54, ptr %total2, align 4
  %55 = load i32, ptr %total1, align 4
  call void @print_i32(i32 %55)
  %56 = load i32, ptr %total2, align 4
  call void @print_i32(i32 %56)
  %result23 = alloca {}, align 1
  br label %loop.body24

if.then16:                                        ; preds = %loop.body13
  %r1 = alloca ptr, align 8
  store ptr %a, ptr %r1, align 8
  %r2 = alloca ptr, align 8
  store ptr %b, ptr %r2, align 8
  %r3 = alloca ptr, align 8
  store ptr %c, ptr %r3, align 8
  %r4 = alloca ptr, align 8
  store ptr %d, ptr %r4, align 8
  %x1 = alloca i32, align 4
  %57 = load ptr, ptr %r1, align 8
  %58 = load ptr, ptr %r2, align 8
  %59 = call i32 @combine(ptr %57, ptr %58)
  store i32 %59, ptr %x1, align 4
  %x2 = alloca i32, align 4
  %60 = load ptr, ptr %r3, align 8
  %61 = load ptr, ptr %r4, align 8
  %62 = call i32 @combine(ptr %60, ptr %61)
  store i32 %62, ptr %x2, align 4
  %63 = load i32, ptr %x1, align 4
  call void @print_i32(i32 %63)
  %64 = load i32, ptr %x2, align 4
  call void @print_i32(i32 %64)
  store {} zeroinitializer, ptr %result12, align 1
  br label %loop.finish14

if.else17:                                        ; preds = %loop.body13
  store {} zeroinitializer, ptr %if.result15, align 1
  br label %if.merge18

if.merge18:                                       ; preds = %if.else17
  %65 = load {}, ptr %if.result15, align 1
  br label %loop.body13

loop.body24:                                      ; preds = %if.merge30, %loop.finish14
  %x26 = alloca ptr, align 8
  store ptr %a, ptr %x26, align 8
  %66 = load ptr, ptr %x26, align 8
  %67 = load i32, ptr %66, align 4
  call void @print_i32(i32 %67)
  %if.result27 = alloca {}, align 1
  %68 = load ptr, ptr %x26, align 8
  %69 = load i32, ptr %68, align 4
  %70 = icmp eq i32 %69, 22
  br i1 %70, label %if.then28, label %if.else29

loop.finish25:                                    ; preds = %if.then28
  %71 = load {}, ptr %result23, align 1
  %ma = alloca ptr, align 8
  store ptr %a, ptr %ma, align 8
  %mb = alloca ptr, align 8
  store ptr %b, ptr %mb, align 8
  %72 = load ptr, ptr %ma, align 8
  %73 = call {} @add(ptr %72, i32 1)
  %74 = load ptr, ptr %mb, align 8
  %75 = call {} @add(ptr %74, i32 2)
  %76 = load ptr, ptr %ma, align 8
  %77 = load i32, ptr %76, align 4
  call void @print_i32(i32 %77)
  %78 = load ptr, ptr %mb, align 8
  %79 = load i32, ptr %78, align 4
  call void @print_i32(i32 %79)
  %final_a = alloca ptr, align 8
  store ptr %a, ptr %final_a, align 8
  %final_b = alloca ptr, align 8
  store ptr %b, ptr %final_b, align 8
  %final_c = alloca ptr, align 8
  store ptr %c, ptr %final_c, align 8
  %final_d = alloca ptr, align 8
  store ptr %d, ptr %final_d, align 8
  %80 = load ptr, ptr %final_a, align 8
  %81 = load i32, ptr %80, align 4
  call void @print_i32(i32 %81)
  %82 = load ptr, ptr %final_b, align 8
  %83 = load i32, ptr %82, align 4
  call void @print_i32(i32 %83)
  %84 = load ptr, ptr %final_c, align 8
  %85 = load i32, ptr %84, align 4
  call void @print_i32(i32 %85)
  %86 = load ptr, ptr %final_d, align 8
  %87 = load i32, ptr %86, align 4
  call void @print_i32(i32 %87)
  ret {} zeroinitializer

if.then28:                                        ; preds = %loop.body24
  store {} zeroinitializer, ptr %result23, align 1
  br label %loop.finish25

if.else29:                                        ; preds = %loop.body24
  store {} zeroinitializer, ptr %if.result27, align 1
  br label %if.merge30

if.merge30:                                       ; preds = %if.else29
  %88 = load {}, ptr %if.result27, align 1
  br label %loop.body24
}
