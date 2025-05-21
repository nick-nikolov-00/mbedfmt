#pragma once

#include <utility>

namespace mbedfmt::internal {

template <char... chars>
struct tstring : std::integer_sequence<char, chars...> {
    static constexpr const char c_str[sizeof...(chars) + 1] = {chars..., '\0'};
};

template <class... Args>
struct arg_pack {};

template <class... Args>
auto toArgPack(const Args&...) {
    return arg_pack<Args...>();
}

} // namespace mbedfmt::internal

template <typename T, T... chars>
constexpr mbedfmt::internal::tstring<chars...> operator""_mbedfmt_tstr() {
    return {};
}