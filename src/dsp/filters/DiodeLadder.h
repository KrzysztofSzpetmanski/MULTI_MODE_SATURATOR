#pragma once

#include "FilterParams.h"

namespace mmf::dsp::filters {

// Diode ladder placeholder.
// Natural future use: distinct, more aggressive ladder character than transistor ladder.
// Limitation: this is a scaffold with basic LP behavior, not a calibrated diode model yet.
class DiodeLadder {
  public:
    void Init(float sampleRate);
    void SetSampleRate(float sampleRate);
    void Reset();
    void SetParams(const FilterSlotParams& params);
    float Process(float x);

  private:
    float sampleRate_ = 48000.0f;
    FilterSlotParams params_{};
    float stage_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

} // namespace mmf::dsp::filters
