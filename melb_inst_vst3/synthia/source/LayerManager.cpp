/**
 * @file LayerManager.cpp
 * @brief Nina Layer Manager class implementation.
 *
 * @copyright Copyright (c) 2022 Melbourne Instruments, Australia
 */
#include "LayerManager.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include <assert.h>

namespace Steinberg {
namespace Vst {
namespace Nina {

void LayerManager::processAudio(ProcessData &data) {
    float *cv_ch_0_outputs[NUM_VOICES];
    float *cv_ch_1_outputs[NUM_VOICES];
    float *tuning_input = data.inputs->channelBuffers32[6];

    _global_tempo = (float)(data.processContext->tempo) / 60.0;

    for (auto &layer : _layers) {
        layer.setTempo(_global_tempo);
    }

    // update the input data struct
    _input_buffers.mix_left = reinterpret_cast<std::array<float, BUFFER_SIZE> *>(data.inputs->channelBuffers32[0]);
    _input_buffers.mix_right = reinterpret_cast<std::array<float, BUFFER_SIZE> *>(data.inputs->channelBuffers32[1]);
    _input_buffers.ex_in_1 = reinterpret_cast<std::array<float, BUFFER_SIZE> *>(data.inputs->channelBuffers32[2]);
    _input_buffers.ex_in_2 = reinterpret_cast<std::array<float, BUFFER_SIZE> *>(data.inputs->channelBuffers32[3]);
    _input_buffers.ex_in_3 = reinterpret_cast<std::array<float, BUFFER_SIZE> *>(data.inputs->channelBuffers32[4]);
    _input_buffers.ex_in_4 = reinterpret_cast<std::array<float, BUFFER_SIZE> *>(data.inputs->channelBuffers32[5]);

    // run the helper that applies the CV calibration
    _reCalcCvs();

    // setup the high res output
    for (uint i = 0; i < NUM_VOICES; i++) {
        _high_res_audio_outputs.at(i) = reinterpret_cast<std::array<float, BUFFER_SIZE> *>(data.outputs[0].channelBuffers32[i]);
    }

    // Run the layer voices in each layer

    if (!_run_cal) {
        bool gpio = false;
        for (uint i = 0; i < NUM_LAYERS; i++) {
            _layers[i].run();
        }
        for (uint i = 0; i < NUM_LAYERS; i++) {
            _layers[i].runWt();
        }
        for (uint i = 0; i < NUM_LAYERS; ++i) {
            _layers[i].runVoices();
        }

        // Get the output for each voice
        for (uint i = 0; i < NUM_VOICES; i++) {
            _analog_voices.at(i).runTuning(tuning_input[i * 4 + 1], tuning_input[i * 4 + 0], tuning_input[i * 4 + 3], tuning_input[i * 4 + 2]);
            VoiceOutput output = {reinterpret_cast<std::array<float, BUFFER_SIZE> *>(data.outputs[0].channelBuffers32[i * 2 + 12]), reinterpret_cast<std::array<float, BUFFER_SIZE> *>(data.outputs[0].channelBuffers32[i * 2 + 13])};
            _analog_voices[i].generateVoiceBuffers(output);
        }
        // Are there any param changes (from the current layer) to return?
        uint num_of_changes = _param_changes[_current_layer].size();

        // if there are alot of changes, we send them 256 at a time to avoid any buffer skips
        if (num_of_changes > MAX_PARAM_CHANGES) {
            num_of_changes = MAX_PARAM_CHANGES;
        }
        auto &curr_layer_changes = _param_changes[_current_layer];
        for (uint i = 0; i < num_of_changes; i++) {
            const ParamChange &pc = curr_layer_changes.back();
            _param_changes[_current_layer].pop_back();
            int32 index = 0;
            IParameterChanges *outParamChanges = data.outputParameterChanges;
            auto paramQueue = outParamChanges->addParameterData(pc.param_id, index);
            if (paramQueue != nullptr) {
                int32 index2 = 0;
                paramQueue->addPoint(0, pc.value, index2);
            }
        }
    } else {
        // run calibration sequenceer
        _calibrator.run(_input_buffers.mix_left, _input_buffers.mix_right);

        // run the analog voices;
        for (uint i = 0; i < NUM_VOICES; i++) {
            _analog_voices.at(i).runTuning(tuning_input[i * 4 + 1], tuning_input[i * 4 + 0], tuning_input[i * 4 + 3], tuning_input[i * 4 + 2]);
            VoiceOutput output = {reinterpret_cast<std::array<float, BUFFER_SIZE> *>(data.outputs[0].channelBuffers32[i * 2 + 12]), reinterpret_cast<std::array<float, BUFFER_SIZE> *>(data.outputs[0].channelBuffers32[i * 2 + 13])};
            _analog_voices[i].generateVoiceBuffers(output);
        }
    }
}

void LayerManager::updateParams(uint num_changes, const ParamChange *changed_params) {
    // Only process global params here
    for (uint i = 0; i < num_changes; i++) {
        const ParamChange &pc = changed_params[i];

        // calculate the actual param id, removeing the select layer bits
        const uint param_id = get_param_id(pc);

        // handy param printer
        // if (param_id < NinaParams::ModMatrixEntries) {
        //     std::string str = NinaParams::_paramTitles[param_id];
        //     std::cout << "\n "
        //               << "param " << str << "  " << pc.value;
        // } else {

        //     std::cout << "\n "
        //               << "param " << param_id << "  " << pc.value;
        // }
        bool cv_1_recalc = false;
        bool cv_2_recalc = false;
        bool cv_3_recalc = false;
        bool cv_4_recalc = false;
        switch (param_id) {
        case NinaParams::MorphValue: {
            // Set the morph value in the current layer
            //_layers[_current_layer].setMorphValue(pc.value);
            break;
        }

        case NinaParams::CurrentLayer: {
            _current_layer = std::round((NUM_LAYERS)*pc.value);
            for (auto &layer : _layers) {
                layer.setNotActive();
            }
            _layers.at(_current_layer).setAsActive();
        } break;
        case NinaParams::MpeYBipolarMode: {
            for (auto &layer : _layers) {
                bool bipolar = pc.value < PARAM_BOOL_FLOAT_COMP;
                layer.setMpeYBipolar(bipolar);
            }
        } break;
        case NinaParams::MorphMode: {
            // Get the mode and set in all layers
            uint mode = (uint)floor(pc.value * MorphMode::NUM_MORPH_MODES);
            if (mode > MorphMode::DJ)
                mode = MorphMode::DJ;
            for (uint i = 0; i < NUM_LAYERS; i++) {
                _layers[i].setMorphMode(MorphMode(mode));
            }
            break;
        }
        case NinaParams::MasterTune: {
            const float detune = calcMasterDetune(pc.value);
            for (auto &layer : _layers) {
                layer.setMasterDetune(detune);
            }
        } break;

        case NinaParams::MpeLowerZonePB: {

            const float &value = changed_params[i].value;
            const uint range = _mpeParamToRange(value);
            for (auto &layer : _layers) {
                layer.setMpeLowerPbRange(range);
            }
        } break;
        case NinaParams::MpeUpperZonePB: {
            const float &value = changed_params[i].value;
            const uint range = _mpeParamToRange(value);
            for (auto &layer : _layers) {
                layer.setMpeUpperPbRange(range);
            }

        } break;
        case NinaParams::MpeLowerZoneNumChannels: {
            const float &value = changed_params[i].value;
            const uint channels = _mpeParamToChannelRange(value);
            for (auto &layer : _layers) {
                layer.setMpeLowerChannels(channels);
            }
        } break;
        case NinaParams::MpeUpperZoneNumChannels: {

            const float &value = changed_params[i].value;
            const uint channels = _mpeParamToChannelRange(value);
            for (auto &layer : _layers) {
                layer.setMpeUpperChannels(channels);
            }
        } break;

        case NinaParams::Layer1NumVoices: {
            // Set the layer 1 number of voices
            uint num_voices = std::round(changed_params[i].value * (NUM_VOICES + 1));
            if (num_voices > NUM_VOICES) {
                num_voices = NUM_VOICES;
            }

            _layer_voices.at(_current_layer) = num_voices;
            _reEvaluateLayerVoices();
        } break;

        case NinaParams::MainOutMuteL: {
            bool left = pc.value > 0.5;
            _analog_voices.at(0).setOutMuteL(left);

        } break;
        case NinaParams::MainOutMuteR: {
            bool right = pc.value > 0.5;
            _analog_voices.at(0).setOutMuteR(right);

        } break;
        case NinaParams::TuningGain: {
            // Set the layer 2 number of voices
            float val = pc.value;
            for (auto &avoice : _analog_voices) {
                avoice.setOscTuningGain(val);
            }
        } break;
        case NinaParams::TriggerPrint: {
            // Set the layer 2 number of voices
            float val = pc.value;
            _analog_voices.at(0).printStuff();
            _layers.at(0).debugPrint();

        } break;
        case NinaParams::vca_offset_1:
            break;
        case NinaParams::RunFilterCal: {
            // printf("\n start filter  tuning");
            const bool run_cal = pc.value > 0.5;
            _calibrator.startFilterCal(run_cal);
            _run_cal = run_cal;
        } break;

        case NinaParams::RunMixVcaCal: {
            // printf("\n start mix vca tuning");
            const bool run_cal = pc.value > 0.5;
            _calibrator.startMixCal(run_cal);
            _run_cal = run_cal;
        } break;

        case NinaParams::RunMainVcaCal: {
            // printf("\n start main vca tuning");
            const bool run_cal = pc.value > 0.5;
            _calibrator.startMainVcaCal(run_cal);
            _run_cal = run_cal;
        } break;

        case NinaParams::Cv1Offset:
            cv_1_recalc = true;
            _global_params[NinaParams::Cv1Offset] = pc.value * 2 - 1;
            break;
        case NinaParams::Cv1Gain:
            cv_1_recalc = true;
            _global_params[NinaParams::Cv1Gain] = pc.value * 2 - 1;
            break;
        case NinaParams::Cv1InputMode:
            cv_1_recalc = true;
            _global_params[NinaParams::Cv1InputMode] = pc.value;
            break;

        case NinaParams::Cv3Offset:
            cv_3_recalc = true;
            _global_params[NinaParams::Cv3Offset] = pc.value * 2 - 1;
            break;
        case NinaParams::Cv3Gain:
            cv_3_recalc = true;
            _global_params[NinaParams::Cv3Gain] = pc.value * 2 - 1;
            break;
        case NinaParams::Cv3InputMode:
            cv_3_recalc = true;
            _global_params[NinaParams::Cv3InputMode] = pc.value;
            break;

        case NinaParams::Cv2Offset:
            cv_2_recalc = true;
            _global_params[NinaParams::Cv2Offset] = pc.value * 2 - 1;
            break;
        case NinaParams::Cv2Gain:
            cv_2_recalc = true;
            _global_params[NinaParams::Cv2Gain] = pc.value * 2 - 1;
            break;
        case NinaParams::Cv2InputMode:
            cv_2_recalc = true;
            _global_params[NinaParams::Cv2InputMode] = pc.value;
            break;
        case NinaParams::Cv4Offset:
            cv_4_recalc = true;
            _global_params[NinaParams::Cv4Offset] = pc.value * 2 - 1;
            break;
        case NinaParams::Cv4Gain:
            cv_4_recalc = true;
            _global_params[NinaParams::Cv4Gain] = pc.value * 2 - 1;
            break;
        case NinaParams::Cv4InputMode:
            cv_4_recalc = true;
            _global_params[NinaParams::Cv4InputMode] = pc.value;
            break;
        case NinaParams::writeTemp: {
            bool write_temps = pc.value > 0.5;
            if (write_temps && !_write_temps) {
                _write_temps = true;
                writeTemps();
            }
            if (!write_temps) {
                _write_temps = false;
            }
        } break;

        case NinaParams::reloadCal: {
            bool reload = pc.value > 0.5;
            if (reload) {
                loadCal();
            }
        } break;

        case NinaParams::AllRunTuning: {
            float val = pc.value;
            const bool run_tune = val > 0.5;
            for (auto &avoice : _analog_voices) {
                if (run_tune)

                {
                    avoice.runOscTuning();
                } else {
                    avoice.stopOscTuning();
                }
            }

        } break;

        default:
            // Ignore all other params
            break;
        }

        if (cv_1_recalc || cv_2_recalc || cv_3_recalc || cv_4_recalc) {
            _setCvParams1();
            _setCvParams2();
            _setCvParams3();
            _setCvParams4();
        }
    }

    // update all the params in each layer. each layer will only respond to params which are sent to that layer
    for (uint i = 0; i < NUM_LAYERS; i++) {
        _layers.at(i).updateParams(num_changes, changed_params);
    }
}

void LayerManager::writeTemps() {
    std::stringstream file;
    file << "/udata/nina/tuning/cal_info.dat";
    std::ofstream out;
    out.open(file.str(), std::ios::out | std::ios::trunc | std::ios::binary);
    std::vector<float> data;
    for (uint i = 0; i < NUM_VOICES; i++) {
        data.push_back(_analog_voices.at(i).getAveOscLevel());
    }
    data.push_back((int)(_calibrator.isTestRunning()));
    out.write(reinterpret_cast<const char *>(data.data()), sizeof(float) * (data.size()));
}

void LayerManager::_reEvaluateLayerVoices() {

    uint leftover_voices = NUM_VOICES;
    uint prev_layer_voices = 0;
    for (auto &voice : _analog_voices) {
        voice.setAllocatedToVoice(false);
    }
    for (uint layer = 0; layer < NUM_LAYERS; layer++) {
        _layers.at(layer).setNumVoices(prev_layer_voices, _layer_voices.at(layer));
        for (uint i = 0; i < _layer_voices.at(layer); i++) {
            if (i + prev_layer_voices >= NUM_VOICES) {
                break;
            }
            _analog_voices.at(i + prev_layer_voices).setAllocatedToVoice(true);
        }
        prev_layer_voices += _layer_voices.at(layer);
    }

    for (auto &layer : _layers) {
        layer.resetVoiceAllocation();
    }
}

void LayerManager::allocateVoices(MidiNote note) {
    // Allocate this note to each layer
    for (uint i = 0; i < NUM_LAYERS; i++) {
        _layers[i].allocateVoices(note);
    }

    for (auto &voice : _analog_voices) {
        voice.reEvaluateMutes();
    }
}

void LayerManager::freeVoices(MidiNote note) {
    // Free the note voices from each layer

    // for now, override relese velocity
    if (note.velocity < MIN_VEL) {
        note.velocity = MID_VEL;
    }
    for (uint i = 0; i < NUM_LAYERS; i++) {
        _layers[i].freeVoices(note);
    }

    for (auto &voice : _analog_voices) {
        voice.reEvaluateMutes();
    }
}

void LayerManager::_reCalcCvs() {
    constexpr int cv_step = SAMPLE_RATE / CV_SAMPLE_RATE;
    for (uint i = 0; i < CV_BUFFER_SIZE; i++) {
        _cv_1[i] = _cv_1_gain * ((*_input_buffers.ex_in_1)[i * cv_step]) + _cv_1_offset;
        _cv_2[i] = _cv_2_gain * (*_input_buffers.ex_in_2)[i * cv_step] + _cv_2_offset;
        _cv_3[i] = _cv_3_gain * (*_input_buffers.ex_in_3)[i * cv_step] + _cv_3_offset;
        _cv_4[i] = _cv_4_gain * (*_input_buffers.ex_in_4)[i * cv_step] + _cv_4_offset;
    }
}

void cvSwitchHelper(CvInputModes mode, float &offset_in, float &gain_in) {
    constexpr float offset_max = 0.06;
    constexpr float gain_max = 0.06;
    constexpr float _0_10_gain = -3.841109;
    constexpr float _10_10_gain = _0_10_gain / 2;

    constexpr float _10_10_offset = 0;
    constexpr float _0_10_offset = -0.751312;
    constexpr float _5_5_gain = _0_10_gain;
    constexpr float _5_5_offset = 0;
    constexpr float _0_5_gain = _0_10_gain * 2;
    constexpr float _0_5_offset = _0_10_offset / 2;
    const float offset = offset_in;
    const float gain = gain_in;
    switch (mode) {
    case CvInputModes::_n10_10:
        offset_in =
            offset_in = _10_10_gain * (1 - offset * offset_max);
        gain_in = _10_10_gain * (1 + gain * gain_max);
        break;
    case CvInputModes::_0_10:
        offset_in = _0_10_offset * (1 - offset * offset_max);
        gain_in = _0_10_gain * (1 + gain * gain_max);
        break;
    case CvInputModes::_0_5:

        offset_in = _0_5_offset * (1 - offset * offset_max);
        gain_in = _0_5_gain * (1 + gain * gain_max);
        break;
    case CvInputModes::_n5_5:
        offset_in = _5_5_offset * (1 - offset * offset_max);
        gain_in = _5_5_gain * (1 + gain * gain_max);
        break;

    default:
        break;
    }
    return;
}

void LayerManager::_setCvParams1() {
    float mode_t = std::round(_global_params.at(NinaParams::Cv1InputMode) * (float)CvInputModes::NumModes);
    CvInputModes mode = (CvInputModes)(int)mode_t;
    float offset = _global_params.at(NinaParams::Cv1Offset);
    float gain = _global_params.at(NinaParams::Cv1Gain);
    cvSwitchHelper(mode, offset, gain);
    _cv_1_offset = offset;
    _cv_1_gain = gain;
}

void LayerManager::_setCvParams2() {

    CvInputModes mode = (CvInputModes)(int)std::round(_global_params.at(NinaParams::Cv2InputMode) * (float)CvInputModes::NumModes);
    float offset = _global_params.at(NinaParams::Cv2Offset);
    float gain = _global_params.at(NinaParams::Cv2Gain);
    cvSwitchHelper(mode, offset, gain);
    _cv_2_offset = offset;
    _cv_2_gain = gain;
}

void LayerManager::_setCvParams3() {

    CvInputModes mode = (CvInputModes)(int)std::round(_global_params.at(NinaParams::Cv3InputMode) * (float)CvInputModes::NumModes);
    float offset = _global_params.at(NinaParams::Cv3Offset);
    float gain = _global_params.at(NinaParams::Cv3Gain);
    cvSwitchHelper(mode, offset, gain);
    _cv_3_offset = offset;
    _cv_3_gain = gain;
}

void LayerManager::_setCvParams4() {

    CvInputModes mode = (CvInputModes)(int)std::round(_global_params.at(NinaParams::Cv4InputMode) * (float)CvInputModes::NumModes);
    float offset = _global_params.at(NinaParams::Cv4Offset);
    float gain = _global_params.at(NinaParams::Cv4Gain);
    cvSwitchHelper(mode, offset, gain);
    _cv_4_offset = offset;
    _cv_4_gain = gain;
}
} // namespace Nina
} // namespace Vst
} // namespace Steinberg
