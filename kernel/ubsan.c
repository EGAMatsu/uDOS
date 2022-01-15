#include <ubsan.h>

void __ubsan_handle_add_overflow(
    struct ubsan_overflow *data)
{
    UBSAN_PRINT_LOCATION("addition overflow", &data->loc);
}

void __ubsan_handle_sub_overflow(
    struct ubsan_overflow *data)
{
    UBSAN_PRINT_LOCATION("subtraction overflow", &data->loc);
}

void __ubsan_handle_mul_overflow(
    struct ubsan_overflow *data)
{
    UBSAN_PRINT_LOCATION("multiple overflow", &data->loc);
}

void __ubsan_handle_divrem_overflow(
    struct ubsan_overflow *data)
{
    UBSAN_PRINT_LOCATION("division overflow", &data->loc);
}

void __ubsan_handle_negate_overflow(
    struct ubsan_overflow *data)
{
    UBSAN_PRINT_LOCATION("negative overflow", &data->loc);
}

void __ubsan_handle_pointer_overflow(
    struct ubsan_overflow *data)
{
    UBSAN_PRINT_LOCATION("pointer overflow", &data->loc);
}

void __ubsan_handle_shift_out_of_bounds(
    struct ubsan_shift_oob *data)
{
    UBSAN_PRINT_LOCATION("shift out of bounds", &data->loc);
}

void __ubsan_handle_load_invalid_value(
    struct ubsan_invalid_value *data)
{
    UBSAN_PRINT_LOCATION("load invalid value", &data->loc);
}

void __ubsan_handle_out_of_bounds(
    struct ubsan_array_oob *data)
{
    UBSAN_PRINT_LOCATION("out of bounds", &data->loc);
}

void __ubsan_handle_type_mismatch_v1(struct ubsan_type_mismatch *data, unsigned int ptr)
{
    UBSAN_PRINT_LOCATION("type mismatch", &data->loc);
}

void __ubsan_handle_vla_bound_not_positive(
    struct ubsan_negative_vla *data)
{
    UBSAN_PRINT_LOCATION("negative vla index", &data->loc);
}

void __ubsan_handle_nonnull_return(
    struct ubsan_non_null_return *data)
{
    UBSAN_PRINT_LOCATION("non-null return has null", &data->loc);
}

void __ubsan_handle_nonnull_arg(
    struct ubsan_non_null_argument *data)
{
    UBSAN_PRINT_LOCATION("non-null argument has null", &data->loc);
}

void __ubsan_handle_builtin_unreachable(
    struct ubsan_unreachable *data)
{
    UBSAN_PRINT_LOCATION("unreachable code", &data->loc);
}

void __ubsan_handle_invalid_builtin(
    struct ubsan_invalid_builtin *data)
{
    UBSAN_PRINT_LOCATION("invalid builtin", &data->loc);
}
