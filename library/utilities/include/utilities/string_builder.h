#pragma once

#include "utilities/sequential_foreach.h"

#include <sstream>

template <typename... Ts>
inline std::ostream& to_stream(std::ostream& os, const Ts&... ts) {
    sequential_foreach([&os](const auto& i) { os << i; }, ts...);
    return os;
}

template <typename... Ts>
inline std::string build_string(const Ts&... ts) {
    std::stringstream ss;
    to_stream(ss, ts...);
    return ss.str();
}

struct Bracketer final {
    explicit Bracketer(std::ostream& os,
                       const char* open = "{",
                       const char* closed = "}");

    ~Bracketer();

private:
    std::ostream& os;
    const char* closed;
};
