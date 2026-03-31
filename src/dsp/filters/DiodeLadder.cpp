#include "DiodeLadder.h"

#include <cmath>

#include "../common/AudioMath.h"
#include "../common/Saturation.h"
#include "FilterTypes.h"

namespace mmf::dsp::filters {

void DiodeLadder::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    Reset();
}

void DiodeLadder::SetSampleRate(float sampleRate) { sampleRate_ = sampleRate; }

void DiodeLadder::Reset() {
    stage_[0] = stage_[1] = stage_[2] = stage_[3] = 0.0f;
}

void DiodeLadder::SetParams(const FilterSlotParams& params) { params_ = params; }

float DiodeLadder::Process(float x) {
    // TODO: Replace with dedicated VA diode ladder core (nonlinear, better tuning, proper resonance law).
    // TODO: Candidate for future "MS-20-ish" experiments by pairing/stacking two diode-flavored stages.
    const float fc = common::Clamp(params_.common.cutoffHz, 20.0f, 0.45f * sampleRate_);
    const float g = 1.0f - std::exp(-common::kTwoPi * fc / sampleRate_);
    const float feedback = common::Clamp(params_.common.resonance, 0.0f, 1.0f) * 3.5f;

    float u = common::Saturation::FastTanh(params_.common.drive * x - feedback * stage_[3]);
    for (int i = 0; i < 4; ++i) {
        const float in = (i == 0) ? u : stage_[i - 1];
        stage_[i] += g * (in - stage_[i]);
    }

    float wet = stage_[3];
    const int tap = params_.model.mode;
    if (tap == static_cast<int>(LadderTap::POLE_1)) {
        wet = stage_[0];
    } else if (tap == static_cast<int>(LadderTap::POLE_2)) {
        wet = stage_[1];
    } else if (tap == static_cast<int>(LadderTap::POLE_3)) {
        wet = stage_[2];
    }

    const float mix = common::Clamp(params_.common.mix, 0.0f, 1.0f);
    return (x + mix * (wet - x)) * params_.common.level;
}

} // namespace mmf::dsp::filters
