#pragma once

#include "waveguide/mesh_descriptor.h"

namespace wayverb {
namespace waveguide {
namespace preprocessor {

class gaussian final {
public:
    static float compute(const glm::vec3& x, float sdev);

    gaussian(const mesh_descriptor& descriptor,
             const glm::vec3& centre_pos,
             float sdev,
             size_t steps);

    bool operator()(cl::CommandQueue& queue,
                    cl::Buffer& buffer,
                    size_t step) const;

private:
    mesh_descriptor descriptor_;
    glm::vec3 centre_pos_;
    float sdev_;
    size_t steps_;
};

}  // namespace preprocessor
}  // namespace waveguide
}  // namespace wayverb
