/**
 * @file NinaController.cpp
 * @brief Nina Edit Controller implementation.
 *
 * @copyright Copyright (c) 2022-2023 Melbourne Instruments, Australia
 */
#include "NinaController.h"
#include "base/source/fstreamer.h"
#include "base/source/fstring.h"
#include "common.h"
#include "pluginterfaces/base/futils.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include <sstream>
#include <string>

namespace Steinberg {
namespace Vst {
namespace Nina {
//#define PRINTCONTROL

/**
 * @note	Define GUID for controller
 *
 */
FUID Controller::uid(0x9946CBA3, 0x7DAE476F, 0x9CA51792, 0x709DB0D3);

tresult PLUGIN_API Controller::initialize(FUnknown *context) {

    // Initialise the base edit controller
    tresult result = EditController::initialize(context);
    if (result == kResultTrue) {
        Parameter *param;

        // Add the global params
        param = NinaParams::NinaParams::toRangeParam(NinaParams::CurrentLayer);
        parameters.addParameter(param);
        current_layer = 0;

        // set default number of voices of layer 1 to 12
        param = NinaParams::NinaParams::toRangeParam(NinaParams::Layer1NumVoices, 0.f, 1.f, 1.0f);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::Morphing);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::MorphValue);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::MorphMode);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::MainOutMuteL);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::MainOutMuteR);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::TuningGain);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Cv1Offset, 0.f, 1.f, 0.5f);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Cv2Offset, 0.f, 1.f, 0.5f);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Cv3Offset, 0.f, 1.f, 0.5f);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Cv4Offset, 0.f, 1.f, 0.5f);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Cv1Gain, 0.f, 1.f, 0.5f);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Cv2Gain, 0.f, 1.f, 0.5f);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Cv3Gain, 0.f, 1.f, 0.5f);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Cv4Gain, 0.f, 1.f, 0.5f);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Cv1InputMode);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Cv2InputMode);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Cv3InputMode);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Cv4InputMode);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MasterTune, 0.f, 1.f, 0.5f);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MpeLowerZonePB, 0.f, 1.f, NinaParams::MpeDefStartValue);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MpeUpperZonePB, 0.f, 1.f, NinaParams::MpeDefStartValue);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MpeYBipolarMode);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MpeLowerZoneNumChannels);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MpeUpperZoneNumChannels);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::TuningGain);
        parameters.addParameter(param);

        // Add the Layer params
        // Add the common Layer params
        param = NinaParams::NinaParams::toRangeParam(NinaParams::LayerState);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::LayerLoading);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::AllNotesOff, 0.f, 1.f, 1.f);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::extInMode);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::extInGain);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::outputRouting);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::midiLowNote);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::midiHighNote);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::midiChannelFilter);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::midiSourceVelMode);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::octaveOffset, 0.f, 1.f, 0.5f);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::cvASelect);
        parameters.addParameter(param);
        param = NinaParams::NinaParams::toRangeParam(NinaParams::cvBSelect, 0.f, 1.f, 0.25f);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::WavetableSelect);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::NumUnison);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::PanNum);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::PanMode);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::UnisonSpread);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::UnisonPan);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Legato);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MpeMode);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MpeX);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MpeY);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MpeZ);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MpeZFallTime);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MpeYFallTime);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Lfo1TempoSync);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Lfo2TempoSync);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::WtInterpolateMode);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MidiPitchBend, 0, 1, 0.5);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MidiModWheel);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MidiAftertouch);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MidiExpression);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MidiSource);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::GlideMode);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Sustain);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphEg1);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphEg2);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphLfo1);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphWave);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphKBD);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphVel);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphAfter);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphMod);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphExp);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphPan);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphTime);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphMidi);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphCvA);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphCvB);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphLfo2);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::MorphOffset);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::AllRunTuning);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::RunFilterCal);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::RunMixVcaCal);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::RunMainVcaCal);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::writeTemp);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::reloadCal);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::vca_offset_1);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::vca_offset_2);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::vca_offset_3);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::vca_offset_4);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::vca_offset_5);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::main_vca_offset_1);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::main_vca_offset_2);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::filter_offset);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::filter_scale);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::filter_t_coeff);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::voice_cal_select);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::voice_cal_write);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::res_high);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::res_low);
        parameters.addParameter(param);
#ifdef PRINTCONTROL
        param = NinaParams::toRangeParam(NinaParams::TriggerPrint);
        parameters.addParameter(param);
#endif
        param = NinaParams::toRangeParam(NinaParams::MiscScale);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::TimeRate);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::PitchBendRange);
        parameters.addParameter(param);

        // Add the Layer State base params
        // VCO 1
        param = NinaParams::toRangeParam(NinaParams::Vco1TuneFine);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Vco1TuneCoarse);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Vco1TuneSemitone);
        parameters.addParameter(param);
        // VCO 2
        param = NinaParams::toRangeParam(NinaParams::Vco2TuneFine);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Vco2TuneCoarse);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Vco2TuneSemitone);
        parameters.addParameter(param);
        // Wave
        param = NinaParams::toRangeParam(NinaParams::WaveTuneFine);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::WaveTuneCoarse);
        parameters.addParameter(param);
        // LFO
        param = NinaParams::toRangeParam(NinaParams::LfoShape);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Lfo2Shape);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Lfo1SyncRate);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Lfo2SyncRate);
        parameters.addParameter(param);
        // VCF
        param = NinaParams::toRangeParam(NinaParams::VcfDrive);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::VcfOverdrive);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::VcfKeyTrack);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::VcfMode);
        parameters.addParameter(param);
        // Others
        param = NinaParams::toRangeParam(NinaParams::Glide);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::SubOscillator);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::HardSync);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::AmpEnvVelSense);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::FiltEnvVelSense);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::KeyPitchOffset);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::LfoSlew);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Lfo2Slew);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::LfoRetrigger);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Lfo2Retrigger);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::PatchVolume, 0, 1, 1.0);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::XorMode);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::VcaUnipolar);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::SpinReset);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::VcaEnvReset);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::VcfEnvReset);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::AmpEnvelopeDrone);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::FineTuneRange);
        parameters.addParameter(param);
        param = NinaParams::toRangeParam(NinaParams::Osc3Low, 0, 1, 0.f);
        parameters.addParameter(param);

        // RO params
        param = NinaParams::toRangeParam(NinaParams::CurrentMorphValue);
        parameters.addParameter(param);

        // State Mod Matrix entries
        uint param_num = NinaParams::ModMatrixEntries;
        for (uint mod_src = 0; mod_src < NinaParams::ModMatrixSrc::NumSrcs; mod_src++) {
            for (uint mod_dst = 0; mod_dst < NinaParams::ModMatrixDst::NumDsts; mod_dst++) {
                param = NinaParams::toRangeParam(param_num++, mod_src, mod_dst);
                parameters.addParameter(param);
            }
        }
    }
    return kResultTrue;
}

tresult PLUGIN_API Controller::terminate() {
    return EditController::terminate();
}

tresult PLUGIN_API Controller::setParamNormalized(ParamID tag, ParamValue value) {
    bool current_layer_param = (mask_param_id(tag) == NinaParams::CurrentLayer);
    bool layers_specified = get_layers(tag);
    tresult res = kResultFalse;

    // Set this param value if:
    // - No layers are specified - this means it has been returned by the processor
    //   via morphing and is always for the current layer, or
    // - The param is for this for this layer, or
    // - It is the "current layer" param
    if ((layers_specified == 0) || is_layer_param(tag, current_layer) || current_layer_param) {
        // Set the param value
        res = EditController::setParamNormalized(tag, value);
        if (res == kResultTrue) {
            // Is this the "current layer" param?
            if (current_layer_param) {
                // Save the current layer as an integer
                current_layer = std::round(NUM_LAYERS * value);
            }
        }
    }
    return res;
}

tresult PLUGIN_API Controller::setParamNormalizedFromFile(ParamID tag,
    ParamValue value) {

    Parameter *pParam = EditController::getParameterObject(tag);

    if (!pParam)
        return kResultFalse;

    return setParamNormalized(tag, pParam->toNormalized(value));
}

tresult PLUGIN_API Controller::setComponentState(IBStream *fileStream) {

    return kResultTrue;
}

/**
 * @note will be queried 129 times for control messages
 *
 */
tresult PLUGIN_API Controller::getMidiControllerAssignment(
    int32 busIndex, int16 channel, CtrlNumber midiControllerNumber,
    ParamID &id /*out*/) {
    if ((midiControllerNumber == kPitchBend)) {
        id = NinaParams::MidiPitchBend;
        return kResultTrue;
    }
    if (midiControllerNumber == kCtrlModWheel) {
        id = NinaParams::MidiModWheel;
        return kResultTrue;
    }
    if (midiControllerNumber == kAfterTouch) {
        id = NinaParams::Aftertouch;
        return kResultTrue;
    }
    if (midiControllerNumber == kCtrlAllNotesOff) {
        id = NinaParams::AllNotesOff;
        return kResultTrue;
    }

    return kResultFalse;
}

ParamValue PLUGIN_API
Controller::plainParamToNormalized(ParamID tag, ParamValue plainValue) {
    return EditController::plainParamToNormalized(tag, plainValue);
}

ParamValue PLUGIN_API
Controller::normalizedParamToPlain(ParamID tag, ParamValue valueNormalized) {
    return EditController::normalizedParamToPlain(tag, valueNormalized);
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
