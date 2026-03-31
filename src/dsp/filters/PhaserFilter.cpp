#include "PhaserFilter.h"

#include <cmath>

#include "../common/AudioMath.h"
#include "../common/Saturation.h"

namespace mmf::dsp::filters {

namespace {
constexpr float kCenterFrequencyHz = 900.0f;
constexpr float kMinModulatedHz = 30.0f;
constexpr float kMaxModulatedHz = 16000.0f;
} // namespace

void PhaserFilter::AllpassStage::Reset() {
    x1_ = 0.0f;
    y1_ = 0.0f;
}

void PhaserFilter::AllpassStage::SetCoefficientFromFrequency(float frequencyHz, float sampleRate) {
    const float fc = common::Clamp(frequencyHz, 5.0f, 0.45f * sampleRate);
    const float g = std::tan(common::kPi * fc / sampleRate);
    a_ = (g - 1.0f) / (g + 1.0f);
}

float PhaserFilter::AllpassStage::Process(float x) {
    const float y = -a_ * x + x1_ + a_ * y1_;
    x1_ = x;
    y1_ = y;
    return y;
}

int PhaserFilter::ModeToStages(int mode) {
    switch (static_cast<PhaserStagesMode>(mode)) {
    case PhaserStagesMode::STAGES_4:
        return 4;
    case PhaserStagesMode::STAGES_6:
        return 6;
    case PhaserStagesMode::STAGES_8:
    default:
        return 8;
    }
}

void PhaserFilter::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    rateSmootherHz_.Init(sampleRate_, 12.0f, 0.35f);
    depthSmoother_.Init(sampleRate_, 8.0f, 0.6f);
    feedbackSmoother_.Init(sampleRate_, 8.0f, 0.2f);
    spreadSmoother_.Init(sampleRate_, 20.0f, 0.25f);
    Reset();
}

void PhaserFilter::SetSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    rateSmootherHz_.Init(sampleRate_, 12.0f, rateSmootherHz_.GetCurrent());
    depthSmoother_.Init(sampleRate_, 8.0f, depthSmoother_.GetCurrent());
    feedbackSmoother_.Init(sampleRate_, 8.0f, feedbackSmoother_.GetCurrent());
    spreadSmoother_.Init(sampleRate_, 20.0f, spreadSmoother_.GetCurrent());
}

void PhaserFilter::Reset() {
    lfoPhase_ = 0.0f;
    lastWet_ = 0.0f;
    for (auto& s : stages_) {
        s.Reset();
    }
}

void PhaserFilter::SetParams(const FilterSlotParams& params) {
    params_ = params;
    rateSmootherHz_.SetTarget(common::Clamp(params.model.p1, 0.02f, 12.0f));
    depthSmoother_.SetTarget(common::Clamp(params.model.p2, 0.0f, 1.0f));
    feedbackSmoother_.SetTarget(common::Clamp(params.model.p3, -0.95f, 0.95f));
    spreadSmoother_.SetTarget(common::Clamp(params.model.p4, 0.0f, 1.0f));
}

float PhaserFilter::Process(float x) {
    const float rateHz = common::Clamp(rateSmootherHz_.Process(), 0.02f, 12.0f);
    const float depth = common::Clamp(depthSmoother_.Process(), 0.0f, 1.0f);
    const float feedback = common::Clamp(feedbackSmoother_.Process(), -0.95f, 0.95f);
    const float spread = common::Clamp(spreadSmoother_.Process(), 0.0f, 1.0f);

    lfoPhase_ += rateHz / sampleRate_;
    if (lfoPhase_ >= 1.0f) {
        lfoPhase_ -= 1.0f;
    }
    const float lfo = std::sin(common::kTwoPi * lfoPhase_);

    const float sweepSemitones = 6.0f + depth * 42.0f;
    const float oct = (sweepSemitones / 12.0f) * lfo;
    const float baseFreq = common::Clamp(kCenterFrequencyHz * std::pow(2.0f, oct), kMinModulatedHz, kMaxModulatedHz);

    const int activeStages = ModeToStages(params_.model.mode);
    const float stageSpreadOct = 0.15f + 0.85f * spread;
    const float stageMid = 0.5f * static_cast<float>(activeStages - 1);

    float in = x + feedback * lastWet_;
    in = common::Saturation::SoftClip(in * 1.2f);

    float wet = in;
    for (int i = 0; i < activeStages; ++i) {
        const float idx = static_cast<float>(i) - stageMid;
        const float norm = (stageMid > 0.0f) ? (idx / stageMid) : 0.0f;
        const float stageFreq = common::Clamp(baseFreq * std::pow(2.0f, norm * stageSpreadOct),
                                              kMinModulatedHz,
                                              kMaxModulatedHz);
        stages_[i].SetCoefficientFromFrequency(stageFreq, sampleRate_);
        wet = stages_[i].Process(wet);
    }
    lastWet_ = wet;

    // TODO: Stereo version with separate L/R state and LFO phase offset.
    // TODO: Add analog-voiced/nonlinear feedback coloration options.
    // TODO: Improve spread law and per-stage center calibration.
    // TODO: Add extra LFO shapes and tempo-sync.
    const float mix = common::Clamp(params_.common.mix, 0.0f, 1.0f);
    const float y = x + mix * (wet - x);
    return y * params_.common.level;
}

} // namespace mmf::dsp::filters
