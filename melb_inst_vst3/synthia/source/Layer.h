/**
 * @file Layer.h
 * @brief Nina Layer class definitions.
 *
 * @copyright Copyright (c) 2023 Melbourne Instruments, Australia
 */
#pragma once
#include "NinaParameters.h"
#include "NinaVoice.h"
#include "common.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include <functional>

namespace Steinberg {
namespace Vst {
namespace Nina {

constexpr float PARAM_BOOL_FLOAT_COMP = 0.25;

enum InputModes {
    input1 = 0,
    input2,
    input3,
    input4,
    inputs1_2,
    inputs3_4,
    outputs1_2,
    numInputModes,
};

class Layer {
  public:
    using MorphFn = float (*)(uint, bool, Layer &);
    using UpdateFn = void (*)(float, Layer &);
    using SmoothFn = float (*)(float, float &, Layer &);
    using TransformFn = float (*)(float, Layer &);

    struct CommonParamAttr {
        ParamID param_id;
        UpdateFn update_fn;
        SmoothFn smooth_fn;
        float set_value = 0;

        CommonParamAttr() {}

        CommonParamAttr(ParamID p, UpdateFn u, SmoothFn s) {
            param_id = p;
            update_fn = u;
            smooth_fn = s;
        };
    };

    enum PanModes {
        off = 0,
        pingPong = 1,
        spread = 2,
        numPanModes
    };

    struct StateParamAttr {
        ParamID param_id;
        MorphFn morph_fn;
        UpdateFn update_fn;
        SmoothFn smooth_fn;
        TransformFn transform_fn;
        float prev_morphed_val = 0;
        bool automate = true;

        StateParamAttr() {}

        StateParamAttr(ParamID p, MorphFn m, UpdateFn u, SmoothFn s, TransformFn t) {
            param_id = p;
            morph_fn = m;
            update_fn = u;
            smooth_fn = s;
            transform_fn = t;
        };
    };

    struct LayerParamAttr {
        std::array<CommonParamAttr, NinaParams::NUM_LAYER_COMMON_PARAMS> common_attrs;
        std::array<StateParamAttr, NinaParams::NUM_LAYER_STATE_PARAMS> state_attrs;
    };

    Layer(uint num_voices, uint first_voice, std::array<AnalogVoice, NUM_VOICES> &analog_voices, std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> &high_res_output, NinaParams::AudioInputBuffers &input_buffers, std::vector<ParamChange> &param_changes, NinaParams::GlobalParams &global_params);
    ~Layer() = default;
    const std::vector<ParamChange> &run();
    void setNumVoices(uint first_voice, uint num_voices);
    void updateParams(uint num_changes, const ParamChange *changed_params);
    void allocateVoices(const MidiNote &note);
    void freeVoices(const MidiNote &note);
    void setMorphValue(float value);
    void setMorphMode(MorphMode mode);

    void setOsc3Slow(bool slow) {
        _layer_params.osc_3_slow_mode = slow;
    }

    void setMasterDetune(float detune) {
        _layer_params.master_detune = detune;
    }

    void setAsActive() {
        _signal_param_refresh = true;
        _current_layer = true;
    }

    void setWtInterpolate(bool value) {
        _layer_params.wave_interpolate = value;
    }

    void setMpeUpperChannels(uint channels) {

        // check if var has actually changed
        if (_mpe_upper_channels == channels) {
            return;
        }
        _mpe_upper_channels = channels;
        _updateMpeConfig();
    }

    void setMidiSourceVelMode(bool mode) {
        _layer_params.release_vel_mode = mode;
    }

    void setMpeLowerChannels(uint channels) {

        // check if var has actually changed
        if (_mpe_lower_channels == channels) {
            return;
        }
        _mpe_lower_channels = channels;
        _updateMpeConfig();
    }

    void setMpeLowerPbRange(uint range) {

        // check if var has actually changed
        if (_mpe_lower_pb_range == range) {
            return;
        }
        _mpe_lower_pb_range = range;
        _updateMpeConfig();
    }

    void setMpeUpperPbRange(uint range) {

        // check if var has actually changed
        if (_mpe_upper_pb_range == range) {
            return;
        }
        _mpe_upper_pb_range = range;
        _updateMpeConfig();
    }

    void polyPressureEvent(Steinberg::Vst::PolyPressureEvent poly_event);

    void resetVoiceAllocation() {
        _round_robin_voice = 0;
    }

    void setSmoothingRates(float rates) {
        _smoothing = rates * PARAM_SMOOTH_COEFF;
    }

    void dump() {
        _layer_voices.at(0).dump();
    }

    inline void setMidiLowFilter(int midi_n) {
        _midi_note_start = midi_n;
        _clearNotes();
    }

    inline void setMidiHighFilter(int midi_n) {
        _midi_note_end = midi_n;
        _clearNotes();
    }

    inline void setMpeMode(NinaParams::MpeModes mode) {

        // if we are entering MPE mode, we reset AT since we may never recieve another poly AT message, leaving the value stuck
        if ((_mpe_mode == NinaParams::MpeModes::Off) && (mode != NinaParams::MpeModes::Off)) {
            for (auto &voice : _layer_voices) {
                voice.resetAT();
            }
        }
        _mpe_mode = mode;
        _updateMpeConfig();
    }

    inline void setVcfMode(bool filter_2_pole) {
        _layer_params.filter_2_pole_mode = filter_2_pole;
    }

    inline void setMidiChannel(uint midi_chan, bool en) {
        _channel_filter_en = en;
        _midi_channel = midi_chan;
        _clearNotes();
    }

    void allNotesOff() {
        _clearNotes();
        _held_notes.clear();
    }

    void setMpeYBipolar(bool bipolar) {
        _layer_params.mpe_y_bipolar = bipolar;
    }

    inline void setMpeYFall(float fall) {

        fall = fall * fall * fall;
        float fall_coeff = (1 - fastpow2(-(1.4 / (CV_SAMPLE_RATE * 5. * fall))));
        fall_coeff = fall_coeff > 1.0 ? 1.0 : fall_coeff;
        fall_coeff = fall_coeff < 0. ? 0.0 : fall_coeff;
        _layer_params.mpe_y_fall_coeff = fall_coeff;
    }

    inline void setMpeZFall(float fall) {
        fall = fall * fall * fall;
        float fall_coeff = (1 - fastpow2(-(1.4 / (CV_SAMPLE_RATE * 5. * fall))));
        fall_coeff = fall_coeff > 1.0 ? 1.0 : fall_coeff;
        fall_coeff = fall_coeff < 0. ? 0.0 : fall_coeff;
        _layer_params.mpe_z_fall_coeff = fall_coeff;
    }

    inline void setInputMode(InputModes mode) {
        _input_mode = mode;
    }

    inline void setTempo(float tempo) {
        _layer_params.global_tempo = tempo;
    }

    inline void setOutputMode(NinaParams::OutputRouting mode) {
        switch (mode) {
        case NinaParams::OutputRouting::One:
            _layer_params.mute_out_1 = false;
            _layer_params.mute_out_2 = true;
            _layer_params.mute_out_3 = true;
            _layer_params.mute_out_4 = true;
            break;
        case NinaParams::OutputRouting::Two:
            _layer_params.mute_out_1 = true;
            _layer_params.mute_out_2 = false;
            _layer_params.mute_out_3 = true;
            _layer_params.mute_out_4 = true;
            break;
        case NinaParams::OutputRouting::Three:
            _layer_params.mute_out_1 = true;
            _layer_params.mute_out_2 = true;
            _layer_params.mute_out_3 = false;
            _layer_params.mute_out_4 = true;
            break;
        case NinaParams::OutputRouting::Four:
            _layer_params.mute_out_1 = true;
            _layer_params.mute_out_2 = true;
            _layer_params.mute_out_3 = true;
            _layer_params.mute_out_4 = false;
            break;
        case NinaParams::OutputRouting::St_L_R:
            _layer_params.mute_out_1 = false;
            _layer_params.mute_out_2 = false;
            _layer_params.mute_out_3 = true;
            _layer_params.mute_out_4 = true;
            break;
        case NinaParams::OutputRouting::St_3_4:
            _layer_params.mute_out_1 = true;
            _layer_params.mute_out_2 = true;
            _layer_params.mute_out_3 = false;
            _layer_params.mute_out_4 = false;
            break;
        case NinaParams::OutputRouting::St_1_2_3_4:
            _layer_params.mute_out_1 = false;
            _layer_params.mute_out_2 = false;
            _layer_params.mute_out_3 = false;
            _layer_params.mute_out_4 = false;
            break;
        default:
            break;
        }
    }

    void setNotActive() {
        _current_layer = false;
    }

    bool _all_voices_released = true;
    uint _layer_num;
    uint _num_voices;
    uint _first_voice;
    uint _last_voice;
    LayerState _layer_state;
    VoiceMode _voice_mode;
    uint _num_unison;
    uint _pan_num = 0;
    uint _current_pan_num = 0;
    PanModes _pan_mode = off;
    uint _round_robin_voice = 0;
    bool _morphing = false;
    NinaParams::MpeModes _mpe_mode = NinaParams::MpeModes::Off;
    uint _mpe_upper_channels = 1;
    uint _mpe_lower_channels = 1;
    uint _mpe_lower_pb_range = DEFAULT_MPE_PB_RANGE;
    uint _mpe_upper_pb_range = DEFAULT_MPE_PB_RANGE;
    uint _mpe_first_ch = 1;
    uint _mpe_last_ch = 1;
    float _cur_morph_value = 0.0f;
    float _prev_morph_value = 0.0f;
    NinaParams::OutputRouting _output_routing = NinaParams::OutputRouting::St_1_2_3_4;
    MorphMode _morph_mode = MorphMode::DANCE;
    std::vector<MidiNote> _held_notes;
    std::vector<LayerVoice *> _active_voices;
    std::array<LayerVoice, NUM_VOICES> _layer_voices;
    NinaParams::LayerParams _layer_params;

    NinaParams::LayerStateParams _state_a_params;
    NinaParams::LayerStateParams _state_b_params;
    NinaParams::LayerStateParams *_current_state_params = &_state_a_params;
    WavetableLoader _wt_loader_a;
    WavetableLoader _wt_loader_b;
    LayerParamAttr _layer_param_attrs;
    NinaParams::LayerStateParams _state_a_smooth;
    NinaParams::LayerStateParams _state_b_smooth;
    NinaParams::LayerStateParams _state_a_transform;
    NinaParams::LayerStateParams _state_b_transform;
    static const std::map<int, std::vector<int>> _unison_detune_positions;
    static const std::map<int, std::vector<float>> _unison_pan_positions;
    float _osc_1_tune = 0, _osc_2_tune = 0, _osc_3_tune = 0;
    std::vector<ParamChange> &_param_changes;
    float _smoothing = 1.0 * PARAM_SMOOTH_COEFF;
    float _compression_signal = 0;
    float &_misc_scale;
    bool print_stuff = false;
    std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> _high_res_input;
    NinaParams::AudioInputBuffers &_high_input_buffers;
    std::array<float, CV_BUFFER_SIZE> &_cv_1 = _high_input_buffers._cv_in_1;
    std::array<float, CV_BUFFER_SIZE> &_cv_2 = _high_input_buffers._cv_in_2;
    std::array<float, CV_BUFFER_SIZE> &_cv_3 = _high_input_buffers._cv_in_3;
    std::array<float, CV_BUFFER_SIZE> &_cv_4 = _high_input_buffers._cv_in_4;
    bool _layer_state_change = false;
    int _midi_note_start = 0;
    int _midi_note_end = 127;
    bool _channel_filter_en = false;
    uint _midi_channel = 0;
    InputModes _input_mode = InputModes::input1;
    float _global_tempo = 10 / 60.f;
    bool _signal_param_refresh = false;
    bool _current_layer = false;
    float &_expression = _layer_params.common_params.at(LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::MidiExpression));
    float &_midi_source = _layer_params.common_params.at(LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::MidiSource));
    std::array<NinaParams::MpeChannelData, MPE_MAX_NUM_CHANNELS + 1> _mpe_channel_data;
    NinaParams::GlobalParams &_global_params;

    void debugPrint() {

        printf("\n");
        _layer_voices.at(0).printstuff();
        print_stuff = true;
    }

    void runVoices() {
        if (_num_voices) {
            for (uint i = _first_voice; i <= _last_voice; ++i) {

                _layer_voices[i].run();
            }
        }
    }

    void runWt() {

        for (uint i = _first_voice; i <= _last_voice; ++i) {
            _layer_voices[i].runWt();
        }
    }

  private:
    void _clearNotes();

    // helper function to update the current mpe config based on the set number of channels and the mode
    void _updateMpeConfig() {
        _clearNotes();
        if (_mpe_mode == NinaParams::MpeModes::Lower) {
            _mpe_first_ch = 1;
            _mpe_last_ch = _mpe_lower_channels;
            _layer_params.mpe_pitchbend_gain = mpePbRangeConvert(_mpe_lower_pb_range);
        } else if (_mpe_mode == NinaParams::MpeModes::Upper) {
            _mpe_last_ch = 14;
            _mpe_first_ch = 15 - _mpe_upper_channels;
            _layer_params.mpe_pitchbend_gain = mpePbRangeConvert(_mpe_upper_pb_range);
        }
        _layer_params.mpe_pitchbend_offset = -(_layer_params.mpe_pitchbend_gain / 2.f);
        _layer_params.mpe_mode_en = _mpe_mode != NinaParams::MpeModes::Off;
    };

    inline void _setCvASource(NinaParams::CvInputMode mode) {
        switch (mode) {
        case NinaParams::CvInputMode::One:
            _layer_params._cv_a = &_cv_1;
            break;
        case NinaParams::CvInputMode::Two:
            _layer_params._cv_a = &_cv_2;
            break;
        case NinaParams::CvInputMode::Three:
            _layer_params._cv_a = &_cv_3;
            break;
        case NinaParams::CvInputMode::Four:
            _layer_params._cv_a = &_cv_4;
            break;

        default:
            break;
        }
    }

    inline void _setCvBSource(NinaParams::CvInputMode mode) {
        switch (mode) {
        case NinaParams::CvInputMode::One:
            _layer_params._cv_b = &_cv_1;
            break;
        case NinaParams::CvInputMode::Two:
            _layer_params._cv_b = &_cv_2;
            break;
        case NinaParams::CvInputMode::Three:
            _layer_params._cv_b = &_cv_3;
            break;
        case NinaParams::CvInputMode::Four:
            _layer_params._cv_b = &_cv_4;
            break;

        default:
            break;
        }
    }

    inline void setLfo1Sync(bool sync) {
        _layer_params.lfo_1_tempo_sync = sync;
    }

    inline void setLfo2Sync(bool sync) {
        _layer_params.lfo_2_tempo_sync = sync;
    }

    inline void setDrone(bool drone) {
        _layer_params.vca_drone = drone;
    }

    void _resetActiveVoices();

    void _update_midi_sources() {
        _layer_params.midi_source = param_smooth(_midi_source, _layer_params.midi_source);
        _layer_params.expression = param_smooth(_expression, _layer_params.expression);
    }

    float inline _getCommonParam(ParamID param_id);
    bool _midiNoteFilter(const uint channel, const uint note_n);
    float inline _getStateParam(ParamID param_id);
    float inline _getCurrentStateParam(ParamID param_id);

    void inline _setCommonParam(ParamID param_id, float value);
    void inline _setStateParam(ParamID param_id, float value);
    void inline _setCurrentStateParam(ParamID param_id, float value);
    void inline _commonParamUpdate(ParamID param_id, float value);
    void inline _stateParamUpdate(ParamID param_id, float value);
    static void _setNumUnison(float value, Layer &layer);
    static void _setPanNum(float value, Layer &layer);
    static void _setPanMode(float value, Layer &layer);
    static void _setVoiceMode(float value, Layer &layer);

    static void _miscScaleThing(float value, Layer &layer) {
        layer._misc_scale = value;
    }

    static void _mpeMode(float value, Layer &layer);

    static void _mpeFallY(float value, Layer &layer) { layer.setMpeYFall(value); }

    static void _mpeFallZ(float value, Layer &layer) { layer.setMpeZFall(value); }

    static void _mpeChannels(float value, Layer &layer);
    static float inline _transformFineTune(float value, Layer &layer);

    static float inline _transformOsc3LowMode(float value, Layer &layer) {
        layer.setOsc3Slow(value > PARAM_BOOL_FLOAT_COMP);
        return 0;
    }

    static float inline _transformFineTuneRange(float value, Layer &layer);
    static float inline _vcaUnipolarTransform(float value, Layer &layer);
    static void inline _odTransform(float value, Layer &layer);
    static void inline _midiLowNoteFilterTransform(float value, Layer &layer);
    static void inline _lfo1TempoSyncTransform(float value, Layer &layer);
    static void inline _lfo2TempoSyncTransform(float value, Layer &layer);

    static void inline _outputRoutingTransform(float value, Layer &layer) {
        NinaParams::OutputRouting mode = (NinaParams::OutputRouting)(std::round(value * (float)NinaParams::OutputRouting::numModes));
        layer.setOutputMode(mode);
    }

    static void inline _midiHighNoteFilterTransform(float value, Layer &layer);
    static void inline _midiChannelNoteFilterTransform(float value, Layer &layer);

    static void inline _midiSourceVelMode(float value, Layer &layer) {
        layer.setMidiSourceVelMode(value > PARAM_BOOL_FLOAT_COMP);
    }

    static void inline _inputModeTransform(float value, Layer &layer);
    static void inline _defaultParamUpdate(float value, Layer &layer);
    static void inline _updateCvA(float value, Layer &layer);
    static void inline _updateCvB(float value, Layer &layer);
    static void inline _wtSelectParamUpdate(float value, Layer &layer);
    static float inline _defaultSmooth(float value, float &prev_value, Layer &layer);
    static float inline _noSmooth(float value, float &prev_value, Layer &layer);
    static float inline _noSmoothCommon(float value, float &prev_value, Layer &layer);

    // Common smooth function with bipolar transformation and no actual smoothing
    static float inline _noSmoothBipolarCommon(float value, float &prev_value, Layer &layer) {
        prev_value = value * 2 - 1;
        return 0;
    }

    static float inline _noSmoothBipolarCommon2xGain(float value, float &prev_value, Layer &layer) {
        prev_value = 2 * (value * 2 - 1);
        return 0;
    }

    static float inline _defaultTransform(float value, Layer &layer);
    static float inline _bipolarTransform(float value, Layer &layer);
    static float inline _unipolarTransform(float value, Layer &layer);
    static float inline _logPotTransform(float value, Layer &layer);
    static float inline _xSquaredTransform(float value, Layer &layer);

    static float inline _xAbsXTransform(float value, Layer &layer) {
        value = (value - .5) * 2;
        return value * std::abs(value);
    };

    static float inline _xorModeTransform(float value, Layer &layer);
    static float inline _widthTransform(float value, Layer &layer);
    /**
     * @brief Always return a zero value
     *
     * @param value
     * @param layer
     * @return float 0.0f
     */
    static float inline _disableTransform(float value, Layer &layer);

    /**
     * @brief scale the key pitch offset value such that 0 -> midi note 0 and 1 -> midi note 127 but in the internal CV scale (+1.0 8va)
     *
     * @param value
     * @param layer
     * @return float
     */
    static float inline _keyPitchOffsetTransform(float value, Layer &layer);

    static void inline _triggerPrint(float value, Layer &layer) {
        layer.debugPrint();
        printf("\ntrigger print");
    }

    static void inline _midiPitchWheel(float value, Layer &layer) {
        float pitch_bend_range = layer._layer_params.common_params.at(LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::PitchBendRange));
        pitch_bend_range = std::round((13.f) * pitch_bend_range) / 12.f;
        float gain = 2.f * pitch_bend_range / noteGain;
        float wheel = layer._layer_params.common_params.at(LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::MidiPitchBend));
        float offset = -(gain / 2.f);
        float num = wheel * gain + offset;
        layer._layer_params.pitchbend = num;
    }

    static void inline _glideMode(float value, Layer &layer) {
        GlideModes mode = (GlideModes)(int)(value * (float)NUM_GLIDE_MODES);
        layer._layer_params.glide_mode = mode;
    }

    static void inline _midiAftertouch(float value, Layer &layer) {
        for (auto &voice : layer._layer_voices) {
            voice.setAftertouch(value);
        }
    }

    static float inline _osc1TuneSum(float value, Layer &layer);
    static float inline _osc2TuneSum(float value, Layer &layer);
    static float inline _osc3TuneSum(float value, Layer &layer);
    static float inline _currentMorphValue(float value, Layer &layer);
    static float inline _transformCoarseTune(float value, Layer &layer);
    static float inline _transformSemitoneTune(float value, Layer &layer);
    static float inline _defaultMorph(uint index, bool morphing, Layer &layer);
    static float inline _lfoMorph(uint index, bool morphing, Layer &layer);
    static float inline _nullMorph(uint index, bool morphing, Layer &layer);
    static float inline _glideTransform(float value, Layer &layer);

    static float inline _SlewTransform(float value, Layer &layer) {
        static constexpr float slew_freq_gain = 13.4;
        static constexpr float slew_freq_offset = 0.1;
        value = exp2f32(slew_freq_gain * value + slew_freq_offset) / (float)CV_SAMPLE_RATE;
        return value;
    }

    static void inline _vcaDroneTransform(float value, Layer &layer);

    static float inline _spinResetTransform(float value, Layer &layer) {
        bool val = value > PARAM_BOOL_FLOAT_COMP;
        layer._layer_params.spin_reset = val;

        return value;
    }

    static float inline _LfoResetTransform(float value, Layer &layer) {
        int select = std::round(3 * value);
        switch (select) {
        case 0:

            layer._layer_params.lfo_reset = false;
            layer._layer_params.lfo_1_global = false;
            break;

        case 1:

            layer._layer_params.lfo_reset = true;
            layer._layer_params.lfo_1_global = false;
            break;
        case 2:

            layer._layer_params.lfo_reset = false;
            layer._layer_params.lfo_1_global = true;
            break;
        default:
            break;
        }
        return value;
    }

    static float inline _VcfModeTransform(float value, Layer &layer) {
        layer.setVcfMode(value > PARAM_BOOL_FLOAT_COMP);
        return value;
    }

    static float inline _Lfo2ResetTransform(float value, Layer &layer) {
        int select = std::round(3 * value);
        switch (select) {
        case 0:

            layer._layer_params.lfo_2_reset = false;
            layer._layer_params.lfo_2_global = false;
            break;

        case 1:

            layer._layer_params.lfo_2_reset = true;
            layer._layer_params.lfo_2_global = false;
            break;
        case 2:

            layer._layer_params.lfo_2_reset = false;
            layer._layer_params.lfo_2_global = true;
            break;
        default:
            break;
        }
        return value;
    }

    static float inline _AmpEnvResetTransform(float value, Layer &layer) {
        layer._layer_params.amp_env_reset = value > PARAM_BOOL_FLOAT_COMP;
        return value;
    }

    static void inline _AllNotesOff(float value, Layer &layer) {
        if (value > 0.5) {
            layer.allNotesOff();
        }
    }

    static float inline _FiltEnvResetTransform(float value, Layer &layer) {
        layer._layer_params.filter_env_reset = value > PARAM_BOOL_FLOAT_COMP;
        return value;
    }

    static void inline _wtInterpolateMode(float value, Layer &layer);
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
