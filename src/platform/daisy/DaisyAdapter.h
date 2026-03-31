#pragma once

#include <cstddef>

#include "../../dsp/engine/DualFilterEngine.h"

namespace mmf::platform::daisy {

class DaisyAdapter {
  public:
    void Init(float sampleRate);

    // Generic block API independent from libDaisy headers.
    void ProcessBlock(const float* inL,
                      const float* inR,
                      float* outL,
                      float* outR,
                      size_t size);

    mmf::dsp::engine::DualFilterEngine& Engine() { return engine_; }
    const mmf::dsp::engine::DualFilterEngine& Engine() const { return engine_; }

  private:
    mmf::dsp::engine::DualFilterEngine engine_;
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
