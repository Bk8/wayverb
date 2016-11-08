#pragma once

#include "combined/model/member.h"

#include "core/attenuator/microphone.h"

namespace wayverb {
namespace combined {
namespace model {

class microphone final : public member<microphone> {
public:
    microphone() = default;

    void set_orientation(float azimuth, float elevation);
    void set_shape(double shape);

    core::attenuator::microphone get() const;

private:
    core::attenuator::microphone microphone_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
