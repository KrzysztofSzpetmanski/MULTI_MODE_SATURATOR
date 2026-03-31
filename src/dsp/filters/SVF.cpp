#include "SVF.h"

#include <cmath>

#include "../common/AudioMath.h"
#include "../common/Saturation.h"

namespace mmf::dsp::filters {

void SVF::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    cutoffSmoother_.Init(sampleRate_, 5.0f, 1000.0f);
    resonanceSmoother_.Init(sampleRate_, 5.0f, 0.0f);
    Reset();
    SetParams(params_);
}

void SVF::SetSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    cutoffSmoother_.Init(sampleRate_, 5.0f, cutoffSmoother_.GetCurrent());
    resonanceSmoother_.Init(sampleRate_, 5.0f, resonanceSmoother_.GetCurrent());
}

void SVF::Reset() {
    ic1eq_ = 0.0f;
    ic2eq_ = 0.0f;
}

void SVF::SetParams(const FilterSlotParams& params) {
    params_ = params;
    cutoffSmoother_.SetTarget(params.common.cutoffHz);
    resonanceSmoother_.SetTarget(params.common.resonance);
}

void SVF::UpdateCoefficients(float cutoffHz, float resonanceNorm) {
    const float fc = common::Clamp(cutoffHz, 20.0f, 0.45f * sampleRate_);
    const float q = 0.5f + resonanceNorm * 24.5f;
    k_ = 1.0f / q;

    const float g = std::tan(common::kPi * fc / sampleRate_);
    a1_ = 1.0f / (1.0f + g * (g + k_));
    a2_ = g * a1_;
    a3_ = g * a2_;
}

float SVF::Process(float x) {
    // TODO: Optional oversampling mode for extreme cutoff FM + high drive use-cases.
    const float cutoff = cutoffSmoother_.Process();
    const float resonance = common::Clamp(resonanceSmoother_.Process(), 0.0f, 1.0f);
    UpdateCoefficients(cutoff, resonance);

    const float drive = params_.common.drive < 0.0f ? 0.0f : params_.common.drive;
    const float in = common::Saturation::FastTanh(x * drive);

    const float v3 = in - ic2eq_ - k_ * ic1eq_;
    const float v1 = a1_ * ic1eq_ + a2_ * v3;
    const float v2 = ic2eq_ + a2_ * ic1eq_ + a3_ * v3;

    ic1eq_ = 2.0f * v1 - ic1eq_;
    ic2eq_ = 2.0f * v2 - ic2eq_;

    float wet = v2;
    switch (static_cast<SvfMode>(params_.model.mode)) {
    case SvfMode::LP:
        wet = v2;
        break;
    case SvfMode::HP:
        wet = v3;
        break;
    case SvfMode::BP:
        wet = v1;
        break;
    case SvfMode::NOTCH:
        wet = v3 + v2;
        break;
    }

    const float mix = common::Clamp(params_.common.mix, 0.0f, 1.0f);
    const float y = x + mix * (wet - x);
    return y * params_.common.level;
}

} // namespace mmf::dsp::filters
