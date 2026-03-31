#pragma once

#include "FilterParams.h"

namespace mmf::dsp::filters {

// Optional common interface for tooling/testing.
// Runtime engine does direct static dispatch per model to avoid virtual calls in hot path.
class IFilterModel {
  public:
    virtual ~IFilterModel() = default;
    virtual void Init(float sampleRate) = 0;
    virtual void Reset() = 0;
    virtual void SetSampleRate(float sampleRate) = 0;
    virtual void SetParams(const FilterSlotParams& params) = 0;
    virtual float Process(float x) = 0;
};

} // namespace mmf::dsp::filters
