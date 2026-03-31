#pragma once

#include <array>
#include <cstddef>

namespace mmf::dsp::common {

template <size_t MaxDelaySamples>
class DelayLine {
  public:
    void Reset() {
        buffer_.fill(0.0f);
        writeIndex_ = 0;
    }

    void Write(float x) {
        buffer_[writeIndex_] = x;
        writeIndex_ = (writeIndex_ + 1) % MaxDelaySamples;
    }

    float ReadDelay(size_t delaySamples) const {
        const size_t d = delaySamples >= MaxDelaySamples ? (MaxDelaySamples - 1) : delaySamples;
        size_t readIndex = (writeIndex_ + MaxDelaySamples - d - 1) % MaxDelaySamples;
        return buffer_[readIndex];
    }

    float ReadDelayFrac(float delaySamples) const {
        if (delaySamples <= 0.0f) {
            return ReadDelay(0);
        }
        const float maxDelay = static_cast<float>(MaxDelaySamples - 2);
        const float d = delaySamples > maxDelay ? maxDelay : delaySamples;

        const size_t d0 = static_cast<size_t>(d);
        const size_t d1 = d0 + 1;
        const float frac = d - static_cast<float>(d0);

        const float y0 = ReadDelay(d0);
        const float y1 = ReadDelay(d1);
        return y0 + (y1 - y0) * frac;
    }

  private:
    std::array<float, MaxDelaySamples> buffer_{};
    size_t writeIndex_ = 0;
};

} // namespace mmf::dsp::common
