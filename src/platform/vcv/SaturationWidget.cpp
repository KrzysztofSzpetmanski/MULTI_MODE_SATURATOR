#include "SaturationModule.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace {

using namespace rack;

static const std::array<std::string, 5> kModelLabels = {
    "OFF", "SOFT", "DIODE", "TUBE", "TAPE"
};

static const std::array<std::string, 3> kRoutingLabels = {
    "DUAL", "SERIAL", "PARALLEL"
};

static const std::array<std::string, 5> kOutputTrimLabels = {"-6 dB", "-3 dB", "0 dB", "+3 dB", "+6 dB"};

static const std::array<int, 14> kDepthMenuSteps = {
    0, 5, 10, 15, 20, 25, 30, 40, 50, 60, 70, 80, 90, 100
};

static std::vector<std::string> GetModeLabelsForModel(int modelIdx) {
    switch (modelIdx) {
    case 0:
        return {"OFF"};
    case 1:
    case 2:
    case 3:
    case 4:
        return {kOutputTrimLabels.begin(), kOutputTrimLabels.end()};
    default:
        return {"OFF"};
    }
}

static std::array<std::string, 4> GetControlBaseLabelsForModel(int modelIdx) {
    switch (modelIdx) {
    case 0:
        return {"OFF", "", "", ""};
    case 1:
    case 2:
    case 3:
    case 4:
        return {"TONE", "BIAS", "DRV", "MIX"};
    default:
        return {"OFF", "", "", ""};
    }
}

struct PanelLabel : TransparentWidget {
    std::string text;
    int fontSize = 7;
    int align = NVG_ALIGN_CENTER | NVG_ALIGN_BASELINE;
    NVGcolor color = nvgRGB(0x0f, 0x17, 0x2a);

    void draw(const DrawArgs& args) override {
        auto font = APP->window->loadFont(asset::system("res/fonts/DejaVuSans.ttf"));
        if (!font) {
            return;
        }
        nvgFontFaceId(args.vg, font->handle);
        nvgFontSize(args.vg, static_cast<float>(fontSize));
        nvgFillColor(args.vg, color);
        nvgTextAlign(args.vg, align);
        nvgText(args.vg, 0.0f, 0.0f, text.c_str(), nullptr);
    }
};

struct DynamicControlLabel : TransparentWidget {
    mmf::platform::vcv::SaturationModule* moduleRef = nullptr;
    bool slotA = true;
    int controlIndex = 0; // 0..3
    bool cvLabel = false; // only meaningful for control 0/1
    int fontSize = 6;
    NVGcolor color = nvgRGB(0x0f, 0x17, 0x2a);

    void draw(const DrawArgs& args) override {
        if (controlIndex < 0 || controlIndex > 3) {
            return;
        }

        int modelIdx = slotA ? 1 : 2; // fallback when module is null (module browser / preview path)
        if (moduleRef) {
            const int modelParam = slotA ? mmf::platform::vcv::SaturationModule::MODEL_A_PARAM
                                         : mmf::platform::vcv::SaturationModule::MODEL_B_PARAM;
            modelIdx = rack::math::clamp(
                static_cast<int>(std::round(moduleRef->params[modelParam].getValue())),
                0,
                4);
        }

        const auto base = GetControlBaseLabelsForModel(modelIdx);
        std::string label = base[controlIndex];
        if (label.empty()) {
            return;
        }
        if (cvLabel) {
            label += " CV";
        }

        auto font = APP->window->loadFont(asset::system("res/fonts/DejaVuSans.ttf"));
        if (!font) {
            return;
        }
        nvgFontFaceId(args.vg, font->handle);
        nvgFontSize(args.vg, static_cast<float>(fontSize));
        nvgFillColor(args.vg, color);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_BASELINE);
        nvgText(args.vg, 0.0f, 0.0f, label.c_str(), nullptr);
    }
};

struct CompactChoice : OpaqueWidget {
    std::string text;
    int fontSize = 7;

    void DrawBackground(const DrawArgs& args) {
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0.0f, 0.0f, box.size.x, box.size.y, 2.0f);
        nvgFillColor(args.vg, nvgRGB(0xec, 0xf2, 0xff));
        nvgFill(args.vg);

        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0.5f, 0.5f, box.size.x - 1.0f, box.size.y - 1.0f, 2.0f);
        nvgStrokeWidth(args.vg, 1.0f);
        nvgStrokeColor(args.vg, nvgRGB(0x33, 0x41, 0x55));
        nvgStroke(args.vg);
    }

    void DrawTextAndArrow(const DrawArgs& args) {
        auto font = APP->window->loadFont(asset::system("res/fonts/DejaVuSans.ttf"));
        if (font) {
            nvgFontFaceId(args.vg, font->handle);
            nvgFontSize(args.vg, static_cast<float>(fontSize));
            nvgFillColor(args.vg, nvgRGB(0x0f, 0x17, 0x2a));
            nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgText(args.vg, box.size.x * 0.5f, box.size.y * 0.5f, text.c_str(), nullptr);
        }

        const float x = box.size.x - 5.5f;
        const float y = box.size.y * 0.5f;
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, x - 2.0f, y - 1.0f);
        nvgLineTo(args.vg, x + 2.0f, y - 1.0f);
        nvgLineTo(args.vg, x, y + 1.8f);
        nvgClosePath(args.vg);
        nvgFillColor(args.vg, nvgRGB(0x1f, 0x29, 0x37));
        nvgFill(args.vg);
    }

    void draw(const DrawArgs& args) override {
        DrawBackground(args);
        DrawTextAndArrow(args);
    }
};

struct ModelChoice : CompactChoice {
    mmf::platform::vcv::SaturationModule* module = nullptr;
    int paramId = -1;

    void step() override {
        if (module && paramId >= 0) {
            const int idx = rack::math::clamp(static_cast<int>(std::round(module->params[paramId].getValue())), 0, 4);
            text = kModelLabels[idx];
        }
        CompactChoice::step();
    }

    void onButton(const event::Button& e) override {
        if (e.action != GLFW_PRESS || e.button != GLFW_MOUSE_BUTTON_LEFT) {
            return;
        }
        ui::Menu* menu = createMenu();
        menu->addChild(createMenuLabel("Model"));
        for (int i = 0; i < static_cast<int>(kModelLabels.size()); ++i) {
            menu->addChild(createCheckMenuItem(
                kModelLabels[i],
                "",
                [=]() {
                    return module && static_cast<int>(std::round(module->params[paramId].getValue())) == i;
                },
                [=]() {
                    if (module) {
                        module->params[paramId].setValue(static_cast<float>(i));
                    }
                }));
        }
        e.consume(this);
    }
};

struct RoutingChoice : CompactChoice {
    mmf::platform::vcv::SaturationModule* module = nullptr;

    void step() override {
        if (module) {
            const int idx = rack::math::clamp(static_cast<int>(std::round(module->params[mmf::platform::vcv::SaturationModule::ROUTING_PARAM].getValue())), 0, 2);
            text = kRoutingLabels[idx];
        }
        CompactChoice::step();
    }

    void onButton(const event::Button& e) override {
        if (e.action != GLFW_PRESS || e.button != GLFW_MOUSE_BUTTON_LEFT) {
            return;
        }
        ui::Menu* menu = createMenu();
        menu->addChild(createMenuLabel("Routing"));
        for (int i = 0; i < static_cast<int>(kRoutingLabels.size()); ++i) {
            menu->addChild(createCheckMenuItem(
                kRoutingLabels[i],
                "",
                [=]() {
                    return module && static_cast<int>(std::round(module->params[mmf::platform::vcv::SaturationModule::ROUTING_PARAM].getValue())) == i;
                },
                [=]() {
                    if (module) {
                        module->params[mmf::platform::vcv::SaturationModule::ROUTING_PARAM].setValue(static_cast<float>(i));
                    }
                }));
        }
        e.consume(this);
    }
};

struct ModeChoice : CompactChoice {
    mmf::platform::vcv::SaturationModule* module = nullptr;
    bool slotA = true;

    int getModelParam() const {
        return slotA ? mmf::platform::vcv::SaturationModule::MODEL_A_PARAM : mmf::platform::vcv::SaturationModule::MODEL_B_PARAM;
    }

    int getModeParam() const {
        return slotA ? mmf::platform::vcv::SaturationModule::A_MODE_PARAM : mmf::platform::vcv::SaturationModule::B_MODE_PARAM;
    }

    void step() override {
        if (module) {
            const int modelIdx = rack::math::clamp(static_cast<int>(std::round(module->params[getModelParam()].getValue())), 0, 4);
            const auto labels = GetModeLabelsForModel(modelIdx);
            const int modeIdx = rack::math::clamp(static_cast<int>(std::round(module->params[getModeParam()].getValue())),
                                                  0,
                                                  static_cast<int>(labels.size()) - 1);
            text = labels[modeIdx];
        }
        CompactChoice::step();
    }

    void onButton(const event::Button& e) override {
        if (e.action != GLFW_PRESS || e.button != GLFW_MOUSE_BUTTON_LEFT || !module) {
            return;
        }

        const int modelIdx = rack::math::clamp(static_cast<int>(std::round(module->params[getModelParam()].getValue())), 0, 4);
        const auto labels = GetModeLabelsForModel(modelIdx);
        ui::Menu* menu = createMenu();
        menu->addChild(createMenuLabel(slotA ? "Output A" : "Output B"));
        for (int i = 0; i < static_cast<int>(labels.size()); ++i) {
            menu->addChild(createCheckMenuItem(
                labels[i],
                "",
                [=]() {
                    return static_cast<int>(std::round(module->params[getModeParam()].getValue())) == i;
                },
                [=]() {
                    module->params[getModeParam()].setValue(static_cast<float>(i));
                }));
        }
        e.consume(this);
    }
};

struct CvDepthKnob : RoundSmallBlackKnob {
    mmf::platform::vcv::SaturationModule* moduleRef = nullptr;
    int depthParam = -1;
    int cvInput = -1;
    std::string depthMenuLabel = "CV depth";

    void draw(const DrawArgs& args) override {
        RoundSmallBlackKnob::draw(args);
        if (!moduleRef || depthParam < 0 || cvInput < 0) {
            return;
        }
        if (!moduleRef->inputs[cvInput].isConnected()) {
            return;
        }

        auto* pq = getParamQuantity();
        if (!pq) {
            return;
        }

        const float minV = pq->getMinValue();
        const float maxV = pq->getMaxValue();
        const float baseV = moduleRef->params[paramId].getValue();
        const float modV = moduleRef->getModulatedRangeValue(paramId, cvInput, depthParam, minV, maxV);

        const float depth = moduleRef->getCvDepthValue(depthParam);
        const float halfRange = 0.5f * (maxV - minV) * depth;
        const float lowV = rack::math::clamp(baseV - halfRange, minV, maxV);
        const float highV = rack::math::clamp(baseV + halfRange, minV, maxV);

        auto normalize = [minV, maxV](float v) {
            if (maxV <= minV) {
                return 0.0f;
            }
            return rack::math::clamp((v - minV) / (maxV - minV), 0.0f, 1.0f);
        };

        const float pi = 3.14159265359f;
        const float ringMinA = minAngle - 0.5f * pi;
        const float ringMaxA = maxAngle - 0.5f * pi;
        auto toAngle = [&](float v) {
            float t = normalize(v);
            return ringMinA + t * (ringMaxA - ringMinA);
        };

        const Vec c = box.size.div(2.0f);
        const float r = std::min(box.size.x, box.size.y) * 0.56f;

        auto drawArc = [&](float fromValue, float toValue, NVGcolor col, float width) {
            float a0 = toAngle(fromValue);
            float a1 = toAngle(toValue);
            if (a1 < a0) {
                std::swap(a0, a1);
            }
            nvgBeginPath(args.vg);
            nvgArc(args.vg, c.x, c.y, r, a0, a1, NVG_CW);
            nvgStrokeWidth(args.vg, width);
            nvgStrokeColor(args.vg, col);
            nvgStroke(args.vg);
        };

        drawArc(minV, maxV, nvgRGBA(71, 85, 105, 120), 1.2f);
        drawArc(lowV, highV, nvgRGBA(37, 99, 235, 220), 2.0f);

        auto drawTick = [&](float value, NVGcolor col, float width, float len) {
            const float a = toAngle(value);
            const float x0 = c.x + std::cos(a) * (r - len);
            const float y0 = c.y + std::sin(a) * (r - len);
            const float x1 = c.x + std::cos(a) * (r + 0.5f);
            const float y1 = c.y + std::sin(a) * (r + 0.5f);
            nvgBeginPath(args.vg);
            nvgMoveTo(args.vg, x0, y0);
            nvgLineTo(args.vg, x1, y1);
            nvgStrokeWidth(args.vg, width);
            nvgStrokeColor(args.vg, col);
            nvgStroke(args.vg);
        };

        drawTick(baseV, nvgRGBA(15, 23, 42, 220), 1.5f, 4.2f);
        drawTick(modV, nvgRGBA(16, 185, 129, 240), 2.2f, 5.8f);
    }

    void appendContextMenu(Menu* menu) override {
        RoundSmallBlackKnob::appendContextMenu(menu);
        if (!moduleRef || depthParam < 0) {
            return;
        }

        const int depthRounded = rack::math::clamp(
            static_cast<int>(std::round(moduleRef->params[depthParam].getValue() * 100.0f)),
            0,
            100);
        const std::string rightText = rack::string::f("%d%%", depthRounded);

        menu->addChild(new MenuSeparator());
        menu->addChild(createSubmenuItem(depthMenuLabel, rightText, [this](Menu* submenu) {
            for (int pct : kDepthMenuSteps) {
                submenu->addChild(createCheckMenuItem(
                    rack::string::f("%d%%", pct),
                    "",
                    [this, pct]() {
                        const int curr = rack::math::clamp(
                            static_cast<int>(std::round(moduleRef->params[depthParam].getValue() * 100.0f)),
                            0,
                            100);
                        return curr == pct;
                    },
                    [this, pct]() {
                        moduleRef->params[depthParam].setValue(pct / 100.0f);
                    }));
            }
        }));
    }
};

} // namespace

namespace mmf::platform::vcv {

using namespace rack;

SaturationWidget::SaturationWidget(SaturationModule* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Saturation.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    auto* routingChoice = createWidget<RoutingChoice>(mm2px(Vec(17.5f, 12.5f)));
    routingChoice->box.size = mm2px(Vec(30.0f, 7.0f));
    routingChoice->module = module;
    addChild(routingChoice);

    auto* modelAChoice = createWidget<ModelChoice>(mm2px(Vec(5.5f, 24.0f)));
    modelAChoice->box.size = mm2px(Vec(24.0f, 7.0f));
    modelAChoice->module = module;
    modelAChoice->paramId = SaturationModule::MODEL_A_PARAM;
    addChild(modelAChoice);

    auto* modelBChoice = createWidget<ModelChoice>(mm2px(Vec(35.5f, 24.0f)));
    modelBChoice->box.size = mm2px(Vec(24.0f, 7.0f));
    modelBChoice->module = module;
    modelBChoice->paramId = SaturationModule::MODEL_B_PARAM;
    addChild(modelBChoice);

    auto* modeAChoice = createWidget<ModeChoice>(mm2px(Vec(5.5f, 33.0f)));
    modeAChoice->box.size = mm2px(Vec(24.0f, 7.0f));
    modeAChoice->module = module;
    modeAChoice->slotA = true;
    addChild(modeAChoice);

    auto* modeBChoice = createWidget<ModeChoice>(mm2px(Vec(35.5f, 33.0f)));
    modeBChoice->box.size = mm2px(Vec(24.0f, 7.0f));
    modeBChoice->module = module;
    modeBChoice->slotA = false;
    addChild(modeBChoice);

    const float colCvA = 10.0f;
    const float colKnobA = 25.0f;
    const float colKnobB = 40.0f;
    const float colCvB = 55.0f;

    const float rowCut = 53.0f;
    const float rowRes = 68.0f;
    const float rowDrv = 83.0f;
    const float rowMix = 98.0f;

    auto* aCutoffKnob = createParamCentered<CvDepthKnob>(mm2px(Vec(colKnobA, rowCut)), module, SaturationModule::A_CUTOFF_PARAM);
    aCutoffKnob->moduleRef = module;
    aCutoffKnob->depthParam = SaturationModule::A_CUTOFF_CV_DEPTH_PARAM;
    aCutoffKnob->cvInput = SaturationModule::A_CUTOFF_CV_INPUT;
    aCutoffKnob->depthMenuLabel = "A TONE CV depth";
    addParam(aCutoffKnob);

    auto* aResKnob = createParamCentered<CvDepthKnob>(mm2px(Vec(colKnobA, rowRes)), module, SaturationModule::A_RESONANCE_PARAM);
    aResKnob->moduleRef = module;
    aResKnob->depthParam = SaturationModule::A_RESONANCE_CV_DEPTH_PARAM;
    aResKnob->cvInput = SaturationModule::A_RESONANCE_CV_INPUT;
    aResKnob->depthMenuLabel = "A BIAS CV depth";
    addParam(aResKnob);

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(colKnobA, rowDrv)), module, SaturationModule::A_DRIVE_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(colKnobA, rowMix)), module, SaturationModule::A_MIX_PARAM));

    auto* bCutoffKnob = createParamCentered<CvDepthKnob>(mm2px(Vec(colKnobB, rowCut)), module, SaturationModule::B_CUTOFF_PARAM);
    bCutoffKnob->moduleRef = module;
    bCutoffKnob->depthParam = SaturationModule::B_CUTOFF_CV_DEPTH_PARAM;
    bCutoffKnob->cvInput = SaturationModule::B_CUTOFF_CV_INPUT;
    bCutoffKnob->depthMenuLabel = "B TONE CV depth";
    addParam(bCutoffKnob);

    auto* bResKnob = createParamCentered<CvDepthKnob>(mm2px(Vec(colKnobB, rowRes)), module, SaturationModule::B_RESONANCE_PARAM);
    bResKnob->moduleRef = module;
    bResKnob->depthParam = SaturationModule::B_RESONANCE_CV_DEPTH_PARAM;
    bResKnob->cvInput = SaturationModule::B_RESONANCE_CV_INPUT;
    bResKnob->depthMenuLabel = "B BIAS CV depth";
    addParam(bResKnob);

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(colKnobB, rowDrv)), module, SaturationModule::B_DRIVE_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(colKnobB, rowMix)), module, SaturationModule::B_MIX_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(colCvA, rowCut)), module, SaturationModule::A_CUTOFF_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(colCvA, rowRes)), module, SaturationModule::A_RESONANCE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(colCvB, rowCut)), module, SaturationModule::B_CUTOFF_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(colCvB, rowRes)), module, SaturationModule::B_RESONANCE_CV_INPUT));

    addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(colCvA + 7.2f, rowCut)), module, SaturationModule::A_CUTOFF_MOD_LIGHT));
    addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(colCvA + 7.2f, rowRes)), module, SaturationModule::A_RESONANCE_MOD_LIGHT));
    addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(colCvB - 7.2f, rowCut)), module, SaturationModule::B_CUTOFF_MOD_LIGHT));
    addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(colCvB - 7.2f, rowRes)), module, SaturationModule::B_RESONANCE_MOD_LIGHT));

    const float ioY = 113.0f;
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(colCvA, ioY)), module, SaturationModule::AUDIO_A_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(colKnobA, ioY)), module, SaturationModule::AUDIO_B_INPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(colKnobB, ioY)), module, SaturationModule::AUDIO_A_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(colCvB, ioY)), module, SaturationModule::AUDIO_B_OUTPUT));

    auto addPanelLabel = [this](float xMm, float yMm, const std::string& txt, int size = 7, NVGcolor color = nvgRGB(0x0f, 0x17, 0x2a)) {
        auto* l = createWidget<PanelLabel>(mm2px(Vec(xMm, yMm)));
        l->text = txt;
        l->fontSize = size;
        l->color = color;
        addChild(l);
    };

    addPanelLabel(33.0f, 8.0f, "DUAL SATURATOR LAB", 9, nvgRGB(0x0b, 0x12, 0x20));
    addPanelLabel(55.0f, 10.0f, rack::string::f("BUILD %d", SaturationModule::kBuildNumber), 7, nvgRGB(0x1f, 0x29, 0x37));

    addPanelLabel(10.0f, 16.0f, "A", 11);
    addPanelLabel(55.0f, 16.0f, "B", 11);

    auto addDynamicControlLabel = [this, module](float xMm, float yMm, bool slotA, int controlIndex, bool cvLabel, int size = 6) {
        auto* l = createWidget<DynamicControlLabel>(mm2px(Vec(xMm, yMm)));
        l->moduleRef = module;
        l->slotA = slotA;
        l->controlIndex = controlIndex;
        l->cvLabel = cvLabel;
        l->fontSize = size;
        addChild(l);
    };

    addDynamicControlLabel(colCvA, 46.0f, true, 0, true, 6);
    addDynamicControlLabel(colKnobA, 46.0f, true, 0, false, 6);
    addDynamicControlLabel(colKnobB, 46.0f, false, 0, false, 6);
    addDynamicControlLabel(colCvB, 46.0f, false, 0, true, 6);

    addDynamicControlLabel(colCvA, 61.0f, true, 1, true, 6);
    addDynamicControlLabel(colKnobA, 61.0f, true, 1, false, 6);
    addDynamicControlLabel(colKnobB, 61.0f, false, 1, false, 6);
    addDynamicControlLabel(colCvB, 61.0f, false, 1, true, 6);

    addDynamicControlLabel(colKnobA, 76.0f, true, 2, false, 6);
    addDynamicControlLabel(colKnobB, 76.0f, false, 2, false, 6);
    addDynamicControlLabel(colKnobA, 91.0f, true, 3, false, 6);
    addDynamicControlLabel(colKnobB, 91.0f, false, 3, false, 6);

    addPanelLabel(33.0f, 106.0f, "I/O", 7);
    addPanelLabel(colCvA, 121.0f, "IN A", 6);
    addPanelLabel(colKnobA, 121.0f, "IN B", 6);
    addPanelLabel(colKnobB, 121.0f, "OUT A", 6);
    addPanelLabel(colCvB, 121.0f, "OUT B", 6);
}

} // namespace mmf::platform::vcv

rack::Model* modelDualSaturation = rack::createModel<mmf::platform::vcv::SaturationModule,
                                                  mmf::platform::vcv::SaturationWidget>("Dual_Saturation_Lab");
