#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

void print_i8(int8_t x) { printf("%d\n", x); }

void print_i16(int16_t x) { printf("%d\n", x); }

void print_i32(int32_t x) { printf("%d\n", x); }

void print_i64(int64_t x) { printf("%lld\n", (long long)x); }

void print_f32(float x) { printf("%f\n", x); }

void print_f64(double x) { printf("%lf\n", x); }

void print_bool(bool x) { printf("%s\n", x ? "true" : "false"); }

void print_char(int8_t x) { printf("%c\n", x); }

void print_u8(uint8_t x) { printf("%u\n", x); }

void print_u16(uint16_t x) { printf("%u\n", x); }

void print_u32(uint32_t x) { printf("%u\n", x); }

void print_u64(uint64_t x) { printf("%llu\n", (unsigned long long)x); }

void print_void() { printf("void\n"); }
