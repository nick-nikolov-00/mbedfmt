# Mbed FMT

A C++17/20 header-only library that converts fmt-style format strings to printf format strings at compile time. This is
particularly useful for embedded systems where fmt might be too heavy, but you still want its nicer syntax and the
built-in safety nets.

## Features

- Converts fmt-style format strings to printf format strings at compile time
- Header-only with minimal dependencies
- Supports most fmt format specifiers
- Extensible type formatting system
- C++17 and C++20 support

## Installation

### Using CMake FetchContent (Recommended)

The simplest way to include mbedfmt in your CMake project is using FetchContent:

```cmake
include(FetchContent)

FetchContent_Declare(
        mbedfmt
        GIT_REPOSITORY git@github.com:nick-nikolov-00/mbedfmt.git
        GIT_TAG main
)

FetchContent_MakeAvailable(mbedfmt)

target_link_libraries(your_target PRIVATE mbedfmt)
```

As a Git Submodule

Alternatively, you can add the library as a git submodule:

```bash
git submodule add git@github.com:nick-nikolov-00/mbedfmt.git external/mbedfmt
```

Then in your CMakeLists.txt:

```cmake
add_subdirectory(external/mbedfmt)
target_link_libraries(your_target PRIVATE mbedfmt)
```

## Core API Reference

### Format String Conversion Macros

#### MBEDFMT_FMT_TO_PRINTF_CSTR (recommended, C++20 only)

```c++
MBEDFMT_FMT_TO_PRINTF_CSTR(fmt, ...)
```

Returns a compile-time string literal translating the fmt formatted string to printf format.

Arguments

- `fmt` - the fmt string
- `...` - arguments that will be formatted using fmt

##### Example

```c++
constexpr const char* c1 = MBEDFMT_FMT_TO_PRINTF_CSTR("ints: {} {}", 1, 1);          // c1 is "ints: %d %d"
constexpr const char* c2 = MBEDFMT_FMT_TO_PRINTF_CSTR("float: {:.3}", (float) 1.0);  // c2 is "float: %.3f"
constexpr const char* c3 = MBEDFMT_FMT_TO_PRINTF_CSTR("string: {}", "s");            // c3 is "string: %s"

printf(c1, 1, 1);
```

#### MBEDFMT_FMT_TO_PRINTF_ARR (C++17 and C++20)

```c++
MBEDFMT_FMT_TO_PRINTF_ARR(fmt, ...)
```

Returns a compile-time null-terminated std::array<char, N> translating the fmt formatted string to printf format. Same arguments as
`MBEDFMT_FMT_TO_PRINTF_CSTR`. Note that since it returns an std::array it needs to be printed using `.data()`.

##### Example

```c++
constexpr auto c1 = MBEDFMT_FMT_TO_PRINTF_ARR("ints: {} {}", (int) 1, (int) 1); // c1 is "ints: %d %d"

printf(c1.data(), 1, 1);
```

## Usage

A small example program that uses printf to print the converted fmt strings. The C++20 syntax is recommended if
available, as it is identical to the equivalent printf usage in terms of generated code. Due to limitations of C++17,
the converted strings are returned as a std::array\<char\>, which may generate slightly different code than using pure
printf.

```c++
#include <cstdio>
#include <string>

#include "mbedfmt.hpp"

template <class... Args>
void convertingPrint(const char* fmt, const Args&... args) {
  printf(fmt, mbedfmt::convert(args)...);
}

// C++17 syntax
#define LOG(fmt, ...)                                                      \
  do {                                                                     \
    static constexpr auto printfLog =                                      \
        MBEDFMT_FMT_TO_PRINTF_ARR(fmt, __VA_ARGS__);                       \
    convertingPrint(printfLog.data() __VA_OPT__(, __VA_ARGS__));           \
  } while (0)

// C++20 syntax
#define LOG(fmt, ...)                                                      \
  do {                                                                     \
    static constexpr const char* printfLog =                               \
        MBEDFMT_FMT_TO_PRINTF_CSTR(fmt, __VA_ARGS__);                      \
    convertingPrint(printfLog __VA_OPT__(, __VA_ARGS__));                  \
  } while (0)
    
enum TestEnum { ZERO, ONE, TWO };
    
int main() {
  unsigned u = 1;
  
  const char* cstr = "string";
  std::string stdString = "std string";
  
  LOG("string: {}\n", cstr);          // converts to "string: %s" and prints "string: string"
  LOG("string: {}\n", stdString);     // converts to "string: %s" and prints "string: std string"
  
  LOG("enum: {}\n", TestEnum::TWO);   // converts to "enum: %u" (compiler dependent) and prints "enum: 2"
  
  LOG("address of u: {} size of u: {:>3}\n",      // converts to "address of u: %p size of u: %3lu" (compiler dependent)
      &u,                                         // prints "address of u: 0x7fff218d78c4 size of u:   4"
      sizeof(u));
  
  LOG("double with precision: {:.4}\n",           // converts to "double with precision: %.4"
      3.14159265);                                // prints "double with precision: 3.1416"
  
  return 0;
}
```

### Custom Type Formatters

You can extend mbedfmt to support custom types by specializing the type_formatter template:

```c++
struct Point2D {
  int x, y;
};

template <>
struct mbedfmt::type_formatter<Point2D> {
  static auto convert(const Point2D& p) {
    std::string formatted = "x:" + std::to_string(p.x) +
                            ",y:" + std::to_string(p.y);
  
    return formatted;
  }
};

// Usage
LOG("point: {}", Point2D(2, 3)); // prints "point: x:2,y:3"
```

#### Custom Enum Formatter Example

```c++
enum class Status {
  OK,
  ERROR,
  PENDING
};

template <>
struct mbedfmt::type_formatter<Status> {
  static auto convert(const Status& s) {
    switch (s) {
    case Status::OK:      return "OK";
    case Status::ERROR:   return "ERROR";
    case Status::PENDING: return "PENDING";
    default:              return "UNKNOWN";
    }
  }
};

// Usage
LOG("status: {}", Status::OK); // prints: "status: OK"
```

## Format Specifiers Support

The library supports most common fmt format specifiers:

- Alignment: `{:<} {:>} {:^}`
- Signs: `{:+} {:-} {: }`
- Width: `{:10}`
- Precision: `{:.2}`
- Type specifiers: `{:x} {:X} {:o} {:f}` etc.

### Limitations

- No support for named arguments
- No support for dynamic width/precision based on some argument
- Center alignment ({:^}) is not supported as printf doesn't support it

### Requirements

- C++17 or later
- [CTRE library](https://github.com/hanickadot/compile-time-regular-expressions) (for compile-time regex)
