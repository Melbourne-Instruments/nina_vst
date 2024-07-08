/**
 * @file AnalogOscGen.cpp
 * @brief Implementation of the analog oscillator calibration class
 * @date 2022-07-01
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */

#include "AnalogOscGen.h"
#include <cmath>

#include <iostream>

namespace Steinberg {
namespace Vst {
namespace Nina {
// Constants

AnalogOscModel::AnalogOscModel(uint voice_num, uint osc_num) :
    _voice_num(voice_num), _osc_num(osc_num) {
    // Reset the data
    reset();
    loadCalibration();
}

AnalogOscModel::~AnalogOscModel() {
}

void AnalogOscModel::OscSynced(bool sync) { _osc_sync = sync; }

void AnalogOscModel::reset() {
}

int osccounter = 0;

void AnalogOscModel::tuningFeedback(float count_up, float count_down) {
    _sample_counter = 0;
    _feedback_time_up = count_up * count_scale;
    _feedback_time_down = count_down * count_scale;

    if (((_new_fb % 700) == 0) && _new_fb > 0) {
        // printf("\nnewfb: %d %d %d", _new_fb, _osc_state, _voice_num);
    }
    // check to see if the feedback measurmement is probably valid
    if (_osc_sync) {
        return;
    }
    if ((_feedback_time_down > thresh_fast) &&
        (_feedback_time_down < thresh_slow) &&
        (_feedback_time_up > thresh_fast) && (_feedback_time_up < thresh_slow)) {
        if (!_valid_feedback) {
            // printf("\n%d go to true %f %f", _voice_num, _feedback_time_up, _feedback_time_down);
        }
        _valid_feedback = true;
    } else {
        if (_valid_feedback) {
            //   printf("\n%d go to false", _voice_num);
        }

        _valid_feedback = false;
        _new_fb++;
        return;
    }

    // if the feedback value is new, then reset the counter. if the feedback value is the same for some timout, we could reset the oscillator
    if (((count_up == _old_fb_up) && (count_down == _old_fb_down)) || !_valid_feedback) {
        _new_fb++;
        if (_new_fb % 700 == 0) {
            // printf("\nnewfb: %d %d %d", _new_fb, _osc_state, _voice_num);
        }
    } else {
        _new_fb = 0;
    }
    _old_fb_down = count_down;
    _old_fb_up = count_up;

    // dont run the feedback if we arn't in one of the tracking states
    if ((_osc_state != FindSyncWait) && (_osc_state != FindSync) && (_osc_state != AOscState::TuningMeasure) && (_osc_state != TuningWait)) {
        calcFeedback();
    }
}

void AnalogOscModel::run(const std::array<float, CV_BUFFER_SIZE> &osc_pitch, const std::array<float, CV_BUFFER_SIZE> &osc_shape, std::array<float, BUFFER_SIZE> &out) {
    float voltage_up, voltage_down;
    float vup = -1;
    float vdown = -1;
    float track_up = -5;
    float track_down = -5;
    bool local_print = print;

    if (_debug) {
        // printf("state: %d", _osc_state);
        //_debug--;
    }
    switch (_osc_state) {

    case AOscState::Restart: {
        // set output to a high value to 'reset' the oscillator circuit
        _sync_counter--;
        if (_sync_counter > 50) {
            // printf("\n pulse low");

            for (int i = 0; i < BUFFER_SIZE; i += CV_MUX_INC) {
                out[i + mux_offset_up] = startup_lev;
                out[i + mux_offset_down] = startup_lev;
            }
        } else {
            //            printf("\n pulse high");
            for (int i = 0; i < BUFFER_SIZE; i += CV_MUX_INC) {
                out[i + mux_offset_up] = reset_lev;
                out[i + mux_offset_down] = reset_lev;
            }
        }

        // if the counter is zero, then start the find sync state with the oscillators set to the slowest value
        if (_sync_counter == 0) {
            if (_voice_num == 0) {
                // printf("\n finish reset goto findsync");
            }

            for (int i = 0; i < BUFFER_SIZE; i += CV_MUX_INC) {
                out[i + mux_offset_up] = startup_lev;
                out[i + mux_offset_down] = startup_lev;
            }
            _find_sync_up = startup_lev;
            _find_sync_down = startup_lev;
            _osc_state = FindSync;

            // make the next state wait for 0.1 seconds so it stablises
            _sync_counter = 0.1 * BUFFER_RATE;
        }
        break;
    }

    case AOscState::FindSync: {

        if (_sync_counter < 0) {

            // We start the oscillator at the lowest output signal and then incrementally increase the signal sent to each CV. when they are running above about 1kHz we then adjust the model to this frequency/temp and then start tracking.
            bool f_up_low = (1. / _feedback_time_up) < warm_up_freq;
            bool f_down_low = (1. / _feedback_time_down) < warm_up_freq;

            // if the feedback time is less than 5000hz (i.e. its probably zero) then its probably invalid and we should treat the frequency as still being too low
            if ((_feedback_time_up < 1 / 5000.)) {
                f_up_low = true;
            }
            if (_feedback_time_down < 1 / 5000.) {
                f_down_low = true;
            }

            // if we went hought the whole range then we should go back to reset
            if ((_find_sync_up > reset_lev) || (_find_sync_down > reset_lev)) {
                _osc_state = Restart;
                _sync_counter = 300;
                if (_voice_num == 0) {
                }
                return;
            }

            // increment the output voltage for each osc side that freq is stil too low
            if ((f_up_low || f_down_low) || !_valid_feedback) {
                if (f_up_low) {
                    _find_sync_up += 0.02f;
                    _sync_counter = .1f * BUFFER_RATE;
                }
                if (f_down_low) {
                    _find_sync_down += 0.02f;
                    _sync_counter = .1f * BUFFER_RATE;
                }
                // printf("\n osc %d %d %f %f %f %f %d %d", _voice_num, _osc_num, _find_sync_down, _find_sync_up, 1. / _feedback_time_down, 1. / _feedback_time_up, f_up_low, f_down_low);

            }

            else {
                if (!_valid_feedback) {

                    if (_voice_num == 0) {
                        // printf("\n   findsync invalid feedback reset");
                    }
                    _osc_state = Restart;
                    _sync_counter = 300;
                    return;
                }

                // oscillators are running, so we now adjust the model until we match the current operating conditions
                track_up = -2;
                track_down = -2;

                for (int i = 0; i < 1000; i++) {
                    const float l2_warmup = std::log2(warm_up_freq);
                    vup = calcVoltage(l2_warmup, l2_warmup, track_up, _osc_up);
                    constexpr float step_size = 0.03;
                    if (vup < _find_sync_up) {
                        track_up += step_size;
                    } else if (vup > _find_sync_up) {
                        track_up -= step_size;
                    }
                    if (std::abs(vup - _find_sync_up) < 0.02) {
                        break;
                    }
                }
                for (int i = 0; i < 1000; i++) {
                    const float l2_warmup = std::log2(warm_up_freq);
                    vdown = calcVoltage(l2_warmup, l2_warmup, track_down, _osc_down);
                    constexpr float step_size = 0.01;
                    if (vdown < _find_sync_down) {
                        track_down += step_size;
                    } else if (vdown > _find_sync_down) {
                        track_down -= step_size;
                    }
                    if (std::abs(vdown - _find_sync_down) < 0.02) {
                        break;
                    }
                }
                _track_osc_down = track_down;
                _track_osc_up = track_up;
                _find_sync_up = vup;
                _find_sync_down = vdown;

                // set the output to the value we found before

                // wait a long time in the next state
                // TODO: [NINA-543] shorten the time to wait in findsync wait
                _sync_counter = 0.5 * BUFFER_RATE;
                _osc_state = AOscState::FindSyncWait;
                // printf("\nvoice: %d state2\n", _voice_num);
                if (_voice_num == 0) {
                    // printf("\n   findsync wait  %f %f %f %f\n", vup, vdown, 1 / _feedback_time_up, 1 / _feedback_time_down);
                }
            }
        }
        _sync_counter--;

        for (int i = 0; i < BUFFER_SIZE; i += CV_MUX_INC) {
            out[i + mux_offset_up] = _find_sync_up;
            out[i + mux_offset_down] = _find_sync_down;
        }

        break;
    }

    case AOscState::FindSyncWait:

        // wait here for some time to let the oscillator start reporting feedback
        _sync_counter--;
        if (_sync_counter <= 0) {
            _osc_state = AOscState::WarmUp;
            _sync_counter = 1.0 * BUFFER_RATE;
            // save gain for after warmup is done
            _warmup_gain = 0.5;

            const float l2_warmup = std::log2(warm_up_freq);
            vup = calcVoltage(l2_warmup, l2_warmup, _track_osc_up, _osc_up);
            // wait in the warm up state for some time to let the tracking correct the oscillator freq
            _sync_counter = (int)(1.5 * BUFFER_RATE);
        }
        for (int i = 0; i < BUFFER_SIZE; i += CV_MUX_INC) {
            out[i + mux_offset_up] = _find_sync_up;
            out[i + mux_offset_down] = _find_sync_down;
        }

        break;

    case AOscState::WarmUp: {

        _error_gain = warm_up_gain2;
        _sync_counter--;
        _freq_osc_up = warm_up_freq;
        _freq_osc_down = warm_up_freq;

        // tracking will cause the oscillator to find the warm up frequency after some time
        const float l2_warmup = std::log2(warm_up_freq);
        vup = calcVoltage(l2_warmup, l2_warmup, _track_osc_up, _osc_up);
        vdown = calcVoltage(l2_warmup, l2_warmup, _track_osc_down, _osc_down);

        if (_sync_counter <= 0) {
            _error_gain = normal_gain;
            _osc_state = Normal;
            _osc_shape_mod = 0;
        }

        for (int i = 0; i < BUFFER_SIZE; i += CV_MUX_INC) {
            out[i + mux_offset_up] = vup;
            out[i + mux_offset_down] = vdown;
        }
        _ave_prev_freq_up = l2_warmup;
        _ave_prev_freq_down = l2_warmup;

        break;
    }
    case AOscState::Normal: {

        // if the tuning button has been pressed then go to the tuning state
        if (_tuning) {
            _osc_state = AOscState::TuningMeasure;
            // printf("tune");
            _error_gain = 0;
        }

        // TODO: [NINA-256] remove .at notation to increase performance
        // could also change how all these arrays are accessed? does it make any difference?
        auto it_f = osc_pitch.begin();
        auto it_sh = osc_shape.begin();

        float sum_up = 0;
        float sum_down = 0;
        int i = 0;

        while (it_f != osc_pitch.end()) {
            // TODO: [NINA-264] this loop currently doesn't seem to unroll. pragma unroll doesn't help
            constexpr float num = 3;
            auto &v_up = out[i + mux_offset_up];
            auto &v_down = out[i + mux_offset_down];
            float freq_up;
            float freq_down;
            generateOscSignals((*it_f) * noteGain, *it_sh, v_up, v_down, freq_up, freq_down);
            if (print) {
                printf("\n a osc: %f", *it_f);
                print = false;
            }
            float delta1 = (_prev_pitch_up - freq_up);
            float delta2 = (_prev_pitch_down - freq_down);
            _osc_f_delta += std::abs(delta1) + std::abs(delta2);
            _prev_pitch_up = freq_up;
            _prev_pitch_down = freq_down;
            sum_up += freq_up;
            sum_down += freq_down;
            i += CV_MUX_INC;
            it_f++;
            it_sh++;
        }
        // we use a log sum of the frequency going to each oscillator for the feedback calculation. this should give us a more accurate representation of the desired frequency.
        _ave_prev_freq_down = sum_down / ((float)CV_BUFFER_SIZE);
        _ave_prev_freq_up = sum_up / ((float)CV_BUFFER_SIZE);

        break;
    }

    case AOscState::TuningMeasure: {

        // if tuning is disabled then go back to the normal state
        if (_tuning == false) {
            _osc_state = AOscState::Normal;
            _error_gain = normal_gain;
            // printf("tune -> normal");
        } else if (_sync_counter-- <= 0) {
            // we are tuning, so then generate a tuning sequence to perform
            _osc_state = TuningWait;
            generate_tune_steps();
            _sync_counter = 0;
            _tuning_counter = -1;
        }
        break;
    }
    case AOscState::TuningWait: {

        // if we have recorded all the mesurements then go back to the tuningmeasure state
        if (_tuning_counter >= mes_size2) {
            // if the old file hasn't been written yet, then we break to wait for the next loop
            calWrite();
            _osc_state = TuningMeasure;
            _sync_counter = (int)(0.1 * BUFFER_RATE);
            break;
        }
        if (_sample_counter == 0 && (_sync_counter < delay)) {

            tuning_samples_up.at(_tuning_sample_counter) = _feedback_time_up;
            tuning_samples_down.at(_tuning_sample_counter) = _feedback_time_down;
            _tuning_sample_counter++;
            if (_tuning_sample_counter >= max_tuning_samples) {
                _tuning_sample_counter = max_tuning_samples - 1;
            }
        }

        // if the counter has expired, then we record the current value and apply the next step
        if (_sync_counter <= 0) {

            // we start with _tuning counter at -1, so we dont write the output the first time we are here
            if (_tuning_counter >= 0) {
                auto *start_up = tuning_samples_up.begin();
                auto *start_down = tuning_samples_down.begin();
                float sum1 = 0;
                float sum2 = 0;
                int i = 0;
                while (i < _tuning_sample_counter) {
                    sum1 += *(start_up);
                    sum2 += *(start_down);
                    *start_up++ = 0;
                    *start_down++ = 0;
                    i++;
                }

                sum1 = sum1 / (float)_tuning_sample_counter;
                sum2 = sum2 / (float)_tuning_sample_counter;

                mes_results.at(_tuning_counter * 4) = out.at(mux_offset_up);
                mes_results.at(_tuning_counter * 4 + 1) = out.at(mux_offset_down);
                mes_results.at(_tuning_counter * 4 + 2) = sum1;
                mes_results.at(_tuning_counter * 4 + 3) = sum2;
                //_output_signal[0] = calcVoltageUp(warm_up_freq, warm_up_freq, _track_osc_up);
                //_output_signal[1] = calcVoltageDown(warm_up_freq, warm_up_freq, _track_osc_down);
            }

            // if we are on the last step then we dont apply the next step so we can write the value next time the _sync_counter expires
            if (_tuning_counter < mes_size2 - 1) {

                float freq = mes_seq.at(((_tuning_counter + 1) * 2));
                freq = std::log2(freq);
                float shape = mes_seq.at(((_tuning_counter + 1) * 2 + 1));
                float vup, vdown;
                float freq_up, freq_down;
                generateOscSignals(freq, shape, vup, vdown, freq_up, freq_down);
                for (int i = 0; i < BUFFER_SIZE; i += CV_MUX_INC) {
                    out[i + mux_offset_up] = vup;
                    out[i + mux_offset_down] = vdown;
                }
                _tuning_sample_counter = 0;
                float mes_time = 1 / _freq_osc_up + 1 / _freq_osc_down;
                _sync_counter = 0 * BUFFER_RATE + 20 * ((int)(BUFFER_RATE * mes_time)) + delay;
            }
            _tuning_counter++;
        }
        _sync_counter--;
        break;
    }

    default: {
    }
    }

    constexpr int fb_timout_thresh = (int)(10 * ((float)CV_SAMPLE_RATE) / 8.0);
    if ((_new_fb > fb_timout_thresh)) {
        _osc_state = AOscState::FindSync;
        _new_fb = 0;
        _sync_counter = 300;
        // printf("\nvoice: %d reset", _voice_num);
    }

    if (osc_disable) {
        const float lev = startup_lev;
        for (int i = 0; i < BUFFER_SIZE; i += CV_MUX_INC) {
            out[i + mux_offset_up] = startup_lev;
            out[i + mux_offset_down] = startup_lev;
        }
    }
}

void AnalogOscModel::voice_allocated() {
    _voice_allocated = true;
}

void AnalogOscModel::voice_deallocated() {
    _voice_allocated = false;
}

void AnalogOscModel::loadCalibration() {

    // Load the calibration from a file in the tuning folder
    std::stringstream fpath;
    std::fstream file_handler;
    fpath << path << "voice_" << _voice_num << "_osc_" << _osc_num << ".model";
    file_handler.open(fpath.str(), std::ios::in);
    // std::cout << fpath.str();
    if (file_handler.is_open()) {
        std::string line;
        std::getline(file_handler, line);
        std::stringstream iss(line);
        std::array<float, 10> model;
        for (int i = 0; i < 10; i++) {
            iss >> model[i];
        }
        _osc_up.a = model[0];
        _osc_up.b = model[1];
        _osc_up.c = model[2];
        _osc_up.d = model[3];
        _osc_up.e = model[4];
        _osc_up.f = model[5];
        _osc_up.g = model[6];
        _osc_up.h = model[7];

        std::getline(file_handler, line);

        std::istringstream iss2(line);
        for (int i = 0; i < 10; i++) {
            iss2 >> model[i];
        }
        _osc_down.a = model[0];
        _osc_down.b = model[1];
        _osc_down.c = model[2];
        _osc_down.d = model[3];
        _osc_down.e = model[4];
        _osc_down.f = model[5];
        _osc_down.g = model[6];
        _osc_down.h = model[7];

        for (int i = 0; i < 10; i++) {
            // printf("\nmodel num %d\t%e", i, model[i]);
        }

        file_handler.close();
    } else {
        printf("osc load fail %d\n", _voice_num);
    }
}

void AnalogOscModel::calWrite() {
    std::stringstream fpath;
    std::fstream file_handler;
    fpath << path_data << "voice_" << _voice_num << "_osc_" << _osc_num << ".txt";

    file_handler.open(fpath.str(), std::ios::out | std::ios::app);
    std::cout << fpath.str();
    if (file_handler.is_open()) {
        for (int i = 0; i < mes_size2; i++) {

            file_handler << mes_results.at(i * 4) << ", " << mes_results.at(i * 4 + 1) << ", " << mes_results.at(i * 4 + 2) << ", " << mes_results.at(i * 4 + 3) << std::endl;
        }
        file_handler << std::endl;
        printf("\nwritedone:%d:%d", _voice_num, _osc_num);
        file_handler.close();
    } else {
        printf("\nIO error\n");
    }
}

const std::array<float, 2 * mes_size2> AnalogOscModel::generate_tune_steps() {
    constexpr int num_shape = 11;
    constexpr std::array<float, num_shape> shapes = {.999f, 0.55f, 0.9f, 0.85, 0.65f, -.15f, -.70f, -.90f, -.99, -.55, -.9999f};

    float mul = 5.0f;

    // generate n steps of a frequency and a shape at random. we generate random steps to try to avoid a bias in the measurements
    for (int i = 0; i < mes_size2 / 2; i++) {
        std::array<float, mes_size2> rand_f;
        float rand_float = ((float)std::rand() / (float)(RAND_MAX));
        rand_f.at(0) = std::exp2f(12 * (0.4 * rand_float + 0.3)) + 20;
        rand_float = ((float)std::rand() / (float)(RAND_MAX));
        rand_f.at(1) = std::exp2f(12 * (0.7 + 0.4 * rand_float));
        int r = std::rand() % num_shape;
        int r2 = std::rand() % num_shape;
        int high = std::rand() % 2;
        mes_seq[i * 4] = rand_f[high];
        mes_seq[i * 4 + 1] = shapes[r];
        mes_seq[i * 4 + 2] = rand_f[1 - high];
        mes_seq[i * 4 + 3] = shapes[r2];
        printf("\n%d %d f1 %f f2 %f s1 %f s2 %f %d %d %f ", _voice_num, _osc_num, rand_f[high], rand_f[1 - high], shapes[r], shapes[r2], r, r2, _error_gain);
    }
    return mes_seq;
}

inline float AnalogOscModel::calcVoltage(const float f1, const float f2,
    const float t, const AnalogModel &o) {

    const float hz_f1 = std::exp2f((float)f1); // exp2f32(f1);
    const float hz_f2 = std::exp2f((float)f2);
    const float hz_f1_2 = f1 * f1;
    const float hz_f1_3 = hz_f1_2 * f1;
    const float hz_f1_4 = hz_f1_3 * f1;

    // const float hz_f2_2 = hz_f2 * hz_f2;
    // const float hz_f2_3 = hz_f2_2 * hz_f2;
    // const float hz_f2_4 = hz_f2_3 * hz_f2;

    float val = o.e * hz_f2 + t + (o.a * t + o.b) * f1 + o.f * f2 + o.d * hz_f1 + o.h * hz_f1_2 + o.g * hz_f1_3 + 0 * o.i * hz_f1_4 + 0.001 / (hz_f1 + o.j);
    return (float)val;
}

void AnalogOscModel::runTuning() { _tuning = true; }

void AnalogOscModel::setTuningGain(float gain) {
    _error_gain = gain * 20;
}

void AnalogOscModel::stopTuning() {
    _tuning = false;
}

int osc_counter = 0;

// 2nd order decay filter counter set value
constexpr int buf_decay_delay_amount = 12000;

void AnalogOscModel::calcFeedback() {
    _osc_f_delta > 1 ? 1 : _osc_f_delta;
    float delayed_decay = (((float)buf_decay_delay_amount - (float)_osc_decay_counter)) / (float)buf_decay_delay_amount;
    float dec = 1.0f - delta_decay * delayed_decay;
    _osc_f_delta *= dec;

    // reset the delta if the oscillator is not in normal mode
    if (_osc_state != Normal) {
        _osc_f_delta = 0;
    }

    // the osc decay counter will 'hold' the error high for longer on large movements of pitch. this helps pitch stability
    if (_osc_decay_counter > 0) {
        _osc_decay_counter--;
    }
    if (_osc_f_delta > delta_limit) {
        if (trackon) {
            trackon = false;
            _osc_decay_counter = buf_decay_delay_amount;
        }
        return;
    }

    // if the delta limit is not currently exceeded, then we run the osc freq tracking
    if (_osc_f_delta < delta_limit) {
        trackon = true;

        float ave_up = _ave_prev_freq_up;
        float ave_down = _ave_prev_freq_down;

        // calculate the log error of the oscillator, log avoids the error getting larger as the frequency gets higher
        float error_up = ave_up - std::log2f(1 / (_feedback_time_up));
        float error_down = ave_down - std::log2f(1 / (_feedback_time_down));

        // We need to clip the error so that numerical errors arn't propagated to the feedback (can cause the osc to lock) and also to limit how quickly the osc can track.
        error_up = error_up < error_limit ? error_up : error_limit;
        error_up = error_up > -error_limit ? error_up : -error_limit;
        error_down = error_down < error_limit ? error_down : error_limit;
        error_down = error_down > -error_limit ? error_down : -error_limit;

        // consider decreasing the gain at high shapes since the tuning is less stable at high shapes. currently this is disabled.
        float gain_up = _shape_old;
        float gain_down = 1.0f - _shape_old;
        gain_up = 1;
        gain_down = 1.;

        // integrate the error for each oscillator
        _track_osc_up += error_up * _error_gain * gain_up * 0.0001f;
        _track_osc_down += (error_down * _error_gain * gain_down * 0.0001f);
    }
}

void AnalogOscModel::generateOscSignals(const float &freq, const float &shape_in, float &v_up, float &v_down, float &freq_up, float &freq_down) {

    float shape = shape_in;
    constexpr float shape_m = 3;

    float shape_clip = fastpow2(freq - max_osc_freq_l2);
    if (shape > 0) {
        shape = (8.f / 7.f) * (1 - ((fastpow2((1 - shape) * shape_m) / 8.f)));
        shape = shape / 2 + 0.5;
        shape = (shape + shape_clip) > 1.f ? 1 - shape_clip : shape;

    } else {

        shape = (8.f / 7.f) * (1 - ((fastpow2((1 + shape) * shape_m) / 8.f)));
        shape = -shape;
        shape = shape / 2 + 0.5;
        shape = (shape < shape_clip) ? shape_clip : shape;
    }
    _shape_old = shape;
    const float shape_up = -fastlog2(shape);
    const float shape_down = -fastlog2(1 - shape);
    constexpr float freq_clip = std::log2f(9000.f);
    constexpr float freq_low_clip = std::log2f(20.f);
    float freq_c = freq > freq_clip ? freq_clip : freq;
    freq_c = freq < freq_low_clip ? freq_low_clip : freq;
    freq_up = freq_c + shape_up;
    freq_down = freq + shape_down;
    v_up = calcVoltage(freq_up, freq_down, _track_osc_up, _osc_up);
    v_down = calcVoltage(freq_down, freq_up, _track_osc_down, _osc_down);
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
