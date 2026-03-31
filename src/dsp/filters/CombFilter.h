#pragma once

#include "FilterParams.h"
#include "FilterTypes.h"
#include "../common/DelayLine.h"
#include "../common/ParameterSmoother.h"

namespace mmf::dsp::filters {

// Comb filter family for resonator/metallic/string-ish behavior.
// Natural use: feedforward, feedback and lowpass-feedback topologies.
// Limitation: first pass uses linear-interpolated fractional delay and simple damping.
class CombFilter {
  public:
    void Init(float sampleRate);
    void SetSampleRate(float sampleRate);
    void Reset();
    void SetParams(const FilterSlotParams& params);
    float Process(float x);

  private:
    static constexpr size_t kMaxDelaySamples = 96000;

    float MapDelaySamples(float normalized) const;

    float sampleRate_ = 48000.0f;
    FilterSlotParams params_{};
    common::ParameterSmoother delaySmoother_;
    common::DelayLine<kMaxDelaySamples> delayLine_;

    float lpState_ = 0.0f;
};

} // namespace mmf::dsp::filters
