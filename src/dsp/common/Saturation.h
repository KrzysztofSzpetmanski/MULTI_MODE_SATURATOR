#pragma once

#include <cmath>

namespace mmf::dsp::common {

struct Saturation {
    static inline float FastTanh(float x) {
        // CPU-cheap tanh approximation, good enough for VA filter drive stage.
        const float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    static inline float SoftClip(float x) {
        return x / (1.0f + std::fabs(x));
    }
};

} // namespace mmf::dsp::common
