#pragma once

#include "FilterParams.h"
#include "FilterTypes.h"
#include "../common/ParameterSmoother.h"

namespace mmf::dsp::filters {

// Utility clean biquad.
// Natural use: LP/HP/BP/Notch/Peak when predictable behavior and low CPU matter.
// Limitation: not meant to emulate analog nonlinearity or ladder-style character.
class BiquadFilter {
  public:
    void Init(float sampleRate);
    void SetSampleRate(float sampleRate);
    void Reset();
    void SetParams(const FilterSlotParams& params);
    float Process(float x);

  private:
    void UpdateCoefficients(float cutoffHz, float resonanceNorm, float peakGainDb);

    float sampleRate_ = 48000.0f;
    FilterSlotParams params_{};

    common::ParameterSmoother cutoffSmoother_;
    common::ParameterSmoother resonanceSmoother_;

    float b0_ = 1.0f;
    float b1_ = 0.0f;
    float b2_ = 0.0f;
    float a1_ = 0.0f;
    float a2_ = 0.0f;

    float z1_ = 0.0f;
    float z2_ = 0.0f;
};

} // namespace mmf::dsp::filters
