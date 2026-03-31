#include "BiquadFilter.h"

#include <cmath>

#include "../common/AudioMath.h"

namespace mmf::dsp::filters {

void BiquadFilter::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    cutoffSmoother_.Init(sampleRate_, 5.0f, 1000.0f);
    resonanceSmoother_.Init(sampleRate_, 5.0f, 0.2f);
    Reset();
}

void BiquadFilter::SetSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    cutoffSmoother_.Init(sampleRate_, 5.0f, cutoffSmoother_.GetCurrent());
    resonanceSmoother_.Init(sampleRate_, 5.0f, resonanceSmoother_.GetCurrent());
}

void BiquadFilter::Reset() {
    z1_ = 0.0f;
    z2_ = 0.0f;
}

void BiquadFilter::SetParams(const FilterSlotParams& params) {
    params_ = params;
    cutoffSmoother_.SetTarget(params.common.cutoffHz);
    resonanceSmoother_.SetTarget(params.common.resonance);
}

void BiquadFilter::UpdateCoefficients(float cutoffHz, float resonanceNorm, float peakGainDb) {
    const float fc = common::Clamp(cutoffHz, 20.0f, 0.45f * sampleRate_);
    const float q = 0.35f + resonanceNorm * 24.0f;

    const float w0 = common::kTwoPi * fc / sampleRate_;
    const float sn = std::sin(w0);
    const float cs = std::cos(w0);
    const float alpha = sn / (2.0f * q);

    const float A = std::pow(10.0f, peakGainDb / 40.0f);

    float b0 = 1.0f;
    float b1 = 0.0f;
    float b2 = 0.0f;
    float a0 = 1.0f;
    float a1 = 0.0f;
    float a2 = 0.0f;

    switch (static_cast<BiquadMode>(params_.model.mode)) {
    case BiquadMode::LP:
        b0 = (1.0f - cs) * 0.5f;
        b1 = 1.0f - cs;
        b2 = (1.0f - cs) * 0.5f;
        a0 = 1.0f + alpha;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha;
        break;
    case BiquadMode::HP:
        b0 = (1.0f + cs) * 0.5f;
        b1 = -(1.0f + cs);
        b2 = (1.0f + cs) * 0.5f;
        a0 = 1.0f + alpha;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha;
        break;
    case BiquadMode::BP:
        b0 = alpha;
        b1 = 0.0f;
        b2 = -alpha;
        a0 = 1.0f + alpha;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha;
        break;
    case BiquadMode::NOTCH:
        b0 = 1.0f;
        b1 = -2.0f * cs;
        b2 = 1.0f;
        a0 = 1.0f + alpha;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha;
        break;
    case BiquadMode::PEAK:
        b0 = 1.0f + alpha * A;
        b1 = -2.0f * cs;
        b2 = 1.0f - alpha * A;
        a0 = 1.0f + alpha / A;
        a1 = -2.0f * cs;
        a2 = 1.0f - alpha / A;
        break;
    }

    b0_ = b0 / a0;
    b1_ = b1 / a0;
    b2_ = b2 / a0;
    a1_ = a1 / a0;
    a2_ = a2 / a0;
}

float BiquadFilter::Process(float x) {
    const float cutoff = cutoffSmoother_.Process();
    const float resonance = common::Clamp(resonanceSmoother_.Process(), 0.0f, 1.0f);
    const float peakGainDb = params_.model.p1 * 24.0f - 12.0f;
    UpdateCoefficients(cutoff, resonance, peakGainDb);

    const float y = b0_ * x + z1_;
    z1_ = b1_ * x - a1_ * y + z2_;
    z2_ = b2_ * x - a2_ * y;

    const float mix = common::Clamp(params_.common.mix, 0.0f, 1.0f);
    const float mixed = x + mix * (y - x);
    return mixed * params_.common.level;
}

} // namespace mmf::dsp::filters
