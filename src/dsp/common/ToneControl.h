#pragma once

#include "AudioMath.h"
#include "OnePole.h"

namespace mmf::dsp::common {

class ToneControl {
  public:
    void Init(float sampleRate) {
        sampleRate_ = sampleRate;
        lowpass_.Init(sampleRate_, 8000.0f);
        highpassProxy_.Init(sampleRate_, 90.0f);
        SetTone(0.5f);
    }

    void SetSampleRate(float sampleRate) {
        sampleRate_ = sampleRate;
        lowpass_.SetSampleRate(sampleRate_);
        highpassProxy_.SetSampleRate(sampleRate_);
    }

    void Reset() {
        lowpass_.Reset();
        highpassProxy_.Reset();
    }

    void SetTone(float tone) {
        tone_ = Clamp(tone, 0.0f, 1.0f);
        const float lpCutoff = Lerp(2200.0f, 18000.0f, tone_);
        const float hpCutoff = Lerp(12.0f, 120.0f, 1.0f - tone_);
        lowpass_.SetCutoff(lpCutoff);
        highpassProxy_.SetCutoff(hpCutoff);
    }

    float Process(float x) {
        const float hp = x - highpassProxy_.Process(x);
        const float lp = lowpass_.Process(hp);
        const float brightBlend = Lerp(0.35f, 1.0f, tone_);
        return Lerp(lp, hp, brightBlend);
    }

  private:
    float sampleRate_ = 48000.0f;
    float tone_ = 0.5f;
    OnePole lowpass_;
    OnePole highpassProxy_;
};

} // namespace mmf::dsp::common
