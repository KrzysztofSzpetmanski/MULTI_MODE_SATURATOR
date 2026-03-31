#include "DualFilterModule.h"

#include <algorithm>
#include <cmath>

#include "../../dsp/common/AudioMath.h"

namespace mmf::platform::vcv {

using namespace rack;

DualFilterModule::DualFilterModule() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configSwitch(MODEL_A_PARAM, 0.0f, 5.0f, 0.0f, "Model A", {"SVF", "Trans Ladder", "Diode Ladder", "LPG", "Comb", "Biquad"});
    configSwitch(MODEL_B_PARAM, 0.0f, 5.0f, 1.0f, "Model B", {"SVF", "Trans Ladder", "Diode Ladder", "LPG", "Comb", "Biquad"});
    configSwitch(ROUTING_PARAM, 0.0f, 2.0f, 0.0f, "Routing", {"Dual", "Serial", "Parallel"});

    configSwitch(A_BYPASS_PARAM, 0.0f, 1.0f, 0.0f, "Bypass A", {"Off", "On"});
    configSwitch(A_MODE_PARAM, 0.0f, 4.0f, 0.0f, "Mode A");
    configParam(A_CUTOFF_PARAM, 0.0f, 1.0f, 0.5f, "A cutoff");
    configParam(A_RESONANCE_PARAM, 0.0f, 1.0f, 0.1f, "A resonance");
    configParam(A_DRIVE_PARAM, 0.1f, 3.0f, 1.0f, "A drive");
    configParam(A_MIX_PARAM, 0.0f, 1.0f, 1.0f, "A mix");

    configSwitch(B_BYPASS_PARAM, 0.0f, 1.0f, 0.0f, "Bypass B", {"Off", "On"});
    configSwitch(B_MODE_PARAM, 0.0f, 4.0f, 0.0f, "Mode B");
    configParam(B_CUTOFF_PARAM, 0.0f, 1.0f, 0.5f, "B cutoff");
    configParam(B_RESONANCE_PARAM, 0.0f, 1.0f, 0.1f, "B resonance");
    configParam(B_DRIVE_PARAM, 0.1f, 3.0f, 1.0f, "B drive");
    configParam(B_MIX_PARAM, 0.0f, 1.0f, 1.0f, "B mix");

    configParam(A_CUTOFF_CV_DEPTH_PARAM, 0.f, 1.f, 1.f, "A cutoff CV depth", "%", 0.f, 100.f);
    configParam(A_RESONANCE_CV_DEPTH_PARAM, 0.f, 1.f, 1.f, "A resonance CV depth", "%", 0.f, 100.f);
    configParam(B_CUTOFF_CV_DEPTH_PARAM, 0.f, 1.f, 1.f, "B cutoff CV depth", "%", 0.f, 100.f);
    configParam(B_RESONANCE_CV_DEPTH_PARAM, 0.f, 1.f, 1.f, "B resonance CV depth", "%", 0.f, 100.f);

    getParamQuantity(MODEL_A_PARAM)->snapEnabled = true;
    getParamQuantity(MODEL_B_PARAM)->snapEnabled = true;
    getParamQuantity(ROUTING_PARAM)->snapEnabled = true;
    getParamQuantity(A_MODE_PARAM)->snapEnabled = true;
    getParamQuantity(B_MODE_PARAM)->snapEnabled = true;

    paramQuantities[A_CUTOFF_CV_DEPTH_PARAM]->randomizeEnabled = false;
    paramQuantities[A_RESONANCE_CV_DEPTH_PARAM]->randomizeEnabled = false;
    paramQuantities[B_CUTOFF_CV_DEPTH_PARAM]->randomizeEnabled = false;
    paramQuantities[B_RESONANCE_CV_DEPTH_PARAM]->randomizeEnabled = false;

    engine_.Init(44100.0f, 1);
    engine_.SetEngineRouting(dsp::filters::EngineRoutingMode::DUAL);
}

void DualFilterModule::process(const ProcessArgs& args) {
    if (args.sampleRate != lastSampleRate_) {
        lastSampleRate_ = args.sampleRate;
        engine_.Init(lastSampleRate_, 1);
    }

    if ((frameCounter_++ & 0x7) == 0) {
        SyncParamsFromUi();
    }

    const bool inAConnected = inputs[AUDIO_A_INPUT].isConnected();
    const bool inBConnected = inputs[AUDIO_B_INPUT].isConnected();
    const float inA = inAConnected ? (inputs[AUDIO_A_INPUT].getVoltage() * 0.2f) : 0.0f;
    const float inB = inBConnected ? (inputs[AUDIO_B_INPUT].getVoltage() * 0.2f) : 0.0f;

    float outA = 0.0f;
    float outB = 0.0f;
    const auto routingMode = static_cast<dsp::filters::EngineRoutingMode>(
        ClampInt(static_cast<int>(std::round(params[ROUTING_PARAM].getValue())), 0, 2));

    if (routingMode == dsp::filters::EngineRoutingMode::DUAL) {
        engine_.ProcessDualFrame(inA, inB, outA, outB, 0);
    } else {
        // SERIAL/PARALLEL convenience mode:
        // - user can plug signal into A or B (or both),
        // - engine receives a merged input,
        // - outputs are mirrored in engine.
        float mergedIn = 0.0f;
        if (inAConnected && inBConnected) {
            mergedIn = 0.5f * (inA + inB);
        } else if (inAConnected) {
            mergedIn = inA;
        } else if (inBConnected) {
            mergedIn = inB;
        }
        engine_.ProcessDualFrame(mergedIn, mergedIn, outA, outB, 0);
    }

    outputs[AUDIO_A_OUTPUT].setVoltage(5.0f * outA);
    outputs[AUDIO_B_OUTPUT].setVoltage(5.0f * outB);

    lights[A_CUTOFF_MOD_LIGHT].setBrightnessSmooth(
        inputs[A_CUTOFF_CV_INPUT].isConnected() ? params[A_CUTOFF_CV_DEPTH_PARAM].getValue() : 0.0f,
        args.sampleTime);
    lights[A_RESONANCE_MOD_LIGHT].setBrightnessSmooth(
        inputs[A_RESONANCE_CV_INPUT].isConnected() ? params[A_RESONANCE_CV_DEPTH_PARAM].getValue() : 0.0f,
        args.sampleTime);
    lights[B_CUTOFF_MOD_LIGHT].setBrightnessSmooth(
        inputs[B_CUTOFF_CV_INPUT].isConnected() ? params[B_CUTOFF_CV_DEPTH_PARAM].getValue() : 0.0f,
        args.sampleTime);
    lights[B_RESONANCE_MOD_LIGHT].setBrightnessSmooth(
        inputs[B_RESONANCE_CV_INPUT].isConnected() ? params[B_RESONANCE_CV_DEPTH_PARAM].getValue() : 0.0f,
        args.sampleTime);
}

float DualFilterModule::ClampNorm(float x) {
    return std::max(0.0f, std::min(x, 1.0f));
}

int DualFilterModule::ClampInt(int x, int minV, int maxV) {
    return std::max(minV, std::min(x, maxV));
}

float DualFilterModule::getCvDepthValue(int depthParam) {
    return ClampNorm(params[depthParam].getValue());
}

float DualFilterModule::getModulatedRangeValue(int baseParam,
                                               int cvInput,
                                               int depthParam,
                                               float minV,
                                               float maxV) {
    float value = params[baseParam].getValue();
    if (inputs[cvInput].isConnected()) {
        const float depth = getCvDepthValue(depthParam);
        const float halfRange = 0.5f * (maxV - minV);
        value += (inputs[cvInput].getVoltage() / 5.0f) * halfRange * depth;
    }
    return rack::math::clamp(value, minV, maxV);
}

dsp::filters::FilterSlotParams DualFilterModule::BuildSlotParams(bool a) {
    dsp::filters::FilterSlotParams p{};

    const int modelParam = a ? MODEL_A_PARAM : MODEL_B_PARAM;
    const int modeParam = a ? A_MODE_PARAM : B_MODE_PARAM;
    const int cutoffParam = a ? A_CUTOFF_PARAM : B_CUTOFF_PARAM;
    const int cutoffCv = a ? A_CUTOFF_CV_INPUT : B_CUTOFF_CV_INPUT;
    const int cutoffDepth = a ? A_CUTOFF_CV_DEPTH_PARAM : B_CUTOFF_CV_DEPTH_PARAM;

    const int resonanceParam = a ? A_RESONANCE_PARAM : B_RESONANCE_PARAM;
    const int resonanceCv = a ? A_RESONANCE_CV_INPUT : B_RESONANCE_CV_INPUT;
    const int resonanceDepth = a ? A_RESONANCE_CV_DEPTH_PARAM : B_RESONANCE_CV_DEPTH_PARAM;

    const int driveParam = a ? A_DRIVE_PARAM : B_DRIVE_PARAM;
    const int mixParam = a ? A_MIX_PARAM : B_MIX_PARAM;
    const int bypassParam = a ? A_BYPASS_PARAM : B_BYPASS_PARAM;

    const int modelIndex = ClampInt(static_cast<int>(std::round(params[modelParam].getValue())), 0, 5);
    const int modeIndex = ClampInt(static_cast<int>(std::round(params[modeParam].getValue())), 0, 4);

    const float cutoffNorm = getModulatedRangeValue(cutoffParam, cutoffCv, cutoffDepth, 0.0f, 1.0f);
    const float resonanceNorm = getModulatedRangeValue(resonanceParam, resonanceCv, resonanceDepth, 0.0f, 1.0f);

    p.common.cutoffHz = dsp::common::NormalizedToFrequency(cutoffNorm);
    p.common.resonance = ClampNorm(resonanceNorm);
    p.common.drive = params[driveParam].getValue();
    p.common.mix = params[mixParam].getValue();
    p.common.level = 1.0f;
    p.bypass = params[bypassParam].getValue() > 0.5f;

    // Model-specific mapping with explicit semantics (instead of opaque p1/p2 knobs on UI).
    p.model.p1 = 0.0f;
    p.model.p2 = 0.0f;
    p.model.p3 = 0.5f;
    p.model.p4 = 0.0f;

    switch (static_cast<dsp::filters::FilterModelType>(modelIndex)) {
    case dsp::filters::FilterModelType::SVF:
        p.model.mode = ClampInt(modeIndex, 0, 3); // LP/HP/BP/NOTCH
        break;
    case dsp::filters::FilterModelType::TransistorLadder:
    case dsp::filters::FilterModelType::DiodeLadder:
        p.model.mode = ClampInt(modeIndex, 0, 3); // 1P/2P/3P/4P taps
        p.model.p1 = 0.2f;                        // light bass compensation placeholder
        break;
    case dsp::filters::FilterModelType::LPG:
        p.model.mode = ClampInt(modeIndex, 0, 2);              // LPG/VCA/LP
        p.model.p1 = p.common.mix;                             // control
        p.model.p2 = 0.2f + 0.8f * p.common.resonance;         // response
        break;
    case dsp::filters::FilterModelType::Comb:
        p.model.mode = ClampInt(modeIndex, 0, 2);              // FF/FB/LP-FB
        p.model.p1 = cutoffNorm;                                // delay/tune
        p.model.p2 = 0.98f * p.common.resonance;                // feedback
        p.model.p3 = ClampNorm((p.common.drive - 0.1f) / 2.9f); // damping
        break;
    case dsp::filters::FilterModelType::Biquad:
        p.model.mode = ClampInt(modeIndex, 0, 4); // LP/HP/BP/NOTCH/PEAK
        p.model.p1 = 0.5f;
        break;
    }

    return p;
}

void DualFilterModule::SyncParamsFromUi() {
    const int aModelIndex = ClampInt(static_cast<int>(std::round(params[MODEL_A_PARAM].getValue())), 0, 5);
    const int bModelIndex = ClampInt(static_cast<int>(std::round(params[MODEL_B_PARAM].getValue())), 0, 5);

    const int routingIndex = ClampInt(static_cast<int>(std::round(params[ROUTING_PARAM].getValue())), 0, 2);

    engine_.SetEngineRouting(static_cast<dsp::filters::EngineRoutingMode>(routingIndex));
    engine_.SetSlotModel(dsp::engine::DualFilterEngine::SlotId::A, static_cast<dsp::filters::FilterModelType>(aModelIndex));
    engine_.SetSlotModel(dsp::engine::DualFilterEngine::SlotId::B, static_cast<dsp::filters::FilterModelType>(bModelIndex));

    engine_.SetSlotParams(dsp::engine::DualFilterEngine::SlotId::A, BuildSlotParams(true));
    engine_.SetSlotParams(dsp::engine::DualFilterEngine::SlotId::B, BuildSlotParams(false));
}

} // namespace mmf::platform::vcv
