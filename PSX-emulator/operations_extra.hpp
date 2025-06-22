#pragma once

#include <optional> // Ensure the header for std::optional is included  
#include <cstdint>  
#include <type_traits>
#include <limits>

template <typename T>
    requires std::is_unsigned_v<T>
std::optional<T> checked_add(T a, T b) {
    T result = a + b;
    if (result < a) return std::nullopt;
    return result;
}

static std::optional<int32_t> checked_add_signed(int32_t a, int32_t b)  {
    int32_t result = a + b;

    // Detect overflow
    if ((a > 0 && b > 0 && result < 0) ||
        (a < 0 && b < 0 && result > 0)) {
        return std::nullopt;
    }

    return result;
}
static std::optional<int32_t> checked_sub_signed(int32_t a, int32_t b) {
    int32_t result = a - b;

    // Detect overflow
    if ((a > 0 && b > 0 && result < 0) ||
        (a < 0 && b < 0 && result > 0)) {
        return std::nullopt;
    }

    return result;
}

