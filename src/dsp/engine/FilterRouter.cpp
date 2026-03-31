#include "FilterRouter.h"

namespace mmf::dsp::engine {

float FilterRouter::Process(filters::FilterRouting routing, FilterSlot& slotA, FilterSlot& slotB, float x) {
    switch (routing) {
    case filters::FilterRouting::A_TO_B: {
        const float a = slotA.ProcessSample(x);
        return slotB.ProcessSample(a);
    }
    case filters::FilterRouting::B_TO_A: {
        const float b = slotB.ProcessSample(x);
        return slotA.ProcessSample(b);
    }
    case filters::FilterRouting::PARALLEL: {
        const float a = slotA.ProcessSample(x);
        const float b = slotB.ProcessSample(x);
        return 0.5f * (a + b);
    }
    }
    return x;
}

} // namespace mmf::dsp::engine
