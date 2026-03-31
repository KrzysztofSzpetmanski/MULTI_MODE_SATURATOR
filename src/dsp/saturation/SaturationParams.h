#pragma once

namespace mmf::dsp::saturation {

struct SaturationCommonParams {
    float drive = 1.0f;
    float tone = 0.5f;
    float mix = 1.0f;
    float output = 1.0f;
};

struct SaturationModelParams {
    float bias = 0.0f;
    float p1 = 0.0f;
    float p2 = 0.0f;
    float p3 = 0.0f;
};

} // namespace mmf::dsp::saturation
