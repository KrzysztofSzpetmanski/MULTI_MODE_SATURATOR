#include "TransistorLadder.h"

#include <cmath>

#include "../common/AudioMath.h"
#include "../common/Saturation.h"

namespace mmf::dsp::filters {

void TransistorLadder::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    cutoffSmoother_.Init(sampleRate_, 5.0f, 1200.0f);
    resonanceSmoother_.Init(sampleRate_, 5.0f, 0.0f);
    Reset();
}

void TransistorLadder::SetSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    cutoffSmoother_.Init(sampleRate_, 5.0f, cutoffSmoother_.GetCurrent());
    resonanceSmoother_.Init(sampleRate_, 5.0f, resonanceSmoother_.GetCurrent());
}

void TransistorLadder::Reset() {
    stage_[0] = stage_[1] = stage_[2] = stage_[3] = 0.0f;
}

void TransistorLadder::SetParams(const FilterSlotParams& params) {
    params_ = params;
    cutoffSmoother_.SetTarget(params.common.cutoffHz);
    resonanceSmoother_.SetTarget(params.common.resonance);
}

void TransistorLadder::UpdateCoefficient(float cutoffHz) {
    const float fc = common::Clamp(cutoffHz, 20.0f, 0.45f * sampleRate_);
    g_ = 1.0f - std::exp(-common::kTwoPi * fc / sampleRate_);
}

float TransistorLadder::Process(float x) {
    // TODO: Refine cutoff tuning and resonance law against measured ladder response.
    // TODO: Upgrade to stronger nonlinear solver while keeping embedded CPU budget reasonable.
    const float cutoff = cutoffSmoother_.Process();
    const float resonance = common::Clamp(resonanceSmoother_.Process(), 0.0f, 1.0f);
    UpdateCoefficient(cutoff);

    const float drive = params_.common.drive < 0.0f ? 0.0f : params_.common.drive;
    const float bassComp = common::Clamp(params_.model.p1, 0.0f, 1.0f);
    const float feedback = 3.95f * resonance;

    float u = x * drive - feedback * stage_[3] + bassComp * 0.25f * stage_[3];
    u = common::Saturation::FastTanh(u);

    for (int i = 0; i < 4; ++i) {
        const float in = (i == 0) ? u : stage_[i - 1];
        const float shapedIn = common::Saturation::FastTanh(in);
        const float shapedState = common::Saturation::FastTanh(stage_[i]);
        stage_[i] += g_ * (shapedIn - shapedState);
    }

    float wet = stage_[3];
    switch (static_cast<LadderTap>(params_.model.mode)) {
    case LadderTap::POLE_1:
        wet = stage_[0];
        break;
    case LadderTap::POLE_2:
        wet = stage_[1];
        break;
    case LadderTap::POLE_3:
        wet = stage_[2];
        break;
    case LadderTap::POLE_4:
        wet = stage_[3];
        break;
    }

    const float mix = common::Clamp(params_.common.mix, 0.0f, 1.0f);
    const float y = x + mix * (wet - x);
    return y * params_.common.level;
}

} // namespace mmf::dsp::filters
