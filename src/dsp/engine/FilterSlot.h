#pragma once

#include "../filters/BiquadFilter.h"
#include "../filters/CombFilter.h"
#include "../filters/DiodeLadder.h"
#include "../filters/FilterParams.h"
#include "../filters/FilterTypes.h"
#include "../filters/LPG.h"
#include "../filters/SVF.h"
#include "../filters/TransistorLadder.h"

namespace mmf::dsp::engine {

class FilterSlot {
  public:
    void Init(float sampleRate);
    void SetSampleRate(float sampleRate);
    void Reset();

    void SetModel(filters::FilterModelType model);
    filters::FilterModelType GetModel() const { return model_; }

    void SetBypass(bool bypass) { bypass_ = bypass; }
    bool IsBypassed() const { return bypass_; }

    void SetParams(const filters::FilterSlotParams& params);
    const filters::FilterSlotParams& GetParams() const { return params_; }

    float ProcessSample(float x);

  private:
    void PushParamsToActiveModel();

    float sampleRate_ = 48000.0f;
    filters::FilterModelType model_ = filters::FilterModelType::SVF;
    filters::FilterSlotParams params_{};
    bool bypass_ = false;

    filters::SVF svf_;
    filters::TransistorLadder transistorLadder_;
    filters::DiodeLadder diodeLadder_;
    filters::LPG lpg_;
    filters::CombFilter comb_;
    filters::BiquadFilter biquad_;
};

} // namespace mmf::dsp::engine
