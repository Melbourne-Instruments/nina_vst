/**
 * @file LayerManager.h
 * @brief Nina Layer Manager class definitions.
 *
 * @copyright Copyright (c) 2023 Melbourne Instruments, Australia
 *
 */
#pragma once
#include "Layer.h"
#include "NinaCalSequencer.h"
#include "NinaParameters.h"
#include "common.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

enum class CvInputModes {
    _n10_10,
    _0_10,
    _n5_5,
    _0_5,
    NumModes
};

constexpr uint MAX_PARAM_CHANGES = 256;

class LayerManager {
  public:
    LayerManager() {
        _layers.at(0).setNumVoices(0, 12);
        _global_params.at(NinaParams::Cv1Gain) = 1.;
        _global_params.at(NinaParams::Cv2Gain) = 1.;
        _global_params.at(NinaParams::Cv3Gain) = 1.;
        _global_params.at(NinaParams::Cv4Gain) = 1.;
        _param_changes[0].reserve(NinaParams::NUM_PARAMS * 2);
        _param_changes[1].reserve(NinaParams::NUM_PARAMS * 2);
        _param_changes[2].reserve(NinaParams::NUM_PARAMS * 2);
        _param_changes[3].reserve(NinaParams::NUM_PARAMS * 2);

        std::fstream file_handler;
        std::stringstream fpath;
        std::string path = "/udata/nina/calibration/";
        fpath << path << "mute_disable.txt";
        file_handler.open(fpath.str(), std::ios::in);
        bool disable_mutes = false;
        if (file_handler.is_open()) {
            disable_mutes = true;
        }
        for (auto &voice : _analog_voices) {
            voice.setMuteDisable(disable_mutes);
        }
    }

    ~LayerManager() = default;
    void processAudio(ProcessData &data);
    void updateParams(uint num_changes, const ParamChange *changed_params);
    void allocateVoices(MidiNote note);
    void freeVoices(MidiNote note);

    void setUnitTestMode() {
        for (auto &aVoice : _analog_voices) {
            aVoice.setUnitTestMode();
        }
    }

    void dump() {
        _layers.at(0).dump();
        _analog_voices.at(0).dump();
    }

    /**
     * @brief Set the parameter smoothing coeff. set to 1.0 for actual smoothing value, set to ~20 or something if you want to accellerate it in testing
     *
     * @param value
     */
    void setLayerSmoothing(float value) {
        for (auto &layer : _layers) {
            layer.setSmoothingRates(value);
        }
    }

    void polyPressureEvent(Steinberg::Vst::PolyPressureEvent poly_event) {
        for (auto &layer : _layers) {
            layer.polyPressureEvent(poly_event);
        }
    }

    void runCalSquence(bool run) {
        //_run_cal = run;
    }

    void writeTemps();

    void loadCal() {
        for (auto &voice : _analog_voices) {
            voice.loadCalibration();
        }
    }

  private:
    uint _mpeParamToRange(float val) {

        return std::round(((float)(MAX_MPE_PB_RANGE + 1)) * val);
    }

    uint _mpeParamToChannelRange(float val) {

        return std::round(val * (float)(MPE_CHANNEL_SETTINGS_MULT));
    }

    void _reEvaluateLayerVoices();
    void _reCalcCvs();
    void _setCvParams1();
    void _setCvParams2();
    void _setCvParams3();
    void _setCvParams4();
    bool _run_cal = false;
    std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> _high_res_audio_outputs;
    std::vector<ParamChange> _param_changes[NUM_LAYERS];
    std::array<AnalogVoice, NUM_VOICES> _analog_voices = {
        AnalogVoice(0),
        AnalogVoice(1),
        AnalogVoice(2),
        AnalogVoice(3),
        AnalogVoice(4),
        AnalogVoice(5),
        AnalogVoice(6),
        AnalogVoice(7),
        AnalogVoice(8),
        AnalogVoice(9),
        AnalogVoice(10),
        AnalogVoice(11)};
    uint _current_layer = 0;
    NinaCalSequencer _calibrator = NinaCalSequencer(_analog_voices, _high_res_audio_outputs);
    std::array<float, CV_BUFFER_SIZE> _cv_1;
    std::array<float, CV_BUFFER_SIZE> _cv_2;
    std::array<float, CV_BUFFER_SIZE> _cv_3;
    std::array<float, CV_BUFFER_SIZE> _cv_4;
    NinaParams::AudioInputBuffers _input_buffers = NinaParams::AudioInputBuffers(_cv_1, _cv_2, _cv_3, _cv_4);
    std::array<uint, NUM_LAYERS> _layer_voices = {0};
    NinaParams::GlobalParams _global_params;
    std::array<Layer, NUM_LAYERS> _layers = {
        Layer(0, 0, _analog_voices, _high_res_audio_outputs, _input_buffers, _param_changes[0], _global_params),
        Layer(0, 0, _analog_voices, _high_res_audio_outputs, _input_buffers, _param_changes[1], _global_params),
        Layer(0, 0, _analog_voices, _high_res_audio_outputs, _input_buffers, _param_changes[2], _global_params),
        Layer(0, 0, _analog_voices, _high_res_audio_outputs, _input_buffers, _param_changes[3], _global_params)};
    float _cv_1_offset = 0;
    float _cv_2_offset = 0;
    float _cv_3_offset = 0;
    float _cv_4_offset = 0;
    float _cv_1_gain = 1;
    float _cv_2_gain = 1;
    float _cv_3_gain = 1;
    float _cv_4_gain = 1;

    float voice_sel = 0;
    float offset_l = 0;
    float offset_r = 0;
    float _global_tempo = 10.0 / 60.0;
    bool _write_temps = false;
    bool _write_temps_latch = false;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
