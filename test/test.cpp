#include "mbedfmt.hpp"

#include <string>

#define TEST_EQ(printf, fmt, ...)                                              \
    do {                                                                       \
        static constexpr auto printfLog =                                      \
            MBEDFMT_FMT_TO_PRINTF_CSTR(fmt, __VA_ARGS__);                      \
        static_assert(verifyEquality(printf, printfLog), "logs don't match");  \
    } while (0)

namespace {

bool constexpr verifyEquality(const char* s, const char* s1) {
    while (*s && *s == *s1) {
        s++;
        s1++;
    }

    return *s == *s1;
}

} // namespace

struct IntWrapper {
    int val;
};

struct IntWrapperWrapper {
    IntWrapper wrapper;
};

template <>
struct mbedfmt::type_formatter<IntWrapper> {
    static auto convert(const IntWrapper& p) {
        return p.val;
    }
};

template <>
struct mbedfmt::type_formatter<IntWrapperWrapper> {
    static auto convert(const IntWrapperWrapper& p) {
        return p.wrapper;
    }
};

__attribute_maybe_unused__ void test() {
    // basic case no formatters
    TEST_EQ("", "");
    TEST_EQ("t", "t");
    TEST_EQ("test", "test");

    // escape %
    TEST_EQ("%%", "%");
    TEST_EQ("%%%%", "%%");

    // escaped brackets
    TEST_EQ("{", "{{");
    TEST_EQ("{{", "{{{{");
    TEST_EQ("}", "}}");
    TEST_EQ("}}", "}}}}");

    // basic type formatters
    TEST_EQ("%hhd", "{}", (signed char) 'a');
    TEST_EQ("%hd", "{}", (short) 1);
    TEST_EQ("%d", "{}", (int) 1);
    TEST_EQ("%ld", "{}", (long) 1);
    TEST_EQ("%lld", "{}", (long long) 1);

    TEST_EQ("%hhu", "{}", (unsigned char) 'a');
    TEST_EQ("%hu", "{}", (unsigned short) 1);
    TEST_EQ("%u", "{}", (unsigned int) 1);
    TEST_EQ("%lu", "{}", (unsigned long) 1);
    TEST_EQ("%llu", "{}", (unsigned long long) 1);

    TEST_EQ("%f", "{}", (float) 1.0);
    TEST_EQ("%f", "{}", (double) 1.0);
    TEST_EQ("%Lf", "{}", (long double) 1.0);

    TEST_EQ("%c", "{}", 'a');
    TEST_EQ("%s", "{}", "string");

    // pointers
    int* intPointer;
    TEST_EQ("%p", "{}", intPointer);
    char* charPointer;
    TEST_EQ("%p", "{}", charPointer);
    void* voidPointer;
    TEST_EQ("%p", "{}", voidPointer);

    std::string stdString;
    TEST_EQ("%s", "{}", stdString);
    TEST_EQ("%s", "{}", stdString.c_str());

    // enum conversion
    enum IntBackedEnum : int { INT_BACKED_ENUM };
    TEST_EQ("%d", "{}", INT_BACKED_ENUM);

    enum ShortBackedEnum : short { SHORT_BACKED_ENUM };
    TEST_EQ("%hd", "{}", SHORT_BACKED_ENUM);

    // multiple args
    TEST_EQ("%d%d%d", "{}{}{}", (int) 1, (int) 1, (int) 1);
    TEST_EQ("a%da%da%da", "a{}a{}a{}a", (int) 1, (int) 1, (int) 1);
    TEST_EQ("a%fa%da%llda", "a{}a{}a{}a", (float) 1.0, (int) 1, (long long) 1);

    // precision
    TEST_EQ("%.3f", "{:.3}", (float) 1.0);
    TEST_EQ("%.13f", "{:.13}", (float) 1.0);

    // width
    TEST_EQ("%3d", "{:3}", (int) 1);
    TEST_EQ("%13d", "{:13}", (int) 1);
    TEST_EQ("%13d", "{:>13}", (int) 1);  // right-justified is default
    TEST_EQ("%-13d", "{:<13}", (int) 1); // left-justified

    // flags
    TEST_EQ("% d", "{: }", (int) 1);
    TEST_EQ("%#d", "{:#}", (int) 1);
    TEST_EQ("%03d", "{:03}", (int) 1);

    // format type conversions
    TEST_EQ("%x", "{:x}", (unsigned int) 1);
    TEST_EQ("%X", "{:X}", (unsigned int) 1);
    TEST_EQ("%o", "{:o}", (unsigned int) 1);
    TEST_EQ("%e", "{:e}", (double) 1);
    TEST_EQ("%E", "{:E}", (double) 1);
    TEST_EQ("%g", "{:g}", (double) 1);
    TEST_EQ("%G", "{:G}", (double) 1);
    TEST_EQ("%a", "{:a}", (double) 1);
    TEST_EQ("%A", "{:A}", (double) 1);

    // var type conversions
    IntWrapper intWrapper{};
    IntWrapperWrapper intWrapperWrapper{};
    TEST_EQ("%d", "{}", intWrapper);
    TEST_EQ("%d", "{}", intWrapperWrapper);

    static_assert(std::is_same<int, decltype(mbedfmt::convert((int) 1))>());
    static_assert(std::is_same<int, decltype(mbedfmt::convert(intWrapper))>());
    static_assert(std::is_same<int, decltype(mbedfmt::convert(intWrapperWrapper))>());
}