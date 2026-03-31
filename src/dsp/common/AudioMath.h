#pragma once

#include <algorithm>
#include <cmath>

namespace mmf::dsp::common {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 2.0f * kPi;

inline float Clamp(float x, float lo, float hi) {
    return std::min(std::max(x, lo), hi);
}

inline float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float DbToLinear(float db) {
    return std::pow(10.0f, db * 0.05f);
}

inline float LinearToDb(float lin) {
    return 20.0f * std::log10(std::max(lin, 1.0e-12f));
}

inline float NormalizedToFrequency(float normalized) {
    const float n = Clamp(normalized, 0.0f, 1.0f);
    // 20 Hz .. 20 kHz exponential map
    return 20.0f * std::pow(1000.0f, n);
}

} // namespace mmf::dsp::common
