#pragma once

#include "Smoother.h"

namespace mmf::dsp::common {

class ParameterSmoother {
  public:
    void Init(float sampleRate, float timeMs, float initialValue = 0.0f) {
        smoother_.Init(sampleRate, timeMs, initialValue);
    }

    void SetSmoothingTimeMs(float timeMs) { smoother_.SetTimeMs(timeMs); }

    void SetTarget(float target) { smoother_.SetTarget(target); }

    void SetImmediate(float value) { smoother_.SetImmediate(value); }

    float Process() { return smoother_.Process(); }

    float GetCurrent() const { return smoother_.GetCurrent(); }

  private:
    Smoother smoother_;
};

} // namespace mmf::dsp::common
