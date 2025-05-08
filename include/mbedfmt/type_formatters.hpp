#pragma once

#include <string>

namespace mbedfmt {

struct type_formatter_attributes {
    const char* lengthMod;
    const char* conversionSpec;
};

template <class T, typename Enable = void>
struct type_formatter;

#define DECLARE_TYPE_FORMATTER(type, lenmod, convspec)                         \
    template <>                                                                \
    struct type_formatter<type> {                                              \
        static constexpr type_formatter_attributes attributes{lenmod,          \
                                                              convspec};       \
    };

// Signed integers
DECLARE_TYPE_FORMATTER(signed char, "hh", "d")
DECLARE_TYPE_FORMATTER(short, "h", "d")
DECLARE_TYPE_FORMATTER(int, "", "d")
DECLARE_TYPE_FORMATTER(long, "l", "d")
DECLARE_TYPE_FORMATTER(long long, "ll", "d")

// Unsigned integers
DECLARE_TYPE_FORMATTER(unsigned char, "hh", "u")
DECLARE_TYPE_FORMATTER(unsigned short, "h", "u")
DECLARE_TYPE_FORMATTER(unsigned int, "", "u")
DECLARE_TYPE_FORMATTER(unsigned long, "l", "u")
DECLARE_TYPE_FORMATTER(unsigned long long, "ll", "u")

// Floating point
DECLARE_TYPE_FORMATTER(float, "", "f")
DECLARE_TYPE_FORMATTER(double, "", "f")
DECLARE_TYPE_FORMATTER(long double, "L", "f")

// Character and string
DECLARE_TYPE_FORMATTER(char, "", "c")
DECLARE_TYPE_FORMATTER(const char*, "", "s")

#undef DECLARE_TYPE_FORMATTER

template <size_t N>
struct type_formatter<char[N]> {
    static constexpr type_formatter_attributes attributes{"", "s"};
};

template <class T>
struct type_formatter<T*> {
    static constexpr type_formatter_attributes attributes{"", "p"};
};

// we need concepts here...
template <typename E>
struct type_formatter<E, std::enable_if_t<std::is_enum<E>::value>> {
    using UnderlyingType = typename std::underlying_type<E>::type;

    static auto convert(const E& val) {
        return static_cast<UnderlyingType>(val);
    }
};

template <>
struct type_formatter<std::string> {
    static auto convert(const std::string& val) { return val.c_str(); }
};

} // namespace mbedfmt