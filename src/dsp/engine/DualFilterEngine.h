#pragma once

#include <array>

#include "FilterRouter.h"

namespace mmf::dsp::engine {

class DualFilterEngine {
  public:
    enum class SlotId { A = 0, B = 1 };

    void Init(float sampleRate, int numChannels = 2);
    void SetSampleRate(float sampleRate);
    void Reset();

    void SetRouting(filters::FilterRouting routing);
    filters::FilterRouting GetRouting() const { return routing_; }
    void SetEngineRouting(filters::EngineRoutingMode routingMode) { routingMode_ = routingMode; }
    filters::EngineRoutingMode GetEngineRouting() const { return routingMode_; }

    void SetSlotModel(SlotId slot, filters::FilterModelType model);
    void SetSlotBypass(SlotId slot, bool bypass);
    void SetSlotParams(SlotId slot, const filters::FilterSlotParams& params);

    float ProcessSample(float x, int channel = 0);
    void ProcessFrame(float inL, float inR, float& outL, float& outR);
    void ProcessDualFrame(float inA, float inB, float& outA, float& outB, int channel = 0);

  private:
    struct ChannelState {
        FilterSlot slotA;
        FilterSlot slotB;
    };

    FilterSlot& GetSlot(ChannelState& ch, SlotId slot);

    float sampleRate_ = 48000.0f;
    int numChannels_ = 2;
    filters::FilterRouting routing_ = filters::FilterRouting::A_TO_B;
    filters::EngineRoutingMode routingMode_ = filters::EngineRoutingMode::DUAL;

    filters::FilterModelType slotAModel_ = filters::FilterModelType::SVF;
    filters::FilterModelType slotBModel_ = filters::FilterModelType::TransistorLadder;
    filters::FilterSlotParams slotAParams_{};
    filters::FilterSlotParams slotBParams_{};

    std::array<ChannelState, 2> channels_{};
};

} // namespace mmf::dsp::engine
