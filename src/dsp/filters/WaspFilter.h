#pragma once

#include "FilterParams.h"
#include "FilterTypes.h"
#include "../common/ParameterSmoother.h"

namespace mmf::dsp::filters {

// Pragmatic VA-inspired Wasp-style filter approximation (not a white-box circuit model).
// Natural use: dirty 2-pole LP/BP/HP with raspy resonance and buzzy drive response.
// Limitation: current model uses CMOS-like nonlinear shaping around a TPT-SVF core.
class WaspFilter {
  public:
    void Init(float sampleRate);
    void SetSampleRate(float sampleRate);
    void Reset();
    void SetParams(const FilterSlotParams& params);
    float Process(float x);

  private:
    static float CmosLikeStage(float x, float dirt, float bias);
    void UpdateCoefficients(float cutoffHz, float resonanceNorm, float dirt);

    float sampleRate_ = 48000.0f;
    FilterSlotParams params_{};

    common::ParameterSmoother cutoffSmoother_;
    common::ParameterSmoother resonanceSmoother_;
    common::ParameterSmoother dirtSmoother_;

    float ic1eq_ = 0.0f;
    float ic2eq_ = 0.0f;

    float a1_ = 0.0f;
    float a2_ = 0.0f;
    float a3_ = 0.0f;
    float k_ = 1.0f;
};

} // namespace mmf::dsp::filters
