#pragma once

#include "FilterParams.h"
#include "FilterTypes.h"
#include "../common/ParameterSmoother.h"

namespace mmf::dsp::filters {

// Classic all-pass phaser approximation with 4/6/8 stage options.
// Natural use: swirl/whoosh/liquid modulation distinct from comb/flanger delay effects.
// Limitation: mono first pass with sine LFO and simplified stage spread law.
class PhaserFilter {
  public:
    void Init(float sampleRate);
    void SetSampleRate(float sampleRate);
    void Reset();
    void SetParams(const FilterSlotParams& params);
    float Process(float x);

  private:
    class AllpassStage {
      public:
        void Reset();
        void SetCoefficientFromFrequency(float frequencyHz, float sampleRate);
        float Process(float x);

      private:
        float a_ = 0.0f;
        float x1_ = 0.0f;
        float y1_ = 0.0f;
    };

    static int ModeToStages(int mode);

    float sampleRate_ = 48000.0f;
    FilterSlotParams params_{};

    common::ParameterSmoother rateSmootherHz_;
    common::ParameterSmoother depthSmoother_;
    common::ParameterSmoother feedbackSmoother_;
    common::ParameterSmoother spreadSmoother_;

    float lfoPhase_ = 0.0f;
    float lastWet_ = 0.0f;
    AllpassStage stages_[8];
};

} // namespace mmf::dsp::filters
