/**
 * @file EffectsController.cpp
 * @brief
 * @version 0.1
 * @date 2021-09-30
 *
 * @copyright Copyright (c) 2023 Melbourne Instruments
 *
 */

#include "FactorySamplerController.h"
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
FUID FactorySamplerController::uid(0xE4A91EB3, 0x7E9E4906, 0x8DC3CC64, 0x4E087C50);

tresult PLUGIN_API FactorySamplerController::initialize(FUnknown *context) {

    tresult result = EditController::initialize(context);
    if (result == kResultTrue) {
        Parameter *param;
        param = new RangeParameter(USTRING("Trigger Sampling"), 0, USTRING(""));
        parameters.addParameter(param);
        auto string = std::stringstream();
        string << "FX"
               << "_sine_level";
        auto cstring = (string.str());
        param = new RangeParameter(USTRING(cstring.c_str()), 1, USTRING(""), 0, 1, 0.5);

        parameters.addParameter(param);
    }

    printf("effects init %d", result);
    return kResultTrue;
}

tresult PLUGIN_API FactorySamplerController::terminate() {
    return EditController::terminate();
}

tresult PLUGIN_API FactorySamplerController::setParamNormalizedFromFile(ParamID tag,
    ParamValue value) {

    Parameter *pParam = EditController::getParameterObject(tag);

    if (!pParam)
        return kResultFalse;

    return setParamNormalized(tag, pParam->toNormalized(value));
}

tresult PLUGIN_API FactorySamplerController::setComponentState(IBStream *fileStream) {

    return kResultTrue;
}

/**
 * @note will be quiried 129 times for control messages
 *
 */
tresult PLUGIN_API FactorySamplerController::getMidiControllerAssignment(
    int32 busIndex, int16 channel, CtrlNumber midiControllerNumber,
    ParamID &id /*out*/) {
    return kResultFalse;
}

ParamValue PLUGIN_API
FactorySamplerController::plainParamToNormalized(ParamID tag, ParamValue plainValue) {
    return EditController::plainParamToNormalized(tag, plainValue);
}

ParamValue PLUGIN_API
FactorySamplerController::normalizedParamToPlain(ParamID tag, ParamValue valueNormalized) {
    return EditController::normalizedParamToPlain(tag, valueNormalized);
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
