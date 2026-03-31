#pragma once

#include <cmath>

namespace mmf::dsp::common {

class Smoother {
  public:
    void Init(float sampleRate, float timeMs, float initialValue = 0.0f) {
        sampleRate_ = sampleRate;
        SetTimeMs(timeMs);
        value_ = initialValue;
        target_ = initialValue;
    }

    void SetTimeMs(float timeMs) {
        const float clamped = timeMs < 0.0f ? 0.0f : timeMs;
        if (clamped <= 0.0001f) {
            coeff_ = 0.0f;
            return;
        }
        const float tauSec = clamped * 0.001f;
        coeff_ = std::exp(-1.0f / (tauSec * sampleRate_));
    }

    void SetTarget(float target) { target_ = target; }

    void SetImmediate(float value) {
        value_ = value;
        target_ = value;
    }

    float Process() {
        value_ = target_ + coeff_ * (value_ - target_);
        return value_;
    }

    float GetCurrent() const { return value_; }

  private:
    float sampleRate_ = 48000.0f;
    float coeff_ = 0.0f;
    float value_ = 0.0f;
    float target_ = 0.0f;
};

} // namespace mmf::dsp::common
