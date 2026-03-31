#pragma once

namespace mmf::dsp::filters {

struct CommonFilterParams {
    float cutoffHz = 1000.0f;
    float resonance = 0.0f; // 0..1 normalized
    float drive = 1.0f;
    float mix = 1.0f;       // dry/wet 0..1
    float level = 1.0f;
};

struct ModelSpecificParams {
    int mode = 0;
    float p1 = 0.0f;
    float p2 = 0.0f;
    float p3 = 0.0f;
    float p4 = 0.0f;
};

struct FilterSlotParams {
    CommonFilterParams common{};
    ModelSpecificParams model{};
    bool bypass = false;
};

} // namespace mmf::dsp::filters
