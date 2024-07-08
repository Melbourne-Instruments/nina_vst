
#pragma once

#include "SynthMath.h"
#include "common.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

class NinaOutPanner {
  public:
    NinaOutPanner(float &left, float &right, float &overdrive_comp, float &max_mix_out_level) :
        _out_left(left), _out_right(right), _overdrive_comp(overdrive_comp), _max_level(max_mix_out_level){};
    ~NinaOutPanner(){};

    void run();
    void reCalculate();

    float *getSpinIn() {
        return &_spin_rate;
    }

    void resetSpin() {
        _spin_pan = 0;
    }

    float *getPanIn() {
        return &_pan_position;
    }

    float *getVcaIn() {
        return &_vca_in;
    }

    bool dump = false;

    float &getMaxVcaVolume() {
        return _max_level;
    }

  private:
    static constexpr float PAN_SMOOTHING = std::exp(-2 * M_PI * 45.0 / BUFFER_RATE);
    static constexpr float SPIN_THRESH = 0.03;
    static constexpr float spin_pan_decay = 2.f / BUFFER_RATE;
    static constexpr float max_spin_rate = 4.5 * 2 * M_PI;
    /**
     * @brief phase increment calculation = 2^(log(2,2*pi/fs) + freq_exponential)
     * so we precalculate the log(2,2*pi/fs factor) TODO: [NINA-268] investigate making the phase factor a constexpr
     *
     */
    const float phase_factor = std::exp2f(2.0f * M_PIf32 / CV_SAMPLE_RATE);
    float _scaled_pan_pos = 0;
    float _sin_pan = 0;
    float _cos_pan = 0;
    float _left_pan = 0;
    float _right_pan = 0;
    float _spin_cut = 0;
    float _filter_a = 0;
    float _filter_b = 0;
    float &_max_level;

    float _spin_inc = 0;
    float _spin_pan = 0;
    float _spin_rate = 0;
    float _pan_position = 0;
    float _volume = 1.0;
    float _vca_in = 0.5;

    float &_out_left;
    float &_out_right;
    float &_overdrive_comp;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg