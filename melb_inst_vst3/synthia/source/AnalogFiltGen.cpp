/**
 * @file AnalogFiltGen.cpp
 * @brief imprementation of our analog filter calibration class
 * @date 2022-07-06
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */

#include "AnalogFiltGen.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

void AnalogFiltCal::run(const std::array<float, CV_BUFFER_SIZE> &cutoff, const std::array<float, CV_BUFFER_SIZE> &resonance, std::array<float, BUFFER_SIZE> &out) {

    // we scale, offset and clip both the resonance and the cutoff according to cal. This linear model seems good enough for now.
    const auto &cal = _calibration;
    int i = 0;
    float tmp_c;
    for (const float &item : cutoff) {

        tmp_c = item < .90 ? item : .90;
        tmp_c = tmp_c > -1.0 ? tmp_c : -1.0;
        tmp_c = tmp_c * (cal.a + cal.b * _current_temp) + cal.c + cal.d * _current_temp;
        out[i + Cv1Order::Cv1FilterCut] = tmp_c;
        i += CV_MUX_INC;
    }
    _last_cut = tmp_c;
    i = 0;
    for (const float &item : resonance) {
        float tmp_res = item * cal.res_gain + cal.res_zero_offset;

        tmp_res = tmp_res < cal.res_high_clip ? tmp_res : cal.res_high_clip;
        tmp_res = tmp_res > cal.res_low_clip ? tmp_res : cal.res_low_clip;
        out[i + Cv1Order::Cv1FilterQ] = tmp_res;
        i += CV_MUX_INC;
    }
    if (print) {
        printf("\n filter: %f %f %f", _current_temp, cal.res_zero_offset, out[0 + Cv1Order::Cv1FilterQ]);
        print = false;
    }
};

bool AnalogFiltCal::setCal(const FilterCalValues filter_cal) {
    _calibration = filter_cal;
    return true;
}

void AnalogFiltCal::setTemp(float temp) {
    _current_temp = temp - _calibration.base_temp;
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
