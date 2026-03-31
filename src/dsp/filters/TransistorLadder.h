#pragma once

#include "FilterParams.h"
#include "FilterTypes.h"
#include "../common/ParameterSmoother.h"

namespace mmf::dsp::filters {

// Mini/Moog-like transistor ladder approximation.
// Natural use: musical low-pass with 1/2/3/4-pole taps.
// Limitation: current version is stable and simple, with mild nonlinearity (not yet full ZDF ladder).
class TransistorLadder {
  public:
    void Init(float sampleRate);
    void SetSampleRate(float sampleRate);
    void Reset();
    void SetParams(const FilterSlotParams& params);
    float Process(float x);

  private:
    void UpdateCoefficient(float cutoffHz);

    float sampleRate_ = 48000.0f;
    FilterSlotParams params_{};
    common::ParameterSmoother cutoffSmoother_;
    common::ParameterSmoother resonanceSmoother_;

    float g_ = 0.1f;
    float stage_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

} // namespace mmf::dsp::filters
