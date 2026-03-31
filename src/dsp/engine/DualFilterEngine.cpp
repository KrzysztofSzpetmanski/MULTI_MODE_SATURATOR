#include "DualFilterEngine.h"

namespace mmf::dsp::engine {

void DualFilterEngine::Init(float sampleRate, int numChannels) {
    sampleRate_ = sampleRate;
    numChannels_ = (numChannels <= 1) ? 1 : 2;

    for (auto& ch : channels_) {
        ch.slotA.Init(sampleRate_);
        ch.slotB.Init(sampleRate_);
        ch.slotA.SetModel(slotAModel_);
        ch.slotB.SetModel(slotBModel_);
        ch.slotA.SetParams(slotAParams_);
        ch.slotB.SetParams(slotBParams_);
    }
}

void DualFilterEngine::SetSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    for (auto& ch : channels_) {
        ch.slotA.SetSampleRate(sampleRate_);
        ch.slotB.SetSampleRate(sampleRate_);
    }
}

void DualFilterEngine::Reset() {
    for (auto& ch : channels_) {
        ch.slotA.Reset();
        ch.slotB.Reset();
    }
}

void DualFilterEngine::SetRouting(filters::FilterRouting routing) { routing_ = routing; }

FilterSlot& DualFilterEngine::GetSlot(ChannelState& ch, SlotId slot) {
    return (slot == SlotId::A) ? ch.slotA : ch.slotB;
}

void DualFilterEngine::SetSlotModel(SlotId slot, filters::FilterModelType model) {
    if (slot == SlotId::A) {
        slotAModel_ = model;
    } else {
        slotBModel_ = model;
    }

    for (auto& ch : channels_) {
        GetSlot(ch, slot).SetModel(model);
    }
}

void DualFilterEngine::SetSlotBypass(SlotId slot, bool bypass) {
    if (slot == SlotId::A) {
        slotAParams_.bypass = bypass;
    } else {
        slotBParams_.bypass = bypass;
    }

    for (auto& ch : channels_) {
        GetSlot(ch, slot).SetBypass(bypass);
    }
}

void DualFilterEngine::SetSlotParams(SlotId slot, const filters::FilterSlotParams& params) {
    if (slot == SlotId::A) {
        slotAParams_ = params;
    } else {
        slotBParams_ = params;
    }

    for (auto& ch : channels_) {
        GetSlot(ch, slot).SetParams(params);
    }
}

float DualFilterEngine::ProcessSample(float x, int channel) {
    const int chIndex = (channel <= 0 || numChannels_ == 1) ? 0 : 1;
    auto& ch = channels_[chIndex];
    return FilterRouter::Process(routing_, ch.slotA, ch.slotB, x);
}

void DualFilterEngine::ProcessFrame(float inL, float inR, float& outL, float& outR) {
    outL = ProcessSample(inL, 0);
    outR = ProcessSample(inR, numChannels_ > 1 ? 1 : 0);
}

void DualFilterEngine::ProcessDualFrame(float inA, float inB, float& outA, float& outB, int channel) {
    const int chIndex = (channel <= 0 || numChannels_ == 1) ? 0 : 1;
    auto& ch = channels_[chIndex];

    switch (routingMode_) {
    case filters::EngineRoutingMode::DUAL:
        outA = ch.slotA.ProcessSample(inA);
        outB = ch.slotB.ProcessSample(inB);
        break;
    case filters::EngineRoutingMode::SERIAL: {
        const float a = ch.slotA.ProcessSample(inA);
        outA = a;
        outB = ch.slotB.ProcessSample(a);
        break;
    }
    case filters::EngineRoutingMode::PARALLEL:
        outA = ch.slotA.ProcessSample(inA);
        outB = ch.slotB.ProcessSample(inA);
        break;
    }
}

} // namespace mmf::dsp::engine
