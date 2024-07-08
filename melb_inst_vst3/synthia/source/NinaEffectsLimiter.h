/**
 * @file NinaEffectsLimiter.cpp
 * @brief
 * @date 2022-12-06
 *
 * Copyright (c) 2023 Melbourne Instruments
 * A simple limiter to make sure that the reverb/delay/chorus don't completely clip
 *
 */
#pragma once
#include "SynthMath.h"
#include "common.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

constexpr float limiter_threshold = -33.0f;
constexpr float release_time_s = 250.f / 1000.f;
constexpr float release_fac = std::exp(-3.f / ((float)SAMPLE_RATE * release_time_s));
constexpr float hold_time_s = 8.5f / 1000.f;
constexpr int hold_samples = (int)(hold_time_s * SAMPLE_RATE);
constexpr float output_gain = 2.f;
constexpr float lim_thresh_linear = std::pow(10.f, limiter_threshold / 20.f);
constexpr float lim_thresh_linear_gain = lim_thresh_linear * output_gain;

class NinaLimiter {
  public:
    NinaLimiter(){};
    ~NinaLimiter(){};

    void setVolume(float vol) {
        _vol = vol * vol;
    }

    void reCalculate();

    void run(float *input_l, float *input_r, float *output_l, float *output_r) {
        float output1;
        _vol_smooth = param_smooth(_vol, _vol_smooth);
        for (uint sample_i = 0; sample_i < BUFFER_SIZE; sample_i++) {
            const float left_in = input_l[sample_i] * _vol_smooth;
            const float right_in = input_r[sample_i] * _vol_smooth;
            float max_abs_sample = std::max<float>(std::abs(left_in), std::abs(right_in));
            _timer_1++;
            _timer_2++;
            if (_timer_1 > hold_samples) {
                _timer_1 = 0;
                _current_max_1 = 0;
            }
            _current_max_1 = std::max<float>(_current_max_1, max_abs_sample);
            if (_timer_2 > hold_samples) {
                _timer_2 = 0;
                _current_max_2 = 0;
            }
            _current_max_2 = std::max<float>(_current_max_2, max_abs_sample);

            _env_time = std::max<float>(_current_max_1, _current_max_2);
            _envelope = _envelope < _env_time ? _env_time : _env_time + release_fac * (_envelope - _env_time);
            float gain = output_gain;
            if (_envelope > lim_thresh_linear) {
                gain = lim_thresh_linear_gain / _envelope;
            }
            output1 = gain;
            output_l[sample_i] = left_in * gain;
            output_r[sample_i] = right_in * gain;
        }
    }

  private:
    float _vol = 1;
    float _vol_smooth = 1;
    int _timer_1 = 0;
    int _timer_2 = 0;
    float _current_max_1 = 0;
    float _current_max_2 = 0;
    float _env_time = 0;
    float _envelope = 0;
};

// limiting threshold, set experimentally, lance
constexpr float out_limiter_threshold = -3.f;
constexpr float out_output_gain = 1.f;
constexpr float out_lim_thresh_linear = std::pow(10.f, out_limiter_threshold / 20.f);
constexpr float out_lim_thresh_linear_gain = out_lim_thresh_linear * out_output_gain;

// overall output gain to compensate for reduced fx gain: lance
constexpr float output_limiter_pregain = 3.98;

class OutputLimiter {
  public:
    OutputLimiter(){

    };
    ~OutputLimiter(){};

    void reCalculate();

    void run(float *input_l, float *input_r, float *output_l, float *output_r) {
        float output1;
        float gain = out_output_gain * _output_gain;
        for (uint sample_i = 0; sample_i < BUFFER_SIZE; sample_i++) {
            const float left_in = input_l[sample_i] * output_limiter_pregain;
            const float right_in = input_r[sample_i] * output_limiter_pregain;
            float max_abs_sample = std::max<float>(std::abs(left_in), std::abs(right_in));
            _timer_1++;
            _timer_2++;
            if (_timer_1 > hold_samples) {
                _timer_1 = 0;
                _current_max_1 = 0;
            }
            _current_max_1 = std::max<float>(_current_max_1, max_abs_sample);
            if (_timer_2 > hold_samples) {
                _timer_2 = 0;
                _current_max_2 = 0;
            }
            _current_max_2 = std::max<float>(_current_max_2, max_abs_sample);

            _env_time = std::max<float>(_current_max_1, _current_max_2);
            _envelope = _envelope < _env_time ? _env_time : _env_time + release_fac * (_envelope - _env_time);
            if (_envelope > out_lim_thresh_linear) {
                gain = out_lim_thresh_linear_gain / _envelope;
            }
            output1 = gain;
            output_l[sample_i] = left_in * gain;
            output_r[sample_i] = right_in * gain;
        }
    }

  private:
    float _vol = 1;
    float _vol_smooth = 1;
    int _timer_1 = 0;
    float _output_gain = 1;
    int _timer_2 = 0;
    float _current_max_1 = 0;
    float _current_max_2 = 0;
    float _env_time = 0;
    float _envelope = 0;
};
} // namespace Nina
} // namespace Vst
} // namespace Steinberg
