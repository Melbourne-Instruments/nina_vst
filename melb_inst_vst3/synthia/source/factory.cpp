/**
 * @file factory.cpp
 * @brief
 * @version 0.1
 * @date 2021-09-30
 *
 * @copyright Copyright (c) 2023 Melbourne Instruments
 *
 */

#include "EffectsController.h"
#include "EffectsProcessor.h"
#include "FactorySamplerController.h"
#include "FactorySamplerProcessor.h"
#include "FactoryTestNinaController.h"
#include "FactoryTestNinaProcessor.h"
#include "NinaController.h"
#include "NinaProcessor.h"
#include "public.sdk/source/main/pluginfactory.h"
#include "version.h"

//-----------------------------------------------------------------------------
bool InitModule() { return true; }

bool DeinitModule() { return true; }

//-----------------------------------------------------------------------------
#define kVersionString FULL_VERSION_STR

BEGIN_FACTORY_DEF(stringCompanyName, "", "")

//-----------------------------------------------------------------------------
// -- Nina VST
DEF_CLASS2(INLINE_UID_FROM_FUID(Vst::Nina::FactoryTestProcessor::uid),
    PClassInfo::kManyInstances, kVstAudioEffectClass, "factory test",
    Vst::kDistributable, Vst::PlugType::kInstrumentSynth, kVersionString,
    kVstVersionString, Vst::Nina::FactoryTestProcessor::createInstance)

DEF_CLASS2(INLINE_UID_FROM_FUID(Vst::Nina::FactoryTestController::uid),
    PClassInfo::kManyInstances, kVstComponentControllerClass,
    "factory test", Vst::kDistributable, "", kVersionString,
    kVstVersionString, Vst::Nina::FactoryTestController::createInstance) //-----------------------------------------------------------------------------
DEF_CLASS2(INLINE_UID_FROM_FUID(Vst::Nina::FactorySamplerProcessor::uid),
    PClassInfo::kManyInstances, kVstAudioEffectClass, "factory sampler",
    Vst::kDistributable, Vst::PlugType::kAnalyzer, kVersionString,
    kVstVersionString, Vst::Nina::FactorySamplerProcessor::createInstance)

DEF_CLASS2(INLINE_UID_FROM_FUID(Vst::Nina::FactorySamplerController::uid),
    PClassInfo::kManyInstances, kVstComponentControllerClass,
    "factory sampler", Vst::kDistributable, "", kVersionString,
    kVstVersionString, Vst::Nina::FactorySamplerController::createInstance) //-----------------------------------------------------------------------------
DEF_CLASS2(INLINE_UID_FROM_FUID(Vst::Nina::EffectsProcessor::uid),
    PClassInfo::kManyInstances, kVstAudioEffectClass, "effects vst",
    Vst::kDistributable, Vst::PlugType::kFxModulation, kVersionString,
    kVstVersionString, Vst::Nina::EffectsProcessor::createInstance)

DEF_CLASS2(INLINE_UID_FROM_FUID(Vst::Nina::EffectsController::uid),
    PClassInfo::kManyInstances, kVstComponentControllerClass,
    "effects vst", Vst::kDistributable, "", kVersionString,
    kVstVersionString, Vst::Nina::EffectsController::createInstance)
DEF_CLASS2(INLINE_UID_FROM_FUID(Vst::Nina::Processor::uid),
    PClassInfo::kManyInstances, kVstAudioEffectClass, "nina vst",
    Vst::kDistributable, Vst::PlugType::kInstrumentSynth, kVersionString,
    kVstVersionString, Vst::Nina::Processor::createInstance)

DEF_CLASS2(INLINE_UID_FROM_FUID(Vst::Nina::Controller::uid),
    PClassInfo::kManyInstances, kVstComponentControllerClass,
    "nina vsts", Vst::kDistributable, "", kVersionString,
    kVstVersionString, Vst::Nina::Controller::createInstance)

END_FACTORY
