#include "LPG.h"

#include <cmath>

#include "../common/AudioMath.h"
#include "FilterTypes.h"

namespace mmf::dsp::filters {

void LPG::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    Reset();
}

void LPG::SetSampleRate(float sampleRate) { sampleRate_ = sampleRate; }

void LPG::Reset() {
    env_ = 0.0f;
    lpState_ = 0.0f;
}

void LPG::SetParams(const FilterSlotParams& params) { params_ = params; }

float LPG::Process(float x) {
    // TODO: Replace with richer vactrol-inspired dual-time-constant response and ringing model.
    const float control = common::Clamp(params_.model.p1, 0.0f, 1.0f);
    const float response = common::Clamp(params_.model.p2, 0.0f, 1.0f);

    const float attack = 1.0f - std::exp(-1.0f / (0.001f * sampleRate_));
    const float decayMs = common::Lerp(10.0f, 1500.0f, response);
    const float decay = 1.0f - std::exp(-1.0f / ((decayMs * 0.001f) * sampleRate_));

    const float target = control;
    if (target > env_) {
        env_ += attack * (target - env_);
    } else {
        env_ += decay * (target - env_);
    }

    const float fc = common::Lerp(80.0f, 12000.0f, env_);
    const float g = 1.0f - std::exp(-common::kTwoPi * fc / sampleRate_);
    lpState_ += g * (x - lpState_);

    const float vca = x * env_;
    const float lp = lpState_;
    const float lpg = lp * env_;

    float wet = lpg;
    switch (static_cast<LpgMode>(params_.model.mode)) {
    case LpgMode::LPG:
        wet = lpg;
        break;
    case LpgMode::VCA:
        wet = vca;
        break;
    case LpgMode::LP:
        wet = lp;
        break;
    }

    const float mix = common::Clamp(params_.common.mix, 0.0f, 1.0f);
    return (x + mix * (wet - x)) * params_.common.level;
}

} // namespace mmf::dsp::filters
