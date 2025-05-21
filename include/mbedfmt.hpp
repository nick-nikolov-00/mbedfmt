#pragma once

#include "mbedfmt/string_utils.hpp"
#include "mbedfmt/template_helpers.hpp"
#include "mbedfmt/type_formatters.hpp"

#include <array>
#include <cassert>

#include <ctre.hpp>

#define EXPAND_AND_CONCAT(format) format##_mbedfmt_tstr

/**
 * @brief Converts an fmt-style format string to a printf-style cstring at
 * compile time
 *
 * This macro processes an fmt format string and its argument types, returning
 * a printf-compatible format string stored in a const char*. The conversion and
 * type checking are done at compile time. The resulting string should be
 * printed with arguments converted using mbedfmt::convert.
 *
 * @param fmt The format string literal
 * @param ... The arguments that will be used to print the string
 * @return const char* pointing to the printf format string
 *
 * @example
 * @code
 * static constexpr const char* fmt = MBEDFMT_FMT_TO_PRINTF_CSTR("Value: {:.2}",
 * (float) 1.0);
 * // fmt contains "Value: %.2f"
 * @endcode
 */
#define MBEDFMT_FMT_TO_PRINTF_CSTR(fmt, ...)                                   \
    decltype(mbedfmt::internal::getPrintfCstr<decltype(EXPAND_AND_CONCAT(      \
                 fmt))>(                                                       \
        decltype(mbedfmt::internal::toArgPack(__VA_ARGS__))()))::c_str

#define CONSTEVAL_ASSERT(c, msg) assert((c) && msg)

namespace mbedfmt {

namespace internal {

template <typename, typename = std::void_t<>>
struct has_convert : std::false_type {};

template <typename T>
struct has_convert<T, std::void_t<decltype(type_formatter<T>::convert(
                          std::declval<const T&>()))>> : std::true_type {};

template <class T>
constexpr type_formatter_attributes recursiveAttributes() {
    if constexpr (has_convert<T>::value) {
        return recursiveAttributes<decltype(type_formatter<T>::convert(
            std::declval<const T&>()))>();
    } else {
        return type_formatter<T>::attributes;
    }
}

enum class Align { LEFT, RIGHT, CENTER };

enum class Sign { PLUS, MINUS, SPACE };

struct fmt_specifiers {
    bool hasArgId;
    size_t argId;

    bool hasFillAndAlign;
    bool hasFill; // fill is optional in fill-and-align
    char fill;
    Align align;

    bool hasSign;
    Sign sign;

    bool hasWidth;
    bool hasArgIdWidth;
    size_t width;

    bool hasPrecision;
    bool hasArgIdPrecision;
    size_t precision;

    bool hasType;
    char type;

    bool hasLocale;
    bool hasAlternateForm;
    bool hasZeroFill;
};

// clang-format off
// 20.20.2 of the C++ standard specifies the following format for an fmt string
static constexpr auto format_spec_regex =
  ctll::fixed_string(R"(\{)"                                   // escaped opening brace, start of fmt
                       R"((0|[1-9]\d*)?)"                      // arg-id CG1
                       R"((?::)"                               // format-specifier OPEN
                         R"((?:([^\{\}])?([<>^]))?)"           // fill-and-align CG2 and CG3
                         R"(([\+\- ])?)"                       // sign CG4
                         R"((#)?(0)?)"                         // #(CG5) and 0(CG6)
                         R"(([1-9]\d*|\{(?:0|[1-9]\d*)?\})?)"  // width CG7
                         R"((?:\.(\d+|\{(?:0|[1-9]\d*)?\}))?)" // precision CG8
                         R"((L)?)"                             // locale CG9
                         R"(([aAbBcdeEfFgGopsxX])?)"           // type CG10
                       R"()?)"                                 // format-specifier CLOSE
                   R"(\})"                                     // escaped closing brace, end of fmt
  );
// clang-format on

constexpr std::pair<fmt_specifiers, size_t> getFmtSpecifiers(const char* s) {
    auto match = ctre::starts_with<format_spec_regex>(s);

    CONSTEVAL_ASSERT(match, "Invalid format specifier");

    fmt_specifiers spec{};

    if (match.get<1>()) {
        spec.hasArgId = true;
        spec.argId = stringToSizeT(match.get<1>().to_view());
    }

    if (match.get<3>()) { // align is always a part of the fill-and-align
        spec.hasFillAndAlign = true;
        spec.hasFill = (bool) match.get<2>();
        if (spec.hasFill) {
            spec.fill = match.get<2>().to_view()[0];
        }
        switch (match.get<3>().to_view().front()) {
        case '<':
            spec.align = Align::LEFT;
            break;
        case '>':
            spec.align = Align::RIGHT;
            break;
        case '^':
            spec.align = Align::CENTER;
            break;
        }
    }

    if (match.get<4>()) {
        spec.hasSign = true;
        switch (match.get<4>().to_view()[0]) {
        case '+':
            spec.sign = Sign::PLUS;
            break;
        case '-':
            spec.sign = Sign::MINUS;
            break;
        case ' ':
            spec.sign = Sign::SPACE;
            break;
        }
    }

    spec.hasAlternateForm = (bool) match.get<5>();
    spec.hasZeroFill = (bool) match.get<6>();

    if (match.get<7>()) {
        spec.hasWidth = true;
        auto widthStr = match.get<7>().to_view();
        if (widthStr[0] == '{') {
            spec.hasArgIdWidth = true;
            // not supported
        } else {
            spec.width = stringToSizeT(match.get<7>().to_view());
        }
    }

    if (match.get<8>()) {
        spec.hasPrecision = true;
        auto widthStr = match.get<8>().to_view();
        if (widthStr[0] == '{') {
            spec.hasArgIdPrecision = true;
            // not supported
        } else {
            spec.precision = stringToSizeT(match.get<8>().to_view());
        }
    }

    spec.hasLocale = (bool) match.get<9>();

    if (match.get<10>()) {
        spec.hasType = true;
        spec.type = match.get<10>().to_view()[0];
    }

    return {spec, match.size()};
}

template <size_t N>
constexpr void
appendPrintfFromFmt(SAString<N>& printfString, const fmt_specifiers& fmt,
                    const type_formatter_attributes& currentArgumentFormatter) {
    CONSTEVAL_ASSERT(!fmt.hasArgId, "printf doesn't support arg-id");
    CONSTEVAL_ASSERT(!fmt.hasLocale, "printf doesn't support locale");

    printfString += '%';

    if (fmt.hasFillAndAlign) {
        // clang-format off
        CONSTEVAL_ASSERT(!fmt.hasFill || fmt.fill == ' ', "printf supports only space fill");
        CONSTEVAL_ASSERT(fmt.align != Align::CENTER, "printf doesn't support center alignment");
        // clang-format on

        if (fmt.align == Align::LEFT) {
            printfString += '-';
        }
    }

    if (fmt.hasSign) {
        switch (fmt.sign) {
        case Sign::PLUS:
            printfString += '+';
            break;
        case Sign::SPACE:
            printfString += ' ';
            break;
        case Sign::MINUS:
            break; // printf doesn't support minus as a sign flag
        }
    }

    if (fmt.hasAlternateForm) {
        printfString += '#';
    }

    if (fmt.hasZeroFill) {
        printfString += '0';
    }

    if (fmt.hasWidth) {
        // clang-format off
        CONSTEVAL_ASSERT(!fmt.hasArgIdWidth, "printf doesn't support width arg-id");
        // clang-format on

        printfString += fmt.width;
    }

    if (fmt.hasPrecision) {
        // clang-format off
        CONSTEVAL_ASSERT(!fmt.hasArgIdPrecision, "printf doesn't support precision arg-id");
        // clang-format on

        printfString += '.';
        printfString += fmt.precision;
    }

    printfString += currentArgumentFormatter.lengthMod;

    if (fmt.hasType) {
        // clang-format off
        CONSTEVAL_ASSERT(fmt.type != 'b' && fmt.type != 'B', "printf doesn't support b and B conversion specifiers");

        CONSTEVAL_ASSERT(fmt.type != 'd' && fmt.type != 'f' && fmt.type != 'c' && fmt.type != 's' && fmt.type != 'p', "The following types: d,f,c,s,p are automatically derived from the passed in argument.");
        // clang-format on

        if (fmt.type == 'o' || fmt.type == 'x' || fmt.type == 'X') {
            bool isUnsignedInt =
                currentArgumentFormatter.conversionSpec[0] == 'u' &&
                currentArgumentFormatter.conversionSpec[1] == '\0';

            // clang-format off
            CONSTEVAL_ASSERT(isUnsignedInt, "The o,x,X types only apply to unsigned integers.");
            // clang-format on

        } else { // one of these is left: a A e E F g G
            bool isDouble = currentArgumentFormatter.conversionSpec[0] == 'f' &&
                            currentArgumentFormatter.conversionSpec[1] == '\0';

            // clang-format off
            CONSTEVAL_ASSERT(isDouble, "The a,A,e,E,F,g,G types only apply to floats/doubles.");
            // clang-format on
        }

        printfString += fmt.type;
    } else {
        printfString += currentArgumentFormatter.conversionSpec;
    }
}

template <class fmt_str, class... Args>
constexpr auto fmtToPrintfStr() {
    constexpr auto& fmt = fmt_str::c_str;

    // TODO: do they need to be decayed with decay_t?
    std::array<type_formatter_attributes, sizeof...(Args)> argFormatters{
        recursiveAttributes<Args>()...};

    SAString<fmt_str().size() * 2> formattedString;
    size_t currentArg = 0;
    for (const char* s = fmt; *s; s++) {
        if (*s == '%') {
            formattedString += "%%";
            continue;
        }

        if (*s != '{') {
            if (*s == '}') {
                CONSTEVAL_ASSERT(s[1] == '}', "Unmatched closing brace");
                s++;
            }

            formattedString += *s;
            continue;
        }

        if (s[1] == '{') {
            formattedString += '{';
            s++;
            continue;
        }

        // clang-format off
        CONSTEVAL_ASSERT(currentArg < argFormatters.size(), "Too few arguments provided");
        // clang-format on

        auto formatters = getFmtSpecifiers(s);
        s += formatters.second - 1;

        appendPrintfFromFmt(formattedString, formatters.first,
                            argFormatters[currentArg++]);
    }

    // clang-format off
    CONSTEVAL_ASSERT(currentArg == sizeof...(Args), "Too many arguments provided");
    // clang-format on

    return formattedString;
}

template <class fmt_str, class... Args>
struct FormattedStringStaticWrapper {
    static constexpr auto formattedString =
        internal::fmtToPrintfStr<fmt_str, Args...>();
};

template <class StaticWrapper, size_t... Idx>
auto staticStringToTStringHelper(std::index_sequence<Idx...>) {
    return tstring<StaticWrapper::formattedString.data[Idx]...>{};
}

template <class StaticWrapper>
auto staticStringToTString() {
    return staticStringToTStringHelper<StaticWrapper>(
        std::make_index_sequence<StaticWrapper::formattedString.size>{});
}

template <class fmt_str, class... Args>
constexpr auto getPrintfCstr(const arg_pack<Args...>&) {
    return staticStringToTString<
        FormattedStringStaticWrapper<fmt_str, Args...>>();
}

} // namespace internal

/**
 * @brief Recursively converts a value using type formatters
 *
 * This function checks if the type has a conversion method defined in its
 * type_formatter. If it does, it applies the conversion and recursively
 * converts the result. If it doesn't, it returns the original value. This
 * function should be used with the results of MBEDFMT_FMT_TO_PRINTF_CSTR.
 *
 * @tparam T The type of the value to convert
 * @param val The value to convert
 * @return The converted value, suitable for use with printf
 *
 * @example
 * @code
 * // For a custom type with a formatter:
 * struct CustomType { int value; };
 *
 * template <>
 * struct mbedfmt::type_formatter<CustomType> {
 *   static auto convert(const CustomType& c) { return c.value; }
 * };
 *
 * CustomType c{42};
 * auto converted = mbedfmt::convert(c);  // converted is now 42
 * @endcode
 */
template <class T>
constexpr auto convert(const T& val) {
    if constexpr (internal::has_convert<T>::value) {
        return convert(type_formatter<T>::convert(val));
    } else {
        return val;
    }
}

} // namespace mbedfmt