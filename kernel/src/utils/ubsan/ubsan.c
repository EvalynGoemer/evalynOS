#include <arch/generic/panic.h>

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_out_of_bounds() {
    panic("UBSAN: Out of bounds");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_pointer_overflow() {
    panic("UBSAN: Pointer overflow");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_load_invalid_value() {
    panic("UBSAN: Load invalid value");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_divrem_overflow() {
    panic("UBSAN: Divide overflow");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_add_overflow() {
    panic("UBSAN: Add overflow");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_type_mismatch_v1() {
    panic("UBSAN: Type missmatch");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_shift_out_of_bounds() {
    panic("UBSAN: Shift out of bounds");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_builtin_unreachable() {
    panic("UBSAN: Reached unreachable code");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_sub_overflow() {
    panic("UBSAN: Subtraction overflow");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_mul_overflow() {
    panic("UBSAN: Multiply overflow");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_function_type_mismatch() {
    panic("UBSAN: Function type missmatch");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_negate_overflow() {
    panic("UBSAN: Negate overflow");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_invalid_builtin() {
    panic("UBSAN: Invaid Builtin");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_vla_bound_not_positive() {
    panic("UBSAN: VLA Bound Not Positive");
}
