#pragma once

#include <cstddef>

#include "../../dsp/engine/SaturationEngine.h"

namespace mmf::platform::daisy {

class DaisySaturationAdapter {
  public:
    void Init(float sampleRate);

    void SetModel(dsp::engine::SaturationEngine::ModelType model);
    void SetCommonParams(const dsp::saturation::SaturationCommonParams& params);
    void SetModelParams(const dsp::saturation::SaturationModelParams& params);

    // Stereo block API for Daisy AudioCallback.
    void ProcessBlock(const float* inL,
                      const float* inR,
                      float* outL,
                      float* outR,
                      size_t size);

  private:
    dsp::engine::SaturationEngine left_;
    dsp::engine::SaturationEngine right_;
};

/*
Example usage in Daisy AudioCallback:

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    adapter.ProcessBlock(in[0], in[1], out[0], out[1], size);
}
*/

} // namespace mmf::platform::daisy
