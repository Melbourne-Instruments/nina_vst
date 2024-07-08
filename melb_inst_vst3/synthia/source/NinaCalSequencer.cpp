#include "NinaCalSequencer.h"
#include <cmath>

namespace Steinberg {
namespace Vst {
namespace Nina {
bool printed = false;

void NinaCalSequencer::runMixVca(std::array<float, BUFFER_SIZE> *left, std::array<float, BUFFER_SIZE> *right) {

    if (_test_startup_counter++ < 10 * 700) {
        if (_test_startup_counter == 1) {
            _analog_voices.at(_voice_counter).setMixVcaOffset(0, 0, 0, 0, 0);
            setupMixCal(_voice_counter);
        }

        // init queue values to a large number so we can reliabely find the min
        for (auto &controller : _controller) {
            controller.run();
        }
        return;
    }
    // all voices mix cal complete
    if (_voice_counter == 12) {
        if (!_test_complete) {
            _test_complete = true;
            printf("\n save all cal");
            for (auto &voice : _analog_voices) {
                voice.saveCalibration();
            }
        }
        return;
    }
    // no. vcas in each voice
    constexpr int num_vcas = 5;

    // no. of times to iterate the mix vca minimisation
    constexpr int num_loops = 3;

    // for each loop of the vca cal, reduce the step size, and lengthen the time to measure the RMS level
    float sign = step_l > 0 ? 1.0 : -1.0;
    if (_test_counter == (num_vcas * (num_loops - 1))) {
        step_l = step * 0.15 * sign;
        buf_ave_cnt = buf_ave_cnt_store * 2;
    } else if (_test_counter == (num_vcas * (num_loops - 2))) {
        step_l = step * 0.25 * sign;
        buf_ave_cnt = buf_ave_cnt_store * 2;
    }

    // voice is done. move onto the next voice
    if (_test_counter == num_vcas * num_loops + 1) {
        printf("\n voice %d mix cal done", _voice_counter);
        _analog_voices.at(_voice_counter).setMixVcaOffset(_tri_0, _tri_1, _sqr_0, _sqr_1, _xor);
        printf("\n vals %f %f %f %f %f", _tri_0, _tri_1, _sqr_0, _sqr_1, _xor);

        // reset and start the next test
        buffer_count = 0;
        _test_counter = 0;
        _voice_counter++;
        _tri_0 = 0, _tri_1 = 0, _sqr_0 = 0, _sqr_1 = 0, _xor = 0;
        step_l = step;
        buf_ave_cnt = buf_ave_cnt_store;
        _current_vca = 0;
        test_counter_2 = 0;
        return;
    }

    if (buffer_count == 0) {
        _analog_voices.at(_voice_counter).setMixVcaOffset(0, 0, 0, 0, 0);
        setupMixCal(_voice_counter);

        // init queue values to a large number so we can reliabely find the min
        for (auto &val : queue_l) {
            val = 1;
        }
        _bandpass_filter.reset();
        queue_i = 0;
        old_delta_l = 100;
        vca_state_l = 0;
    }

    // run the rms calculator for the buffer
    for (int i = 0; i < BUFFER_SIZE; i++) {
        _high_res_audio_outputs.at(_voice_counter)->at(i) = 0;
        float lin = left->at(i);
        max = lin > max ? lin : max;
        min = lin < min ? lin : min;
        float l = lin - _highpass_mix.process(lin);
        level_l += l * l;
    }

    // set the VCA levels for this buffer
    _controller.at(_voice_counter).setMixVca(_tri_0, _tri_1, _sqr_0, _sqr_1, _xor, _shape_1, _shape_2);
    float *sel_vca = &_xor;
    float q = 0.1;
    constexpr float f_osc_1 = 2049.0 / 2;
    constexpr float f_osc_2 = 2896.0 / 2;
    _shape_1 = 0;
    _shape_2 = 0;
    if (_current_vca == 1) {
        sel_vca = &_tri_1;
        _shape_1 = 0;
        _shape_2 = 1;
    }
    if (_current_vca == 2) {
        sel_vca = &_sqr_0;
        _shape_1 = -.2;
        _shape_2 = 1;
    }
    if (_current_vca == 3) {
        sel_vca = &_sqr_1;
        _shape_1 = 1;
        _shape_2 = 0.2;
    }
    if (_current_vca == 4) {
        sel_vca = &_tri_0;
        _shape_1 = 1;
        _shape_2 = 0;
    }

    for (auto &controller : _controller) {
        controller.run();
    }

    // if the buffer counter is a multiple of the RMS average length, we remesure the rms level and check if the value is minimised
    if ((buffer_count % buf_ave_cnt) == 0) {
        test_counter_2++;
        if (test_counter_2 > 0) {
            level_l = std::sqrt(level_l);
            old_delta_l = -old_level_l + level_l;
            printf("\n levels: %d %f %f %f %f ", _current_vca, *sel_vca, level_l, step_l, old_delta_l);

            queue_l.at(queue_i) = *sel_vca;
            queue_i += 1;
            queue_i = queue_i == 10 ? 0 : queue_i;
            float oldest_val_l = queue_l.at(queue_i);
            float oldest_val_r = queue_r.at(queue_i);
            printf("\ncheck %f %f", oldest_val_l, *sel_vca);
            if (abs(oldest_val_l - *sel_vca) < abs(2 * step_l + 0.0000001)) {
                float sum = 0;
                printf("\n values: ");
                for (uint i = 5; i < 10; i++) {
                    sum += queue_l.at(i);
                    printf("cal  %f ", queue_l.at(i));
                }
                sum = sum / 5.;
                *sel_vca = sum;
                printf("\nfinal val L: %f %d", sum, _test_counter);
                done_l = true;
            }

            old_level_l = level_l;
            test_counter_2 = 0;
            if (old_delta_l > 0) {
                step_l = -step_l;
            }
            *sel_vca += step_l;
            level_l = 0;
        }
        level_l = 0;
    }

    if (done_l) {
        test_counter_2 = 0;
        _test_counter++;
        done_l = false;
        buffer_count = -1;

        // cal the next mix vca
        _current_vca++;

        if (_current_vca == 5) {
            _current_vca = 0;
        }

        printf("\n swap vca %d %d", test_counter, _current_vca);
    }
    buffer_count++;
}

void NinaCalSequencer::runFilter(std::array<float, BUFFER_SIZE> *left, std::array<float, BUFFER_SIZE> *right) {

    if (test_counter == NUM_VOICES) {
        _test_complete = true;
        return;
    }
    if (_call_counter == 0) {
        setupFilter(test_counter);
        printf("\n filter setup voice %d", test_counter);
    }

    _controller.at(test_counter).setFilterTuneTest(stim);
    for (auto &controller : _controller) {
        controller.run();
    }
    if (_call_counter % 100 == 0) {

        printf("\n %d stim %f tc %d", _call_counter, stim, test_counter);
    }

    if (_call_counter == (int)(counter * counter * 0.15 * l + l * counter)) {

        printf("\n bufferweight %d %f", l * counter, stim);
        counter++;
        stim -= 0.11;
    }

    if (_call_counter > 10 && _call_counter < 1400) {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            _audio_data_out.push_back(left->at(i));
            _stimulus_out.push_back(_analog_voices.at(test_counter).getFilterCutOut());
        }
    }

    if (_call_counter == 1401) {
        printf("\n Test Complete");

        float osc_t = 0;
        for (auto &voice : _analog_voices) {
            osc_t += voice.getAveOscLevel();
        }
        osc_t = osc_t / (float)NUM_VOICES;
        osc_t = _analog_voices.at(test_counter).getAveOscLevel();
        _out.write(reinterpret_cast<const char *>(&osc_t), sizeof(float));
        printf("\nsize:  %d", (int)_stimulus_out.size());
        _out.write(reinterpret_cast<const char *>(_audio_data_out.data()), sizeof(float) * (_audio_data_out.size() - 1));
        _out.write(reinterpret_cast<const char *>(_stimulus_out.data()), sizeof(float) * (_stimulus_out.size() - 1));
        _out.close();
        _call_counter = -1;
        counter = 1;
        test_counter++;
        _audio_data_out.clear();
        _stimulus_out.clear();
        stim = 0.95;
    }

    _call_counter++;
}

void NinaCalSequencer::runMainVca(std::array<float, BUFFER_SIZE> *left, std::array<float, BUFFER_SIZE> *right) {
    if (test_counter == NUM_VOICES) {
        if (!_test_complete) {
            _test_complete = true;
            printf("\n save all cal");
            for (auto &voice : _analog_voices) {
                voice.saveCalibration();
            }
        }
        return;
    }

    if (buffer_count == 0) {
        _analog_voices.at(test_counter).setMainLVcaOffset(0);
        _analog_voices.at(test_counter).setMainRVcaOffset(0);
        min_l = 1000000;
        min_r = 1000000;
        _lowpass_l.reset();
        _highpass_l.reset();
        _lowpass_r.reset();
        _highpass_r.reset();
        setupMainVcaCal(test_counter);
        for (auto &val : queue_l) {
            val = 1;
        }
        for (auto &val : queue_r) {
            val = 1;
        }
        queue_i = 0;
        old_level_l = 100;
        old_level_r = 100;
        vca_state_l = 0;
        vca_state_r = 0;
    }

    for (int i = 0; i < BUFFER_SIZE; i++) {
        output_sine_phase += 2 * M_PI * 2000. / 96000.;

        if (output_sine_phase > 2 * M_PI) {
            output_sine_phase -= 2 * M_PI;
        }

        _high_res_audio_outputs.at(test_counter)->at(i) = 0.1 * std::sin(output_sine_phase);
        float lin = left->at(i);
        float rin = right->at(i);
        float rin_lp = _lowpass_r.process(rin);
        rin = rin_lp - _highpass_r.process(rin);
        float lin_lp = _lowpass_l.process(lin);
        lin = lin_lp - _highpass_l.process(lin);
        max = lin > max ? lin : max;
        min = lin < min ? lin : min;
        float l = lin;
        float r = rin;
        level_l += l * l;
        level_r += r * r;
    }

    if (test_counter == NUM_VOICES) {
        return;
    }

    _controller.at(test_counter).setMainOutVca(vca_state_l, vca_state_r);

    for (auto &controller : _controller) {
        controller.run();
    }

    if (buffer_count % 400 == 0 && buffer_count > 0) {
        test_counter_2++;
        if (test_counter_2 > 0) {
            level_l = std::sqrt(level_l);
            level_r = std::sqrt(level_r);
            _audio_data_out.push_back((vca_state_l));
            _audio_data_out.push_back((level_l));
            _audio_data_out.push_back((level_r));
            old_delta_l = -old_level_l + level_l;
            old_delta_r = -old_level_r + level_r;
            printf("\n levels: %f %f %f %f %f %f ", vca_state_l, level_l, old_delta_l, vca_state_r, level_r, old_delta_r);
            if (buffer_count % 400 == 0) {

                if (min_l > level_l) {
                    min_l = level_l;
                    min_val_l = vca_state_l;
                    printf("\n new min %f %f ", min_l, min_val_l);
                }
                queue_l.at(queue_i) = vca_state_l;
                queue_r.at(queue_i) = vca_state_r;
                queue_i += 1;
                queue_i = queue_i == 20 ? 0 : queue_i;
                float oldest_val_l = queue_l.at(queue_i);
                float oldest_val_r = queue_r.at(queue_i);
                printf("\ncheck %f %f", oldest_val_l, vca_state_l);
                if (abs(oldest_val_l - vca_state_l) < abs(2 * step_l + 0.0000001)) {
                    float sum = 0;
                    for (auto val : queue_l) {
                        sum += val;
                        printf("cal:  %f ", val);
                    }
                    sum = sum / 20.;
                    vca_output_l = sum;
                    printf("\n min: %f %f ", min_l, min_val_l);
                    printf("\nfinal val L: %f %d", sum, test_counter);
                    done_l = true;
                }
                if (abs(oldest_val_r - vca_state_r) < abs(2 * step_r + 0.0000001)) {
                    _first_pass_r = true;
                    if (!_first_pass_r) {
                        printf("\n first pass");
                    } else {
                        float sum = 0;
                        for (auto val : queue_r) {
                            sum += val;
                        }
                        sum = sum / 20.;
                        vca_output_r = sum;
                        printf("\nfinal val R: %f %d", sum, test_counter);
                        done_r = true;
                    }
                }

                old_level_l = level_l;
                old_level_r = level_r;
            }
        }
        level_l = 0;
        level_r = 0;
    }

    if (buffer_count % 400 == 0) {
        test_counter_2 = 0;
        if (old_delta_l > 0) {
            step_l = -step_l;
        } else {
        }
        if (old_delta_r > 0) {
            step_r = -step_r;
        } else {
        }
        float oldest_val_r = queue_r.at(queue_i);
        if (abs(oldest_val_r - vca_state_r) < abs(3 * step_l)) {
            if (level_l < 0.0009) {
            }
        }
        vca_state_l += step_l;
        vca_state_r += step_r;
    }
    if (done_l && done_r) {
        _analog_voices.at(test_counter).setMainRVcaOffset(vca_output_r);
        _analog_voices.at(test_counter).setMainLVcaOffset(vca_output_l);
        done_l = false;
        done_r = false;
        buffer_count = -1;
        printf("\nstop test");
        _out.write(reinterpret_cast<const char *>(_audio_data_out.data()), sizeof(float) * (_audio_data_out.size()));
        _out.close();
        test_counter++;
        printf("\ndone");
    }
    buffer_count++;
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
