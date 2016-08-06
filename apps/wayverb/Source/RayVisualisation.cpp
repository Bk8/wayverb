#include "RayVisualisation.hpp"

#include "OtherComponents/MoreConversions.hpp"

#include "common/cl_traits.h"

namespace {

aligned::vector<glm::vec3> extract_positions(
        const aligned::vector<aligned::vector<raytracer::impulse>>& impulses,
        const glm::vec3& source) {
    aligned::vector<glm::vec3> ret;
    ret.reserve(impulses.size() * impulses.front().size() + 1);

    //  first in the buffer is the source position
    ret.push_back(source);

    //  now add positions ray-by-ray from the input
    for (const auto& ray : impulses) {
        for (const auto& reflection : ray) {
            ret.push_back(to_glm_vec3(reflection.position));
        }
    }

    return ret;
}

aligned::vector<float> extract_pressures(
        const aligned::vector<aligned::vector<raytracer::impulse>>& impulses) {
    aligned::vector<float> ret;
    ret.reserve(impulses.size() * impulses.front().size() + 1);

    //  source
    ret.push_back(1);

    //  impulses
    for (const auto& ray : impulses) {
        for (const auto& reflection : ray) {
            ret.push_back(std::abs(mean(reflection.volume)) * (1 << 20));
        }
    }

    return ret;
}

aligned::vector<GLuint> compute_indices(
        const aligned::vector<aligned::vector<raytracer::impulse>>& impulses) {
    aligned::vector<GLuint> ret;

    //  impulses
    auto counter = 1u;
    for (const auto& ray : impulses) {
        if (!ray.empty()) {
            ret.push_back(0);  //  source

            const auto lim = counter + ray.size() - 1;
            for (; counter != lim; ++counter) {
                for (auto i = 0; i != 2; ++i) {
                    ret.push_back(counter);
                }
            }

            ret.push_back(counter);
            counter += 1;
        }
    }

    return ret;
}

}  // namespace

//----------------------------------------------------------------------------//

RayShader::RayShader()
        : program(mglu::program::from_sources(vert, frag)) {}

void RayShader::set_model_matrix(const glm::mat4& m) const {
    program.set("v_model", m);
}
void RayShader::set_view_matrix(const glm::mat4& m) const {
    program.set("v_view", m);
}
void RayShader::set_projection_matrix(const glm::mat4& m) const {
    program.set("v_projection", m);
}

const char* RayShader::vert{R"(
#version 150
in vec3 v_position;
in float v_pressure;
out float f_pressure;

uniform mat4 v_model;
uniform mat4 v_view;
uniform mat4 v_projection;

void main() {
    vec4 modelview = v_view * v_model * vec4(v_position, 1.0);
    gl_Position = v_projection * modelview;

    f_pressure = v_pressure;
}
)"};

const char* RayShader::frag{R"(
#version 150
in float f_pressure;
out vec4 frag_color;

void main() {
    frag_color = vec4(vec3(f_pressure), 1.0);
//    frag_color = vec4(1.0);
}
)"};

//----------------------------------------------------------------------------//

RayVisualisation::RayVisualisation(
        const std::shared_ptr<RayShader>& shader,
        const aligned::vector<aligned::vector<raytracer::impulse>>& impulses,
        const glm::vec3& source,
        const glm::vec3& receiver)
        : shader(shader)
        , positions(extract_positions(impulses, source))
        , pressures(extract_pressures(impulses))
        , ibo(compute_indices(impulses))
        , source(source)
        , receiver(receiver) {
    const auto s_vao = vao.get_scoped();
    mglu::enable_and_bind_buffer(vao,
                                 positions,
                                 shader->get_attrib_location_v_position(),
                                 3,
                                 GL_FLOAT);
    mglu::enable_and_bind_buffer(vao,
                                 pressures,
                                 shader->get_attrib_location_v_pressure(),
                                 1,
                                 GL_FLOAT);
    ibo.bind();
}

void RayVisualisation::set_time(float t) {
    //  TODO update positions of virtual wavefront
}

void RayVisualisation::do_draw(const glm::mat4& modelview_matrix) const {
    auto s_shader = shader->get_scoped();
    shader->set_model_matrix(modelview_matrix);

    auto s_vao = vao.get_scoped();
    glDrawElements(GL_LINES, ibo.size(), GL_UNSIGNED_INT, nullptr);
}

glm::mat4 RayVisualisation::get_local_modelview_matrix() const {
    return glm::mat4{};
}