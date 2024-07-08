/**
 * @file NinaVoice.h
 * @brief definition of the analog voice
 * @date 2022-07-28
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */
#pragma once

#include "AnalogVoice.h"
#include "NinaDriveCompensator.h"
#include "NinaEnvelope.h"
#include "NinaLfo.h"
#include "NinaMatrix.h"
#include "NinaOscMix.h"
#include "NinaOutputPanner.h"
#include "NinaParameters.h"
#include "NinaXorMix.h"
#include "NoiseOscillator.h"
#include "WavetableOsc.h"
#include "common.h"
#include <array>

namespace Steinberg {
namespace Vst {
namespace Nina {

constexpr float MAX_TIME_VALUE = 2.0;

float inline _logPotTransform(float value) {
    constexpr float log_pot_slope = 3.f;
    constexpr float zero_offset = std::exp2(0.f);
    constexpr float max_level_gain = 1.f / (std::exp2(log_pot_slope * 1.f) - zero_offset);

    const float val = (fastpow2(log_pot_slope * value) - zero_offset) * max_level_gain;
    return val;
}

float inline _xSquaredTransform(float value) {
    return (value * value) * 2 - 1;
}

float inline _xAbsXTransform(float value) {
    value = (value - .5) * 2;
    return value * std::abs(value);
};

class LayerVoice {

  public:
    LayerVoice(VoiceInput &analog_input,
        NinaParams::LayerParams &layer_params,
        NinaParams::LayerStateParams &layer_a_params,
        NinaParams::LayerStateParams &layer_b_params,
        WavetableLoader &wt_loader_a,
        WavetableLoader &wt_loader_b,
        std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> &high_res_output,
        std::array<float, BUFFER_SIZE> *&high_res_input,
        uint voice_num);
    ~LayerVoice() = default;

    void setMpeChannelData(NinaParams::MpeChannelData *mpe_channel_data) {
        _mpe_data = mpe_channel_data;

        // reset the data post smoothing to avoid lag on mpe reallocation
        _mpe_data_smooth.mpe_x = _mpe_data->mpe_x;
        _mpe_data_smooth.mpe_y = _mpe_data->mpe_y;
        _mpe_data_smooth.mpe_z = _mpe_data->mpe_z;

        // reset the fall parameters on mpe channel reallocation
        _mpe_z_fall_value = _mpe_data_smooth.mpe_z;
        if (_mpe_y_bipolar) {
            _mpe_y_bipolar_value = (midiSourceScaleBipolar(_mpe_data_smooth.mpe_y));
        } else {
            _mpe_y_bipolar_value = (midiSourceScaleUnipolar(_mpe_data_smooth.mpe_y));
        }
    }

    void runWt() {
        _wt_osc.reCalculate();
    }

    void allocate(MidiNote note, float unison_pan, float unison_detune);
    void allocate(MidiNote note);
    void free();
    void free(float vel);
    bool allocated();
    bool blacklisted();

    void setBlacklisted(bool bl) {
        _blacklisted = bl;
    }

    void setLegato(bool leg) {
        _legato = leg;
    }

    void enterReleaseState();
    void setFreq(const MidiNote &note);
    void retrigger();
    const MidiNote *getMidiNote();
    void run();

    void setLastAllocated(bool last) {
        _analog_input._last_allocated = last;
    }

    /**
     * @brief Set the Aftertouch value, its per voice so we can add poly-AT
     *
     * @param aftertouch
     */
    inline void setAftertouch(float aftertouch) {
        _aftertouch = aftertouch * AFTERTOUCH_MAG;
    }

    void resetAT() {
        _aftertouch = 0.f;
    }

    void printstuff() {
        float num = _analog_input.osc_1_shape_in_n;
        printf("\nfc: %f %f %f", _key_offset, _keyboard_pitch, (midiNoteToCv(_midi_note.pitch)));
        _wt_osc.printstuff();
        _panner.dump = true;
        _amp_env.print = true;
        _osc_mixer_1.print();
        _osc_mixer_2.print();
        _matrix.print = true;
        _drive_compensator.print();
        _xor_mixer.print();
        dump2 = true;
        _lfo_1.dump = true;
        _lfo_2.dump = true;
    }

    bool dump2 = false;

    void dump() {
        _amp_env.dump = true;
        dump2 = true;
        _panner.dump = true;
        _matrix.dump = true;
    }

    // Set the current voice to be the source of the modulated morph value
    void setMorphModVoice(bool mod_voice) {
        _morph_mod_voice = mod_voice;
    }

    inline bool pendingRel() { return _pending_rel; };

  private:
    NinaParams::MpeChannelData _mpe_data_smooth;
    NinaParams::MpeChannelData *_mpe_data = &_mpe_data_smooth;
    float _release_vel = 0.5;
    float _midi_source = 0;
    bool &_mpe_mode_en;
    bool &_mpe_y_bipolar;
    const uint _voice_num;
    float _keyboard_pitch = 0;
    float _keyboard_pitch_glide = 0;
    float _keyboard_pitch_glide_pitchwheel = 0;
    float &_glide_rate;
    GlideModes &_glide_mode;
    float &_pitchbend;
    NinaParams::LayerParams &_layer_params;
    NinaParams::LayerStateParams _local_morphed_state_params;
    bool _morph_mod_voice = false;
    float &_mpe_fall_y = _layer_params.mpe_y_fall_coeff;
    float &_mpe_fall_z = _layer_params.mpe_z_fall_coeff;
    float &_master_detune = _layer_params.master_detune;
    float &_mpe_pitchbend_gain = _layer_params.mpe_pitchbend_gain;
    float _mpe_y_bipolar_value = 0;
    float _mpe_z_fall_value = 0;
    float &_mpe_pitchbend_offset = _layer_params.mpe_pitchbend_offset;
    bool &_release_vel_mode = _layer_params.release_vel_mode;
    float &_patch_volume = _layer_params.common_params.at(LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::PatchVolume));
    bool &_lfo_1_global = _layer_params.lfo_1_global;
    bool &_lfo_2_global = _layer_params.lfo_2_global;

    float _unison_detune;
    float _constant = 1.0f;
    float _key_velocity = .0f;
    float _pan_position = 0.0f;
    float _aftertouch = 0.0f;
    float _aftertouch_smooth = 0;
    float &_sustain_pedal;
    float time_inc = 0.0001;
    float _time;
    float &_morph_knob_value;
    float _morph_value;
    float &_amp_vel_sense;
    float &_filt_vel_sense;
    float &_key_offset;
    float &_lfo_slew;
    float &_lfo_2_slew;
    float _zero = 0.f;
    float &_octave_offset = _layer_params.common_params.at(LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::octaveOffset));
    float &_sub_osc;
    float &_sync_osc;
    float &_f_overdrive;
    bool &_overdrive;
    float &_overdrive_comp;
    float &_overdrive_mix_comp;
    float &misc_scale;
    bool _voice_trigger = true;
    bool &_spin_reset;
    bool &_lfo_reset;
    bool &_lfo_2_reset;
    bool &_amp_env_reset;
    bool &_filt_env_reset;
    float noise_osc1 = 0, noise_osc2 = 0;
    float &_unison_detune_amount;
    bool f = true;
    bool _false = false;
    float &_compression_signal;
    float &_modwheel;
    float _max_mix_out_level = 0;
    bool &_vca_drone;
    float &_global_tempo;
    bool &_filter_2_pole_mode = _layer_params.filter_2_pole_mode;
    float &_lfo_1_global_rate = _local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo1Rate)));
    float &_lfo_2_global_rate = _local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo2Rate)));
    float &_lfo_1_global_phase;
    float &_lfo_2_global_phase;
    bool &_mute_1 = _layer_params.mute_out_1;
    bool &_mute_2 = _layer_params.mute_out_2;
    bool &_mute_3 = _layer_params.mute_out_3;
    bool &_mute_4 = _layer_params.mute_out_4;
    DriveCompensator _drive_compensator = DriveCompensator(_overdrive, _compression_signal, _max_mix_out_level, _patch_volume);
    NinaParams::LayerStateParams &_state_a;
    NinaParams::LayerStateParams &_state_b;
    float &_lfo_1_shape_a = _state_a.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::LfoShape));
    float &_lfo_1_shape_b = _state_b.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::LfoShape));
    float &_lfo_2_shape_a = _state_a.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Lfo2Shape));
    float &_lfo_2_shape_b = _state_b.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Lfo2Shape));
    NinaLfo _lfo_1 = NinaLfo(_lfo_1_shape_a, _lfo_1_shape_b, _morph_value, _lfo_slew, _voice_trigger, _lfo_reset, _global_tempo, _layer_params.lfo_1_tempo_sync, _layer_params.lfo_1_global, _lfo_1_global_phase, _voice_num == 0);
    NinaLfo _lfo_2 = NinaLfo(_lfo_2_shape_a, _lfo_2_shape_b, _morph_value, _lfo_2_slew, _voice_trigger, _lfo_2_reset, _global_tempo, _layer_params.lfo_2_tempo_sync, _layer_params.lfo_2_global, _lfo_2_global_phase, _voice_num == 0);
    GenAdsrEnvelope _amp_env = GenAdsrEnvelope(_key_velocity, _amp_vel_sense, misc_scale, _amp_env_reset, _vca_drone);
    GenAdsrEnvelope _filt_env = GenAdsrEnvelope(_key_velocity, _filt_vel_sense, misc_scale, _filt_env_reset, _false);
    VoiceInput &_analog_input;
    OscMixer _osc_mixer_1 = OscMixer(_analog_input.sqr_1_lev_in, _analog_input.tri_1_lev_in, _drive_compensator.getMixerVcaCompensation());
    OscMixer _osc_mixer_2 = OscMixer(_analog_input.sqr_2_lev_in, _analog_input.tri_2_lev_in, _drive_compensator.getMixerVcaCompensation());
    WavetableOsc _wt_osc;
    NoiseOscillator _noise_osc;
    NinaOutPanner _panner = NinaOutPanner(_analog_input.vca_l_in_n, _analog_input.vca_r_in_n, _drive_compensator.getMixbusCompensation(), _max_mix_out_level);
    std::array<float, BUFFER_SIZE> *&_wt_output;
    std::array<float, BUFFER_SIZE> *&_aux_input;
    float &_aux_in_gain;
    NoiseXorMix _xor_mixer = NoiseXorMix(_analog_input.xor_lev_in, _layer_params.xor_mode, _aux_input, _wt_output, _drive_compensator.getMixerVcaCompensation(), _aux_in_gain);
    float &_time_rate;
    float _cv_a_val = 0;
    float _cv_b_val = 0;
    bool _allocated = false;
    bool _blacklisted = false;
    bool _pending_rel = false;
    bool _legato = false;
    float _midi_expression = 0;
    float _voice_morph_value = 0;

    MidiNote _midi_note;

    std::array<float *, NinaParams::NumDsts * NinaParams::NumSrcs> _setupLayerParams(NinaParams::LayerParams &layer_params, NinaParams::LayerStateParams &sp);

    std::array<float *, NinaParams::NumDsts * NinaParams::NumSrcs> _getGainAddr() {
        return _gains;
    }

    inline void _updateLocalParams() {

        // voice morph position is layer morph pos + voice mod morph value (which is scaled to match the 0->1 range of morph)

        const float val = (_voice_morph_value) / 2.f;
        const float mod_pos = unipolarClip(_layer_params.morph_pos + val);
        _morph_value = mod_pos;
        const float a = (1 - mod_pos);
        const float b = mod_pos;
        const auto &a_p = _state_a;
        const auto &b_p = _state_b;
        for (int i = 0; i < NinaParams::NUM_LAYER_STATE_PARAMS; i++) {

            float tmp = a * (a_p[i]) + b * (b_p[i]);

            switch (i) {
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::Osc1Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo2, NinaParams::Osc1Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::Osc1Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Wavetable, NinaParams::Osc1Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Time, NinaParams::Osc1Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::Aftertouch, NinaParams::Osc1Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyVelocity, NinaParams::Osc1Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::PanPosition, NinaParams::Osc1Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::FilterEnvelope, NinaParams::Osc1Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::Osc2Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo2, NinaParams::Osc2Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::Osc2Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Wavetable, NinaParams::Osc2Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Time, NinaParams::Osc2Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::Aftertouch, NinaParams::Osc2Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyVelocity, NinaParams::Osc2Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::PanPosition, NinaParams::Osc2Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::FilterEnvelope, NinaParams::Osc2Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::Osc3Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo2, NinaParams::Osc3Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::Osc3Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Wavetable, NinaParams::Osc3Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Time, NinaParams::Osc3Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::Aftertouch, NinaParams::Osc3Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyVelocity, NinaParams::Osc3Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::PanPosition, NinaParams::Osc3Pitch)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::FilterEnvelope, NinaParams::Osc3Pitch)):

                tmp = _xAbsXTransform(tmp);
                break;

            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Level)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc2Level)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc3Level)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::XorLevel)):

                tmp = _xSquaredTransform(tmp);
                break;

            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Width)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo2Gain)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo1Gain)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc2Width)):
            case LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterResonance)):

                tmp = _logPotTransform(tmp);
                break;
            default:
                break;
            }

            _local_morphed_state_params[i] = tmp;
        }

        // update the tune values of each oscillator
        constexpr float gain = 5.f;
        constexpr float offset = 0;
        float osc_tune_1 = ((std::floor((_local_morphed_state_params[LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Vco1TuneCoarse)] + 0.1) * gain) + offset) / noteGain) +
                           _local_morphed_state_params[LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Vco1TuneFine)] +
                           -_local_morphed_state_params[LAYER_STATE_PARAMID_TO_INDEX(NinaParams::KeyPitchOffset)];
        _local_morphed_state_params[LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Pitch))] = osc_tune_1;
        float osc_tune_2 = ((std::floor((_local_morphed_state_params[LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Vco2TuneCoarse)] + 0.1) * gain) + offset) / noteGain) +
                           _local_morphed_state_params[LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Vco2TuneFine)] +
                           -_local_morphed_state_params[LAYER_STATE_PARAMID_TO_INDEX(NinaParams::KeyPitchOffset)];
        _local_morphed_state_params[LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc2Pitch))] = osc_tune_2;
        float osc_tune_3 = ((std::floor((_local_morphed_state_params[LAYER_STATE_PARAMID_TO_INDEX(NinaParams::WaveTuneCoarse)] + 0.1) * gain) + offset) / noteGain) +
                           _local_morphed_state_params[LAYER_STATE_PARAMID_TO_INDEX(NinaParams::WaveTuneFine)] +
                           -_local_morphed_state_params[LAYER_STATE_PARAMID_TO_INDEX(NinaParams::KeyPitchOffset)];
        _local_morphed_state_params[LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc3Pitch))] = osc_tune_3;

        if (_layer_params.lfo_1_tempo_sync) {
            _local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo1Rate))) = _local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Lfo1SyncRate));
        }
        if (_layer_params.lfo_2_tempo_sync) {
            _local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo2Rate))) = _local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Lfo2SyncRate));
        }
    }

    std::array<float *, NinaParams::NumDsts> _getDsts() { return _dsts; };

    std::array<float *, NinaParams::NumSrcs> _getSrcs() {
        return _srcs;
    };

    /**
     * @brief in this function, we calculate and initialise all the matrix variables
     *
     * @return std::array<float*, NinaParams::NUM_FAST_DST>
     */
    std::array<float *, NinaParams::NUM_FAST_DST> _getFastDsts() {

        for (auto &item : _src_is_fast) {
            item = false;
        }
        for (auto &item : _dst_is_fast) {
            item = false;
        }
        auto fast_src_it = _src_is_fast.begin();
        auto fast_dst_it = _dst_is_fast.begin();

        // find the address of all the fast sources in order and add them to the fast gains array
        // add fast sources to matrix and to fast matrix src array
        _srcs.at(NinaParams::ModMatrixSrc::Lfo1) = _lfo_1.getOutput();
        _src_is_fast.at(NinaParams::ModMatrixSrc::Lfo1) = true;
        _lfo_1.getOutput();
        _srcs.at(NinaParams::ModMatrixSrc::Lfo2) = _lfo_2.getOutput();
        _src_is_fast.at(NinaParams::ModMatrixSrc::Lfo2) = true;
        _lfo_2.getOutput();
        _srcs.at(NinaParams::ModMatrixSrc::AmpEnvelope) = _amp_env.getOutput();
        _src_is_fast.at(NinaParams::ModMatrixSrc::AmpEnvelope) = true;
        _amp_env.getOutput();
        _srcs.at(NinaParams::ModMatrixSrc::FilterEnvelope) = _filt_env.getOutput();
        _src_is_fast.at(NinaParams::ModMatrixSrc::FilterEnvelope) = true;
        _srcs.at(NinaParams::ModMatrixSrc::Wavetable) = _wt_osc.getWtCvOut();
        _src_is_fast.at(NinaParams::ModMatrixSrc::Wavetable) = true;

        // add slow sources
        _srcs.at(NinaParams::ModMatrixSrc::KeyPitch) = &_keyboard_pitch_glide_pitchwheel;
        _srcs.at(NinaParams::ModMatrixSrc::KeyVelocity) = &_key_velocity;
        _srcs.at(NinaParams::ModMatrixSrc::Aftertouch) = &_aftertouch_smooth;
        _srcs.at(NinaParams::ModMatrixSrc::PanPosition) = &_pan_position;
        _srcs.at(NinaParams::ModMatrixSrc::Time) = &_time;
        _srcs.at(NinaParams::ModMatrixSrc::Constant) = &_constant;
        _srcs.at(NinaParams::ModMatrixSrc::Offset) = &_constant;
        _srcs.at(NinaParams::ModMatrixSrc::Midi) = &_midi_source;
        _srcs.at(NinaParams::ModMatrixSrc::CVA) = &_cv_a_val;
        _srcs.at(NinaParams::ModMatrixSrc::CVB) = &_cv_b_val;
        _srcs.at(NinaParams::ModMatrixSrc::Modwheel) = &_modwheel;
        _srcs.at(NinaParams::ModMatrixSrc::Expression) = &_midi_expression;

        // add dsts
        _dsts.at(NinaParams::ModMatrixDst::Morph) = &_voice_morph_value;
        _dsts.at(NinaParams::ModMatrixDst::Lfo1Gain) = _lfo_1.getGainIn();
        _dst_is_fast.at(NinaParams::ModMatrixDst::Lfo1Gain) = true;
        _dsts.at(NinaParams::ModMatrixDst::Lfo1Rate) = _lfo_1.getPitchIn();
        _dsts.at(NinaParams::ModMatrixDst::Lfo2Gain) = _lfo_2.getGainIn();
        _dst_is_fast.at(NinaParams::ModMatrixDst::Lfo2Gain) = true;
        _dsts.at(NinaParams::ModMatrixDst::Lfo2Rate) = _lfo_2.getPitchIn();

        _dsts.at(NinaParams::ModMatrixDst::Osc1Pitch) = _analog_input.osc_1_freq_in;
        _dst_is_fast.at(NinaParams::ModMatrixDst::Osc1Pitch) = true;
        _dsts.at(NinaParams::ModMatrixDst::Osc1Width) = _analog_input.osc_1_shape_in;
        _dst_is_fast.at(NinaParams::ModMatrixDst::Osc1Width) = true;
        _dsts.at(NinaParams::ModMatrixDst::Osc2Pitch) = _analog_input.osc_2_freq_in;
        _dst_is_fast.at(NinaParams::ModMatrixDst::Osc2Pitch) = true;
        _dsts.at(NinaParams::ModMatrixDst::Osc2Width) = _analog_input.osc_2_shape_in;
        _dst_is_fast.at(NinaParams::ModMatrixDst::Osc2Width) = true;

        _dsts.at(NinaParams::ModMatrixDst::Osc1Blend) = _osc_mixer_1.getBlendInput();
        _dst_is_fast.at(NinaParams::ModMatrixDst::Osc1Blend) = true;
        _dsts.at(NinaParams::ModMatrixDst::Osc1Level) = _osc_mixer_1.getGainInput();
        _dst_is_fast.at(NinaParams::ModMatrixDst::Osc1Level) = true;
        _dsts.at(NinaParams::ModMatrixDst::Osc2Blend) = _osc_mixer_2.getBlendInput();
        _dst_is_fast.at(NinaParams::ModMatrixDst::Osc2Blend) = true;
        _dsts.at(NinaParams::ModMatrixDst::Osc2Level) = _osc_mixer_2.getGainInput();
        _dst_is_fast.at(NinaParams::ModMatrixDst::Osc2Level) = true;
        _dsts.at(NinaParams::ModMatrixDst::XorLevel) = _xor_mixer.getGainInput();
        _dst_is_fast.at(NinaParams::ModMatrixDst::XorLevel) = true;
        _dsts.at(NinaParams::ModMatrixDst::VcaIn) = _panner.getVcaIn();
        _dst_is_fast.at(NinaParams::ModMatrixDst::VcaIn) = true;

        _dsts.at(NinaParams::ModMatrixDst::FilterCutoff) = _analog_input.filt_cut_in;
        _dst_is_fast.at(NinaParams::ModMatrixDst::FilterCutoff) = true;

        _dsts.at(NinaParams::ModMatrixDst::FilterResonance) = _analog_input.filt_res_in;
        _dst_is_fast.at(NinaParams::ModMatrixDst::FilterResonance) = true;
        _dsts.at(NinaParams::ModMatrixDst::Osc3Pitch) = _wt_osc.getWtPitch();
        _dst_is_fast.at(NinaParams::ModMatrixDst::Osc3Pitch) = true;
        _dsts.at(NinaParams::ModMatrixDst::Osc3Level) = _wt_osc.getWtVol();
        _dst_is_fast.at(NinaParams::ModMatrixDst::Osc3Level) = true;
        _dsts.at(NinaParams::ModMatrixDst::Osc3Shape) = _wt_osc.getWtPosition();
        _dst_is_fast.at(NinaParams::ModMatrixDst::Osc3Shape) = true;
        _dsts.at(NinaParams::ModMatrixDst::Drive) = _drive_compensator.getdriveInput();

        _dsts.at(NinaParams::ModMatrixDst::AmpEnvelopeAtt) = _amp_env.getAttIn();
        _dsts.at(NinaParams::ModMatrixDst::AmpEnvelopeDec) = _amp_env.getDecIn();
        _dsts.at(NinaParams::ModMatrixDst::AmpEnvelopeSus) = _amp_env.getSusIn();
        _dsts.at(NinaParams::ModMatrixDst::AmpEnvelopeRel) = _amp_env.getRelIn();
        _dsts.at(NinaParams::ModMatrixDst::AmpEnvelopeLevel) = _amp_env.getGainIn();

        _dsts.at(NinaParams::ModMatrixDst::FilterEnvelopeAtt) = _filt_env.getAttIn();
        _dsts.at(NinaParams::ModMatrixDst::FilterEnvelopeDec) = _filt_env.getDecIn();
        _dsts.at(NinaParams::ModMatrixDst::FilterEnvelopeSus) = _filt_env.getSusIn();
        _dsts.at(NinaParams::ModMatrixDst::FilterEnvelopeRel) = _filt_env.getRelIn();
        _dsts.at(NinaParams::ModMatrixDst::FilterEnvelopeLevel) = _filt_env.getGainIn();

        _dsts.at(NinaParams::ModMatrixDst::Pan) = _panner.getPanIn();
        _dsts.at(NinaParams::ModMatrixDst::Spin) = _panner.getSpinIn();

        // add the fast dsts to the fast dst list. by doign it this way, they are in the same order that they are in the dst/src array
        int f_c = 0;
        for (int i = 0; i < NinaParams::NumDsts; i++) {
            if (_dst_is_fast.at(i)) {
                _fast_dst.at(f_c) = _dsts.at(i);
                f_c++;
            }
        }
        f_c = 0;
        for (int i = 0; i < NinaParams::NumSrcs; i++) {
            if (_src_is_fast.at(i)) {
                _fast_src.at(f_c) = _srcs.at(i);
                f_c++;
            }
        }
        auto f_gain_it = _fast_gains.begin();
        for (auto &f_source : _fast_src) {
            uint src_i = std::distance(_srcs.begin(), std::find(_srcs.begin(), _srcs.end(), f_source));
            for (auto &f_dst : _fast_dst) {
                uint dst_i = std::distance(_dsts.begin(), std::find(_dsts.begin(), _dsts.end(), f_dst));
                *f_gain_it = _gains.at((src_i)*NinaParams::NumDsts + (dst_i));
                _gains.at((src_i)*NinaParams::NumDsts + (dst_i)) = &_zero;
                f_gain_it++;
            }
        }
        return _fast_dst;
    }

    std::array<float *, NinaParams::NUM_FAST_SRC> _getFastSrcs() {
        return _fast_src;
    }

    std::array<float *, NinaParams::NUM_FAST_SRC * NinaParams::NUM_FAST_DST> _getFastGains() {
        return _fast_gains;
    }

    std::array<float *, NinaParams::NumDsts * NinaParams::NumSrcs> _gains;
    std::array<float *, NinaParams::NUM_FAST_DST *NinaParams::NUM_FAST_SRC> _fast_gains = {NULL};
    std::array<float *, NinaParams::NUM_FAST_SRC> _fast_src;
    std::array<float *, NinaParams::NUM_FAST_DST> _fast_dst;
    std::array<float *, NinaParams::NumSrcs> _srcs;
    std::array<float *, NinaParams::NumDsts> _dsts;
    std::array<bool, NinaParams::NumSrcs> _src_is_fast = {false};
    std::array<bool, NinaParams::NumDsts> _dst_is_fast = {false};
    NinaMatrix _matrix = {_fast_dst, _fast_src, _fast_gains, _dsts, _srcs, _gains};

    float _dummydst = 0;
    float _dummysrc = 0;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
