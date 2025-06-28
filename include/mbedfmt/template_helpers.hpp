#pragma once

#include <utility>

namespace {
template <char... chars>
struct tstring : std::integer_sequence<char, chars...> {
    alignas(1) static constexpr const char c_str[] = {chars..., '\0'};
};
}

namespace mbedfmt::internal {

template <class... Args>
struct arg_pack {};

template <class... Args>
auto toArgPack(const Args&...) {
    return arg_pack<Args...>();
}

} // namespace mbedfmt::internal

template <typename T, T... chars>
constexpr tstring<chars...> operator""_mbedfmt_tstr() {
    return {};
}