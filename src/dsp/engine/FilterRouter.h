#pragma once

#include "FilterSlot.h"

namespace mmf::dsp::engine {

class FilterRouter {
  public:
    static float Process(filters::FilterRouting routing, FilterSlot& slotA, FilterSlot& slotB, float x);
};

} // namespace mmf::dsp::engine
