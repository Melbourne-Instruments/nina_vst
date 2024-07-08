/**
 * @file NinaVoice.cpp
 * @brief Implementation of a Layer Voice.
 *
 * @copyright Copyright (c) 2023 Melbourne Instruments, Australia
 */
#include "NinaVoice.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

LayerVoice::LayerVoice(VoiceInput &analog_input,
    NinaParams::LayerParams &layer_params,
    NinaParams::LayerStateParams &layer_a_params,
    NinaParams::LayerStateParams &layer_b_params,
    WavetableLoader &wt_loader_a,
    WavetableLoader &wt_loader_b,
    std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> &high_res_output,
    std::array<float, BUFFER_SIZE> *&high_res_input,
    uint voice_num) :
    _layer_params(layer_params),
    _analog_input(analog_input),
    _mpe_mode_en(layer_params.mpe_mode_en),
    _voice_num(voice_num),
    _lfo_1_global_phase(layer_params.global_lfo_1_phase),
    _lfo_2_global_phase(layer_params.global_lfo_2_phase),
    _sustain_pedal(*NinaParams::getLayerCommonParamAddr(NinaParams::Sustain, layer_params.common_params)),
    _fast_dst(_getFastDsts()),
    _state_a(layer_a_params),

    _state_b(layer_b_params),
    _morph_knob_value(layer_params.morph_pos),
    _wt_output(high_res_output.at(voice_num)),
    _aux_input(high_res_input),
    _wt_osc(wt_loader_a, wt_loader_b, _morph_value, high_res_output.at(voice_num), _drive_compensator.getMixerVcaCompensation(), layer_params.wave_interpolate, layer_params.osc_3_slow_mode),
    _amp_vel_sense(_local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::AmpEnvVelSense))),
    _filt_vel_sense(_local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::FiltEnvVelSense))),
    _key_offset(_local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::KeyPitchOffset))),
    _gains(_setupLayerParams(layer_params, _local_morphed_state_params)),
    _sub_osc(_local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::SubOscillator))),
    _sync_osc(_local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::HardSync))),
    _f_overdrive(_local_morphed_state_params.at(LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::VcfOverdrive))),
    misc_scale(layer_params.common_params.at(LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::MiscScale))),
    _time_rate(layer_params.common_params.at(LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::TimeRate))),
    _glide_rate(_local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Glide))),
    _pitchbend(layer_params.pitchbend),
    _lfo_slew(_local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::LfoSlew))),
    _lfo_2_slew(_local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Lfo2Slew))),
    _unison_detune_amount(_local_morphed_state_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::UnisonSpread))),
    _overdrive(layer_params.overDrive),
    _modwheel(layer_params.common_params.at(LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::MidiModWheel))),
    _overdrive_comp(layer_params.drive_compensation),
    _overdrive_mix_comp(layer_params.drive_factor),
    _spin_reset(layer_params.spin_reset),
    _compression_signal(layer_params.compression_signal),
    _lfo_reset(layer_params.lfo_reset),
    _lfo_2_reset(layer_params.lfo_2_reset),
    _amp_env_reset(layer_params.amp_env_reset),
    _glide_mode(layer_params.glide_mode),
    _vca_drone(layer_params.vca_drone),
    _filt_env_reset(layer_params.filter_env_reset),
    _aux_in_gain(layer_params.common_params.at(LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::extInGain))),
    _global_tempo(layer_params.global_tempo),
    _mpe_y_bipolar(layer_params.mpe_y_bipolar) {

    _matrix.voicen = _voice_num;
}

void LayerVoice::allocate(MidiNote note, float unison_pan, float unison_detune) {
    _unison_detune = unison_detune;
    _pan_position = unison_pan;
    LayerVoice::allocate(note);
}

void LayerVoice::allocate(MidiNote note) {
    {
        _pending_rel = false;
        if (!_amp_env.inReleaseState() && _legato && (note.pitch != _midi_note.pitch)) {

        } else {

            _time = 0;
            _amp_env.trigger();
            _filt_env.trigger();
            _voice_trigger = true;
            if (_spin_reset) {
                _panner.resetSpin();
            }
            // if portamento glide is enabled, and we are currently in the release state, then we reset the glide filter to stop the voice from gliding
            if (_amp_env.inReleaseState() && _glide_mode == GlideModes::PORTAMENTO_LINEAR) {
                _keyboard_pitch_glide = (midiNoteToCv(note.pitch) / noteGain) + _key_offset;
            }
        }

        _amp_env.gateOn();
        _filt_env.gateOn();
        //_lfo_1.zeroPhase();

        // key pitch offset is centered around the value key offset
        _keyboard_pitch = (midiNoteToCv(note.pitch) / noteGain) + _key_offset;

        // set other values
        _midi_note = note;
        _key_velocity = note.velocity * 2;

        // reset release velocity mod on noteon
        _release_vel = MID_VEL;
    }
}

void LayerVoice::setFreq(const MidiNote &note) {
}

void LayerVoice::free() {
    // Free this voice
    _pending_rel = true;
}

void LayerVoice::free(float vel) {
    // Free this voice
    _pending_rel = true;
    _release_vel = vel;
}

bool LayerVoice::allocated() {
    // If this voice is allocated and the amp ADSR envelope is
    // idle, the voice is free
    if (_amp_env.inReleaseState()) {
        return false;
    }

    return true;
}

bool LayerVoice::blacklisted() {
    // Return whether this voices is blacklisted or not
    return _blacklisted;
}

void LayerVoice::enterReleaseState() {
    // Enter the release state for all envelopes
    _amp_env.gateOff();
    _filt_env.gateOff();
    _amp_env.forceReset();
    // printf("release voice %d", _voice_num);
}

void LayerVoice::retrigger() {
    // Re-trigger all envelopes
    _amp_env.trigger();
    _filt_env.trigger();
    _voice_trigger = true;
}

const MidiNote *LayerVoice::getMidiNote() {
    // If the voice is allocated
    // Return the MIDI note
    return &_midi_note;
}

void LayerVoice::run() {
    _updateLocalParams();
    float octave_offset = ((std::round(_octave_offset * 11.f) - 5.f)) / noteGain;
    const float time_val = _time_rate * _time_rate * _time_rate;
    time_inc = (1 - fastpow2(-(1.4 / ((float)BUFFER_RATE * 5. * time_val))));
    _time = _time + (MAX_TIME_VALUE - _time) * .01 * time_inc;

    if (_glide_mode == LOG) {
        _keyboard_pitch_glide += (_keyboard_pitch - _keyboard_pitch_glide) * _glide_rate;
    } else {
        float glide_rate = _glide_rate / 10;
        // linear glide mode
        float glide_tmp = _keyboard_pitch - _keyboard_pitch_glide;
        glide_tmp = glide_tmp > glide_rate ? glide_rate : glide_tmp;
        glide_tmp = glide_tmp < -glide_rate ? -glide_rate : glide_tmp;
        _keyboard_pitch_glide = glide_tmp + _keyboard_pitch_glide;
    }

    float mpe_pitchbend = 0;
    _midi_expression = _layer_params.expression;
    float at_mpe = _aftertouch;

    // add mpe values if we are in mpe mode in this voice
    float mpe_y_bipolar;
    if (_mpe_mode_en) {
        // smoothing avoids the clicking of the data due to midi precision limits
        mpeParamSmooth(_mpe_data->mpe_x, _mpe_data_smooth.mpe_x);
        mpeParamSmooth(_mpe_data->mpe_y, _mpe_data_smooth.mpe_y);
        mpeParamSmooth(_mpe_data->mpe_z, _mpe_data_smooth.mpe_z);
        if (_mpe_y_bipolar) {
            mpe_y_bipolar = (midiSourceScaleBipolar(_mpe_data_smooth.mpe_y));
        } else {
            mpe_y_bipolar = (midiSourceScaleUnipolar(_mpe_data_smooth.mpe_y));
        }
        mpeParamFall(mpe_y_bipolar, _mpe_y_bipolar_value, _mpe_fall_y);
        mpeParamFall(_mpe_data_smooth.mpe_z, _mpe_z_fall_value, _mpe_fall_z);

        at_mpe += AFTERTOUCH_MAG * _mpe_z_fall_value;
        _midi_expression += _mpe_y_bipolar_value;
        mpe_pitchbend += _mpe_data_smooth.mpe_x * _mpe_pitchbend_gain + _mpe_pitchbend_offset;
    }

    // if we are in rel vel mode, then swap the mono midi signal for the poly rel vel signal
    if (_release_vel_mode) {
        _midi_source = midiSourceScaleUnipolar(_release_vel);
    } else {
        _midi_source = _layer_params.midi_source;
    }

    // add all other pitch source inc master detune
    _keyboard_pitch_glide_pitchwheel = _keyboard_pitch_glide + octave_offset + _pitchbend + mpe_pitchbend + _unison_detune * _unison_detune_amount * _unison_detune_amount / noteGain + _master_detune;

    if (dump2) {
        // printf("\nvoice pitch %f %f", _keyboard_pitch, _pitchbend);
        dump2 = false;
    }

    _aftertouch_smooth = param_smooth(at_mpe, _aftertouch_smooth);
    _matrix.run_slow_slots();
    _lfo_1.reCalculate();
    _lfo_2.reCalculate();
    _amp_env.reCalculate();
    _filt_env.reCalculate();
    _panner.reCalculate();
    _osc_mixer_1.reCalculate();
    _osc_mixer_2.reCalculate();
    _xor_mixer.reCalculate();
    _drive_compensator.reCalculate();
    _max_mix_out_level = 0;
    *_analog_input.filter_2_pole_in = _filter_2_pole_mode;
    for (int i = 0; i < CV_BUFFER_SIZE; ++i) {
        _cv_a_val = (*_layer_params._cv_a)[i];
        _cv_b_val = (*_layer_params._cv_b)[i];

        _matrix.run_fast_slots();
        _drive_compensator.run();
        _lfo_1.run();
        _lfo_2.run();
        _amp_env.run();
        _filt_env.run();
        _panner.run();
        _osc_mixer_1.run();
        _osc_mixer_2.run();
        _wt_osc.run();
        _xor_mixer.run();
        _analog_input.sub_osc_in_n = _sub_osc > 0.5;
        _analog_input.hard_sync_in_n = _sync_osc > 0.5;
        _analog_input.overdrive_in_n = _overdrive;
        _analog_input.mute_1_in_n = _mute_1;
        _analog_input.mute_2_in_n = _mute_2;
        _analog_input.mute_3_in_n = _mute_3;
        _analog_input.mute_4_in_n = _mute_4;
        _analog_input.setSampleCounter(i);
    }
    _voice_trigger = false;
    if (_pending_rel) {
        if (_sustain_pedal < 0.5) {
            _pending_rel = false;
            _allocated = false;
            _amp_env.gateOff();
            _filt_env.gateOff();
        }
    }
}

std::array<float *, NinaParams::NumSrcs * NinaParams::NumDsts> LayerVoice::_setupLayerParams(NinaParams::LayerParams &layer_params, NinaParams::LayerStateParams &sp) {
    // Setup the matrix gain pointers
    std::array<float *, NinaParams::NumSrcs * NinaParams::NumDsts> gains;
    auto *gain_addr = NinaParams::getLayerStateParamAddr(MAKE_MOD_MATRIX_PARAMID((NinaParams::ModMatrixSrc(0)), NinaParams::ModMatrixDst(0)), sp);

    for (uint s = 0; s < NinaParams::ModMatrixSrc::NumSrcs; s++) {
        for (uint d = 0; d < NinaParams::ModMatrixDst::NumDsts; d++) {
            gains.at(s * NinaParams::NumDsts + d) = gain_addr++;
        }
    }

    // override the gain values for the morph mod controls
    gains.at(NinaParams::ModMatrixSrc::FilterEnvelope * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr((ParamID)NinaParams::MorphEg1, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::AmpEnvelope * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphEg2, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::Lfo1 * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphLfo1, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::Lfo2 * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphLfo2, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::Wavetable * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphWave, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::KeyPitch * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphKBD, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::KeyVelocity * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphVel, layer_params.common_params);
    gains.at(NinaParams::Aftertouch * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphAfter, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::Modwheel * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphMod, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::Expression * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphExp, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::PanPosition * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphPan, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::Time * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphTime, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::Midi * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphMidi, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::CVA * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphCvA, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::CVB * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphCvB, layer_params.common_params);
    gains.at(NinaParams::ModMatrixSrc::Offset * NinaParams::NumDsts + NinaParams::Morph) = NinaParams::getLayerCommonParamAddr(NinaParams::MorphOffset, layer_params.common_params);
    return gains;
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
