#include "SoftAsymSaturator.h"

#include <algorithm>

#include "../common/AudioMath.h"
#include "../common/SaturationCurves.h"

namespace mmf::dsp::saturation {

void SoftAsymSaturator::Init(float sampleRate) {
    sampleRate_ = sampleRate;

    driveSmoother_.Init(sampleRate_, 8.0f, 1.0f);
    toneSmoother_.Init(sampleRate_, 10.0f, 0.5f);
    mixSmoother_.Init(sampleRate_, 8.0f, 1.0f);
    outputSmoother_.Init(sampleRate_, 8.0f, 1.0f);
    biasSmoother_.Init(sampleRate_, 10.0f, 0.0f);

    preHighpassProxy_.Init(sampleRate_, 45.0f);
    preToneLowpass_.Init(sampleRate_, 12000.0f);
    postLowpass_.Init(sampleRate_, 12000.0f);
    dcBlock_.Init(sampleRate_, 6.0f);
    Reset();
}

void SoftAsymSaturator::Reset() {
    preHighpassProxy_.Reset();
    preToneLowpass_.Reset();
    postLowpass_.Reset();
    dcBlock_.Reset();
}

void SoftAsymSaturator::SetSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    preHighpassProxy_.SetSampleRate(sampleRate_);
    preToneLowpass_.SetSampleRate(sampleRate_);
    postLowpass_.SetSampleRate(sampleRate_);
    dcBlock_.Init(sampleRate_, 6.0f);
}

void SoftAsymSaturator::SetCommonParams(const SaturationCommonParams& params) {
    common_ = params;
    driveSmoother_.SetTarget(common_.drive);
    toneSmoother_.SetTarget(common_.tone);
    mixSmoother_.SetTarget(common_.mix);
    outputSmoother_.SetTarget(common_.output);
}

void SoftAsymSaturator::SetModelParams(const SaturationModelParams& params) {
    model_ = params;
    biasSmoother_.SetTarget(model_.bias);
}

float SoftAsymSaturator::Process(float x) {
    const float drive = std::max(0.0f, driveSmoother_.Process());
    const float tone = common::Clamp(toneSmoother_.Process(), 0.0f, 1.0f);
    const float mix = common::Clamp(mixSmoother_.Process(), 0.0f, 1.0f);
    const float out = std::max(0.0f, outputSmoother_.Process());
    const float bias = common::Clamp(biasSmoother_.Process(), -1.0f, 1.0f);

    preHighpassProxy_.SetCutoff(common::Lerp(20.0f, 120.0f, 1.0f - tone));
    preToneLowpass_.SetCutoff(common::Lerp(4500.0f, 18000.0f, tone));
    postLowpass_.SetCutoff(common::Lerp(6000.0f, 19000.0f, tone));

    const float hp = x - preHighpassProxy_.Process(x);
    const float pre = preToneLowpass_.Process(hp);

    const float driven = pre * (1.0f + 7.0f * drive);
    const float shaped = common::SaturationCurves::AsymSoft(driven, bias);
    const float deDc = dcBlock_.Process(shaped);
    const float wet = postLowpass_.Process(deDc);

    // TODO: Add oversampling and anti-alias filtering for stronger drive settings.
    const float y = x + mix * (wet - x);
    return y * out;
}

} // namespace mmf::dsp::saturation
