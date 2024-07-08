/**
 * @file EffectsParameters.h
 * @brief Set the order and number of parameters in this file, this allows easy
 * reference in the processor and the controller as to which parameter we are
 * accessing
 * @version 0.1
 * @date 2021-10-15
 *
 * @copyright Copyright (c) 2023 Melbourne Instruments
 */

#pragma once
#include <array>
#include <string>

namespace Steinberg {
namespace Vst {
namespace Nina {
enum EffectsParamOrder {
    effectsMode,
    chorusmode,
    endChorusParams,
    delayTime,
    delayTimeSync,
    delayFeedback,
    delayTone,
    endDelayParams,
    reverbPreset,
    reverbDecay,
    reverbPreDelay,
    reverbEarlyMix,
    reverbTone,
    reverbShimmer,
    tempoSync,
    volume,
    chorusLevel,
    delayLevel,
    ReverbLevel,
    chorusSlot,
    delaySlot,
    reverbSlot,
    print,
    NumEffectsParams,
    effectspb,
    effectsmw
};

enum ChorusModes {
    I,
    II,
    I_II,
    numChorusModes
};
} // namespace Nina
} // namespace Vst
} // namespace Steinberg