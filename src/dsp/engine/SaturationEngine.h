#pragma once

#include "../saturation/DiodeSaturator.h"
#include "../saturation/SoftAsymSaturator.h"
#include "../saturation/TapeishSaturator.h"
#include "../saturation/TubeishSaturator.h"

namespace mmf::dsp::engine {

class SaturationEngine {
  public:
    enum class ModelType {
        SoftAsym = 0,
        Diode = 1,
        Tubeish = 2,
        Tapeish = 3,
    };

    void Init(float sampleRate);
    void SetSampleRate(float sampleRate);
    void Reset();

    void SetModel(ModelType model);
    ModelType GetModel() const { return model_; }

    void SetCommonParams(const saturation::SaturationCommonParams& params);
    void SetModelParams(const saturation::SaturationModelParams& params);

    float ProcessSample(float x);

  private:
    saturation::ISaturationModel& ActiveModel();
    void PushParamsToAllModels();

    float sampleRate_ = 48000.0f;
    ModelType model_ = ModelType::SoftAsym;

    saturation::SaturationCommonParams common_{};
    saturation::SaturationModelParams modelParams_{};

    saturation::SoftAsymSaturator softAsym_;
    saturation::DiodeSaturator diode_;
    saturation::TubeishSaturator tubeish_;
    saturation::TapeishSaturator tapeish_;
};

} // namespace mmf::dsp::engine
