#pragma once

#include "core/cl/scene_structs.h"

#include "utilities/mapping_iterator_adapter.h"

#include <iterator>
#include <type_traits>

namespace wayverb {
namespace core {

namespace detail {

class cl_type_index_mapper final {
public:
    cl_type_index_mapper(size_t index)
            : index_(index) {}

    template <typename T>
    constexpr auto& operator()(T& t) const {
        return t.s[index_];
    }

private:
    size_t index_{0};
};

}  // namespace detail

/// An iterator adapter that is designed to allow iterating over a single 'band'
/// of a collection of cl_ vectors.
/// For example, given a std::vector<cl_float4>, you can use this adapter to
/// iterate over only the x component of each cl_float4.
template <typename It>
using cl_type_iterator =
        util::mapping_iterator_adapter<It, detail::cl_type_index_mapper>;

/// Utility function that allows a wrapped iterator to be created from another
/// iterator type, without needing to be explicit about the wrapped type.
template <class It>
constexpr cl_type_iterator<It> make_cl_type_iterator(It it, size_t index) {
    return util::make_mapping_iterator_adapter(
            std::move(it), detail::cl_type_index_mapper{index});
}

}  // namespace core
}  // namespace wayverb
