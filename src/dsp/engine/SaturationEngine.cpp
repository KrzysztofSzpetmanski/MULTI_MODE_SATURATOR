#include "SaturationEngine.h"

namespace mmf::dsp::engine {

void SaturationEngine::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    softAsym_.Init(sampleRate_);
    diode_.Init(sampleRate_);
    tubeish_.Init(sampleRate_);
    tapeish_.Init(sampleRate_);
    PushParamsToAllModels();
}

void SaturationEngine::SetSampleRate(float sampleRate) {
    sampleRate_ = sampleRate;
    softAsym_.SetSampleRate(sampleRate_);
    diode_.SetSampleRate(sampleRate_);
    tubeish_.SetSampleRate(sampleRate_);
    tapeish_.SetSampleRate(sampleRate_);
}

void SaturationEngine::Reset() {
    softAsym_.Reset();
    diode_.Reset();
    tubeish_.Reset();
    tapeish_.Reset();
}

void SaturationEngine::SetModel(ModelType model) {
    model_ = model;
    ActiveModel().SetCommonParams(common_);
    ActiveModel().SetModelParams(modelParams_);
}

void SaturationEngine::SetCommonParams(const saturation::SaturationCommonParams& params) {
    common_ = params;
    PushParamsToAllModels();
}

void SaturationEngine::SetModelParams(const saturation::SaturationModelParams& params) {
    modelParams_ = params;
    PushParamsToAllModels();
}

float SaturationEngine::ProcessSample(float x) {
    return ActiveModel().Process(x);
}

saturation::ISaturationModel& SaturationEngine::ActiveModel() {
    switch (model_) {
    case ModelType::SoftAsym:
        return softAsym_;
    case ModelType::Diode:
        return diode_;
    case ModelType::Tubeish:
        return tubeish_;
    case ModelType::Tapeish:
        return tapeish_;
    }
    return softAsym_;
}

void SaturationEngine::PushParamsToAllModels() {
    softAsym_.SetCommonParams(common_);
    softAsym_.SetModelParams(modelParams_);

    diode_.SetCommonParams(common_);
    diode_.SetModelParams(modelParams_);

    tubeish_.SetCommonParams(common_);
    tubeish_.SetModelParams(modelParams_);

    tapeish_.SetCommonParams(common_);
    tapeish_.SetModelParams(modelParams_);
}

} // namespace mmf::dsp::engine
