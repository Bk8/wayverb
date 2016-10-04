#pragma once

#include "raytracer/cl/structs.h"

#include "common/map_to_vector.h"

#include "glm/glm.hpp"

namespace raytracer {

template <typename Method, size_t channels>
auto attenuate(const Method& method,
               const glm::vec3& position,
               const impulse<channels>& i) {
    const auto dir{to_vec3(i.position) - position};
    const auto att{attenuation(method, dir)};
    return attenuated_impulse<channels>{i.volume * att, i.distance};
}

template <typename Method, typename It>
auto attenuate(const Method& method,
               const glm::vec3& position,
               It begin,
               It end) {
    return map_to_vector(begin, end, [&](const auto& impulse) {
        return attenuate(method, position, impulse);
    });
}

}  // namespace raytracer