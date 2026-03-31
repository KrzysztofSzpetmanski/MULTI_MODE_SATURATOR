#include "DualFilterModule.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace {

using namespace rack;

static constexpr std::array<int, 14> kDepthMenuSteps = {
    0, 5, 10, 15, 20, 25, 30, 40, 50, 60, 70, 80, 90, 100
};

struct PanelLabel : TransparentWidget {
    std::string text;
    int fontSize = 8;
    int align = NVG_ALIGN_CENTER | NVG_ALIGN_BASELINE;
    NVGcolor color = nvgRGB(0xe5, 0xe7, 0xeb);

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

struct CvDepthKnob : RoundSmallBlackKnob {
    mmf::platform::vcv::DualFilterModule* moduleRef = nullptr;
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

        drawTick(baseV, nvgRGBA(241, 245, 249, 230), 1.4f, 4.0f);
        drawTick(modV, nvgRGBA(16, 185, 129, 240), 2.1f, 5.5f);
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

DualFilterWidget::DualFilterWidget(DualFilterModule* module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DualFilter.svg")));

    addChild(createWidget<ScrewSilver>(mm2px(Vec(2.0f, 2.0f))));
    addChild(createWidget<ScrewSilver>(mm2px(Vec(2.0f, 112.0f))));
    addChild(createWidget<ScrewSilver>(mm2px(Vec(58.0f, 2.0f))));
    addChild(createWidget<ScrewSilver>(mm2px(Vec(58.0f, 112.0f))));

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(10.0f, 16.0f)), module, DualFilterModule::MODEL_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(30.0f, 16.0f)), module, DualFilterModule::MODEL_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(50.0f, 16.0f)), module, DualFilterModule::ROUTING_PARAM));

    addParam(createParamCentered<CKSS>(mm2px(Vec(10.0f, 28.0f)), module, DualFilterModule::A_BYPASS_PARAM));

    auto* aCutoffKnob = createParamCentered<CvDepthKnob>(mm2px(Vec(10.0f, 40.0f)), module, DualFilterModule::A_CUTOFF_PARAM);
    aCutoffKnob->moduleRef = module;
    aCutoffKnob->depthParam = DualFilterModule::A_CUTOFF_CV_DEPTH_PARAM;
    aCutoffKnob->cvInput = DualFilterModule::A_CUTOFF_CV_INPUT;
    aCutoffKnob->depthMenuLabel = "A CUTOFF CV depth";
    addParam(aCutoffKnob);

    auto* aResKnob = createParamCentered<CvDepthKnob>(mm2px(Vec(10.0f, 52.0f)), module, DualFilterModule::A_RESONANCE_PARAM);
    aResKnob->moduleRef = module;
    aResKnob->depthParam = DualFilterModule::A_RESONANCE_CV_DEPTH_PARAM;
    aResKnob->cvInput = DualFilterModule::A_RESONANCE_CV_INPUT;
    aResKnob->depthMenuLabel = "A RESONANCE CV depth";
    addParam(aResKnob);

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(10.0f, 64.0f)), module, DualFilterModule::A_DRIVE_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(10.0f, 76.0f)), module, DualFilterModule::A_MIX_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(10.0f, 88.0f)), module, DualFilterModule::A_MODE_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(10.0f, 98.0f)), module, DualFilterModule::A_P1_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(10.0f, 106.0f)), module, DualFilterModule::A_P2_PARAM));

    addParam(createParamCentered<CKSS>(mm2px(Vec(30.0f, 28.0f)), module, DualFilterModule::B_BYPASS_PARAM));

    auto* bCutoffKnob = createParamCentered<CvDepthKnob>(mm2px(Vec(30.0f, 40.0f)), module, DualFilterModule::B_CUTOFF_PARAM);
    bCutoffKnob->moduleRef = module;
    bCutoffKnob->depthParam = DualFilterModule::B_CUTOFF_CV_DEPTH_PARAM;
    bCutoffKnob->cvInput = DualFilterModule::B_CUTOFF_CV_INPUT;
    bCutoffKnob->depthMenuLabel = "B CUTOFF CV depth";
    addParam(bCutoffKnob);

    auto* bResKnob = createParamCentered<CvDepthKnob>(mm2px(Vec(30.0f, 52.0f)), module, DualFilterModule::B_RESONANCE_PARAM);
    bResKnob->moduleRef = module;
    bResKnob->depthParam = DualFilterModule::B_RESONANCE_CV_DEPTH_PARAM;
    bResKnob->cvInput = DualFilterModule::B_RESONANCE_CV_INPUT;
    bResKnob->depthMenuLabel = "B RESONANCE CV depth";
    addParam(bResKnob);

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(30.0f, 64.0f)), module, DualFilterModule::B_DRIVE_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(30.0f, 76.0f)), module, DualFilterModule::B_MIX_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(30.0f, 88.0f)), module, DualFilterModule::B_MODE_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(30.0f, 98.0f)), module, DualFilterModule::B_P1_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(30.0f, 106.0f)), module, DualFilterModule::B_P2_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50.0f, 40.0f)), module, DualFilterModule::AUDIO_IN_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50.0f, 54.0f)), module, DualFilterModule::A_CUTOFF_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50.0f, 66.0f)), module, DualFilterModule::A_RESONANCE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50.0f, 78.0f)), module, DualFilterModule::B_CUTOFF_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(50.0f, 90.0f)), module, DualFilterModule::B_RESONANCE_CV_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(50.0f, 106.0f)), module, DualFilterModule::AUDIO_OUT_OUTPUT));

    addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(54.0f, 54.0f)), module, DualFilterModule::A_CUTOFF_MOD_LIGHT));
    addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(54.0f, 66.0f)), module, DualFilterModule::A_RESONANCE_MOD_LIGHT));
    addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(54.0f, 78.0f)), module, DualFilterModule::B_CUTOFF_MOD_LIGHT));
    addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(54.0f, 90.0f)), module, DualFilterModule::B_RESONANCE_MOD_LIGHT));

    auto addPanelLabel = [this](float xMm, float yMm, const std::string& txt, int size = 7, NVGcolor color = nvgRGB(0xe5, 0xe7, 0xeb)) {
        auto* l = createWidget<PanelLabel>(mm2px(Vec(xMm, yMm)));
        l->text = txt;
        l->fontSize = size;
        l->color = color;
        addChild(l);
    };

    addPanelLabel(30.0f, 9.0f, "DUAL FILTER LAB", 9, nvgRGB(0xf8, 0xfa, 0xfc));
    addPanelLabel(50.0f, 12.8f, rack::string::f("BUILD %d", DualFilterModule::kBuildNumber), 7, nvgRGB(0x93, 0xc5, 0xfd));

    addPanelLabel(10.0f, 23.0f, "MODEL A", 6);
    addPanelLabel(30.0f, 23.0f, "MODEL B", 6);
    addPanelLabel(50.0f, 23.0f, "ROUTING", 6);

    addPanelLabel(10.0f, 34.0f, "A BYP", 6);
    addPanelLabel(10.0f, 37.0f, "CUT", 6);
    addPanelLabel(10.0f, 49.0f, "RES", 6);
    addPanelLabel(10.0f, 61.0f, "DRV", 6);
    addPanelLabel(10.0f, 73.0f, "MIX", 6);
    addPanelLabel(10.0f, 85.0f, "MODE", 6);
    addPanelLabel(10.0f, 95.0f, "P1", 6);
    addPanelLabel(10.0f, 103.0f, "P2", 6);

    addPanelLabel(30.0f, 34.0f, "B BYP", 6);
    addPanelLabel(30.0f, 37.0f, "CUT", 6);
    addPanelLabel(30.0f, 49.0f, "RES", 6);
    addPanelLabel(30.0f, 61.0f, "DRV", 6);
    addPanelLabel(30.0f, 73.0f, "MIX", 6);
    addPanelLabel(30.0f, 85.0f, "MODE", 6);
    addPanelLabel(30.0f, 95.0f, "P1", 6);
    addPanelLabel(30.0f, 103.0f, "P2", 6);

    addPanelLabel(50.0f, 34.0f, "IN", 6);
    addPanelLabel(50.0f, 48.0f, "A CUT CV", 6);
    addPanelLabel(50.0f, 60.0f, "A RES CV", 6);
    addPanelLabel(50.0f, 72.0f, "B CUT CV", 6);
    addPanelLabel(50.0f, 84.0f, "B RES CV", 6);
    addPanelLabel(50.0f, 100.0f, "OUT", 6);
}

} // namespace mmf::platform::vcv

rack::Model* modelDualFilter = rack::createModel<mmf::platform::vcv::DualFilterModule,
                                                  mmf::platform::vcv::DualFilterWidget>("DualFilterLab");
