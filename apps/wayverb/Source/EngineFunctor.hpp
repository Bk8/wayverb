#pragma once

#include "FullModel.hpp"

#include "combined/engine.h"

#include <string>

/// Runs the wayverb engine on all combinations of sources and receivers in a
/// scene, sending notifications to a listener about the current state of the
/// engine.
class EngineFunctor final {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;

        virtual void engine_encountered_error(const std::string& str) = 0;
        virtual void engine_state_changed(wayverb::state state,
                                          double progress) = 0;
        virtual void engine_nodes_changed(
                const aligned::vector<glm::vec3>& positions) = 0;
        virtual void engine_waveguide_visuals_changed(
                const aligned::vector<float>& pressures) = 0;
        virtual void engine_raytracer_visuals_changed(
                const aligned::vector<aligned::vector<raytracer::impulse>>&
                        impulses) = 0;
        virtual void engine_finished() = 0;

    protected:
        ~Listener() noexcept = default;
    };

    EngineFunctor(Listener& listener,
                  std::atomic_bool& keep_going,
                  const std::string& file_name,
                  const model::Persistent& persistent,
                  const copyable_scene_data& scene_data,
                  bool visualise);

    /// Call this to start the simulation.
    void operator()() const;

private:
    void single_pair(Listener& listener,
                     const std::string& file_name,
                     const model::SingleShot& single_shot,
                     const copyable_scene_data& scene_data,
                     bool visualise,
                     const compute_context& compute_context) const;

    /// Receives notifications.
    Listener& listener;

    /// Calling scope may flip this to get the simulation to quit early.
    std::atomic_bool& keep_going;

    /// State.
    std::string file_name;
    model::Persistent persistent;
    copyable_scene_data scene_data;
    bool visualise;
};