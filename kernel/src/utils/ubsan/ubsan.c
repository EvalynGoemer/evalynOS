#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <arch/generic/cpu/interrupts.h>
#include <arch/generic/panic.h>

#define NUBSAN_VOID [[gnu::no_sanitize("undefined")]] void

struct ubsan_source_location {
    const char *file;
    uint32_t line;
    uint32_t column;
};

struct ubsan_type_descriptor {
    uint16_t kind;
    uint16_t info;
    char name[];
};

struct ubsan_overflow_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
};

struct ubsan_shift_out_of_bounds_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *left_type;
    struct ubsan_type_descriptor *right_type;
};

struct ubsan_invalid_value_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
};

struct ubsan_array_out_of_bounds_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *array_type;
    struct ubsan_type_descriptor *index_type;
};

struct ubsan_type_mismatch_v1_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
    unsigned char log_alignment;
    unsigned char type_check_kind;
};

struct ubsan_negative_vla_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
};

struct ubsan_nonnull_return_data {
    struct ubsan_source_location location;
};

struct ubsan_nonnull_arg_data {
    struct ubsan_source_location location;
};

struct ubsan_unreachable_data {
    struct ubsan_source_location location;
};

struct ubsan_invalid_builtin_data {
    struct ubsan_source_location location;
    unsigned char kind;
};

struct ubsan_float_cast_overflow_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *from_type;
    struct ubsan_type_descriptor *to_type;
};

struct ubsan_pointer_overflow_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
};

struct ubsan_function_type_mismatch_data {
    struct ubsan_source_location location;
    struct ubsan_type_descriptor *type;
};

[[gnu::no_sanitize("undefined")]]
static inline void print_location(const char *message, const struct ubsan_source_location loc) {
    const char *file = loc.file ? loc.file : "<unknown>";
    LOG_TAGGED("UBSAN", ANSI_BYELLOW, "%s at %s:%u:%u", message, file, loc.line, loc.column);
}

[[gnu::no_sanitize("undefined")]]
static inline void print_type(const char *label, const struct ubsan_type_descriptor *type) {
    if (!type || !type->name[0])
        return;
    LOG_TAGGED("UBSAN", ANSI_BYELLOW, "%s type: %s", label, type->name);
}

[[gnu::no_sanitize("undefined")]]
static void report(const char *message, const struct ubsan_source_location loc, const struct ubsan_type_descriptor *type) {
    disable_interrupts();
    print_location(message, loc);
    print_type("value", type);
    panic("UBSAN FAILURE");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_shift_out_of_bounds(struct ubsan_shift_out_of_bounds_data *data) {
    disable_interrupts();
    print_location("shift out of bounds", data->location);
    print_type("left", data->left_type);
    print_type("right", data->right_type);
    panic("UBSAN FAILURE");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_out_of_bounds(struct ubsan_array_out_of_bounds_data *data) {
    disable_interrupts();
    print_location("array out of bounds", data->location);
    print_type("array", data->array_type);
    print_type("index", data->index_type);
    panic("UBSAN FAILURE");
}

[[gnu::no_sanitize("undefined")]]
void __ubsan_handle_type_mismatch_v1(struct ubsan_type_mismatch_v1_data *data, uintptr_t ptr) {
    if (!ptr) {
        report("use of NULL pointer", data->location, data->type);
    } else if (ptr & (((uint64_t)1 << data->log_alignment) - 1)) {
        report("use of misaligned pointer", data->location, data->type);
    } else {
        report("insufficient space for object", data->location, data->type);
    }
}

NUBSAN_VOID __ubsan_handle_add_overflow(struct ubsan_overflow_data *data) { report("addition overflow", data->location, data->type); }
NUBSAN_VOID __ubsan_handle_sub_overflow(struct ubsan_overflow_data *data) { report("subtraction overflow", data->location, data->type); }
NUBSAN_VOID __ubsan_handle_mul_overflow(struct ubsan_overflow_data *data) { report("multiplication overflow", data->location, data->type); }
NUBSAN_VOID __ubsan_handle_divrem_overflow(struct ubsan_overflow_data *data) { report("division overflow", data->location, data->type); }
NUBSAN_VOID __ubsan_handle_negate_overflow(struct ubsan_overflow_data *data) { report("negation overflow", data->location, data->type); }
NUBSAN_VOID __ubsan_handle_pointer_overflow(struct ubsan_overflow_data *data) { report("pointer overflow", data->location, data->type); }
NUBSAN_VOID __ubsan_handle_load_invalid_value(struct ubsan_invalid_value_data *data) { report("invalid load value", data->location, data->type); }
NUBSAN_VOID __ubsan_handle_vla_bound_not_positive(struct ubsan_negative_vla_data *data) { report("VLA bound not positive", data->location, data->type); }
NUBSAN_VOID __ubsan_handle_nonnull_return(struct ubsan_nonnull_return_data *data) { report("non-null return is null", data->location, NULL); }
NUBSAN_VOID __ubsan_handle_nonnull_arg(struct ubsan_nonnull_arg_data *data) { report("non-null argument is null", data->location, NULL); }
NUBSAN_VOID __ubsan_handle_builtin_unreachable(struct ubsan_unreachable_data *data) { report("unreachable code reached", data->location, NULL); }
NUBSAN_VOID __ubsan_handle_invalid_builtin(struct ubsan_invalid_builtin_data *data) { report("invalid builtin", data->location, NULL); }
NUBSAN_VOID __ubsan_handle_function_type_mismatch(struct ubsan_function_type_mismatch_data *data) { report("function type mismatch", data->location, data->type); }
NUBSAN_VOID __ubsan_handle_add_overflow_abort(struct ubsan_overflow_data *data) { __ubsan_handle_add_overflow(data); }
NUBSAN_VOID __ubsan_handle_sub_overflow_abort(struct ubsan_overflow_data *data) { __ubsan_handle_sub_overflow(data); }
NUBSAN_VOID __ubsan_handle_mul_overflow_abort(struct ubsan_overflow_data *data) { __ubsan_handle_mul_overflow(data); }
NUBSAN_VOID __ubsan_handle_divrem_overflow_abort(struct ubsan_overflow_data *data) { __ubsan_handle_divrem_overflow(data); }
NUBSAN_VOID __ubsan_handle_negate_overflow_abort(struct ubsan_overflow_data *data) { __ubsan_handle_negate_overflow(data); }
NUBSAN_VOID __ubsan_handle_pointer_overflow_abort(struct ubsan_overflow_data *data) { __ubsan_handle_pointer_overflow(data); }
NUBSAN_VOID __ubsan_handle_shift_out_of_bounds_abort(struct ubsan_shift_out_of_bounds_data *data) { __ubsan_handle_shift_out_of_bounds(data); }
NUBSAN_VOID __ubsan_handle_load_invalid_value_abort(struct ubsan_invalid_value_data *data) { __ubsan_handle_load_invalid_value(data); }
NUBSAN_VOID __ubsan_handle_out_of_bounds_abort(struct ubsan_array_out_of_bounds_data *data) { __ubsan_handle_out_of_bounds(data); }
NUBSAN_VOID __ubsan_handle_type_mismatch_v1_abort(struct ubsan_type_mismatch_v1_data *data, uintptr_t ptr) { __ubsan_handle_type_mismatch_v1(data, ptr); }
NUBSAN_VOID __ubsan_handle_vla_bound_not_positive_abort(struct ubsan_negative_vla_data *data) { __ubsan_handle_vla_bound_not_positive(data); }
NUBSAN_VOID __ubsan_handle_nonnull_return_abort(struct ubsan_nonnull_return_data *data) { __ubsan_handle_nonnull_return(data); }
NUBSAN_VOID __ubsan_handle_nonnull_arg_abort(struct ubsan_nonnull_arg_data *data) { __ubsan_handle_nonnull_arg(data); }
NUBSAN_VOID __ubsan_handle_builtin_unreachable_abort(struct ubsan_unreachable_data *data) { __ubsan_handle_builtin_unreachable(data); }
NUBSAN_VOID __ubsan_handle_invalid_builtin_abort(struct ubsan_invalid_builtin_data *data) { __ubsan_handle_invalid_builtin(data); }

// modified from https://github.com/rdmsr/tinyubsan/blob/master/tinyubsan.c (public domain)
