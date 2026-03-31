#include "CombFilter.h"

#include "../common/AudioMath.h"

namespace mmf::dsp::filters {

void CombFilter::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    delaySmoother_.Init(sampleRate_, 10.0f, 220.0f);
    Reset();
}

void CombFilter::SetSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    delaySmoother_.Init(sampleRate_, 10.0f, delaySmoother_.GetCurrent());
}

void CombFilter::Reset() {
    delayLine_.Reset();
    lpState_ = 0.0f;
}

void CombFilter::SetParams(const FilterSlotParams& params) {
    params_ = params;
    delaySmoother_.SetTarget(MapDelaySamples(params.model.p1));
}

float CombFilter::MapDelaySamples(float normalized) const {
    // p1 is normalized in UI/runtime; architecture ready for future tune->fractional delay mapping.
    const float n = common::Clamp(normalized, 0.0f, 1.0f);
    const float minDelay = 1.0f;
    const float maxDelay = static_cast<float>(kMaxDelaySamples - 2);
    return minDelay + n * (maxDelay - minDelay);
}

float CombFilter::Process(float x) {
    // TODO: Add modulation input path with anti-zipper strategy for chorus/flanger-like movement.
    const float delaySamples = delaySmoother_.Process();
    const float delayed = delayLine_.ReadDelayFrac(delaySamples);

    const float feedback = common::Clamp(params_.model.p2, -0.99f, 0.99f);
    const float damping = common::Clamp(params_.model.p3, 0.0f, 1.0f);

    float wet = delayed;
    float writeValue = x;

    switch (static_cast<CombMode>(params_.model.mode)) {
    case CombMode::FEEDFORWARD:
        wet = x + feedback * delayed;
        writeValue = x;
        break;
    case CombMode::FEEDBACK:
        wet = delayed;
        writeValue = x + delayed * feedback;
        break;
    case CombMode::LP_FEEDBACK:
        lpState_ += damping * (delayed - lpState_);
        wet = delayed;
        writeValue = x + lpState_ * feedback;
        break;
    }

    delayLine_.Write(writeValue);

    const float mix = common::Clamp(params_.common.mix, 0.0f, 1.0f);
    const float y = x + mix * (wet - x);
    return y * params_.common.level;
}

} // namespace mmf::dsp::filters
