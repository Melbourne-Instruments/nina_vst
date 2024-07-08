
/**
 * @brief This delay algorithm is a mashup of a few algorithms sourced as below:
        Credit the actual delay algo goes to Arne Scheffler with his MDA Dub Delay plugin.
 *  mda-vst3
 *@subsection delay source delay source
 *  Created by Arne Scheffler on 6/14/08.
 *
 *
 *  Copyright (c) 2008 Paul Kellett
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * @subsection distortion source
 * Credit for the in-loop saturation goes to Schwa, sourced from the Soft Clipper/Limiter JS plugin in reaper
 *
 */
#pragma once
#include "NinaParameters.h"
#include "SynthMath.h"
#include "common.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

static constexpr int DELAY_SIZE = (int)(2.0 * SAMPLE_RATE);
static constexpr int TEMPO_SYNC_STEPS = NinaParams::TempoSyncMultipliers::num_sync_tempos;

class BBDDelay {
  public:
    BBDDelay() {}

    ~BBDDelay() {}

    void mute() {
    }

    void setTime(float time) {
        _delay_time_setting = time;
    }

    void setTimeSync(float time) {
        _delay_time_setting_sync = time;
    }

    void setTempoSync(bool temposync) {

        _tempo_sync = temposync;
    }

    void setTempo(float tempo) {

        _tempo = tempo;
    }

    void setFB(float fb) {
        _feedback = 1.1 * fb;
    }

    void setFilter(float cf) {
        float cutoff = fastpow2(5 + cf * 9);
        float ang_cut = 2 * M_PI * cutoff;
        constexpr float samp_period = 1 / (float)SAMPLE_RATE;
        float a = (ang_cut * samp_period) / (1.f + ang_cut * samp_period);
        float b = 1.f - a;
        _filt_a0 = a;
        _filt_b1 = b;
    }

    /**
     * @brief zero the delay line values
     *
     */
    void reset() {
        _delay_buffer.fill(0);
    }

    void setLFO(float rate, float amount) {
        _lfo_rate = rate;
        _lfo_amount_set = amount;
        _lfo_phase_inc = 628.31853f * (float)fastpow2(3.4 * ((float)(3.0f * _lfo_rate - 2.0f))) / (float)SAMPLE_RATE;
    }

    void setLFOGain(float gain) {
        _lfo_amount_set = gain;
    }

    void setLevel(float level) {
        _gain = max_gain * level * level;
    }

    void setDelayMixAmount(float mix) {
        _wet_mix = mix;
    }

    int buf_counter = 0;

    void run(float *(input), float *output) {
        calc_time();
        buf_counter++;
        _lfo_amount = 100 * _lfo_amount_set * (0.2 + 0.2 * _time_setting);
        // update the delay line
        float loop_buffer = _loop_buffer;
        _loop_buffer += 0.01 * (_time - _loop_buffer - 0 * _lfo_amount - _lfo_amount * (float)std::sin(_lfo_phase));
        // find the size of the linear step for the loop increment
        float delay_loop_inc = (1.0 / (float)BUFFER_SIZE) * (_loop_buffer - loop_buffer);
        _gain_smooth = param_smooth(_gain, _gain_smooth);
        float wet = _wet_mix;

        _lfo_phase += _lfo_phase_inc;
        if (_lfo_phase > 2 * M_PI) {
            _lfo_phase -= 2 * M_PI;
        }
        for (int buf_i = 0; buf_i < BUFFER_SIZE; buf_i++) {
            // get input
            const float input_sample = input[buf_i] * _gain_smooth;
            float tmp = 0;
            // lin interp between points
            loop_buffer += delay_loop_inc;

            // delay positions
            _buffer_ptr--;
            if (_buffer_ptr < 0)
                _buffer_ptr = DELAY_SIZE;

            int l = std::floor(loop_buffer);

            // get remainder for lin interp
            tmp = loop_buffer - (float)l;

            l += _buffer_ptr;
            if (l > DELAY_SIZE)
                l -= (DELAY_SIZE + 1);

            // delay output
            float delay_out = _delay_buffer[l];

            // linear interp
            l++;
            if (l > DELAY_SIZE) {
                l = 0;
            }
            delay_out += tmp * (_delay_buffer[l] - delay_out);

            // dc blocking filter to stop dc components accumulating when feedback is greater than 1
            tmp = input_sample + _feedback * delay_out;
            tmp = tmp - _highpass_state;
            _highpass_state += highpass_gain * tmp;

            float input_filter = tmp;
            float filter_out = _filt_a0 * input_filter + _filt_b1 * _filt_Y1;
            tmp = filter_out;
            _filt_Y1 = filter_out;

            float log_input_level = _clipper_amp_dB * fastlog2(std::abs(tmp));
            if (log_input_level > _threshold_dB) {
                float over_dB = log_input_level - _threshold_dB;
                over_dB = _clipper_linear_coeff * over_dB + _clipper_squared_coeff * over_dB * over_dB;
                log_input_level = std::min<float>(_threshold_dB + over_dB, _limit_dB);
            }

            tmp = fastpow2(log_input_level / _clipper_amp_dB) * std::copysignf(1.0, tmp);
            // delay input
            _delay_buffer[_buffer_ptr] = tmp;

            // sum output into output buffer
            output[buf_i] += delay_out * wet;
        }
    }

    void calc_time() {
        float time = _delay_time_setting;

        if (_tempo_sync) {
            time = _delay_time_setting_sync;
            int time_multiplier_set = std::round((TEMPO_SYNC_STEPS) * (1 - time)) - 1;
            float time_multiplier = _tempo_settings.getTempoSyncMultiplier(time_multiplier_set);
            float time = 1.f / (_tempo * time_multiplier);
            float time_samples = time * SAMPLE_RATE;
            time_samples = time_samples > ((float)DELAY_SIZE * 0.8) ? ((float)DELAY_SIZE * 0.8f) : time_samples;
            time_samples = time_samples < 1 ? 1 : time_samples;
            _time = time_samples;
        } else {

            _time_setting = time;
            constexpr float time_offset = 0.18;
            constexpr float time_gain = 1;
            time = time_offset + (time_gain - time_offset) * time;
            _time = time * time * (float)DELAY_SIZE * 0.8;
        }
    }

  private:
    static constexpr float cutoff_frequency = 200.0;
    static constexpr float highpass_gain = cutoff_frequency / (float)(2.f * M_PI * SAMPLE_RATE);
    static constexpr float max_gain = 2.5;
    float _highpass_state = 0;
    std::array<float, DELAY_SIZE + 2> _delay_buffer;
    float _clipper_amp_dB = 8.6562;
    float _baseline_threshold_dB = -9.0;
    float _clipper_linear_coeff = 1.017;
    float _clipper_squared_coeff = -0.025;
    float _limit_dB = -6.9;
    float _threshold_dB = _baseline_threshold_dB + _limit_dB;

    float _filt_a0 = 0;
    float _filt_a1 = 0;
    float _filt_a2 = 0;
    float _filt_b1 = 0;
    float _filt_b2 = 0;
    float _filt_X1 = 0;
    float _filt_X2 = 0;
    float _filt_Y1 = 0;
    float _filt_Y2 = 0;
    float _time_setting = 0;
    float _time = 0.8;
    float _feedback = .5;
    float _feedback_tone = 0.5;
    float _lfo_amount = 0.1;
    float _lfo_amount_set = 0;
    float _lfo_rate = 0.5;
    float _lfo_phase_inc = 0;
    float _lfo_phase = 0;
    float _loop_buffer = 0;
    float _env = 0;
    float _rel = 0;
    float _gain = 0;
    float _gain_smooth = 0;
    float _wet_mix = 1;
    int _buffer_ptr = 0;
    bool _tempo_sync = false;
    float _tempo = 1.f;
    float _delay_time_setting = 0;
    float _delay_time_setting_sync = 0;
    NinaParams::TempoSyncMultipliers _tempo_settings;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg