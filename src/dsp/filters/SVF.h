#pragma once

#include "FilterParams.h"
#include "FilterTypes.h"
#include "../common/ParameterSmoother.h"

namespace mmf::dsp::filters {

// SEM-style state variable filter (TPT form).
// Natural use: LP/HP/BP/Notch, clean and modulation-friendly.
// Limitation: this is intentionally neutral; use ladder models for stronger color.
class SVF {
  public:
    void Init(float sampleRate);
    void SetSampleRate(float sampleRate);
    void Reset();
    void SetParams(const FilterSlotParams& params);
    float Process(float x);

  private:
    void UpdateCoefficients(float cutoffHz, float resonanceNorm);

    float sampleRate_ = 48000.0f;
    FilterSlotParams params_{};

    common::ParameterSmoother cutoffSmoother_;
    common::ParameterSmoother resonanceSmoother_;

    float ic1eq_ = 0.0f;
    float ic2eq_ = 0.0f;

    float a1_ = 0.0f;
    float a2_ = 0.0f;
    float a3_ = 0.0f;
    float k_ = 1.0f;
};

} // namespace mmf::dsp::filters
