#include <arch/generic/panic.h>

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_out_of_bounds() {
    panic("UBASAN: Out of bounds");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_pointer_overflow() {
    panic("UBASAN: Pointer overflow");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_load_invalid_value() {
    panic("UBASAN: Load invalid value");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_divrem_overflow() {
    panic("UBASAN: Divide overflow");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_add_overflow() {
    panic("UBASAN: Add overflow");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_type_mismatch_v1() {
    panic("UBASAN: Type missmatch");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_shift_out_of_bounds() {
    panic("UBASAN: Shift out of bounds");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_builtin_unreachable() {
    panic("UBASAN: Reached unreachable code");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_sub_overflow() {
    panic("UBASAN: Subtraction overflow");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_mul_overflow() {
    panic("UBASAN: Multiply overflow");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_function_type_mismatch() {
    panic("UBASAN: Function type missmatch");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_negate_overflow() {
    panic("UBASAN: Negate overflow");
}

__attribute__((no_sanitize("undefined")))
void __ubsan_handle_invalid_builtin() {
    panic("UBASAN: Invaid Builtin");
}
