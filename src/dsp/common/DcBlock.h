#pragma once

#include <cmath>

#include "AudioMath.h"

namespace mmf::dsp::common {

class DcBlock {
  public:
    void Init(float sampleRate, float cutoffHz = 5.0f) {
        sampleRate_ = sampleRate;
        SetCutoff(cutoffHz);
        Reset();
    }

    void SetCutoff(float cutoffHz) {
        const float fc = Clamp(cutoffHz, 1.0f, 40.0f);
        r_ = std::exp(-kTwoPi * fc / sampleRate_);
    }

    void Reset() {
        x1_ = 0.0f;
        y1_ = 0.0f;
    }

    float Process(float x) {
        const float y = x - x1_ + r_ * y1_;
        x1_ = x;
        y1_ = y;
        return y;
    }

  private:
    float sampleRate_ = 48000.0f;
    float r_ = 0.995f;
    float x1_ = 0.0f;
    float y1_ = 0.0f;
};

} // namespace mmf::dsp::common
