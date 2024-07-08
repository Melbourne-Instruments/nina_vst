/**
 * @file NinaLfo.h
 * @brief Declaration of a simple LFO for nina
 * @date 2022-07-17
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */
#pragma once

#include "AnalogVoice.h"
#include "NinaParameters.h"
#include "NoiseOscillator.h"
#include "SynthMath.h"
#include "common.h"
#include <complex>
#include <fstream>
#include <sstream>

namespace Steinberg {
namespace Vst {
namespace Nina {

class BiquadFilter2 {
  public:
    BiquadFilter2(float a1, float a2, float b0, float b1, float b2) :
        _a1(a1), _a2(a2), _b0(b0), _b1(b1), _b2(b2) {
        // Reset the filer
        reset();
    };

    ~BiquadFilter2() {

        printf("\n");
    }

    inline void reset() {
        // Reset the filter
        _m1 = 0;
        _m2 = 0;
        _dn = 1e-20f;
    };

    inline float process(float input) {
        float w = input - (_a1 * _m1) - (_a2 * _m2) + _dn;
        float out = (_b1 * _m1) + (_b2 * _m2) + (_b0 * w);
        _m2 = _m1;
        _m1 = w;
        _dn = -_dn;
        return out;
    }

  private:
    const float _a1, _a2, _b0, _b1, _b2;
    float _m1, _m2;
    float _dn;
};

template <uint FC>
class AnalysisLowpassFilter {
  public:
    AnalysisLowpassFilter() :
        _biquad_1((_a1 / _a0), (_a2 / _a0), (_b0 / _a0), (_b1 / _a0), (_b2 / _a0)),
        _biquad_2((_a1 / _a0), (_a2 / _a0), (_b0 / _a0), (_b1 / _a0), (_b2 / _a0)),
        _biquad_3((_a1 / _a0), (_a2 / _a0), (_b0 / _a0), (_b1 / _a0), (_b2 / _a0)),
        _biquad_4((_a1 / _a0), (_a2 / _a0), (_b0 / _a0), (_b1 / _a0), (_b2 / _a0)){};

    inline void reset() {
        // Reset the biquad filters
        _biquad_1.reset();
        _biquad_2.reset();
        _biquad_3.reset();
        _biquad_4.reset();
    }

    ~AnalysisLowpassFilter() {
        printf("\n");
    }

    inline float process(float input) {
        // Process the biquad filters in sequence
        return _biquad_4.process(_biquad_3.process(_biquad_2.process(_biquad_1.process(input))));
    }

  private:
    static constexpr float _q = 0.70f;
    static constexpr float _w0 = (2 * M_PIf32 * FC) / SAMPLE_RATE;
    static constexpr float _alpha = std::sin(_w0) / (2 * _q);
    static constexpr float _b0 = (1 - std::cos(_w0)) / 2;
    static constexpr float _b1 = 1 - std::cos(_w0);
    static constexpr float _b2 = (1 - std::cos(_w0)) / 2;
    static constexpr float _a0 = 1 + _alpha;
    static constexpr float _a1 = -2 * std::cos(_w0);
    static constexpr float _a2 = 1 - _alpha;
    BiquadFilter2 _biquad_1;
    BiquadFilter2 _biquad_2;
    BiquadFilter2 _biquad_3;
    BiquadFilter2 _biquad_4;
};

class biquadBandpass {
  public:
    biquadBandpass(){};
    ~biquadBandpass(){};

    void reset() {

        _x1 = 0;
        _y1 = 0;
        _x2 = 0;
        _y2 = 0;
    };

    void setup(float freq, float q) {
        float dbGain = 0;
        float A = std::pow(10, dbGain / 40);
        float omega = 2 * M_PI * freq / _sample_rate;
        float sn = std::sin(omega);
        float cs = std::cos(omega);
        float alpha = sn / (2 * q);
        float beta = std::sqrt(A + A);
        _b0 = alpha;
        _b1 = 0;
        _b2 = -alpha;
        _a0 = 1 + alpha;
        _a1 = -2 * cs;
        _a2 = 1 - alpha;
        _b0 /= _a0;
        _b1 /= _a0;
        _b2 /= _a0;
        _a1 /= _a0;
        _a2 /= _a0;
    }

    float run(float x) {
        float y = _b0 * x + _b1 * _x1 + _b2 * _x2 - _a1 * _y1 - _a2 * _y2;
        _x2 = _x1;
        _x1 = x;
        _y2 = _y1;
        _y1 = y;
        return y;
    }

  private:
    const float _sample_rate = SAMPLE_RATE;
    float _a0, _a1, _a2, _b0, _b1, _b2;
    float _x1 = 0;
    float _x2 = 0;
    float _y1 = 0;
    float _y2 = 0;
};

class VoiceCalController {
  private:
    AnalogVoice &_voice;
    float _pitch_1 = 6. / noteGain;
    float _pitch_2 = 6. / noteGain;
    float _filter = 0;
    float _res = 0;
    float _mix_tri1 = 0;
    float _mix_sq1 = 0;
    float _mix_tri2 = 0;
    float _mix_sqr2 = 0;
    float _mix_xor = 0;
    float _mix_l = 0;
    float _mix_r = 0;
    float _mute = true;
    float _shape_1 = 0;
    float _shape_2 = 0;

  public:
    VoiceCalController(AnalogVoice &voice) :
        _voice(voice) {
    }

    ~VoiceCalController(){};

    void run() {
        auto &voice_input = _voice.getVoiceInputBuffers();
        voice_input.filt_cut.fill(_filter);
        voice_input.filt_res.fill(_res);
        voice_input.tri_1_lev.fill(_mix_tri1);
        voice_input.sqr_1_lev.fill(_mix_sq1);
        voice_input.tri_2_lev.fill(_mix_tri2);
        voice_input.sqr_2_lev.fill(_mix_sqr2);
        voice_input.xor_lev.fill(_mix_xor);
        voice_input.vca_l.fill(_mix_l);
        voice_input.vca_r.fill(_mix_r);
        voice_input.mute_1.fill(_mute);
        voice_input.mute_2.fill(_mute);
        voice_input.mute_3.fill(_mute);
        voice_input.mute_4.fill(_mute);
        voice_input.osc_1_pitch.fill(_pitch_1);
        voice_input.osc_2_pitch.fill(_pitch_2);
        voice_input.osc_1_shape.fill(_shape_1);
        voice_input.osc_2_shape.fill(_shape_2);
        voice_input.sub_osc.fill(false);
        voice_input.hard_sync.fill(false);
        voice_input.mix_mute_1.fill(false);
        _voice.setOutMuteL(false);
        _voice.setOutMuteR(false);
    };

    void setOff() {
        _filter = 1;
        _res = 1;
        _mix_l = 1;
        _mix_r = 1;
        _mix_tri1 = 0;
        _mix_tri2 = 0;
        _mix_sq1 = 0;
        _mix_sqr2 = 0;
        _mix_xor = 0;
        _mute = true;
        _pitch_1 = 6. / noteGain;
        _pitch_2 = 6. / noteGain;
        _shape_1 = 0;
        _shape_2 = 0;
    }

    void setFilterTuneTest(float cut) {
        _filter = cut;
        _res = 1;
        _mix_l = 1;
        _mix_r = 1;
        _mix_tri1 = 0;
        _mix_tri2 = 0;
        _mix_sq1 = 0;
        _mix_sqr2 = 0;
        _mix_xor = 0;
        _mute = false;
        _pitch_1 = 6. / noteGain;
        _pitch_2 = 6. / noteGain;
        _shape_1 = 0;
        _shape_2 = 0;
    }

    void setMainOutVca(float left, float right) {
        _filter = .6;
        _res = 0.1;
        _mix_l = left;
        _mix_r = right;
        _mix_tri1 = 0;
        _mix_tri2 = 0;
        _mix_sq1 = 0;
        _mix_sqr2 = 0;
        _mix_xor = 0.0;
        _mute = false;
        _pitch_1 = 6. / noteGain;
        _pitch_2 = 6. / noteGain;
        _shape_1 = 0;
        _shape_2 = 0;
    }

    void setMixVca(float tri1, float tri2, float sqr1, float sqr2, float xor_l, float shape_1, float shape_2) {
        _filter = .6;
        _res = 0.01;
        _mix_l = 1;
        _mix_r = 1;
        _mix_tri1 = tri1;
        _mix_tri2 = tri2;
        _mix_sq1 = sqr1;
        _mix_sqr2 = sqr2;
        _mix_xor = xor_l;
        _mute = false;
        _pitch_1 = 10. / noteGain;
        _pitch_2 = 10.9 / noteGain;
        _shape_1 = shape_1;
        _shape_2 = shape_2;
    };
};

class NinaCalSequencer {
  private:
    std::array<AnalogVoice, NUM_VOICES> &_analog_voices;
    std::array<VoiceCalController, NUM_VOICES> _controller = {_analog_voices[0], _analog_voices[1], _analog_voices[2], _analog_voices[3], _analog_voices[4], _analog_voices[5], _analog_voices[6], _analog_voices[7], _analog_voices[8], _analog_voices[9], _analog_voices[10], _analog_voices[11]};
    std::ofstream _out;
    std::ofstream _out_2;
    std::vector<float> _audio_data_out;
    std::vector<float> _stimulus_out;
    uint _call_counter = 0;
    std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> &_high_res_audio_outputs;
    NoiseOscillator _noise;
    AnalysisLowpassFilter<2100> _lowpass_l;
    AnalysisLowpassFilter<1900> _highpass_l;
    AnalysisLowpassFilter<2100> _lowpass_r;
    AnalysisLowpassFilter<2000> _highpass_r;
    AnalysisLowpassFilter<800> _highpass_mix;
    biquadBandpass _bandpass_filter;
    std::vector<float> _audio_dft_buffer;
    uint dft_size = 4906;
    uint N = 4096;
    uint dft_buf_ptr = 0;
    float _shape_1 = 0;
    float _shape_2 = 0;
    int _test_startup_counter = 0;

    void setupMainVcaCal(uint voicen) {
        std::stringstream file;
        file << "/udata/nina/tuning/"
             << "voice_" << voicen << "_main_vca.dat";
        _out.open(file.str(), std::ios::out | std::ios::trunc | std::ios::binary);
        for (uint i = 0; i < NUM_VOICES; i++) {
            _controller.at(i).setOff();
            _analog_voices.at(i).setAllocatedToVoice(true);
        }
        _highpass_l.reset();
        _lowpass_l.reset();
        _highpass_r.reset();
        _lowpass_r.reset();
    }

    void setupFilter(uint voicen) {
        std::stringstream file;
        file << "/udata/nina/tuning/"
             << "voice_" << voicen << "_filter.dat";
        _out.open(file.str(), std::ios::out | std::ios::trunc | std::ios::binary);
        file.str("");

        file << "/udata/nina/tuning/"
             << "voice_" << voicen << "_signal.dat";
        _out_2.open(file.str(), std::ios::out | std::ios::trunc | std::ios::binary);
        FilterCalValues filter_cal;
        filter_cal.fc_high_clip = 0.5f;
        filter_cal.fc_low_clip = -0.2f;
        filter_cal.fc_gain = (filter_cal.fc_high_clip - filter_cal.fc_low_clip) / 2;
        filter_cal.fc_offset = filter_cal.fc_low_clip + filter_cal.fc_gain;
        filter_cal.fc_temp_track = 0.0f;
        filter_cal.res_high_clip = -0.0f;
        filter_cal.res_zero_offset = 0.0f;
        filter_cal.res_gain = -.46f;
        filter_cal.res_low_clip = -.46f;
        for (uint i = 0; i < NUM_VOICES; i++) {
            _controller.at(i).setOff();
            _analog_voices.at(i).setAllocatedToVoice(true);
            _analog_voices.at(i).setFilterCal(filter_cal);
        }
    }

    void setupMixCal(uint voicen) {
        std::stringstream file;
        file << "voice_" << voicen << "_mix_vca_audio.dat";
        _out.open(file.str(), std::ios::out | std::ios::trunc | std::ios::binary);
        file.str("");
        file << "voice_" << voicen << "_mix_vca_signal.dat";
        _out_2.open(file.str(), std::ios::out | std::ios::trunc | std::ios::binary);
        for (uint i = 0; i < NUM_VOICES; i++) {
            _controller.at(i).setOff();
            _analog_voices.at(i).setAllocatedToVoice(true);
            _analog_voices.at(i).setOscTuningGain(0.3);
        }
        _highpass_mix.reset();
    };

    void setVoiceShape(float shape_1, float shape_2) {
        _shape_1 = shape_1;
        _shape_2 = shape_2;
    };

    void runMixVca(std::array<float, BUFFER_SIZE> *left, std::array<float, BUFFER_SIZE> *right);

    void runMainVca(std::array<float, BUFFER_SIZE> *left, std::array<float, BUFFER_SIZE> *right);

    void runFilter(std::array<float, BUFFER_SIZE> *left, std::array<float, BUFFER_SIZE> *right);

  public:
    NinaCalSequencer(std::array<AnalogVoice, NUM_VOICES> &analog_voices, std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> &high_res_audio_outputs) :
        _high_res_audio_outputs(high_res_audio_outputs),
        _analog_voices(analog_voices) {
        _audio_data_out.reserve(SAMPLE_RATE * 2);
        _stimulus_out.reserve(SAMPLE_RATE * 2);
        _noise.setVolume(0.9);
    };

    bool isTestRunning() {
        return !_test_complete;
    };

    ~NinaCalSequencer(){

    };

    /**
     * @brief Init the Mix vca cal. The mix vca cal works by adjusting the vca offset for each mix VCA to minimise the ouptut level into the filter. It measures this by running 1 voice looped back though the FX input and measuring a RMS type level. It iterates each VCA 1 at a time, running though all the VCA's several times to find the best offset values. When each voice is complete, we move onto the next next voice.
     *
     * Overall this method is very slow as there is no interpolation of the mesurements, so we step very slowely to the optimal values. Additionally the SNR of the mesurement is VERY low, so its hard to tell if you have the correct value.
     *
     * Redoing this method to minimise the abs level of FFT bins where each fundemental is expected would be dramatically faster.
     *
     * @param run
     */
    void startMixCal(bool run) {
        if (run) {
            printf("\n setup mix cal");
            _voice_counter = 0;
            _test_complete = false;
            _testing = MixVca;
            vca_start = -0.000;
            vca_state_l = vca_start;
            vca_state_r = vca_start;
            level_l = 0;
            level_r = 0;
            dc_l = 0;
            dc_r = 0;
            wo = false;
            step = .0005;
            buf_ave_cnt = 100;
            buf_ave_cnt_store = 100;
            step_l = step;
            step_r = step;
            old_level_l = 100;
            old_level_r = 100;
            old_delta_l = 0;
            old_delta_r = 0;
            buffer_count = 0;
            min = 1000;
            max = -1000;
            sample_counter = 0;
            output_sine_phase = 0;
            queue_i = 0;
            done_l = false;
            _highpass_l.reset();
            _lowpass_l.reset();
        }
    }

    /**
     * @brief Init the Main LR vca cal. This works in a similar way to the mix VCA cal. A tone is run though the voice digital output into the filer. the LR vca's are adjusted to minimise the RMS level at the output. Each voice is set one after the other
     *
     * @param run
     */
    void startMainVcaCal(bool run) {
        if (run) {
            printf("\n setup main vca cal");
            _testing = MainVca;
            _test_complete = false;
            vca_start = -0.001;
            vca_state_l = vca_start;
            vca_state_r = vca_start;
            level_l = 0;
            level_r = 0;
            dc_l = 0;
            dc_r = 0;
            wo = false;
            step = .0001;
            step_l = step;
            step_r = step;
            old_level_l = 100;
            old_level_r = 100;
            old_delta_l = 0;
            old_delta_r = 0;
            buffer_count = 0;
            min = 1000;
            max = -1000;
            sample_counter = 0;
            output_sine_phase = 0;
            queue_i = 0;
            done_l = false;
        }
    }

    /**
     * @brief The filter testing works by turning the filter resonance up, and saving audio samples of a series of test points. this data is analysed in a python script which calculates the res freq of each testpoint, calculating the filter cal values that best match. the Osc 'temp' reading is also saved as part of this data, so it can be used to set the relationship between the osc 'temp' and the filter cal. (this is because we open loop correct the temp of the filter, by using the temp of the osc as a stand in value)
     *
     * @param run
     */
    void startFilterCal(bool run) {
        if (run) {
            printf("\n setup filter testing");
            _test_complete = false;
            _testing = Filter;
            _call_counter = 0;
            done_l = false;
            done_r = false;
            buffer_count = 0;
            stim = 0.95;
            _audio_data_out.clear();
            _stimulus_out.clear();
            test_counter = 0;
        }
    };

    void runCal(uint voice_num);

    float stim = 0.95;
    bool _first_pass_l = false;
    bool _first_pass_r = false;
    float min_l = 1;
    float min_r = 1;
    float min_val_l, min_val_r;
    int l = 20;
    int counter = 1;
    int _test_counter = 0;
    int test_counter = 0;
    int test_counter_2 = 0;
    uint _voice_counter = 0;
    bool _test_complete = false;

    enum testRunning {
        NoTest = 0,
        MixVca,
        MainVca,
        Filter
    };

    testRunning _testing = NoTest;
    float vca_start = -0.001;
    float vca_state_l = vca_start;
    float vca_state_r = vca_start;
    float vca_output_l;
    float vca_output_r;
    float level_l = 0;
    float level_r = 0;
    float dc_l = 0;
    float dc_r = 0;
    bool wo = false;
    float step = .00005;
    float step_l = step;
    float step_r = step;
    int buf_ave_cnt = 1;
    int buf_ave_cnt_store = 1;
    float old_level_l = 100;
    float old_level_r = 100;
    float old_delta_l = 0;
    float old_delta_r = 0;
    uint buffer_count = 0;
    float min = 1000;
    float max = -1000;
    int sample_counter = 0;
    float output_sine_phase = 0;
    uint queue_i = 0;
    std::array<float, 20> queue_l;
    std::array<float, 20> queue_r;
    bool done_l = false;
    bool done_r = false;
    float _tri_0, _tri_1, _sqr_0, _sqr_1, _xor;
    uint _current_vca = 0;

    void run(std::array<float, BUFFER_SIZE> *left, std::array<float, BUFFER_SIZE> *right) {
        switch (_testing) {
        case NoTest:
            break;
        case MixVca:
            runMixVca(left, right);
            break;

        case MainVca:
            runMainVca(left, right);
            break;

        case Filter:
            runFilter(left, right);
            break;

        default:
            break;
        }
    };
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg