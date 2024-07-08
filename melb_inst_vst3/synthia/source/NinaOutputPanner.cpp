#include "NinaOutputPanner.h"
#include <cmath>

namespace Steinberg {
namespace Vst {
namespace Nina {

void NinaOutPanner::reCalculate() {
    float spin_nor = fabsf32(_spin_rate);
    _spin_inc = (spin_nor * _spin_rate) * (max_spin_rate / (float)CV_SAMPLE_RATE);
    float spin_cut = -6 * (8 + 40 * spin_nor) / (float)BUFFER_RATE + 1.f;
    _filter_a = std::exp(-2 * M_PI * (4 + 40 * spin_nor) / CV_SAMPLE_RATE);
    _filter_b = 1 - _filter_a;

    // if the spin value is near 0, we decay the spin position to move the panning back to center smoothly
    if (spin_nor < SPIN_THRESH) {
        _spin_inc = 0;
        _spin_pan -= _spin_pan * spin_pan_decay;
    }
    _spin_pan = _spin_pan > M_PI ? _spin_pan - 2 * M_PI : _spin_pan;
    _spin_pan = _spin_pan < -M_PI ? _spin_pan + 2 * M_PI : _spin_pan;
    float scaled_pan_pos = (M_PI / 4) * _pan_position;
    const float pan = (_spin_pan - scaled_pan_pos) + M_PIf32 / (4);
    if (dump) {
    }
    _sin_pan = (std::sin(pan));
    _cos_pan = (std::cos(pan));

    // reset the max level value
}

void NinaOutPanner::run() {
    _spin_pan += _spin_inc;

    // transform the assymetric vca input range of +1,-3 to the range 1,-1. _overdrive_comp includes a scaling factor of 0.5
    float vol = cv_clip((_vca_in + 1.f) * _overdrive_comp);
    _left_pan = _sin_pan * _filter_b + _left_pan * _filter_a;
    _right_pan = _cos_pan * _filter_b + _right_pan * _filter_a;
    _out_left = _left_pan * vol;
    _out_right = _right_pan * vol;

    // find the max output level for this buffer
    _max_level = vol > _max_level ? vol : _max_level;

    // should be cool

    /**
        if (dump) {
            printf("\nspin: %f %f %f %f %f", _out_left, _out_right, _spin_rate, _spin_inc, _spin_pan);
            _spin_pan = 0;
            dump = false;
        }
        **/
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
