#pragma once

#include "FilterParams.h"

namespace mmf::dsp::filters {

// Low-pass gate scaffold.
// Natural use: plucky/organic amplitude+tone coupling (LPG), plus VCA-only and LP-only variants.
// Limitation: simplified response model; not a full physical vactrol simulation yet.
class LPG {
  public:
    void Init(float sampleRate);
    void SetSampleRate(float sampleRate);
    void Reset();
    void SetParams(const FilterSlotParams& params);
    float Process(float x);

  private:
    float sampleRate_ = 48000.0f;
    FilterSlotParams params_{};
    float env_ = 0.0f;
    float lpState_ = 0.0f;
};

} // namespace mmf::dsp::filters
