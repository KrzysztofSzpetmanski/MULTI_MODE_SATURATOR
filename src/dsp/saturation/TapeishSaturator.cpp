#include "TapeishSaturator.h"

#include <algorithm>
#include <cmath>

#include "../common/AudioMath.h"
#include "../common/SaturationCurves.h"

namespace mmf::dsp::saturation {

void TapeishSaturator::Init(float sampleRate) {
    sampleRate_ = sampleRate;

    driveSmoother_.Init(sampleRate_, 14.0f, 1.0f);
    toneSmoother_.Init(sampleRate_, 15.0f, 0.5f);
    mixSmoother_.Init(sampleRate_, 12.0f, 1.0f);
    outputSmoother_.Init(sampleRate_, 12.0f, 1.0f);
    biasSmoother_.Init(sampleRate_, 16.0f, 0.0f);

    preHighpassProxy_.Init(sampleRate_, 32.0f);
    preLowpass_.Init(sampleRate_, 10500.0f);
    postLowpass_.Init(sampleRate_, 8200.0f);
    Reset();
}

void TapeishSaturator::Reset() {
    preHighpassProxy_.Reset();
    preLowpass_.Reset();
    postLowpass_.Reset();
    envelope_ = 0.0f;
}

void TapeishSaturator::SetSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    preHighpassProxy_.SetSampleRate(sampleRate_);
    preLowpass_.SetSampleRate(sampleRate_);
    postLowpass_.SetSampleRate(sampleRate_);
}

void TapeishSaturator::SetCommonParams(const SaturationCommonParams& params) {
    common_ = params;
    driveSmoother_.SetTarget(common_.drive);
    toneSmoother_.SetTarget(common_.tone);
    mixSmoother_.SetTarget(common_.mix);
    outputSmoother_.SetTarget(common_.output);
}

void TapeishSaturator::SetModelParams(const SaturationModelParams& params) {
    model_ = params;
    biasSmoother_.SetTarget(model_.bias);
}

float TapeishSaturator::Process(float x) {
    const float drive = std::max(0.0f, driveSmoother_.Process());
    const float tone = common::Clamp(toneSmoother_.Process(), 0.0f, 1.0f);
    const float mix = common::Clamp(mixSmoother_.Process(), 0.0f, 1.0f);
    const float out = std::max(0.0f, outputSmoother_.Process());
    const float bias = common::Clamp(biasSmoother_.Process(), -1.0f, 1.0f);

    preHighpassProxy_.SetCutoff(common::Lerp(16.0f, 55.0f, 1.0f - tone));
    preLowpass_.SetCutoff(common::Lerp(2200.0f, 12500.0f, tone));

    const float hp = x - preHighpassProxy_.Process(x);
    const float pre = preLowpass_.Process(hp);

    envelope_ += 0.0025f * (std::fabs(pre) - envelope_);
    const float compress = 1.0f / (1.0f + 0.85f * envelope_ * common::Clamp(0.3f + 0.4f * drive, 0.0f, 2.0f));

    const float driven = pre * (1.0f + 3.2f * drive) * compress + 0.08f * bias;
    float wet = common::SaturationCurves::TapeSoft(driven);

    const float hfCutoff = common::Lerp(3800.0f, 11800.0f, tone) - 2100.0f * common::Clamp(envelope_, 0.0f, 1.0f);
    postLowpass_.SetCutoff(common::Clamp(hfCutoff, 2500.0f, 15000.0f));
    wet = postLowpass_.Process(wet);

    // TODO: Add dedicated tape-bias signal and hysteresis-inspired branch.
    // TODO: Add oversampling path for higher drive values and better HF retention.
    const float y = x + mix * (wet - x);
    return y * out;
}

} // namespace mmf::dsp::saturation
