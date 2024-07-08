/**
 * @file AnalogOscGen.h
 * @brief Definition of the analog oscillator calibration class
 * @date 2022-06-30
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */
#pragma once

#include "SynthMath.h"
#include "common.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

namespace Steinberg {
namespace Vst {
namespace Nina {

/**
 * @brief Enum to define the states that the oscillator model can be ing
 *
 */
enum AOscState {
    /**
     * @brief oscillator is running correctly
     *
     */
    Normal,

    /**
     * @brief use this state to reset the osc when it is 'stuck', not running, or state is unclear
     *
     */
    Restart,

    /**
     * @brief this state increases the osc signal until it starts running at a moderate frequency ex. 500hz
     *
     */
    FindSync,

    /**
     * @brief this state adjusts the model until it is synced with the current osc voltage level. we can then start tracking the osc
     *
     */
    FindSyncWait,

    /**
     * @brief start tracking the osc at a fixed frequency. when we exit this state, hopefully the osc is tracked to a small error and we can start using the osc
     *
     */
    WarmUp,

    /**
     * @brief begin the tuning proceedure
     *
     */
    TuningMeasure,

    /**
     * @brief takes a tuning sample after stablising
     *
     */
    TuningWait
};

class OscErrorIir2 {
  public:
    float run(float input) {
        float mid_sum = input * b0 + x1 * b1 + x2 * b2;
        float output = mid_sum * a0 + y1 * a1 + y2 * a2;
        x2 = x1;
        x1 = input;
        return output;
    }

    void zero_ir() {
        x1 = 0.0f;
        x2 = 0.0f;
        y1 = 0.0f;
        y2 = 0.0f;
    }

    void set_ir(float setpoint) {
        x1 = 0.0f;
        x2 = 0.0f;
        y1 = setpoint;
        y2 = setpoint;
    }

  private:
    // 0.3hz Fc
    float dummy = 0.0f;
    float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;
    float a0 = 1.000000000000000000f;
    float a1 = -1.999777855854005360f;
    float a2 = 0.999777880525276053f;
    float b0 = 0.000000006167817666f;
    float b1 = 0.000000012335635331f;
    float b2 = 0.000000006167817666f;
};

/**
 * @brief number of measurements in a single sample
 *
 */
static constexpr int mes_size2 = 2;

struct AnalogModel {
    float a = -1.9e-2f;
    float b = 5.2e-2f;
    float c = -9.6e-8f;
    float d = 1e-7f;
    float e = -4.4e-9f;
    float f = -2.5e-5f;
    float g = -9.2e-16f;
    float h = 3.3e-11f;
    float i = 1.1e-20f;
    float j = 0.0f;
};

class AnalogOscModel {

  public:
    AnalogOscModel(uint voice_num, uint osc_num);
    ~AnalogOscModel();

    void reset();
    void tuningFeedback(float count_up, float count_down);

    /**
     * @brief runs the osc calibration in place on the two input signals
     *
     * @param input_1 input array of OSC freq signals, returns signal for osc up
     * @param input_2 input array for OSC shape signals. returns signal for osc down
     */
    void run(const std::array<float, CV_BUFFER_SIZE> &input_1, const std::array<float, CV_BUFFER_SIZE> &input_2, std::array<float, BUFFER_SIZE> &out);
    void calcFeedback();
    void resetCounter();
    void setTuningGain(float gain);

    void debugPrinting() {
        print = true;
    }

    const std::array<float, static_cast<int>(mes_size2 * 2)> generate_tune_steps();
    void write_to_file();

    std::tuple<float, float> queryOscModel() {
        float val_1 = calcVoltage(std::log2(500.f), std::log2(500.f), _track_osc_up, _osc_up);
        float val_2 = calcVoltage(std::log2(500.f), std::log2(500.f), _track_osc_down, _osc_down);
        return {val_1, val_2};
    }

    void runTuning();
    void stopTuning();
    void voice_allocated();
    void voice_deallocated();

    bool isNormal() {
        return _osc_state == Normal;
    }

    void OscSynced(bool sync);

    void setUnitTestMode() {
        _osc_state = Normal;
        _track_osc_up = -0.3;
        _track_osc_down = -0.3;
        _error_gain = 0;
    }

    void dump() {
        _dump = true;
    }

    void calWrite();
    void generateOscSignals(const float &freq, const float &shape, float &v_up, float &v_down, float &freq_up, float &freq_down);

    /**
     * @brief run this with a thread to save tuning data
     *
     */
    void fileThread();
    void loadCalibration();

    bool enable_logging = false;
    // Logger * logger = 0;

    bool osc_disable = false;
    bool print = false;

  private:
    // delta error decays to 50% in delta decay time seconds
    static constexpr float normal_gain = 1;
    static constexpr float delta_decay_time = 0.01;
    static constexpr float delta_decay = std::pow(0.5, 1. / (delta_decay_time * (float)BUFFER_RATE));
    static constexpr float delta_limit = 00.3f;
    static constexpr float error_limit = 1.f;
    static constexpr int tune_int_dly = BUFFER_SIZE;
    /**
     * @brief this is the max frequency we allow the osc to run at
     *
     */
    static constexpr float max_osc_freq_l2 = std::log2(30000);
    /**
     * @brief log2(10) is the min freq we allow the osc to run at
     *
     */
    static constexpr float min_osc_freq2 = 3.3f;
    static constexpr float warm_up_gain2 = 15.0f;
    bool _dump = false;
    int _debug = 0;
    int64 sample_cnt = 0;
    float _glide_rate = 1.0f;
    float _glide_inc = 0.0f;
    float _old_pitch = 10.0f;
    int _osc_decay_counter = 0;
    float _pitch_target = 10.0f;
    bool _gliding = false;
    float _old_error = 0.0f;
    float _shape_old = 0.5f;
    float _osc_f_delta = 0.0f;
    float _prev_pitch_up = 10.0f;
    float _prev_pitch_down = 10.0f;
    float _cents_tune = 0.0f;
    float _shape_smooth = 0.0f;
    float _warmup_gain = 0.0f;
    std::string path = "/udata/nina/calibration/";
    std::string path_data = "/udata/nina/tuning/";
    static constexpr int max_tuning_samples = (int)(0.5 * CV_SAMPLE_RATE / CV_BUFFER_SIZE);
    std::array<float, max_tuning_samples> tuning_samples_up;
    std::array<float, max_tuning_samples> tuning_samples_down;
    int _tuning_sample_counter = 0;
    uint _voice_num;
    uint _osc_num;
    bool _thread_quit = false;
    bool _write_to_file = false;
    bool _write_done = false;
    float _freq_osc_up = 20.0f;
    std::thread _file_writer;
    float _freq_osc_down = 20.0f;
    float _track_osc_up = -3.0f;
    float _track_osc_down = -3.0f;
    float _old_fb_up = 0.0f;
    float _old_fb_down = 0.0f;
    int _new_fb = 0;
    bool _track_sync = false;
    bool _osc_sync = false;
    float _osc_freq_mod_exp = 0.0f;
    float _osc_shape_mod = 0.0f;
    float _osc_pitch_offset = 0.0f;
    AnalogModel _osc_up, _osc_down;

    int _tuning_counter = 0;
    int error_counter = 0;
    static const int delay = 0.2 * BUFFER_RATE;
    std::array<float, 2 * mes_size2> mes_seq;
    std::array<float, 4 * mes_size2> mes_results;
    AOscState _osc_state = Restart;
    int _sync_counter = 150;
    bool _valid_feedback = false;
    float _feedback_time_up = 0.0f;
    float _feedback_time_down = 0.0f;
    bool _voice_allocated = false;

    delayLineBuffer<BUFFER_SIZE + tune_int_dly,
        BUFFER_SIZE + tune_int_dly / 2>
        _up_out_freq;
    delayLineBuffer<BUFFER_SIZE + tune_int_dly,
        BUFFER_SIZE + tune_int_dly / 2>
        _down_out_freq;
    OscErrorIir2 filter_up;
    OscErrorIir2 filter_down;
    float _error_gain = 0.1f;
    static const int _feedback_sample_delay = 32;
    float _ave_prev_freq_up = 0.0f;
    float _ave_prev_freq_down = 0.0f;
    uint _sample_counter = 0;
    bool _tuning = false;

    // new stuff for new class
    /**
     * @brief output signal which can 'reset' the analog oscillators if they are stuck and not running
     *
     */
    static constexpr float reset_lev = 0.9999f;

    /**
     * @brief when starting oscs we begin at this level to make sure the startup is safe and the osc's dont get stuck
     *
     */
    static constexpr float startup_lev = -0.95f;
    static constexpr float shape_clamp_min = 0.0001f;
    static constexpr float shape_clamp_max = 1.0f - shape_clamp_min;
    static constexpr float count_scale = (float)(2 << 22) / (73.75e6f / 2.0f); //
    static constexpr float thresh_fast = 1.0f / 100000.f;
    static constexpr float thresh_slow = 1.0f / 5.f;

    static constexpr float warm_up_freq = (100.0f);
    float calcVoltage(const float freq, const float freq_2, const float track_offset, const AnalogModel &osc);
    float _find_sync_up, _find_sync_down;
    float error_old = 0.0f;
    float error_old2 = 0.0f;
    bool trackon = false;
    const uint mux_offset_up = _osc_num * 2U;
    const uint mux_offset_down = _osc_num * 2U + 1U;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
