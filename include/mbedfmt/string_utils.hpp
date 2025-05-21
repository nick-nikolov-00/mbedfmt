#pragma once

#include <array>
#include <string_view>

namespace mbedfmt::internal {

// struct for statically allocated string as C++17 does not support dynamic
// allocation in constexpr
template <size_t N>
struct SAString {
    constexpr SAString<N>& operator+=(const char& c) {
        data[size++] = c;
        return *this;
    }

    constexpr SAString<N>& operator+=(const char* s) {
        for (auto curr = s; *curr; ++curr) {
            data[size++] = *curr;
        }
        return *this;
    }

    constexpr SAString<N>& operator+=(size_t val) {
        if (val == 0) {
            return *this += '0';
        }

        char buffer[20] = {};
        int i = 0;

        while (val > 0) {
            buffer[i++] = '0' + (char) (val % 10);
            val /= 10;
        }

        for (int j = i - 1; j >= 0; --j) {
            *this += buffer[j];
        }

        return *this;
    }

    constexpr auto operator[](const size_t& i) const { return data[i]; }

    std::array<char, N> data{};
    size_t size{};
};

constexpr size_t stringToSizeT(const std::basic_string_view<char>& sv) {
    size_t result = 0;
    for (const auto& c : sv) {
        result += c - '0';
        result *= 10;
    }

    return result / 10;
}

} // namespace mbedfmt::internal