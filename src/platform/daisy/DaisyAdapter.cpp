#include "DaisyAdapter.h"

namespace mmf::platform::daisy {

void DaisyAdapter::Init(float sampleRate) {
    engine_.Init(sampleRate, 2);
}

void DaisyAdapter::ProcessBlock(const float* inL,
                                const float* inR,
                                float* outL,
                                float* outR,
                                size_t size) {
    for (size_t i = 0; i < size; ++i) {
        float yL = 0.0f;
        float yR = 0.0f;
        engine_.ProcessFrame(inL[i], inR[i], yL, yR);
        outL[i] = yL;
        outR[i] = yR;
    }
}

} // namespace mmf::platform::daisy
