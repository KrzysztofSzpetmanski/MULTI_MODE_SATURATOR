#pragma once

#include <cmath>

#include "AudioMath.h"
#include "Saturation.h"

namespace mmf::dsp::common {

struct SaturationCurves {
    static inline float AsymSoft(float x, float bias) {
        const float b = Clamp(bias, -1.0f, 1.0f);
        const float xb = x + 0.35f * b;
        const float posGain = 1.0f + 0.85f * std::max(0.0f, b);
        const float negGain = 1.0f + 0.85f * std::max(0.0f, -b);
        return (xb >= 0.0f) ? Saturation::FastTanh(posGain * xb) : Saturation::FastTanh(negGain * xb);
    }

    static inline float DiodePair(float x, float hardness) {
        const float h = Clamp(hardness, 0.5f, 12.0f);
        if (x >= 0.0f) {
            return 1.0f - std::exp(-h * x);
        }
        return -(1.0f - std::exp(h * x));
    }

    static inline float TubeVoice(float x, float asym) {
        const float a = Clamp(asym, -1.0f, 1.0f);
        const float pos = Saturation::FastTanh((0.95f - 0.15f * a) * x);
        const float neg = Saturation::FastTanh((1.30f + 0.20f * a) * x);
        return (x >= 0.0f) ? pos : neg;
    }

    static inline float TapeSoft(float x) {
        const float x2 = x * x;
        const float cubic = x * (1.0f - 0.22f * x2);
        return 0.65f * Saturation::FastTanh(0.8f * x) + 0.35f * cubic;
    }
};

} // namespace mmf::dsp::common
