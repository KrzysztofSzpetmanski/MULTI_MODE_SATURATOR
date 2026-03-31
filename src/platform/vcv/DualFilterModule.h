#pragma once

#include <cstdint>

#include "../../dsp/engine/DualFilterEngine.h"
#include "../../plugin.hpp"

namespace mmf::platform::vcv {

class DualFilterModule : public rack::Module {
  public:
    static constexpr int kBuildNumber = 21;

    enum ParamIds {
        MODEL_A_PARAM,
        MODEL_B_PARAM,
        ROUTING_PARAM,

        A_MODE_PARAM,
        A_CUTOFF_PARAM,
        A_RESONANCE_PARAM,
        A_DRIVE_PARAM,
        A_MIX_PARAM,

        B_MODE_PARAM,
        B_CUTOFF_PARAM,
        B_RESONANCE_PARAM,
        B_DRIVE_PARAM,
        B_MIX_PARAM,

        A_CUTOFF_CV_DEPTH_PARAM,
        A_RESONANCE_CV_DEPTH_PARAM,
        B_CUTOFF_CV_DEPTH_PARAM,
        B_RESONANCE_CV_DEPTH_PARAM,

        NUM_PARAMS
    };

    enum InputIds {
        AUDIO_A_INPUT,
        AUDIO_B_INPUT,

        A_CUTOFF_CV_INPUT,
        A_RESONANCE_CV_INPUT,
        B_CUTOFF_CV_INPUT,
        B_RESONANCE_CV_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        AUDIO_A_OUTPUT,
        AUDIO_B_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds {
        A_CUTOFF_MOD_LIGHT,
        A_RESONANCE_MOD_LIGHT,
        B_CUTOFF_MOD_LIGHT,
        B_RESONANCE_MOD_LIGHT,
        NUM_LIGHTS
    };

    DualFilterModule();
    void process(const ProcessArgs& args) override;

    float getModulatedRangeValue(int baseParam, int cvInput, int depthParam, float minV, float maxV);
    float getCvDepthValue(int depthParam);

  private:
    static float ClampNorm(float x);
    static int ClampInt(int x, int minV, int maxV);

    dsp::filters::FilterSlotParams BuildSlotParams(bool a);
    void SyncParamsFromUi();

    dsp::engine::DualFilterEngine engine_;
    float lastSampleRate_ = 44100.0f;
    uint32_t frameCounter_ = 0;
};

class DualFilterWidget : public rack::ModuleWidget {
  public:
    explicit DualFilterWidget(DualFilterModule* module);
};

} // namespace mmf::platform::vcv
