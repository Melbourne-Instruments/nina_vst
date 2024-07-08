/**
 * @file NinaParameters.cpp
 * @brief Specifies the Nina parameters available to the processor.
 *
 * @copyright Copyright (c) 2023 Melbourne Instruments, Australia
 */
#include "NinaParameters.h"
#include "pluginterfaces/base/ustring.h"
#include <assert.h>
#include <sstream>
#include <unordered_map>

namespace Steinberg {
namespace Vst {
namespace Nina {
// Global param titles - must match the order in NinaParam
const char *NinaParams::_paramTitles[NinaParams::ModMatrixEntries] = {
    "Current Layer",
    "Morphing",
    "Morph Value",
    "Morph Mode",
    "Main Mute L",
    "Main Mute R",
    "Tuning Gain",
    "CV Offset 1",
    "CV Offset 2",
    "CV Offset 3",
    "CV Offset 4",
    "CV Gain 1",
    "CV Gain 2",
    "CV Gain 3",
    "CV Gain 4",
    "CV Input Mode 1",
    "CV Input Mode 2",
    "CV Input Mode 3",
    "CV Input Mode 4",
    "Master Tune",
    "MPE Lower Zone Pitchbend Range",
    "MPE Upper Zone Pitchbend Range",
    "MPE Y Bipolar Mode",
    "MPE Lower Zone Num Channels",
    "MPE Upper Zone Num Channels",
    "Layer State",
    "Layer Load",
    "All Notes Off",
    "Ex Input Mode",
    "Ex Input Gain",
    "Output Routing",
    "Layer Num Voices",
    "Midi Low Note Filter",
    "Midi High Note Filter",
    "Midi Channel Filter",
    "Midi Source Vel Mode",
    "Octave Offset",
    "CV A Select",
    "CV B Select",
    "Patch Volume",
    "Unison",
    "Unison Pan",
    "Pan Number",
    "Pan Mode",
    "Legato",
    "MPE Mode",
    "MPE X",
    "MPE Y",
    "MPE Z",
    "MPE Y Fall Time",
    "MPE Z Fall Time",
    "Lfo 1 Tempo Sync",
    "Lfo 2 Tempo Sync",
    "Wavetable Interpolate",
    "Pitch Bend",
    "Pitch Bend Range",
    "Mod Wheel",
    "Aftertouch",
    "Poly Aftertouch",
    "Expression",
    "Midi Mod Source",
    "VCF Overdrive",
    "Glide Mode",
    "Amp Env Drone",
    "Sustain",
    "Mod_Filter_Envelope:Morph",
    "Mod_Amp_Envelope:Morph",
    "Mod_LFO_1:Morph",
    "Mod_Wavetable:Morph",
    "Mod_Key_Pitch:Morph",
    "Mod_Key_Velocity:Morph",
    "Mod_Aftertouch:Morph",
    "Mod_Modwheel:Morph",
    "Mod_Expression_Pedal:Morph",
    "Mod_Pan_Position:Morph",
    "Mod_Time:Morph",
    "Mod_MIDI:Morph",
    "Mod_CV_A:Morph",
    "Mod_CV_B:Morph",
    "Mod_LFO_2:Morph",
    "Mod_Offset:Morph",
    "Run Tuning",
    "Main Vca Cal",
    "Mix Vca Cal",
    "Filter Cal",
    "Write Temps",
    "Reload Cal",
    "vca offset 1",
    "vca offset 2",
    "vca offset 3",
    "vca offset 4",
    "vca offset 5",
    "vca left offset",
    "vca right offset",
    "filter offset",
    "filter scale",
    "filter t track",
    "voice select",
    "voice cal write",
    "res high cal",
    "res low cal",
    "Trigger Print",
    "Misc Scale",
    "Time Rate",
    "VCO 1 Tune Fine",
    "VCO 1 Tune Octave",
    "VCO 1 Tune Semitone",
    "VCO 2 Tune Fine",
    "VCO 2 Tune Octave",
    "VCO 2 Tune Semitone",
    "Wave Tune Fine",
    "Wave Tune Coarse",
    "WT Select",
    "LFO Shape",
    "LFO 2 Shape",
    "LFO 1 Sync Rate",
    "LFO 2 Sync Rate",
    "VCF Drive",
    "VCF Key Track",
    "VCF 2 Pole Mode",
    "Glide",
    "Sub Osc",
    "Hard Sync",
    "Amp Env Velocity Sense",
    "Filt Env Velocity Sense",
    "Unison Spread",
    "Key Pitch Offset",
    "LFO Slew",
    "LFO Retrigger",
    "LFO 2 Slew",
    "LFO 2 Retrigger",
    "Noise Mode",
    "VCA unipolar mode",
    "Spin Reset",
    "Amp Env Reset",
    "Filt Env Reset",
    "Fine Tune Range",
    "Wave Slow Mode",
    "Current Morph Value"};

const char *NinaParams::_modMatrixSrcTitles[NinaParams::NumSrcs] =
    {
        "Filter Envelope",
        "Amp Envelope",
        "LFO 1",
        "Wavetable",
        "Key Pitch",
        "Key Velocity",
        "Aftertouch",
        "Modwheel",
        "Expression Pedal",
        "Pan Position",
        "Time",
        "MIDI",
        "CV A",
        "CV B",
        "LFO 2",
        "Offset",
        "Setting"};

const char *NinaParams::_modMatrixDstTitles[NinaParams::NumDsts] =
    {
        "LFO 1 Rate",
        "LFO 1 Gain",
        "LFO 2 Rate",
        "LFO 2 Gain",
        "VCA In",
        "Pan",
        "OSC 1 Pitch",
        "OSC 1 Width",
        "OSC 1 Blend",
        "OSC 1 Level",
        "OSC 2 Pitch",
        "OSC 2 Width",
        "OSC 2 Blend",
        "OSC 2 Level",
        "OSC 3 Pitch",
        "OSC 3 Shape",
        "OSC 3 Level",
        "XOR Level",
        "Filter Cutoff",
        "Filter Resonance",
        "Drive",
        "VCF Attack",
        "VCF Decay",
        "VCF Sustain",
        "VCF Release",
        "VCF Level",
        "VCA Attack",
        "VCA Decay",
        "VCA Sustain",
        "VCA Release",
        "VCA Level",
        "Spin Rate",

        // the dummy morph parameters are not actually used, they are just part of the mod matrix specification. The actual parameters controlling the mod gains are the "Morph..." common parameters
        "DummyMorph"};

const std::unordered_map<ParamID, const char *> NinaParams::_modMatrixAltTitles = {
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,         ModMatrixDst::Osc1Width),
     "VCO 1 Width"  },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,         ModMatrixDst::Osc1Blend),
     "VCO 1 Blend"  },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,         ModMatrixDst::Osc1Level),
     "Mix VCO 1"    },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,         ModMatrixDst::Osc2Width),
     "VCO 2 Width"  },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,         ModMatrixDst::Osc2Blend),
     "VCO 2 Blend"  },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,         ModMatrixDst::Osc2Level),
     "Mix VCO 2"    },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,         ModMatrixDst::Osc3Shape),
     "Wave Shape"   },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,         ModMatrixDst::Osc3Level),
     "Mix Wave"     },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,          ModMatrixDst::XorLevel),
     "Mix XOR"      },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,          ModMatrixDst::Lfo1Rate),
     "LFO 1 Rate"   },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,          ModMatrixDst::Lfo1Gain),
     "LFO 1 Gain"   },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,          ModMatrixDst::Lfo2Rate),
     "LFO 2 Rate"   },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,          ModMatrixDst::Lfo2Gain),
     "LFO 2 Gain"   },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,   ModMatrixDst::FilterResonance),
     "VCF Resonance"},
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,      ModMatrixDst::FilterCutoff),
     "VCF Cutoff"   },
    {MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::FilterEnvelope,      ModMatrixDst::FilterCutoff),
     "VCF Env Mod"  },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant, ModMatrixDst::FilterEnvelopeAtt),
     "VCF Attack"   },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant, ModMatrixDst::FilterEnvelopeDec),
     "VCF Decay"    },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant, ModMatrixDst::FilterEnvelopeSus),
     "VCF Sustain"  },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant, ModMatrixDst::FilterEnvelopeRel),
     "VCF Release"  },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,    ModMatrixDst::AmpEnvelopeAtt),
     "VCA Attack"   },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,    ModMatrixDst::AmpEnvelopeDec),
     "VCA Decay"    },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,    ModMatrixDst::AmpEnvelopeSus),
     "VCA Sustain"  },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,    ModMatrixDst::AmpEnvelopeRel),
     "VCA Release"  },
    {      MAKE_MOD_MATRIX_PARAMID(ModMatrixSrc::Constant,              ModMatrixDst::Spin),
     "Spin Rate"    },
};

float NinaParams::getGlobalParam(ParamID param_id, const GlobalParams &params) {
    assert(param_id < FIRST_LAYER_COMMON_PARAM);
    return params[param_id];
}

float NinaParams::getLayerCommonParam(ParamID param_id, const LayerCommonParams &params) {
    assert((param_id >= FIRST_LAYER_COMMON_PARAM) && (param_id < FIRST_LAYER_STATE_PARAM));
    return params[param_id - FIRST_LAYER_COMMON_PARAM];
}

float NinaParams::getLayerStateParam(ParamID param_id, const LayerStateParams &params) {
    assert((param_id >= FIRST_LAYER_STATE_PARAM) && (param_id < (FIRST_LAYER_STATE_PARAM + NUM_LAYER_STATE_PARAMS)));
    return params[param_id - FIRST_LAYER_STATE_PARAM];
}

float *NinaParams::getLayerCommonParamAddr(ParamID param_id, LayerCommonParams &params) {
    assert((param_id >= FIRST_LAYER_COMMON_PARAM) && (param_id < FIRST_LAYER_STATE_PARAM));
    return &params[param_id - FIRST_LAYER_COMMON_PARAM];
}

float *NinaParams::getLayerStateParamAddr(ParamID param_id, LayerStateParams &params) {
    assert((param_id >= FIRST_LAYER_STATE_PARAM) && (param_id < (FIRST_LAYER_STATE_PARAM + NUM_LAYER_STATE_PARAMS)));
    return &params[param_id - FIRST_LAYER_STATE_PARAM];
}

void NinaParams::setGlobalParam(ParamID param_id, float value, GlobalParams &params) {
    assert(param_id < FIRST_LAYER_COMMON_PARAM);
    params[param_id] = value;
}

void NinaParams::setLayerCommonParam(ParamID param_id, float value, LayerCommonParams &params) {
    assert((param_id >= FIRST_LAYER_COMMON_PARAM) && (param_id < FIRST_LAYER_STATE_PARAM));
    params[param_id - FIRST_LAYER_COMMON_PARAM] = value;
}

void NinaParams::setLayerStateParam(ParamID param_id, float value, LayerStateParams &params) {
    assert((param_id >= FIRST_LAYER_STATE_PARAM) && (param_id < (FIRST_LAYER_STATE_PARAM + NUM_LAYER_STATE_PARAMS)));
    params[param_id - FIRST_LAYER_STATE_PARAM] = value;
}

RangeParameter *NinaParams::toRangeParam(uint param_num, ParamValue minPlain, ParamValue maxPlain, ParamValue defaultValuePlain) {
    // Create the range param
    return new RangeParameter(USTRING(_paramTitles[param_num]), param_num, USTRING(""), minPlain, maxPlain, defaultValuePlain);
}

RangeParameter *NinaParams::toRangeParam(uint param_num, uint mod_src, uint mod_dst) {
    std::stringstream title;

    // Check the source and destination are within range
    assert(mod_src < ModMatrixSrc::NumSrcs);
    assert(mod_dst < ModMatrixDst::NumDsts);

    // Is there an alternate title for this entry?
    auto itr = _modMatrixAltTitles.find(param_num);
    if (itr != _modMatrixAltTitles.end()) {
        // Set the alternate title
        title << itr->second;
    } else {
        // Set the standard Mod Matrix entry title
        title << "Mod " << _modMatrixSrcTitles[mod_src] << ":" << _modMatrixDstTitles[mod_dst];
    }

    // Create the Mod Matrix range param
    return new RangeParameter(USTRING(title.str().c_str()), param_num, USTRING(""), 0, 1, 0.5);
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
