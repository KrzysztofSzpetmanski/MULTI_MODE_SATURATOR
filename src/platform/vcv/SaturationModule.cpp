#include "SaturationModule.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace mmf::platform::vcv {

using namespace rack;

namespace {

constexpr std::array<float, 5> kOutputTrim = {
    0.501187f, // -6 dB
    0.707946f, // -3 dB
    1.0f,      //  0 dB
    1.412538f, // +3 dB
    1.995262f  // +6 dB
};

} // namespace

SaturationModule::SaturationModule() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configSwitch(MODEL_A_PARAM,
                 0.0f,
                 4.0f,
                 1.0f,
                 "Model A",
                 {"OFF", "Soft/Asym", "Diode", "Tube-ish", "Tape-ish"});
    configSwitch(MODEL_B_PARAM,
                 0.0f,
                 4.0f,
                 2.0f,
                 "Model B",
                 {"OFF", "Soft/Asym", "Diode", "Tube-ish", "Tape-ish"});
    configSwitch(ROUTING_PARAM, 0.0f, 2.0f, 0.0f, "Routing", {"Dual", "Serial", "Parallel"});

    configSwitch(A_MODE_PARAM, 0.0f, 4.0f, 2.0f, "A output trim", {"-6 dB", "-3 dB", "0 dB", "+3 dB", "+6 dB"});
    configParam(A_CUTOFF_PARAM, 0.0f, 1.0f, 0.5f, "A tone");
    configParam(A_RESONANCE_PARAM, 0.0f, 1.0f, 0.5f, "A bias");
    configParam(A_DRIVE_PARAM, 0.1f, 3.0f, 1.0f, "A drive");
    configParam(A_MIX_PARAM, 0.0f, 1.0f, 1.0f, "A mix");

    configSwitch(B_MODE_PARAM, 0.0f, 4.0f, 2.0f, "B output trim", {"-6 dB", "-3 dB", "0 dB", "+3 dB", "+6 dB"});
    configParam(B_CUTOFF_PARAM, 0.0f, 1.0f, 0.5f, "B tone");
    configParam(B_RESONANCE_PARAM, 0.0f, 1.0f, 0.5f, "B bias");
    configParam(B_DRIVE_PARAM, 0.1f, 3.0f, 1.0f, "B drive");
    configParam(B_MIX_PARAM, 0.0f, 1.0f, 1.0f, "B mix");

    configParam(A_CUTOFF_CV_DEPTH_PARAM, 0.f, 1.f, 1.f, "A tone CV depth", "%", 0.f, 100.f);
    configParam(A_RESONANCE_CV_DEPTH_PARAM, 0.f, 1.f, 1.f, "A bias CV depth", "%", 0.f, 100.f);
    configParam(B_CUTOFF_CV_DEPTH_PARAM, 0.f, 1.f, 1.f, "B tone CV depth", "%", 0.f, 100.f);
    configParam(B_RESONANCE_CV_DEPTH_PARAM, 0.f, 1.f, 1.f, "B bias CV depth", "%", 0.f, 100.f);

    getParamQuantity(MODEL_A_PARAM)->snapEnabled = true;
    getParamQuantity(MODEL_B_PARAM)->snapEnabled = true;
    getParamQuantity(ROUTING_PARAM)->snapEnabled = true;
    getParamQuantity(A_MODE_PARAM)->snapEnabled = true;
    getParamQuantity(B_MODE_PARAM)->snapEnabled = true;

    paramQuantities[A_CUTOFF_CV_DEPTH_PARAM]->randomizeEnabled = false;
    paramQuantities[A_RESONANCE_CV_DEPTH_PARAM]->randomizeEnabled = false;
    paramQuantities[B_CUTOFF_CV_DEPTH_PARAM]->randomizeEnabled = false;
    paramQuantities[B_RESONANCE_CV_DEPTH_PARAM]->randomizeEnabled = false;

    engineA_.Init(44100.0f);
    engineB_.Init(44100.0f);
    routingMode_ = RoutingMode::DUAL;
}

void SaturationModule::process(const ProcessArgs& args) {
    if (args.sampleRate != lastSampleRate_) {
        lastSampleRate_ = args.sampleRate;
        engineA_.Init(lastSampleRate_);
        engineB_.Init(lastSampleRate_);
    }

    if ((frameCounter_++ & 0x7) == 0) {
        SyncParamsFromUi();
    }

    const bool inAConnected = inputs[AUDIO_A_INPUT].isConnected();
    const bool inBConnected = inputs[AUDIO_B_INPUT].isConnected();
    const float inA = inAConnected ? (inputs[AUDIO_A_INPUT].getVoltage() * 0.2f) : 0.0f;
    const float inB = inBConnected ? (inputs[AUDIO_B_INPUT].getVoltage() * 0.2f) : 0.0f;

    auto processSlot = [](dsp::engine::SaturationEngine& engine, const SlotParams& p, float x) {
        return p.bypass ? x : engine.ProcessSample(x);
    };

    float outA = 0.0f;
    float outB = 0.0f;

    if (routingMode_ == RoutingMode::DUAL) {
        outA = processSlot(engineA_, slotAParams_, inA);
        outB = processSlot(engineB_, slotBParams_, inB);
    } else {
        float mergedIn = 0.0f;
        if (inAConnected && inBConnected) {
            mergedIn = 0.5f * (inA + inB);
        } else if (inAConnected) {
            mergedIn = inA;
        } else if (inBConnected) {
            mergedIn = inB;
        }

        if (routingMode_ == RoutingMode::SERIAL) {
            const float a = processSlot(engineA_, slotAParams_, mergedIn);
            const float y = processSlot(engineB_, slotBParams_, a);
            outA = y;
            outB = y;
        } else {
            const float yA = processSlot(engineA_, slotAParams_, mergedIn);
            const float yB = processSlot(engineB_, slotBParams_, mergedIn);
            const float y = 0.5f * (yA + yB);
            outA = y;
            outB = y;
        }
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

float SaturationModule::ClampNorm(float x) {
    return std::max(0.0f, std::min(x, 1.0f));
}

int SaturationModule::ClampInt(int x, int minV, int maxV) {
    return std::max(minV, std::min(x, maxV));
}

float SaturationModule::getCvDepthValue(int depthParam) {
    return ClampNorm(params[depthParam].getValue());
}

float SaturationModule::getModulatedRangeValue(int baseParam,
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

SaturationModule::SlotParams SaturationModule::BuildSlotParams(bool a) {
    SlotParams p{};

    const int modelParam = a ? MODEL_A_PARAM : MODEL_B_PARAM;
    const int modeParam = a ? A_MODE_PARAM : B_MODE_PARAM;
    const int toneParam = a ? A_CUTOFF_PARAM : B_CUTOFF_PARAM;
    const int toneCv = a ? A_CUTOFF_CV_INPUT : B_CUTOFF_CV_INPUT;
    const int toneDepth = a ? A_CUTOFF_CV_DEPTH_PARAM : B_CUTOFF_CV_DEPTH_PARAM;

    const int biasParam = a ? A_RESONANCE_PARAM : B_RESONANCE_PARAM;
    const int biasCv = a ? A_RESONANCE_CV_INPUT : B_RESONANCE_CV_INPUT;
    const int biasDepth = a ? A_RESONANCE_CV_DEPTH_PARAM : B_RESONANCE_CV_DEPTH_PARAM;

    const int driveParam = a ? A_DRIVE_PARAM : B_DRIVE_PARAM;
    const int mixParam = a ? A_MIX_PARAM : B_MIX_PARAM;

    const int modelIndex = ClampInt(static_cast<int>(std::round(params[modelParam].getValue())), 0, 4);
    const int trimIndex = ClampInt(static_cast<int>(std::round(params[modeParam].getValue())), 0, 4);

    const float toneNorm = getModulatedRangeValue(toneParam, toneCv, toneDepth, 0.0f, 1.0f);
    const float biasNorm = getModulatedRangeValue(biasParam, biasCv, biasDepth, 0.0f, 1.0f);

    p.common.drive = params[driveParam].getValue();
    p.common.tone = ClampNorm(toneNorm);
    p.common.mix = ClampNorm(params[mixParam].getValue());
    p.common.output = kOutputTrim[trimIndex];

    p.modelSpecific.bias = 2.0f * ClampNorm(biasNorm) - 1.0f;
    p.modelSpecific.p1 = static_cast<float>(trimIndex) * 0.25f;
    p.modelSpecific.p2 = p.common.tone;
    p.modelSpecific.p3 = p.common.drive;

    p.bypass = (modelIndex == 0);
    switch (modelIndex) {
    case 1:
        p.model = dsp::engine::SaturationEngine::ModelType::SoftAsym;
        break;
    case 2:
        p.model = dsp::engine::SaturationEngine::ModelType::Diode;
        break;
    case 3:
        p.model = dsp::engine::SaturationEngine::ModelType::Tubeish;
        break;
    case 4:
        p.model = dsp::engine::SaturationEngine::ModelType::Tapeish;
        break;
    default:
        p.model = dsp::engine::SaturationEngine::ModelType::SoftAsym;
        break;
    }

    return p;
}

void SaturationModule::SyncParamsFromUi() {
    const int routingIndex = ClampInt(static_cast<int>(std::round(params[ROUTING_PARAM].getValue())), 0, 2);
    routingMode_ = static_cast<RoutingMode>(routingIndex);

    slotAParams_ = BuildSlotParams(true);
    slotBParams_ = BuildSlotParams(false);

    engineA_.SetModel(slotAParams_.model);
    engineA_.SetCommonParams(slotAParams_.common);
    engineA_.SetModelParams(slotAParams_.modelSpecific);

    engineB_.SetModel(slotBParams_.model);
    engineB_.SetCommonParams(slotBParams_.common);
    engineB_.SetModelParams(slotBParams_.modelSpecific);
}

} // namespace mmf::platform::vcv
