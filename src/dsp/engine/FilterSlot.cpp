#include "FilterSlot.h"

namespace mmf::dsp::engine {

void FilterSlot::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    svf_.Init(sampleRate_);
    transistorLadder_.Init(sampleRate_);
    diodeLadder_.Init(sampleRate_);
    lpg_.Init(sampleRate_);
    comb_.Init(sampleRate_);
    biquad_.Init(sampleRate_);
    PushParamsToActiveModel();
}

void FilterSlot::SetSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    svf_.SetSampleRate(sampleRate_);
    transistorLadder_.SetSampleRate(sampleRate_);
    diodeLadder_.SetSampleRate(sampleRate_);
    lpg_.SetSampleRate(sampleRate_);
    comb_.SetSampleRate(sampleRate_);
    biquad_.SetSampleRate(sampleRate_);
}

void FilterSlot::Reset() {
    svf_.Reset();
    transistorLadder_.Reset();
    diodeLadder_.Reset();
    lpg_.Reset();
    comb_.Reset();
    biquad_.Reset();
}

void FilterSlot::SetModel(filters::FilterModelType model) {
    model_ = model;
    PushParamsToActiveModel();
}

void FilterSlot::SetParams(const filters::FilterSlotParams& params) {
    params_ = params;
    bypass_ = params.bypass;
    PushParamsToActiveModel();
}

void FilterSlot::PushParamsToActiveModel() {
    switch (model_) {
    case filters::FilterModelType::SVF:
        svf_.SetParams(params_);
        break;
    case filters::FilterModelType::TransistorLadder:
        transistorLadder_.SetParams(params_);
        break;
    case filters::FilterModelType::DiodeLadder:
        diodeLadder_.SetParams(params_);
        break;
    case filters::FilterModelType::LPG:
        lpg_.SetParams(params_);
        break;
    case filters::FilterModelType::Comb:
        comb_.SetParams(params_);
        break;
    case filters::FilterModelType::Biquad:
        biquad_.SetParams(params_);
        break;
    }
}

float FilterSlot::ProcessSample(float x) {
    if (bypass_) {
        return x;
    }

    switch (model_) {
    case filters::FilterModelType::SVF:
        return svf_.Process(x);
    case filters::FilterModelType::TransistorLadder:
        return transistorLadder_.Process(x);
    case filters::FilterModelType::DiodeLadder:
        return diodeLadder_.Process(x);
    case filters::FilterModelType::LPG:
        return lpg_.Process(x);
    case filters::FilterModelType::Comb:
        return comb_.Process(x);
    case filters::FilterModelType::Biquad:
        return biquad_.Process(x);
    }
    return x;
}

} // namespace mmf::dsp::engine
