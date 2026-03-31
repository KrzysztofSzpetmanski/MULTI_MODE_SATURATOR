#include "DiodeSaturator.h"

#include <algorithm>

#include "../common/AudioMath.h"
#include "../common/Saturation.h"
#include "../common/SaturationCurves.h"

namespace mmf::dsp::saturation {

void DiodeSaturator::Init(float sampleRate) {
    sampleRate_ = sampleRate;

    driveSmoother_.Init(sampleRate_, 6.0f, 1.0f);
    toneSmoother_.Init(sampleRate_, 9.0f, 0.5f);
    mixSmoother_.Init(sampleRate_, 6.0f, 1.0f);
    outputSmoother_.Init(sampleRate_, 6.0f, 1.0f);
    biasSmoother_.Init(sampleRate_, 9.0f, 0.0f);

    preLowpass_.Init(sampleRate_, 5200.0f);
    postLowpass_.Init(sampleRate_, 11000.0f);
    dcBlock_.Init(sampleRate_, 6.0f);
    Reset();
}

void DiodeSaturator::Reset() {
    preLowpass_.Reset();
    postLowpass_.Reset();
    dcBlock_.Reset();
}

void DiodeSaturator::SetSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    preLowpass_.SetSampleRate(sampleRate_);
    postLowpass_.SetSampleRate(sampleRate_);
    dcBlock_.Init(sampleRate_, 6.0f);
}

void DiodeSaturator::SetCommonParams(const SaturationCommonParams& params) {
    common_ = params;
    driveSmoother_.SetTarget(common_.drive);
    toneSmoother_.SetTarget(common_.tone);
    mixSmoother_.SetTarget(common_.mix);
    outputSmoother_.SetTarget(common_.output);
}

void DiodeSaturator::SetModelParams(const SaturationModelParams& params) {
    model_ = params;
    biasSmoother_.SetTarget(model_.bias);
}

float DiodeSaturator::Process(float x) {
    const float drive = std::max(0.0f, driveSmoother_.Process());
    const float tone = common::Clamp(toneSmoother_.Process(), 0.0f, 1.0f);
    const float mix = common::Clamp(mixSmoother_.Process(), 0.0f, 1.0f);
    const float out = std::max(0.0f, outputSmoother_.Process());
    const float bias = common::Clamp(biasSmoother_.Process(), -1.0f, 1.0f);

    // Pragmatic physically-informed v1: pre-emphasis low-pass before static nonlinearity.
    preLowpass_.SetCutoff(common::Lerp(1100.0f, 12000.0f, tone));
    postLowpass_.SetCutoff(common::Lerp(4500.0f, 15000.0f, tone));

    const float pre = preLowpass_.Process(x);
    const float v = pre * (1.0f + 10.0f * drive) + 0.20f * bias;

    const float hardness = 1.2f + 8.0f * drive;
    const float diode = common::SaturationCurves::DiodePair(v, hardness);
    const float extraEdge = common::Saturation::SoftClip((3.0f + 4.0f * drive) * v);
    const float shaped = common::Lerp(diode, extraEdge, 0.18f + 0.08f * common::Clamp(model_.p1, 0.0f, 1.0f));

    const float deDc = dcBlock_.Process(shaped);
    const float wet = postLowpass_.Process(deDc);

    // TODO: Replace static transfer with higher-fidelity diode network approximation.
    const float y = x + mix * (wet - x);
    return y * out;
}

} // namespace mmf::dsp::saturation
