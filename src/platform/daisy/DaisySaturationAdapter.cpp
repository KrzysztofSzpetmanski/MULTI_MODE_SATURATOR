#include "DaisySaturationAdapter.h"

namespace mmf::platform::daisy {

void DaisySaturationAdapter::Init(float sampleRate) {
    left_.Init(sampleRate);
    right_.Init(sampleRate);
}

void DaisySaturationAdapter::SetModel(dsp::engine::SaturationEngine::ModelType model) {
    left_.SetModel(model);
    right_.SetModel(model);
}

void DaisySaturationAdapter::SetCommonParams(const dsp::saturation::SaturationCommonParams& params) {
    left_.SetCommonParams(params);
    right_.SetCommonParams(params);
}

void DaisySaturationAdapter::SetModelParams(const dsp::saturation::SaturationModelParams& params) {
    left_.SetModelParams(params);
    right_.SetModelParams(params);
}

void DaisySaturationAdapter::ProcessBlock(const float* inL,
                                          const float* inR,
                                          float* outL,
                                          float* outR,
                                          size_t size) {
    for (size_t i = 0; i < size; ++i) {
        outL[i] = left_.ProcessSample(inL[i]);
        outR[i] = right_.ProcessSample(inR[i]);
    }
}

} // namespace mmf::platform::daisy
