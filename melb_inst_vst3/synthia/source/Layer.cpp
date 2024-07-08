/**
 * @file Layer.cpp
 * @brief Nina Layer class implementation.
 *
 * @copyright Copyright (c) 2023 Melbourne Instruments, Australia
 */
#include "Layer.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

const std::map<int, std::vector<int>> Layer::_unison_detune_positions = {
    { 1,                                    {0}},
    { 2,                                 {0, 1}},
    { 3,                             {0, 1, -1}},
 //  shuffling starts here:
    { 4,                          {2, 0, -1, 1}},
    { 5,                       {2, 0, 0, -2, 1}},
    { 6,                   {1, -2, 2, 0, -3, 3}},
    { 7,                 {1, 4, 2, 0, 0, -3, 3}},
    { 8,               {1, 4, 2, 0, 0, 0, 5, 3}},
    { 9,            {1, 4, 2, 0, 5, 0, 0, 0, 3}},
    {10,         {1, 4, 2, 0, 5, 0, 0, 0, 0, 3}},
    {11,      {1, 4, 2, 0, 5, 0, 0, 0, 0, 0, 3}},
    {12, {3, 9, 6, 10, 7, 4, 0, 11, 5, 1, 8, 2}}
};

const std::map<int, std::vector<float>> Layer::_unison_pan_positions = {
    { 1,                                                                                                        {0.f}},
    { 2,                                                                                                      {-1, 1}},
    { 3,                                                                                                 {-1, 0.f, 1}},
    { 4,                                                                                       {-1, -0.33f, 0.33f, 1}},
    { 5,                                                                                      {-1, -0.5f, 0, 0.5f, 1}},
    { 6,                                                                        {-1.f, -0.6f, -0.2f, 0.2f, 0.6f, 1.f}},
    { 7,                                                                         {-1, -0.66f, -.33, 0, 0.33f, .66, 1}},
    { 8,                                                                {-1, -0.75f, -.5, -0.25, 0.25f, 0.5, 0.75, 1}},
    { 9,                                                                {-1, -0.75f, -0.5, -0.25, 0, .25, .5, .75, 1}},
    {10,                                                                 {-1, -0.8, -.6, -.4, -.2, .2, .4, .6, .8, 1}},
    {11,                                                             {-1, -0.8, -0.6, -.4, -.2, 0, .2, .4, .6, .8, 1}},
    {12, {-1.f, -0.8181f, -0.6363f, -0.45454f, -0.2787f, -0.0909f, 0.0909f, 0.2787f, 0.45454f, 0.6363f, 0.8181f, 1.f}}
};

static uint layers_created = 0;

Layer::Layer(uint num_voices, uint first_voice, std::array<AnalogVoice, NUM_VOICES> &analog_voices, std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> &high_res_out, NinaParams::AudioInputBuffers &input_buffers, std::vector<ParamChange> &param_changes, NinaParams::GlobalParams &global_params) :
    _layer_state(STATE_A),
    _voice_mode(VoiceMode::LEGATO),
    _num_unison(1),
    _high_input_buffers(input_buffers),
    _layer_voices{
        LayerVoice(analog_voices[0].getVoiceInputBuffers(),
            _layer_params, _state_a_transform, _state_b_transform, _wt_loader_a, _wt_loader_b, high_res_out, _high_res_input[0], 0),
        LayerVoice(analog_voices[1].getVoiceInputBuffers(),
            _layer_params, _state_a_transform, _state_b_transform, _wt_loader_a, _wt_loader_b, high_res_out, _high_res_input[1], 1),
        LayerVoice(analog_voices[2].getVoiceInputBuffers(),
            _layer_params, _state_a_transform, _state_b_transform, _wt_loader_a, _wt_loader_b, high_res_out, _high_res_input[2], 2),
        LayerVoice(analog_voices[3].getVoiceInputBuffers(),
            _layer_params, _state_a_transform, _state_b_transform, _wt_loader_a, _wt_loader_b, high_res_out, _high_res_input[3], 3),
        LayerVoice(analog_voices[4].getVoiceInputBuffers(),
            _layer_params, _state_a_transform, _state_b_transform, _wt_loader_a, _wt_loader_b, high_res_out, _high_res_input[4], 4),
        LayerVoice(analog_voices[5].getVoiceInputBuffers(),
            _layer_params, _state_a_transform, _state_b_transform, _wt_loader_a, _wt_loader_b, high_res_out, _high_res_input[5], 5),
        LayerVoice(analog_voices[6].getVoiceInputBuffers(),
            _layer_params, _state_a_transform, _state_b_transform, _wt_loader_a, _wt_loader_b, high_res_out, _high_res_input[6], 6),
        LayerVoice(analog_voices[7].getVoiceInputBuffers(),
            _layer_params, _state_a_transform, _state_b_transform, _wt_loader_a, _wt_loader_b, high_res_out, _high_res_input[7], 7),
        LayerVoice(analog_voices[8].getVoiceInputBuffers(),
            _layer_params, _state_a_transform, _state_b_transform, _wt_loader_a, _wt_loader_b, high_res_out, _high_res_input[8], 8),
        LayerVoice(analog_voices[9].getVoiceInputBuffers(),
            _layer_params, _state_a_transform, _state_b_transform, _wt_loader_a, _wt_loader_b, high_res_out, _high_res_input[9], 9),
        LayerVoice(analog_voices[10].getVoiceInputBuffers(),
            _layer_params, _state_a_transform, _state_b_transform, _wt_loader_a, _wt_loader_b, high_res_out, _high_res_input[10], 10),
        LayerVoice(analog_voices[11].getVoiceInputBuffers(),
            _layer_params, _state_a_transform, _state_b_transform, _wt_loader_a, _wt_loader_b, high_res_out, _high_res_input[11], 11)},
    _misc_scale(_layer_params.common_params.at(LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::MiscScale))),
    _param_changes(param_changes),
    _global_params(global_params) {
    _layer_params._cv_a = &_cv_1;
    _layer_params._cv_b = &_cv_2;
    _layer_num = layers_created++;
    _param_changes.reserve(NinaParams::NUM_PARAMS * 2);

    // Ensure the layer param arrays initialised - all to zeros
    std::fill(_layer_params.common_params.begin(), _layer_params.common_params.end(), 0);
    std::fill(_layer_params.state_params.begin(), _layer_params.state_params.end(), 0.5);
    std::fill(_state_a_params.begin(), _state_a_params.end(), 0.5);
    std::fill(_state_b_params.begin(), _state_b_params.end(), 0.5);

    // initialise all the mpe layers
    for (uint i = 0; i < MPE_MAX_NUM_CHANNELS + 1; i++) {
        _mpe_channel_data.at(i).channel = i;
    }

    // set the init state of the voice. this will be the state after the vst has loaded up. it will make some sound
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc1Pitch))) = 1.0;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc2Pitch))) = 1;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc3Pitch))) = 1;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Blend))) = 0;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeSus))) = 1;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeAtt))) = 0;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeRel))) = 0;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff))) = 1;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeLevel))) = 1;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Level))) = 1;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::VcaIn))) = 1;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::VcaIn))) = 0;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(MAKE_MOD_MATRIX_PARAMID(NinaParams::Wavetable, NinaParams::VcaIn))) = .5;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::LfoSlew)) = 1;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Glide)) = 0;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Vco1TuneSemitone)) = 1;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Vco2TuneSemitone)) = 1;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Vco1TuneCoarse)) = 0;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::Vco2TuneCoarse)) = 0;
    _state_a_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::WavetableSelect)) = 0;
    _state_b_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::WavetableSelect)) = 0;

    //_layer_params._cv_b = NinaParams::CvInputMode::Two;
    // set the voice mode to poly
    _layer_params.common_params.at((LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::Legato))) = 1;
    _voice_mode = POLY;

    // set the default key pitch offset to C4
    _layer_params.common_params.at((LAYER_STATE_PARAMID_TO_INDEX(NinaParams::KeyPitchOffset))) = 60.f / 127.f;

    // blacklist voices
    for (int i = 0; i < NUM_VOICES; i++) {
        _layer_voices[i].setBlacklisted(analog_voices[i].blacklisted());
    }
    // Set the number of voices
    setNumVoices(first_voice, num_voices);

    // Reserve space for the active voices
    _active_voices.reserve(NUM_VOICES);

    // Create the common param attributes array
    auto param_id = NinaParams::FIRST_LAYER_COMMON_PARAM;
    for (uint i = 0; i < NinaParams::NUM_LAYER_COMMON_PARAMS; i++) {
        // Handle specific cases here
        if (param_id == NinaParams::NumUnison) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _setNumUnison, _noSmoothCommon);
        } else if (param_id == NinaParams::PanNum) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _setPanNum, _noSmoothCommon);
        } else if (param_id == NinaParams::PanMode) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _setPanMode, _noSmoothCommon);
        } else if (param_id == NinaParams::Legato) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _setVoiceMode, _noSmoothCommon);
        } else if (param_id == NinaParams::TriggerPrint) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _triggerPrint, _noSmoothCommon);
        } else if (param_id == NinaParams::MidiPitchBend) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _midiPitchWheel, _noSmoothCommon);
        } else if (param_id == NinaParams::PitchBendRange) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _midiPitchWheel, _noSmoothCommon);
        } else if (param_id == NinaParams::MidiAftertouch) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _midiAftertouch, _noSmoothCommon);
        } else if (param_id == NinaParams::extInMode) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _inputModeTransform, _noSmoothCommon);
        } else if (param_id == NinaParams::VcfOverdrive) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _odTransform, _noSmoothCommon);
        } else if (param_id == NinaParams::GlideMode) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _glideMode, _noSmoothCommon);
        } else if (param_id == NinaParams::midiLowNote) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _midiLowNoteFilterTransform, _noSmoothCommon);
        } else if (param_id == NinaParams::midiHighNote) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _midiHighNoteFilterTransform, _noSmoothCommon);
        } else if (param_id == NinaParams::midiChannelFilter) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _midiChannelNoteFilterTransform, _noSmoothCommon);
        } else if (param_id == NinaParams::midiSourceVelMode) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _midiSourceVelMode, _noSmoothCommon);
        } else if (param_id == NinaParams::Lfo1TempoSync) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _lfo1TempoSyncTransform, _noSmoothCommon);
        } else if (param_id == NinaParams::Lfo2TempoSync) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _lfo2TempoSyncTransform, _noSmoothCommon);
        } else if (param_id == NinaParams::WtInterpolateMode) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _wtInterpolateMode, _noSmoothCommon);
        } else if (param_id == NinaParams::AmpEnvelopeDrone) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _vcaDroneTransform, _noSmoothCommon);
        } else if (param_id == NinaParams::PatchVolume) {

            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _defaultParamUpdate, _noSmoothCommon);
        } else if (param_id == NinaParams::outputRouting) {

            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _outputRoutingTransform, _noSmoothCommon);
        } else if (param_id == NinaParams::cvASelect) {

            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _updateCvA, _noSmoothCommon);
        } else if (param_id == NinaParams::cvBSelect) {

            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _updateCvB, _noSmoothCommon);
        } else if (param_id == NinaParams::MidiExpression) {

            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _defaultParamUpdate, _noSmoothCommon);
        } else if (param_id == NinaParams::MidiSource) {

            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _defaultParamUpdate, _noSmoothCommon);
        } else if (param_id == NinaParams::AllNotesOff) {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id,
                _AllNotesOff,
                _noSmoothCommon);
        } else if (param_id == NinaParams::MpeMode) {

            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _mpeMode, _noSmoothCommon);
        } else if (param_id == NinaParams::MpeYFallTime) {

            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _mpeFallY, _noSmoothCommon);
        } else if (param_id == NinaParams::MpeZFallTime) {

            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _mpeFallZ, _noSmoothCommon);
        } else if (param_id == NinaParams::MiscScale) {

            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _miscScaleThing, _noSmoothCommon);
        } else if (true && (((param_id == NinaParams::MorphMod)))) {

            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _defaultParamUpdate, _noSmoothBipolarCommon2xGain);
        } else if (true && ((param_id == NinaParams::MorphEg1) ||
                               (param_id == NinaParams::MorphEg2) ||
                               (param_id == NinaParams::MorphLfo1) ||
                               (param_id == NinaParams::MorphWave) ||
                               (param_id == NinaParams::MorphKBD) ||
                               (param_id == NinaParams::MorphVel) ||
                               (param_id == NinaParams::MorphAfter) ||
                               (param_id == NinaParams::MorphExp) ||
                               (param_id == NinaParams::MorphPan) ||
                               (param_id == NinaParams::MorphTime) ||
                               (param_id == NinaParams::MorphMidi) ||
                               (param_id == NinaParams::MorphCvA) ||
                               (param_id == NinaParams::MorphCvB) ||
                               (param_id == NinaParams::MorphLfo2) ||
                               (param_id == NinaParams::MorphOffset))) {

            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _defaultParamUpdate, _noSmoothBipolarCommon);
        } else {
            _layer_param_attrs.common_attrs.at(i) = CommonParamAttr(param_id, _defaultParamUpdate, _noSmoothCommon);
        }
        param_id++;
    }

    // Create the state param attributes array
    param_id = NinaParams::FIRST_LAYER_STATE_PARAM;
    for (uint i = 0; i < NinaParams::NUM_LAYER_STATE_PARAMS; i++) {
        if (param_id == NinaParams::Vco1TuneFine) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _transformFineTune);
        } else if (param_id == NinaParams::Vco1TuneCoarse) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _transformCoarseTune);
        } else if (param_id == NinaParams::FineTuneRange) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _transformFineTuneRange);
        } else if (param_id == NinaParams::Vco1TuneSemitone) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _transformSemitoneTune);
        } else if (param_id == NinaParams::Vco2TuneSemitone) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _transformSemitoneTune);
        } else if (param_id == NinaParams::Lfo1SyncRate ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo1Rate)) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _defaultTransform);
        } else if (param_id == NinaParams::Lfo2SyncRate ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo2Rate)) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _defaultTransform);
        } else if (param_id == NinaParams::Vco2TuneFine) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _transformFineTune);
        } else if (param_id == NinaParams::VcfMode) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _VcfModeTransform);
        } else if (param_id == NinaParams::Vco2TuneCoarse) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _transformCoarseTune);
        } else if (param_id == NinaParams::WaveTuneFine) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _transformFineTune);
        } else if (param_id == NinaParams::Osc3Low) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _transformOsc3LowMode);
        } else if (param_id == NinaParams::WaveTuneCoarse) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _transformCoarseTune);
        } else if (param_id == NinaParams::CurrentMorphValue) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _nullMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _currentMorphValue);
        } else if (param_id == NinaParams::WavetableSelect) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _nullMorph,
                _wtSelectParamUpdate,
                _noSmooth,
                _defaultTransform);
        } else if (param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc3Shape)) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _bipolarTransform);
        } else if (param_id == NinaParams::KeyPitchOffset) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _keyPitchOffsetTransform);
        } else if (param_id == NinaParams::SpinReset) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _spinResetTransform);
        } else if (param_id == NinaParams::Lfo2Slew) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _SlewTransform);
        } else if (param_id == NinaParams::Lfo2Retrigger) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _Lfo2ResetTransform);
        } else if (param_id == NinaParams::LfoSlew) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _SlewTransform);
        } else if (param_id == NinaParams::LfoRetrigger) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _LfoResetTransform);
        } else if (param_id == NinaParams::VcaEnvReset) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _AmpEnvResetTransform);
        } else if (param_id == NinaParams::VcfEnvReset) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _FiltEnvResetTransform);
        } else if (param_id == NinaParams::FiltEnvVelSense) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _bipolarTransform);
        } else if (param_id == NinaParams::AmpEnvVelSense) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _bipolarTransform);
        } else if (param_id == NinaParams::VcaUnipolar) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _vcaUnipolarTransform);
        } else if (param_id == NinaParams::Glide) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _glideTransform);
        } else if (param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyVelocity, NinaParams::AmpEnvelopeLevel) || param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyVelocity, NinaParams::FilterEnvelopeLevel)) {

            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _nullMorph,
                _defaultParamUpdate,
                _noSmooth,
                _disableTransform);
        } else if (param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Pitch)) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _nullMorph,
                _defaultParamUpdate,
                _noSmooth,
                _osc1TuneSum);
        } else if (param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc2Pitch)) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _nullMorph,
                _defaultParamUpdate,
                _noSmooth,
                _osc2TuneSum);
        } else if (param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::Osc1Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::Osc1Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo2, NinaParams::Osc1Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::Osc1Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Wavetable, NinaParams::Osc1Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Time, NinaParams::Osc1Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::Aftertouch, NinaParams::Osc1Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyVelocity, NinaParams::Osc1Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::PanPosition, NinaParams::Osc1Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::FilterEnvelope, NinaParams::Osc1Pitch) || param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::Osc2Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::Osc2Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo2, NinaParams::Osc2Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::Osc2Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Wavetable, NinaParams::Osc2Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Time, NinaParams::Osc2Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::Aftertouch, NinaParams::Osc2Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyVelocity, NinaParams::Osc2Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::PanPosition, NinaParams::Osc2Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::FilterEnvelope, NinaParams::Osc2Pitch) || param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::Osc3Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::Osc3Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo2, NinaParams::Osc3Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::Osc3Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Wavetable, NinaParams::Osc3Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Time, NinaParams::Osc3Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::Aftertouch, NinaParams::Osc3Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyVelocity, NinaParams::Osc3Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::PanPosition, NinaParams::Osc3Pitch) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc::FilterEnvelope, NinaParams::Osc3Pitch)) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _defaultTransform);
        } else if (param_id == NinaParams::XorMode) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _noSmooth,
                _xorModeTransform);
        } else if (param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc3Pitch)) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _nullMorph,
                _defaultParamUpdate,
                _noSmooth,
                _osc3TuneSum);
        } else if (
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeAtt) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeDec) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeRel) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeSus) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterEnvelopeSus) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterEnvelopeAtt) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterEnvelopeDec) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterEnvelopeRel) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Blend) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc2Blend)) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _defaultTransform);
        } else if (param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Width) ||
                   param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc2Width)) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _widthTransform);
        } else if (
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Level) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc2Level) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc3Level) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::XorLevel)) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _defaultTransform);
        } else if (
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Width) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo2Gain) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo1Gain) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc2Width) ||
            param_id == MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterResonance)) {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _defaultTransform);
        } else if (param_id >= MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc(0), NinaParams::ModMatrixDst(0))) {

            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _bipolarTransform);
        }

        else {
            _layer_param_attrs.state_attrs.at(i) = StateParamAttr(param_id,
                _defaultMorph,
                _defaultParamUpdate,
                _defaultSmooth,
                _defaultTransform);
        }
        param_id++;
    }
}

void Layer::_clearNotes() {
    for (auto &voice : _layer_voices) {
        voice.enterReleaseState();
    }
}

const std::vector<ParamChange> &Layer::run() {
    // check if all voices are unallocated, if so, then set the all voices released flag so we know that we should set the next allocated voice as the one which is contolling the morph mod

    // update the high res inputs
    switch (_input_mode) {
    case InputModes::inputs1_2: {
        for (uint voice_n = 0; voice_n < 6; voice_n++) {
            _high_res_input[voice_n * 2 + 0] = _high_input_buffers.ex_in_1;
            _high_res_input[voice_n * 2 + 1] = _high_input_buffers.ex_in_2;
        }
    } break;
    case InputModes::inputs3_4: {
        for (uint voice_n = 0; voice_n < 6; voice_n++) {
            _high_res_input[voice_n * 2 + 0] = _high_input_buffers.ex_in_3;
            _high_res_input[voice_n * 2 + 1] = _high_input_buffers.ex_in_4;
        }
    } break;

    case InputModes::input1:
        _high_res_input.fill(_high_input_buffers.ex_in_1);
        break;
    case InputModes::input2:
        _high_res_input.fill(_high_input_buffers.ex_in_2);
        break;
    case InputModes::input3:
        _high_res_input.fill(_high_input_buffers.ex_in_3);
        break;
    case InputModes::input4:
        _high_res_input.fill(_high_input_buffers.ex_in_4);
        break;

    case InputModes::outputs1_2: {
        for (uint voice_n = 0; voice_n < 6; voice_n++) {

            _high_res_input[voice_n * 2 + 0] = _high_input_buffers.mix_left;
            _high_res_input[voice_n * 2 + 1] = _high_input_buffers.mix_left;
        }
    } break;
    default:
        _high_res_input.fill(_high_input_buffers.ex_in_1);
        break;
    }

    // If this layer actually has any voices
    bool morphing = false;
    bool leaving_morph = false;

    if (_signal_param_refresh) {
        _signal_param_refresh = false;
        for (uint i = 0; i < NinaParams::NUM_LAYER_STATE_PARAMS; i++) {

            _param_changes.push_back(ParamChange((NinaParams::FIRST_LAYER_STATE_PARAM + i), _current_state_params->at(i)));
        }
        for (uint i = 1; i < NinaParams::NUM_LAYER_COMMON_PARAMS; i++) {

            _param_changes.push_back(ParamChange((NinaParams::FIRST_LAYER_COMMON_PARAM + i), _layer_params.common_params.at(i)));
        }
    }

    // Decide if we are morphing or not
    float current_morph = _cur_morph_value;
    if ((current_morph > 0.0f) && (current_morph < 1.0f)) {

        // We're morphing
        morphing = true;
    }

    // printf("\n layer morph %d  %f", morphing, _cur_morph_value );

    // Check if we are entering or leaving morph mode, and add
    // a param change if we are entering morph mode
    // Note we export this param change to any notification listeners
    const bool morph_knob_active = ((_cur_morph_value > 0.0f) && (_cur_morph_value < 1.0f));
    if (_current_layer) {
        if (morphing && !_morphing) {
            _param_changes.push_back(ParamChange(NinaParams::Morphing, 1.0, true));
        } else if (!morphing && _morphing) {
            leaving_morph = true;
        }
    }

    for (uint i = 0; i < NinaParams::NUM_LAYER_COMMON_PARAMS; i++) {
        // Get the State A and B param values
        // Smooth the value
        float &tmp = _layer_params.common_params[i];
        float value = _layer_param_attrs.common_attrs[i].set_value;
        _layer_param_attrs.common_attrs[i].smooth_fn(value, tmp, *this);

        // Transform the value and save in the layer state params
    }

    // Process each Layer state parameter
    for (uint i = 0; i < NinaParams::NUM_LAYER_STATE_PARAMS; i++) {
        // If there was a layer state change reset automation setting for this parameter
        if (_layer_state_change) {
            _layer_param_attrs.state_attrs[i].automate = true;
        }

        // Should we automate this parameter?
        float value;
        if (_layer_param_attrs.state_attrs[i].automate) {
            // Morph the parameter
            value = _layer_param_attrs.state_attrs[i].morph_fn(i, morphing, *this);
        } else {
            // No automation
            value = (*_current_state_params)[i];
            _state_a_params[i] = value;
            _state_b_params[i] = value;
        }

        // processing for voice morph
        {

            _layer_param_attrs.state_attrs[i].smooth_fn(_state_a_params[i], _state_a_smooth[i], *this);

            _layer_param_attrs.state_attrs[i].smooth_fn(_state_b_params[i], _state_b_smooth[i], *this);

            _state_a_transform[i] = _layer_param_attrs.state_attrs[i].transform_fn(_state_a_smooth[i], *this);
            _state_b_transform[i] = _layer_param_attrs.state_attrs[i].transform_fn(_state_b_smooth[i], *this);

            _layer_params.state_params[i] = _layer_param_attrs.state_attrs[i].transform_fn(value, *this);
        }
    }

    // Add a param change if we are leaving morph mode
    // Note we export this param change to any notification listeners
    if (leaving_morph) {
        _param_changes.push_back(ParamChange(NinaParams::Morphing, 0.0, true));
    }

    // Set the prev morph values and reset the layer state change indicator
    _morphing = morphing;
    _layer_params.morph_pos = param_smooth(_cur_morph_value, _layer_params.morph_pos);
    _prev_morph_value = _cur_morph_value;
    _layer_state_change = false;

    // Set various composite parameters that depend on multiple values
    _update_midi_sources();
    return _param_changes;
}

void Layer::setNumVoices(uint first_voice, uint num_voices) {
    assert((num_voices <= NUM_VOICES));

    // enter the release state for all voices so there are no notes left hanging
    _clearNotes();

    // Truncate the number of voices if needed
    if ((first_voice + num_voices) >= NUM_VOICES) {
        num_voices = NUM_VOICES - first_voice;
    }

    // Set the number of voices and first/last voice
    _num_voices = num_voices;
    _first_voice = first_voice;
    if (_num_voices) {
        uint n_voices_set_false = 0;
        if (num_voices > 0) {
            n_voices_set_false = num_voices - 1;
        }

        _last_voice = _first_voice + (_num_voices - 1);
        if (_last_voice > NUM_VOICES - 1) {
            _last_voice = NUM_VOICES - 1;
        }
    } else {
        _last_voice = 0;
    }
}

void Layer::updateParams(uint num_changes, const ParamChange *changed_params) {
    // Check for a layer state change first, as this can affect the desintation
    // for subsequent param updates
    const uint layer_num = _layer_num;

    // Update the reference to the current state params
    _current_state_params = (_layer_state == STATE_A) ? &_state_a_params : &_state_b_params;
    // Process each param change
    for (uint i = 0; i < num_changes; i++) {
        const ParamChange &pc = changed_params[i];

        // calculate the selected layer. only process the param change if this is the selected layer

        if (is_layer_param(pc.param_id, layer_num)) {
            // If the parameter is a state AB change message, then change the current set state and reset morph
            const uint param_id = get_param_id(pc);
            if (param_id == NinaParams::LayerState) {

                // Has the layer state changed?
                auto layer_state = (pc.value < 0.5) ? STATE_A : STATE_B;
                _morphing = false;
                if (_layer_state != layer_state) {
                    // Set the new layer state and indicate it has changed
                    _layer_state = layer_state;
                    _layer_state_change = true;
                    _current_state_params = (_layer_state == STATE_A) ? &_state_a_params : &_state_b_params;
                }
                _cur_morph_value = (_layer_state == STATE_A) ? 0.0 : 1.0;
                _prev_morph_value = _cur_morph_value;
            }
            // get the actual param id of the param change
            // printf("\n l %d param %d", _layer_num, param_id);

            if (param_id == NinaParams::MorphValue) {
                setMorphValue(pc.value);
                // printf("\n l:%d, morph %f morphing %d", _layer_num, pc.value, _morphing);
                continue;
            }
            // Ignore state changes and global params
            if ((param_id < NinaParams::FIRST_LAYER_COMMON_PARAM) ||
                (param_id == NinaParams::LayerState))
                continue;
            // Common Layer param?
            if (param_id < NinaParams::FIRST_LAYER_STATE_PARAM) {
                // Update the Layer common param
                _commonParamUpdate(param_id, pc.value);
            } else {

                // Update the Layer current state param
                _stateParamUpdate(param_id, pc.value);
            }
        }

        // mpe param handling
        if ((_mpe_mode != NinaParams::MpeModes::Off)) {
            const uint param_id = get_param_id(pc);

            // get the midi channel of the param change
            uint channel = get_mpe_param_channel(pc.param_id);
            auto &mpe_ch = _mpe_channel_data.at(channel);

            switch (param_id) {
            case NinaParams::MpeX:
                mpe_ch.mpe_x = pc.value;
                break;
            case NinaParams::MpeY:
                mpe_ch.mpe_y = pc.value;
                break;
            case NinaParams::MpeZ:
                mpe_ch.mpe_z = pc.value;
                break;
            default:
                break;
            }
        }
    }
}

void Layer::polyPressureEvent(Steinberg::Vst::PolyPressureEvent poly_event) {
    if (!_midiNoteFilter(poly_event.channel, poly_event.pitch)) {
        return;
    } else {
        for (auto &voice : _layer_voices) {
            auto *curr_note = voice.getMidiNote();

            if (curr_note->pitch == poly_event.pitch) {
                voice.setAftertouch(poly_event.pressure);
            }
        }
    }
}

void Layer::allocateVoices(const MidiNote &note) {
    // If this layer has any voices available
    if (!_midiNoteFilter(note.channel, note.pitch)) {
        return;
    }

    if (_num_voices) {
        // Num unison cannot be zero
        assert(_num_unison > 0);

        // Clear the active voices array
        _resetActiveVoices();

        // Currently !poly means mono, so we just allocate voices starting from the first voice
        // until we have enough for the unison number
        if (_voice_mode != VoiceMode::POLY) {
            // Keep track of the currently held mono notes so we can swap to held notes on
            // release of the current note
            _held_notes.push_back(note);

            // Allocate the first unison number of voices
            uint allocated = 0;
            uint checked = 0;
            for (uint i = _first_voice; i <= _last_voice; i++) {
                // Is this voice not blacklisted?
                if (!_layer_voices[i].blacklisted()) {
                    // Add this voice (always steal voices in unison mode)
                    _active_voices.push_back(&_layer_voices[i]);
                    allocated++;
                }

                // If we have checked all possible voices, or we have allocated
                // enough voices, exit the allocation loop
                // Note this may be less than unison if there are not enough voices
                // to allocate
                if (++checked >= _num_voices) {
                    break;
                }
                if (allocated >= _num_unison) {
                    break;
                }
            }
        } else {
            // Simple round robin allocation strategy
            uint num_allocated = 0;
            for (int i = _first_voice; i <= _last_voice; i++) {
                // Get the next voice to allocate
                // Note - wrap around if needed
                int v = i + _round_robin_voice;
                if (v > _last_voice) {
                    v -= _num_voices;
                }
                // Allocate this voice if it is free or released
                if (!_layer_voices[v].blacklisted() && !_layer_voices[v].allocated()) {
                    // Allocate this voice
                    _active_voices.push_back(&_layer_voices[v]);

                    // If we have allocated enough voices?
                    if (++num_allocated >= _num_unison) {
                        // Yes, set the round robin voice to be used in the next
                        // allocation
                        // Note - wrap around to the first voice if needed
                        _round_robin_voice = v + 1 - _first_voice;
                        if (_round_robin_voice + _first_voice > _last_voice) {
                            _round_robin_voice = 0;
                        }
                        break;
                    }
                }
            }

            // Have we allocated enough voices AND there are other non-free voices available, here we steal voices with pending releases
            if ((num_allocated < _num_unison) && (num_allocated < _num_voices)) {
                // Steal the oldest allocated voices until we have enough
                for (int i = _first_voice; i <= _last_voice; i++) {

                    // Get the next voice to allocate
                    // Note - wrap around if needed
                    int v = i + _round_robin_voice;
                    if (v > _last_voice) {
                        v -= _num_voices;
                    }

                    // Steal this voice if it is currently being used and there is a pending release
                    if (!_layer_voices[v].blacklisted() && _layer_voices[v].allocated() && _layer_voices[v].pendingRel()) {
                        // Allocate this voice
                        _active_voices.push_back(&_layer_voices[v]);

                        // If we have allocated enough voices?
                        if (++num_allocated >= _num_unison) {
                            // Yes, set the round robin voice to be used in the next
                            // allocation
                            // Note - wrap around to the first voice if needed
                            _round_robin_voice = v + 1 - _first_voice;
                            if (_round_robin_voice + _first_voice > _last_voice) {
                                _round_robin_voice = 0;
                            }
                            break;
                        }
                    }
                }
            }

            // we still dont have enough voices, so now steal voices without pending releases
            if ((num_allocated < _num_unison) && (num_allocated < _num_voices)) {
                // Steal the oldest allocated voices until we have enough
                for (int i = _first_voice; i <= _last_voice; i++) {

                    // Get the next voice to allocate
                    // Note - wrap around if needed
                    int v = i + _round_robin_voice;
                    if (v > _last_voice) {
                        v -= _num_voices;
                    }

                    // Steal this voice if it is currently being used and there is a pending release
                    if (!_layer_voices[v].blacklisted() && _layer_voices[v].allocated()) {
                        // Allocate this voice
                        _active_voices.push_back(&_layer_voices[v]);

                        // If we have allocated enough voices?
                        if (++num_allocated >= _num_unison) {
                            // Yes, set the round robin voice to be used in the next
                            // allocation
                            // Note - wrap around to the first voice if needed
                            _round_robin_voice = v + 1 - _first_voice;
                            if (_round_robin_voice + _first_voice > _last_voice) {
                                _round_robin_voice = 0;
                            }
                            break;
                        }
                    }
                }
            }
        }

        // Setup each active voice
        const std::vector<float> &detune = _unison_pan_positions.at(_num_unison);
        int pan_it = 0;
        int detune_it = 0;
        for (LayerVoice *voice : _active_voices) {
            // calculate the pan position based on the current panning mode/number
            float voice_pan = 0;
            switch (_pan_mode) {
            case pingPong:
                if (_current_pan_num % 2 == 0) {
                    voice_pan = 1.f;
                    _current_pan_num = 0;
                } else {
                    voice_pan = -1.f;
                }
                break;

            case spread: {
                if (_pan_num > 1) {
                    float span_range = 2.0f / (float)((_pan_num - 1));
                    voice_pan = ((float)_current_pan_num * span_range) - 1.f;
                } else {
                    voice_pan = 0;
                }
            } break;

            default:
                break;
            }

            _current_pan_num++;
            if (_current_pan_num >= _pan_num) {
                _current_pan_num = 0;
            }

            // Retrigger if not in Legato
            if (_voice_mode != VoiceMode::LEGATO) {
                // voice->retrigger();
            }

            // Allocate and setup the voice
            voice->allocate(note, voice_pan, detune.at(pan_it));

            // if we are in mpe mode, then set the voice's mpe data based on the incoming channel.
            if (_mpe_mode != NinaParams::MpeModes::Off) {
                voice->setMpeChannelData(&_mpe_channel_data.at(note.channel));
            }

            pan_it++;
        }
    }
}

void Layer::freeVoices(const MidiNote &note) {
    if (!_midiNoteFilter(note.channel, note.pitch)) {
        return;
    }
    // If the voices mode is Legato or Mono-retrigger, enter the release state
    // for all voices
    if (_voice_mode != VoiceMode::POLY) {
        // Legato or Mono-retrigger mode
        // Find all held notes with the same pitch and remove them
        for (auto it = _held_notes.begin(); it != _held_notes.end();) {
            if (note.pitch == it->pitch) {
                it = _held_notes.erase(it);
            } else {
                ++it;
            }
        }

        // If there are no held notes remaining
        if (_held_notes.size() == 0) {
            // Free each active voice
            for (LayerVoice *voice : _active_voices) {
                voice->free(note.velocity);
            }
        } else {
            // There are held notes remaining, process the newest
            for (LayerVoice *voice : _active_voices) {
                if (voice->getMidiNote()->pitch != _held_notes.back().pitch) {
                    voice->allocate(_held_notes.back());
                }
            }
        }
    } else {
        // Poly mode
        // Free each active voice with this note value
        for (auto &voice : _layer_voices) {
            // Get the voice MIDI note (if any), and check if it is the same as the
            // passed note (pitch)
            if (voice.allocated()) {
                auto voice_note = voice.getMidiNote();
                if ((voice_note->pitch == note.pitch)) {
                    // Free this voice
                    voice.free(note.velocity);
                }
            }
        }
    }
}

void Layer::setMorphValue(float value) {
    _cur_morph_value = value;
}

void Layer::setMorphMode(MorphMode mode) {
    _morph_mode = mode;
}

void Layer::_resetActiveVoices() {
    // Reset the active voices
    _active_voices.clear();
}

float Layer::_getCommonParam(ParamID param_id) {
    return NinaParams::getLayerCommonParam(param_id, _layer_params.common_params);
}

float Layer::_getStateParam(ParamID param_id) {
    return NinaParams::getLayerStateParam(param_id, _layer_params.state_params);
}

float Layer::_getCurrentStateParam(ParamID param_id) {
    return NinaParams::getLayerStateParam(param_id, *_current_state_params);
}

void Layer::_setCommonParam(ParamID param_id, float value) {
    NinaParams::setLayerCommonParam(param_id, value, _layer_params.common_params);

    // also set the 'set' value which is used if we smooth or transform the value
    _layer_param_attrs.common_attrs.at(param_id - NinaParams::FIRST_LAYER_COMMON_PARAM).set_value = value;
}

void Layer::_setStateParam(ParamID param_id, float value) {
    NinaParams::setLayerStateParam(param_id, value, _layer_params.state_params);
}

void Layer::_setCurrentStateParam(ParamID param_id, float value) {
    NinaParams::setLayerStateParam(param_id, value, *_current_state_params);
}

void inline Layer::_commonParamUpdate(ParamID param_id, float value) {
    _setCommonParam(param_id, value);
    uint val = LAYER_COMMON_PARAMID_TO_INDEX(param_id);
    _layer_param_attrs.common_attrs[val].update_fn(value, *this);
}

void inline Layer::_stateParamUpdate(ParamID param_id, float value) {
    _setCurrentStateParam(param_id, value);
    _layer_param_attrs.state_attrs[LAYER_STATE_PARAMID_TO_INDEX(param_id)].update_fn(value, *this);
    if (((_cur_morph_value > MORPH_ZERO_THRESH) && (_cur_morph_value < MORPH_ONE_THRESH)) ||
        ((_cur_morph_value <= MORPH_ZERO_THRESH) && (_layer_state == STATE_B)) ||
        ((_cur_morph_value >= MORPH_ONE_THRESH) && (_layer_state == STATE_A))) {
        _layer_param_attrs.state_attrs[LAYER_STATE_PARAMID_TO_INDEX(param_id)].automate = false;
    }
}

bool Layer::_midiNoteFilter(const uint channel, const uint note_n) {
    if (_mpe_mode == NinaParams::MpeModes::Off) {
        if ((int)note_n > _midi_note_start && (int)note_n < _midi_note_end) {
            if (_channel_filter_en && (channel == _midi_channel)) {
                return true;
            } else if (!_channel_filter_en) {
                return true;
            } else {
                return false;
            }
        }
    }

    // in mpe mode, we process all notes that are in the mpe channel range and the split range
    else {
        if (channel < _mpe_first_ch || channel > _mpe_last_ch) {
            return false;
        } else if (note_n < _midi_note_start && note_n > _midi_note_end) {
            return false;
        } else {
            return true;
        }
    }
    return false;
}

void Layer::_setNumUnison(float value, Layer &layer) {

    // Get the unison number - note this param is zero based in the patch,
    // and hence needs to be incremented to range from 1 - NUM_VOICES
    layer._num_unison = (uint)floor(value * NUM_VOICES) + 1;
    if (layer._num_unison > NUM_VOICES)
        layer._num_unison = NUM_VOICES;
}

void Layer::_setPanNum(float value, Layer &layer) {

    // Get the unison number - note this param is zero based in the patch,
    // and hence needs to be incremented to range from 1 - NUM_VOICES
    layer._pan_num = (uint)std::round(value * NUM_VOICES) + 1;
    if (layer._num_unison > NUM_VOICES)
        layer._num_unison = NUM_VOICES;
}

void Layer::_mpeMode(float value, Layer &layer) {
    const uint mpe_mode_num = std::round(value * (float)NinaParams::MpeModes::NumMpeModes);
    layer.setMpeMode((NinaParams::MpeModes)mpe_mode_num);
}

void Layer::_setPanMode(float value, Layer &layer) {

    // Get the unison number - note this param is zero based in the patch,
    // and hence needs to be incremented to range from 1 - NUM_VOICES
    layer._pan_mode = (PanModes)(int)floor(value * numPanModes);
}

void Layer::_setVoiceMode(float value, Layer &layer) {
    // Get the mode
    int mode = (uint)floor(value * VoiceMode::NumVoiceModes);
    if (mode > VoiceMode::POLY)
        mode = VoiceMode::POLY;
    if (mode != layer._voice_mode) {
        if (mode == VoiceMode::POLY) {
            // If we are changing to Poly and were in Legato
            if (layer._num_voices && (layer._voice_mode == VoiceMode::LEGATO)) {
                // Free all current voices
                for (int i = layer._first_voice; i <= layer._last_voice; i++) {
                    layer._layer_voices[i].free();
                }
                layer._held_notes.clear();
            }
        } else {

            for (int i = layer._first_voice; i <= layer._last_voice; i++) {
                layer._layer_voices[i].free();
            }
            layer._held_notes.clear();
        }

        layer._voice_mode = VoiceMode(mode);
        for (auto &voice : layer._layer_voices) {
            voice.setLegato((layer._voice_mode == VoiceMode::LEGATO));
        }
    }
}

float Layer::_transformFineTuneRange(float value, Layer &layer) {
    constexpr float log2_440_over_32 = 3.7813597135f;
    constexpr float interval2 = (log2_440_over_32 + ((float)14 - 9.f) / 12.f);
    constexpr float interval1 = (log2_440_over_32 + ((float)15 - 9.f) / 12.f);
    // return (value * 12 + 1) * (interval1 - interval2);
    return (6 + 1) * (interval1 - interval2);
}

float Layer::_transformFineTune(float value, Layer &layer) {

    //+- 1 semitone,
    const float interval = layer._layer_params.state_params.at(LAYER_STATE_PARAMID_TO_INDEX(NinaParams::FineTuneRange));
    float gain = (2 * interval) / noteGain;
    float offset = -gain / 2;
    constexpr float lin = 1;
    constexpr float scale = 1. / (1 + lin);

    // this is the x*abs(x) +x curve we might use
    // value  = (value - 0.5) * 2;
    // value = scale * (value * std::abs(value) + lin * value);
    // value = (value + 1) / 2;
    return (value * gain + offset);
}

float inline Layer::_transformSemitoneTune(float value, Layer &layer) {
    constexpr float log2_440_over_32 = 3.7813597135f;
    constexpr float semitone = (log2_440_over_32 + ((float)15 - 9.f) / 12.f) - (log2_440_over_32 + ((float)14 - 9.f) / 12.f);

    constexpr float semitone_tune_range = 12;
    int semitone_sel = std::roundf(semitone_tune_range * value);
    float semitone_offset = semitone_sel * semitone * (1 / noteGain);
    return semitone_offset;
}

void Layer::_defaultParamUpdate(float value, Layer &layer) {
    // No additional processing
}

float Layer::_noSmoothCommon(float value, float &prev_value, Layer &layer) {
    return 0;
}

float Layer::_noSmooth(float value, float &prev_value, Layer &layer) {
    prev_value = value;
    return value;
}

float Layer::_defaultSmooth(float value, float &prev_value, Layer &layer) {
    const float diff = value - prev_value;
    prev_value += (diff * ((PARAM_SMOOTH_COEFF) + (DYN_COEFF * std::fabs(diff))));
    return prev_value;
}

float Layer::_defaultTransform(float value, Layer &layer) {

    return value;
}

void inline Layer::_vcaDroneTransform(float value, Layer &layer) {
    bool tmp = value > PARAM_BOOL_FLOAT_COMP;

    layer.setDrone(tmp);
};

float Layer::_bipolarTransform(float value, Layer &layer) {

    return value * 2.f - 1.f;
}

float Layer::_transformCoarseTune(float value, Layer &layer) {

    //+-5 oct
    constexpr float gain = 5.f;
    constexpr float offset = -2.f;
    float num1 = (std::floor((value + 0.1) * gain) + offset) / noteGain;
    return num1;
}

float inline Layer::_defaultMorph(uint index, bool morphing, Layer &layer) {
    float a = layer._state_a_params[index];
    float b = layer._state_b_params[index];
    auto value = a;

    // Only morph if a and b actually differ
    if (a != b) {
        // The current morph value is the morph knob pos + the mod morph pos. We clip the value to make sure its limited to 0->1
        float morph_value = layer._cur_morph_value;

        // Calculate the min and max
        float min;
        float max;
        if (a < b) {
            // Morphing will increase the value from a to b
            min = a;
            max = b;
        } else {
            // Morphing will decrease the value from a to b
            // Note we also need to adjust the morph value so that
            // it decreases from a to the morph
            min = b;
            max = a;
            morph_value = 1.0 - morph_value;
        }

        // Perform the morph - linear interpolation
        value = (morph_value * (max - min)) + min;

        // Push the morph change to the param changes vector if either:
        // - We have just started or finished morphing OR
        // - We are currently morphing AND
        // - The morph value has changed AND
        // - We are in morph dance mode AND
        // - This is the 'active' or 'selected' layer
        // Note do not export this param change to any notification listeners
        if ((morphing || (morphing != layer._morphing)) &&
            ((layer._morph_mode == MorphMode::DANCE) && layer._current_layer)) {
            layer._param_changes.push_back(ParamChange((NinaParams::FIRST_LAYER_STATE_PARAM + index), value));
        }
    }
    return value;
}

float inline Layer::_nullMorph(uint index, bool morphing, Layer &layer) {
    return 0;
}

float inline Layer::_osc1TuneSum(float value, Layer &layer) {
    return 0;
}

float inline Layer::_osc2TuneSum(float value, Layer &layer) {
    return 0;
}

float inline Layer::_osc3TuneSum(float value, Layer &layer) {
    return 0;
}

float inline Layer::_currentMorphValue(float value, Layer &layer) {
    return layer._cur_morph_value;
}

float inline Layer::_disableTransform(float value, Layer &layer) {
    return 0.f;
}

float inline Layer::_keyPitchOffsetTransform(float value, Layer &layer) {
    value = (float)value * 127.f;
    value = 60.0;
    return -(midiNoteToCv((int)value)) / noteGain;
}

float inline Layer::_logPotTransform(float value, Layer &layer) {
    constexpr float log_pot_slope = 3.f;
    constexpr float zero_offset = std::exp2(0.f);
    constexpr float max_level_gain = 1.f / (std::exp2(log_pot_slope * 1.f) - zero_offset);

    const float val = (fastpow2(log_pot_slope * value) - zero_offset) * max_level_gain;
    return val;
}

float inline Layer::_xSquaredTransform(float value, Layer &layer) {
    return (value * value) * 2 - 1;
}

float inline Layer::_widthTransform(float value, Layer &layer) {
    return value;
}

float inline Layer::_glideTransform(float value, Layer &layer) {
    float val = value * value * value;
    return (1 - fastpow2(-(1.4 / ((float)BUFFER_RATE * 5. * val))));
    // return val;
}

void inline Layer::_wtSelectParamUpdate(float value, Layer &layer) {
    layer._wt_loader_a.loadWavetable(layer._state_a_params[LAYER_STATE_PARAMID_TO_INDEX(NinaParams::WavetableSelect)]);
    layer._wt_loader_b.loadWavetable(layer._state_b_params[LAYER_STATE_PARAMID_TO_INDEX(NinaParams::WavetableSelect)]);
}

float inline Layer::_vcaUnipolarTransform(float value, Layer &layer) {
    layer._layer_params.vca_unipolar = value > 0.5;
    return value;
}

void inline Layer::_odTransform(float value, Layer &layer) {

    float od_switch = layer._layer_params.state_params.at(LAYER_COMMON_PARAMID_TO_INDEX(NinaParams::VcfOverdrive));
    od_switch = value;
    od_switch = od_switch > 0.5 ? 1.f : 0.f;
    layer._layer_params.overDrive = (bool)(int)od_switch;
}

void inline Layer::_inputModeTransform(float value, Layer &layer) {
    InputModes mode = InputModes(std::round(value * InputModes::numInputModes));
    layer.setInputMode(mode);
}

float inline Layer::_xorModeTransform(float value, Layer &layer) {

    layer._layer_params.xor_mode = (NinaParams::XorNoiseModes)(int)std::round(value * 4);
    return value;
}

void inline Layer::_midiLowNoteFilterTransform(float value, Layer &layer) {

    int val = std::round(128.f * value);
    val = val - 1;
    layer.setMidiLowFilter(val);
}

void inline Layer::_midiHighNoteFilterTransform(float value, Layer &layer) {

    const uint val = std::round(128.f * value) + 1;
    layer.setMidiHighFilter(val);
}

void inline Layer::_lfo1TempoSyncTransform(float value, Layer &layer) {
    layer.setLfo1Sync(value > PARAM_BOOL_FLOAT_COMP);
}

void inline Layer::_lfo2TempoSyncTransform(float value, Layer &layer) {
    layer.setLfo2Sync(value > PARAM_BOOL_FLOAT_COMP);
}

void inline Layer::_wtInterpolateMode(float value, Layer &layer) {
    layer.setWtInterpolate(value > PARAM_BOOL_FLOAT_COMP);
}

void inline Layer::_midiChannelNoteFilterTransform(float value, Layer &layer) {
    uint chan = std::round(17 * value);
    bool enable = chan > 0;
    if (enable) {
        chan = chan - 1;
    }
    layer.setMidiChannel(chan, enable);
}

void inline Layer::_updateCvA(float value, Layer &layer) {
    NinaParams::CvInputMode mode = (NinaParams::CvInputMode)(int)std::round((float)NinaParams::CvInputMode::numModes * value);
    layer._setCvASource(mode);
}

void inline Layer::_updateCvB(float value, Layer &layer) {
    NinaParams::CvInputMode mode = (NinaParams::CvInputMode)(int)std::round((float)NinaParams::CvInputMode::numModes * value);
    layer._setCvBSource(mode);
}
} // namespace Nina
} // namespace Vst
} // namespace Steinberg
