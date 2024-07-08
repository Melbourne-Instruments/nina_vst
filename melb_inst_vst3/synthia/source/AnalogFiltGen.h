/**
 * @file AnalogFiltGen.h
 * @brief Declaration of the analog filter calibrator class
 * @date 2022-07-06
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */

#pragma once

#include "SynthMath.h"
#include "common.h"
#include <array>
#include <cmath>
#include <vector>

namespace Steinberg {
namespace Vst {
namespace Nina {

/**
 * @brief structure defining the filter calibration values. it contains sensible default values that should work for pre-calibration
 *
 */
struct FilterCalValues {
    float fc_high_clip = 0.35f;
    float fc_low_clip = -0.2f;
    float fc_gain = (fc_high_clip - fc_low_clip) / 2;
    float fc_offset = fc_low_clip + fc_gain;
    float fc_temp_track = 0.0f;
    float res_high_clip = -0.0f;
    float res_zero_offset = 0.0f;
    float res_gain = -.46f;
    float res_low_clip = -.46f;
    float base_temp = 0;
    float a = .25;
    float c = .11;

    // these values are used for the temp tracking. they are not adjusted in the model
    static constexpr float b = -0.104001;
    static constexpr float d = 0.810733;
};

class AnalogFiltCal {
  public:
    AnalogFiltCal() = default;
    ~AnalogFiltCal() = default;
    void run(const std::array<float, CV_BUFFER_SIZE> &cutoff, const std::array<float, CV_BUFFER_SIZE> &resonance, std::array<float, BUFFER_SIZE> &out);
    bool setCal(const FilterCalValues filter_cal);

    FilterCalValues getCal() {
        return _calibration;
    }

    void setTemp(float temp);

    float getLastCutoff() {
        return _last_cut;
    }

    float getLastRes() {
        return _resonance_out.at(0);
    }

    bool print = false;

  private:
    float _last_cut = 0;
    float _last_res = 0;
    std::array<float, CV_BUFFER_SIZE> _cutoff_out;
    std::array<float, CV_BUFFER_SIZE> _resonance_out;
    FilterCalValues _calibration;
    float _current_temp = 0.0f;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
