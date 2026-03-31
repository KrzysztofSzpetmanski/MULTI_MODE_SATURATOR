#pragma once

#include "ISaturationModel.h"

#include "../common/DcBlock.h"
#include "../common/OnePole.h"
#include "../common/ParameterSmoother.h"

namespace mmf::dsp::saturation {

class DiodeSaturator : public ISaturationModel {
  public:
    void Init(float sampleRate) override;
    void Reset() override;
    void SetSampleRate(float sampleRate) override;
    void SetCommonParams(const SaturationCommonParams& params) override;
    void SetModelParams(const SaturationModelParams& params) override;
    float Process(float x) override;

  private:
    float sampleRate_ = 48000.0f;
    SaturationCommonParams common_{};
    SaturationModelParams model_{};

    common::ParameterSmoother driveSmoother_;
    common::ParameterSmoother toneSmoother_;
    common::ParameterSmoother mixSmoother_;
    common::ParameterSmoother outputSmoother_;
    common::ParameterSmoother biasSmoother_;

    common::OnePole preLowpass_;
    common::OnePole postLowpass_;
    common::DcBlock dcBlock_;
};

} // namespace mmf::dsp::saturation
