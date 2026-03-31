#include "DualFilterModule.h"

#include <algorithm>
#include <cmath>

#include "../../dsp/common/AudioMath.h"

namespace mmf::platform::vcv {

using namespace rack;

DualFilterModule::DualFilterModule() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

    configParam(MODEL_A_PARAM, 0.0f, 5.0f, 0.0f, "Model A");
    configParam(MODEL_B_PARAM, 0.0f, 5.0f, 1.0f, "Model B");
    configParam(ROUTING_PARAM, 0.0f, 2.0f, 0.0f, "Routing");

    configSwitch(A_BYPASS_PARAM, 0.0f, 1.0f, 0.0f, "Bypass A", {"Off", "On"});
    configParam(A_CUTOFF_PARAM, 0.0f, 1.0f, 0.5f, "A cutoff");
    configParam(A_RESONANCE_PARAM, 0.0f, 1.0f, 0.1f, "A resonance");
    configParam(A_DRIVE_PARAM, 0.1f, 3.0f, 1.0f, "A drive");
    configParam(A_MIX_PARAM, 0.0f, 1.0f, 1.0f, "A mix");
    configParam(A_MODE_PARAM, 0.0f, 4.0f, 0.0f, "A mode");
    configParam(A_P1_PARAM, 0.0f, 1.0f, 0.0f, "A model p1");
    configParam(A_P2_PARAM, -1.0f, 1.0f, 0.0f, "A model p2");

    configSwitch(B_BYPASS_PARAM, 0.0f, 1.0f, 0.0f, "Bypass B", {"Off", "On"});
    configParam(B_CUTOFF_PARAM, 0.0f, 1.0f, 0.5f, "B cutoff");
    configParam(B_RESONANCE_PARAM, 0.0f, 1.0f, 0.1f, "B resonance");
    configParam(B_DRIVE_PARAM, 0.1f, 3.0f, 1.0f, "B drive");
    configParam(B_MIX_PARAM, 0.0f, 1.0f, 1.0f, "B mix");
    configParam(B_MODE_PARAM, 0.0f, 4.0f, 0.0f, "B mode");
    configParam(B_P1_PARAM, 0.0f, 1.0f, 0.0f, "B model p1");
    configParam(B_P2_PARAM, -1.0f, 1.0f, 0.0f, "B model p2");

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
}

void DualFilterModule::process(const ProcessArgs& args) {
    if (args.sampleRate != lastSampleRate_) {
        lastSampleRate_ = args.sampleRate;
        engine_.Init(lastSampleRate_, 1);
    }

    if ((frameCounter_++ & 0x7) == 0) {
        SyncParamsFromUi();
    }

    const float inV = inputs[AUDIO_IN_INPUT].getVoltage();
    const float x = inV * 0.2f;
    const float y = engine_.ProcessSample(x, 0);
    outputs[AUDIO_OUT_OUTPUT].setVoltage(5.0f * y);

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

mmf::dsp::filters::FilterSlotParams DualFilterModule::BuildSlotParams(bool a) {
    mmf::dsp::filters::FilterSlotParams p{};

    const int cutoffParam = a ? A_CUTOFF_PARAM : B_CUTOFF_PARAM;
    const int cutoffCv = a ? A_CUTOFF_CV_INPUT : B_CUTOFF_CV_INPUT;
    const int cutoffDepth = a ? A_CUTOFF_CV_DEPTH_PARAM : B_CUTOFF_CV_DEPTH_PARAM;

    const int resonanceParam = a ? A_RESONANCE_PARAM : B_RESONANCE_PARAM;
    const int resonanceCv = a ? A_RESONANCE_CV_INPUT : B_RESONANCE_CV_INPUT;
    const int resonanceDepth = a ? A_RESONANCE_CV_DEPTH_PARAM : B_RESONANCE_CV_DEPTH_PARAM;

    const float cutoffNorm = getModulatedRangeValue(cutoffParam, cutoffCv, cutoffDepth, 0.0f, 1.0f);
    const float resonanceNorm = getModulatedRangeValue(resonanceParam, resonanceCv, resonanceDepth, 0.0f, 1.0f);

    p.common.cutoffHz = mmf::dsp::common::NormalizedToFrequency(cutoffNorm);
    p.common.resonance = ClampNorm(resonanceNorm);
    p.common.drive = params[a ? A_DRIVE_PARAM : B_DRIVE_PARAM].getValue();
    p.common.mix = params[a ? A_MIX_PARAM : B_MIX_PARAM].getValue();
    p.common.level = 1.0f;

    p.model.mode = static_cast<int>(std::round(params[a ? A_MODE_PARAM : B_MODE_PARAM].getValue()));
    p.model.p1 = params[a ? A_P1_PARAM : B_P1_PARAM].getValue();
    p.model.p2 = params[a ? A_P2_PARAM : B_P2_PARAM].getValue();
    p.model.p3 = 0.5f;
    p.model.p4 = 0.0f;

    p.bypass = params[a ? A_BYPASS_PARAM : B_BYPASS_PARAM].getValue() > 0.5f;
    return p;
}

void DualFilterModule::SyncParamsFromUi() {
    auto aModel = static_cast<mmf::dsp::filters::FilterModelType>(
        static_cast<int>(std::round(params[MODEL_A_PARAM].getValue())));
    auto bModel = static_cast<mmf::dsp::filters::FilterModelType>(
        static_cast<int>(std::round(params[MODEL_B_PARAM].getValue())));

    auto routing = static_cast<mmf::dsp::filters::FilterRouting>(
        static_cast<int>(std::round(params[ROUTING_PARAM].getValue())));

    engine_.SetRouting(routing);
    engine_.SetSlotModel(mmf::dsp::engine::DualFilterEngine::SlotId::A, aModel);
    engine_.SetSlotModel(mmf::dsp::engine::DualFilterEngine::SlotId::B, bModel);

    engine_.SetSlotParams(mmf::dsp::engine::DualFilterEngine::SlotId::A, BuildSlotParams(true));
    engine_.SetSlotParams(mmf::dsp::engine::DualFilterEngine::SlotId::B, BuildSlotParams(false));
}

} // namespace mmf::platform::vcv
