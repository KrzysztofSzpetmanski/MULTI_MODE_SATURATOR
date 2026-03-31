#pragma once

#include <cmath>

#include "AudioMath.h"

namespace mmf::dsp::common {

class OnePole {
  public:
    void Init(float sampleRate, float cutoffHz = 1000.0f) {
        sampleRate_ = sampleRate;
        SetCutoff(cutoffHz);
        Reset();
    }

    void SetSampleRate(float sampleRate) {
        sampleRate_ = sampleRate;
        SetCutoff(cutoffHz_);
    }

    void SetCutoff(float cutoffHz) {
        cutoffHz_ = Clamp(cutoffHz, 5.0f, 0.45f * sampleRate_);
        a_ = std::exp(-kTwoPi * cutoffHz_ / sampleRate_);
        b_ = 1.0f - a_;
    }

    void Reset(float value = 0.0f) { z_ = value; }

    float Process(float x) {
        z_ = b_ * x + a_ * z_;
        return z_;
    }

  private:
    float sampleRate_ = 48000.0f;
    float cutoffHz_ = 1000.0f;
    float a_ = 0.0f;
    float b_ = 1.0f;
    float z_ = 0.0f;
};

} // namespace mmf::dsp::common
