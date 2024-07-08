/**
 * @file EffectsController.cpp
 * @brief
 * @version 0.1
 * @date 2021-09-30
 *
 * @copyright Copyright (c) 2023 Melbourne Instruments
 *
 */

#include "EffectsController.h"
#include "EffectsParameters.h"
#include "base/source/fstreamer.h"
#include "base/source/fstring.h"
#include "pluginterfaces/base/futils.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include <sstream>
#include <string>

namespace Steinberg {
namespace Vst {
namespace Nina {

/**
 * @note	Define GUID for controller
 *
 */
FUID EffectsController::uid(0x5A5E23D1, 0xA462491E, 0xB0C1169D, 0x168B345B);

tresult PLUGIN_API EffectsController::initialize(FUnknown *context) {

    tresult result = EditController::initialize(context);
    if (result == kResultTrue) {
        Parameter *param;
        param = new RangeParameter(USTRING("FX Type Select"), EffectsParamOrder::effectsMode, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Chorus Mode"), EffectsParamOrder::chorusmode, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Delay Time"), EffectsParamOrder::delayTime, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Delay Time Sync"), EffectsParamOrder::delayTimeSync, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Delay Feedback"), EffectsParamOrder::delayFeedback, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Delay Tone"), EffectsParamOrder::delayTone, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Reverb Preset"), EffectsParamOrder::reverbPreset, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Reverb Tone"), EffectsParamOrder::reverbTone, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Reverb Decay"), EffectsParamOrder::reverbDecay, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Reverb Early Mix"), EffectsParamOrder::reverbEarlyMix, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Reverb Predelay"), EffectsParamOrder::reverbPreDelay, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Reverb Shimmer"), EffectsParamOrder::reverbShimmer, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Tempo Sync"), EffectsParamOrder::tempoSync, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("FX Send Level"), EffectsParamOrder::volume, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Chorus Level"), EffectsParamOrder::chorusLevel, USTRING(""), 0.f, 1.f, 1.f);
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Delay Level"), EffectsParamOrder::delayLevel, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Reverb Level"), EffectsParamOrder::ReverbLevel, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Chorus Slot"), EffectsParamOrder::chorusSlot, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Delay Slot"), EffectsParamOrder::delaySlot, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("Reverb Slot"), EffectsParamOrder::reverbSlot, USTRING(""));
        parameters.addParameter(param);
        param = new RangeParameter(USTRING("FX Print"), EffectsParamOrder::print, USTRING(""));
        parameters.addParameter(param);
    }

    return kResultTrue;
    printf("effects init");
}

tresult PLUGIN_API EffectsController::terminate() {
    return EditController::terminate();
}

tresult PLUGIN_API EffectsController::setParamNormalizedFromFile(ParamID tag,
    ParamValue value) {

    Parameter *pParam = EditController::getParameterObject(tag);

    if (!pParam)
        return kResultFalse;

    return setParamNormalized(tag, pParam->toNormalized(value));
}

tresult PLUGIN_API EffectsController::setComponentState(IBStream *fileStream) {

    return kResultTrue;
}

/**
 * @note will be quiried 129 times for control messages
 *
 */
tresult PLUGIN_API EffectsController::getMidiControllerAssignment(
    int32 busIndex, int16 channel, CtrlNumber midiControllerNumber,
    ParamID &id /*out*/) {
    if ((channel == 0) && (midiControllerNumber == kPitchBend)) {
        id = EffectsParamOrder::effectspb;
        return kResultTrue;
    }
    if (midiControllerNumber == kCtrlModWheel) {
        id = EffectsParamOrder::effectsmw;
        return kResultTrue;
    }

    return kResultFalse;
}

ParamValue PLUGIN_API
EffectsController::plainParamToNormalized(ParamID tag, ParamValue plainValue) {
    return EditController::plainParamToNormalized(tag, plainValue);
}

ParamValue PLUGIN_API
EffectsController::normalizedParamToPlain(ParamID tag, ParamValue valueNormalized) {
    return EditController::normalizedParamToPlain(tag, valueNormalized);
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
