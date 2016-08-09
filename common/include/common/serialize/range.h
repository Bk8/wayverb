#pragma once

#include "common/boundaries.h"
#include "common/serialize/vec.h"

template <typename t>
template <typename archive>
void util::range<t>::serialize(archive& a) {
    a(cereal::make_nvp("min", min), cereal::make_nvp("max", max));
}

template<typename T>
JSON_OSTREAM_OVERLOAD(util::range<T>);