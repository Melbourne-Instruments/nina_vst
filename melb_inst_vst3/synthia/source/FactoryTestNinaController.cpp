/**
 * @file NinaController.cpp
 * @brief Nina Edit Controller implementation.
 *
 * @copyright Copyright (c) 2022-2023 Melbourne Instruments
 *
 */
#include "FactoryTestNinaController.h"
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

/**
 * @note	Define GUID for controller
 *
 */
FUID FactoryTestController::uid(0x0731F529, 0x598B4613, 0xAE34E4D7, 0xA01B7DE1);

tresult PLUGIN_API FactoryTestController::initialize(FUnknown *context) {

    // Initialise the base edit controller
    tresult result = EditController::initialize(context);
    if (result == kResultTrue) {
        Parameter *param;
        std::stringstream string;
        string << "mute_loopback_left";
        std::string cstring = (string.str());
        param = new RangeParameter(USTRING(cstring.c_str()), 0, USTRING(""));

        parameters.addParameter(param);
        string = std::stringstream();
        string << "mute_loopback_right";
        cstring = (string.str());
        param = new RangeParameter(USTRING(cstring.c_str()), 1, USTRING(""));

        parameters.addParameter(param);
        constexpr int num_voice_params = 23;
        for (int voice = 0; voice < 12; voice++) {
            string = std::stringstream();
            string << "voice_" << voice << "_vca_sqr_1";
            cstring = (string.str());

            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 0 + voice * num_voice_params, USTRING(""));
            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_vca_tri_1";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 1 + voice * num_voice_params, USTRING(""));

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_vca_sqr_2";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 2 + voice * num_voice_params, USTRING(""));

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_vca_tri_2";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 3 + voice * num_voice_params, USTRING(""));

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_vca_xor";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 4 + voice * num_voice_params, USTRING(""));

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_wt_noise";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 5 + voice * num_voice_params, USTRING(""));

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_res";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 6 + voice * num_voice_params, USTRING(""));

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_cutoff";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 7 + voice * num_voice_params, USTRING(""));

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_overdrive";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 8 + voice * num_voice_params, USTRING(""));

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_vca_left";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 9 + voice * num_voice_params, USTRING(""));

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_vca_right";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 10 + voice * num_voice_params, USTRING(""));
            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_osc_sub";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 11 + voice * num_voice_params, USTRING(""));
            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_osc_sync";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 12 + voice * num_voice_params, USTRING(""));
            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_mute_1";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 2 + 13 + voice * num_voice_params, USTRING(""));

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_mute_2";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 16 + voice * num_voice_params, USTRING(""));

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_mute_3";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 17 + voice * num_voice_params, USTRING(""));

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_mute_4";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 18 + voice * num_voice_params, USTRING(""));

            parameters.addParameter(param);
            string = std::stringstream();
            string << "voice_" << voice << "_osc_shape_1";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 19 + voice * num_voice_params, USTRING(""), 0, 1, 0.5);

            parameters.addParameter(param);
            string = std::stringstream();
            string << "voice_" << voice << "_osc_shape_2";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 20 + voice * num_voice_params, USTRING(""), 0, 1, 0.5);

            parameters.addParameter(param);
            string = std::stringstream();
            string << "voice_" << voice << "_sine";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 21 + voice * num_voice_params, USTRING(""), 0, 1, 0.5);

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_hardsync";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 22 + voice * num_voice_params, USTRING(""), 0, 1, 0.5);

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_osc_2_oct_down";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 23 + voice * num_voice_params, USTRING(""), 0, 1, 0.5);

            parameters.addParameter(param);

            string = std::stringstream();
            string << "voice_" << voice << "_2_pole";
            cstring = (string.str());
            param = new RangeParameter(USTRING(cstring.c_str()), 24 + voice * num_voice_params, USTRING(""), 0, 1, 0.5);

            parameters.addParameter(param);
        }
    }
    return kResultTrue;
}

tresult PLUGIN_API FactoryTestController::terminate() {
    return EditController::terminate();
}

tresult PLUGIN_API FactoryTestController::setParamNormalizedFromFile(ParamID tag,
    ParamValue value) {

    Parameter *pParam = EditController::getParameterObject(tag);

    if (!pParam)
        return kResultFalse;

    return setParamNormalized(tag, pParam->toNormalized(value));
}

tresult PLUGIN_API FactoryTestController::setComponentState(IBStream *fileStream) {

    return kResultTrue;
}

/**
 * @note will be queried 129 times for control messages
 *
 */
tresult PLUGIN_API FactoryTestController::getMidiControllerAssignment(
    int32 busIndex, int16 channel, CtrlNumber midiControllerNumber,
    ParamID &id /*out*/) {

    return kResultFalse;
}

ParamValue PLUGIN_API
FactoryTestController::plainParamToNormalized(ParamID tag, ParamValue plainValue) {
    return EditController::plainParamToNormalized(tag, plainValue);
}

ParamValue PLUGIN_API
FactoryTestController::normalizedParamToPlain(ParamID tag, ParamValue valueNormalized) {
    return EditController::normalizedParamToPlain(tag, valueNormalized);
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
