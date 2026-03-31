#include "TubeishSaturator.h"

#include <algorithm>
#include <cmath>

#include "../common/AudioMath.h"
#include "../common/SaturationCurves.h"

namespace mmf::dsp::saturation {

void TubeishSaturator::Init(float sampleRate) {
    sampleRate_ = sampleRate;

    driveSmoother_.Init(sampleRate_, 8.0f, 1.0f);
    toneSmoother_.Init(sampleRate_, 11.0f, 0.5f);
    mixSmoother_.Init(sampleRate_, 8.0f, 1.0f);
    outputSmoother_.Init(sampleRate_, 8.0f, 1.0f);
    biasSmoother_.Init(sampleRate_, 10.0f, 0.0f);

    preHighpassProxy_.Init(sampleRate_, 35.0f);
    preLowpass_.Init(sampleRate_, 14000.0f);
    postLowpass_.Init(sampleRate_, 9500.0f);
    Reset();
}

void TubeishSaturator::Reset() {
    preHighpassProxy_.Reset();
    preLowpass_.Reset();
    postLowpass_.Reset();
    envelope_ = 0.0f;
}

void TubeishSaturator::SetSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    preHighpassProxy_.SetSampleRate(sampleRate_);
    preLowpass_.SetSampleRate(sampleRate_);
    postLowpass_.SetSampleRate(sampleRate_);
}

void TubeishSaturator::SetCommonParams(const SaturationCommonParams& params) {
    common_ = params;
    driveSmoother_.SetTarget(common_.drive);
    toneSmoother_.SetTarget(common_.tone);
    mixSmoother_.SetTarget(common_.mix);
    outputSmoother_.SetTarget(common_.output);
}

void TubeishSaturator::SetModelParams(const SaturationModelParams& params) {
    model_ = params;
    biasSmoother_.SetTarget(model_.bias);
}

float TubeishSaturator::Process(float x) {
    const float drive = std::max(0.0f, driveSmoother_.Process());
    const float tone = common::Clamp(toneSmoother_.Process(), 0.0f, 1.0f);
    const float mix = common::Clamp(mixSmoother_.Process(), 0.0f, 1.0f);
    const float out = std::max(0.0f, outputSmoother_.Process());
    const float bias = common::Clamp(biasSmoother_.Process(), -1.0f, 1.0f);

    preHighpassProxy_.SetCutoff(common::Lerp(15.0f, 70.0f, 1.0f - tone));
    preLowpass_.SetCutoff(common::Lerp(3500.0f, 16000.0f, tone));

    const float hp = x - preHighpassProxy_.Process(x);
    const float conditioned = preLowpass_.Process(hp);

    // Very light level dependence for "amp stage" feel.
    envelope_ += 0.004f * (std::fabs(conditioned) - envelope_);
    const float dynamicDrive = 1.0f + 6.5f * drive * (1.0f + 0.20f * envelope_);
    const float driven = conditioned * dynamicDrive + 0.18f * bias;

    const float triodeLike = common::SaturationCurves::TubeVoice(driven, bias);
    const float harmonicLift = common::SaturationCurves::TubeVoice(0.75f * driven, -0.4f * bias);
    float wet = common::Lerp(triodeLike, harmonicLift, 0.14f + 0.08f * common::Clamp(model_.p1, 0.0f, 1.0f));

    const float hfDampingCutoff = common::Lerp(4200.0f, 14500.0f, tone) - 1200.0f * common::Clamp(envelope_, 0.0f, 1.0f);
    postLowpass_.SetCutoff(common::Clamp(hfDampingCutoff, 3200.0f, 18000.0f));
    wet = postLowpass_.Process(wet);

    // TODO: Add more explicit triode plate/cathode parameterisation for v2 voicing.
    const float y = x + mix * (wet - x);
    return y * out;
}

} // namespace mmf::dsp::saturation
