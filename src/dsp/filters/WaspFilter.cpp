#include "WaspFilter.h"

#include <cmath>

#include "../common/AudioMath.h"
#include "../common/Saturation.h"

namespace mmf::dsp::filters {

void WaspFilter::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    cutoffSmoother_.Init(sampleRate_, 4.0f, 1000.0f);
    resonanceSmoother_.Init(sampleRate_, 4.0f, 0.25f);
    dirtSmoother_.Init(sampleRate_, 6.0f, 0.45f);
    Reset();
}

void WaspFilter::SetSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    cutoffSmoother_.Init(sampleRate_, 4.0f, cutoffSmoother_.GetCurrent());
    resonanceSmoother_.Init(sampleRate_, 4.0f, resonanceSmoother_.GetCurrent());
    dirtSmoother_.Init(sampleRate_, 6.0f, dirtSmoother_.GetCurrent());
}

void WaspFilter::Reset() {
    ic1eq_ = 0.0f;
    ic2eq_ = 0.0f;
}

void WaspFilter::SetParams(const FilterSlotParams& params) {
    params_ = params;
    cutoffSmoother_.SetTarget(params.common.cutoffHz);
    resonanceSmoother_.SetTarget(params.common.resonance);
    dirtSmoother_.SetTarget(common::Clamp(params.model.p1, 0.0f, 1.0f));
}

float WaspFilter::CmosLikeStage(float x, float dirt, float bias) {
    const float d = common::Clamp(dirt, 0.0f, 1.0f);
    const float xb = x + bias;

    const float pos = common::Saturation::FastTanh((1.0f + 3.2f * d) * xb);
    const float neg = common::Saturation::FastTanh((0.75f + 2.1f * d) * xb);
    const float asym = (xb >= 0.0f) ? pos : neg;

    // Adds inverter-like grain near zero crossing rather than plain input tanh.
    const float crossover = common::Saturation::SoftClip((3.5f + 4.0f * d) * (xb - 0.03f * d));
    return 0.72f * asym + 0.28f * crossover;
}

void WaspFilter::UpdateCoefficients(float cutoffHz, float resonanceNorm, float dirt) {
    const float fc = common::Clamp(cutoffHz, 20.0f, 0.42f * sampleRate_);
    const float d = common::Clamp(dirt, 0.0f, 1.0f);

    // Wasp-style response: rougher resonance law than clean SEM SVF.
    const float q = 0.5f + resonanceNorm * (14.0f - 4.5f * d);
    k_ = 1.0f / q;

    const float g = std::tan(common::kPi * fc / sampleRate_);
    a1_ = 1.0f / (1.0f + g * (g + k_));
    a2_ = g * a1_;
    a3_ = g * a2_;
}

float WaspFilter::Process(float x) {
    const float cutoff = cutoffSmoother_.Process();
    const float resonance = common::Clamp(resonanceSmoother_.Process(), 0.0f, 1.0f);
    const float dirt = common::Clamp(dirtSmoother_.Process(), 0.0f, 1.0f);
    UpdateCoefficients(cutoff, resonance, dirt);

    const float drive = std::max(params_.common.drive, 0.0f);
    const float inputBias = -0.035f + 0.055f * dirt;
    const float in = CmosLikeStage(x * drive, dirt, inputBias);

    const float fb = CmosLikeStage(ic1eq_ + 0.45f * ic2eq_, dirt, 0.015f * (1.0f - dirt));
    const float v3 = in - CmosLikeStage(ic2eq_, dirt, -0.02f * dirt) - (1.0f + 0.65f * dirt) * k_ * fb;

    const float v1 = a1_ * ic1eq_ + a2_ * v3;
    const float v2 = ic2eq_ + a2_ * ic1eq_ + a3_ * v3;

    const float integShape = 0.22f + 0.48f * dirt;
    const float v1nl = common::Lerp(v1, CmosLikeStage(v1, dirt, 0.01f), integShape);
    const float v2nl = common::Lerp(v2, CmosLikeStage(v2, dirt, -0.012f), integShape);

    ic1eq_ = 2.0f * v1nl - ic1eq_;
    ic2eq_ = 2.0f * v2nl - ic2eq_;

    float wet = v2nl;
    switch (static_cast<WaspMode>(params_.model.mode)) {
    case WaspMode::LP:
        wet = v2nl;
        break;
    case WaspMode::BP:
        wet = v1nl;
        break;
    case WaspMode::HP:
        wet = v3;
        break;
    }

    const float rasp = CmosLikeStage(wet, dirt, -0.008f);
    wet = common::Lerp(wet, rasp, 0.18f + 0.36f * dirt);

    // TODO: Replace shaping stages with calibrated CMOS inverter transfer model.
    // TODO: Introduce OTA current-domain model and bias/starve behavior.
    // TODO: Refine cutoff tracking and resonance law per sample-rate.
    const float mix = common::Clamp(params_.common.mix, 0.0f, 1.0f);
    const float y = x + mix * (wet - x);
    return y * params_.common.level;
}

} // namespace mmf::dsp::filters
