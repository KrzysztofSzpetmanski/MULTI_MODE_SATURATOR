#pragma once

#include "SaturationParams.h"

namespace mmf::dsp::saturation {

class ISaturationModel {
  public:
    virtual ~ISaturationModel() = default;
    virtual void Init(float sampleRate) = 0;
    virtual void Reset() = 0;
    virtual void SetSampleRate(float sampleRate) = 0;
    virtual void SetCommonParams(const SaturationCommonParams& params) = 0;
    virtual void SetModelParams(const SaturationModelParams& params) = 0;
    virtual float Process(float x) = 0;
};

} // namespace mmf::dsp::saturation
