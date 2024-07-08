/**
 * @file EffectsProcessor.h
 * @brief
 * @version 0.1
 * @date 2021-09-30
 *
 * @copyright Copyright (c) 2023 Melbourne Instruments
 */

#pragma once

#include "ChorusEngine.h"
#include "EffectsParameters.h"
#include "NinaDelay.h"
#include "NinaEffectsLimiter.h"
#include "NinaExpander.h"
#include "NinaLogger.h"
#include "NinaReverb.h"
#include "common.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "public.sdk/source/vst/vstparameters.h"
#include <atomic>
#include <thread>

namespace Steinberg {
namespace Vst {
namespace Nina {

//#define DEBUGPRINTING
constexpr uint FX_ROWS = 3;
constexpr uint FX_SLOTS = 3;

// Nina Processor class
class EffectsProcessor : public AudioEffect {
  public:
    EffectsProcessor();
    ~EffectsProcessor();

    /**
     * @brief
     *
     * @param context
     * @return tresult
     */
    tresult PLUGIN_API initialize(FUnknown *context);

    /**
     * @brief Set the Bus Arrangements object
     *
     * @param inputs speaker arrangement
     * @param numIns number of inputs
     * @param outputs speaker arrangement
     * @param numOuts
     * @return tresult
     */
    tresult PLUGIN_API setBusArrangements(SpeakerArrangement *inputs,
        int32 numIns,
        SpeakerArrangement *outputs,
        int32 numOuts);

    /**
     * @brief
     *
     * @param symbolicSampleSize
     * @return tresult
     */
    tresult PLUGIN_API canProcessSampleSize(int32 symbolicSampleSize);

    /**
     * @brief Set the Active object
     *
     * @param state
     * @return tresult
     */
    tresult PLUGIN_API setActive(TBool state);

    /**
     * @brief Set the State object
     *
     * @param fileStream
     * @return tresult
     */
    tresult PLUGIN_API setState(IBStream *fileStream);
    /**
     * @brief Get the State object
     *
     * @param fileStream
     * @return tresult
     */
    tresult PLUGIN_API getState(IBStream *fileStream);

    /**
     * @brief
     *
     * @param data
     * @return tresult
     */
    tresult PLUGIN_API process(ProcessData &data);

    /**
     * @brief Create a Instance object
     *
     * @return FUnknown*
     */
    static FUnknown *createInstance(void *) {
        return (IAudioProcessor *)new EffectsProcessor();
    }

    void debugPrinting(float data1[12][BUFFER_SIZE], float data2[12][BUFFER_SIZE], float data3[BUFFER_SIZE]);

    ChorusEngine cengine = ChorusEngine(SAMPLE_RATE);
    NinaReverb _reverb;
    BBDDelay _delay_left;
    BBDDelay _delay_right;
    NinaLimiter _limiter;
    NinaLimiter _limiter_2;
    OutputLimiter _output_limiter;
    float _volume = 1;
    float vol_smooth;
    float _chorus_lfo_1 = 0;
    int printcounter = 0;
    float _chorus_lfo_2 = 0;
    ChorusModes _chorus_mode = I;
    bool _output_duck = true;
    int _duck_counter = 0;
    bool _tempo_sync = false;
    float _global_tempo = 1.f;
    bool _print = false;
    float _chorus_level = 1;
    float _chorus_evel_smooth = 1;
    float _delay_level = 1;
    float _reverb_level = 1;
    float _dry_level_filtered = 0;
    NinaExpander _expander;
    uint _delay_slot = 0;
    uint _chorus_slot = 0;
    ;
    uint _reverb_slot = 0;

    static constexpr float SERVO_CUT = 0.1f;
    static constexpr float SERVO_RATE = (2.f * M_PI * SERVO_CUT * (1 / (float)SAMPLE_RATE)) / (2.f * M_PI * SERVO_CUT * (1 / (float)SAMPLE_RATE) + 1);
    static constexpr int DELAY_SIZE = 323766; // 705600; //95998; //32766;  //set max delay time at max sample rate
    std::array<float, DELAY_SIZE + 2> _delay_buffer;

    consteval float sample_rate() { return 96000.f; }

    /**
     * @brief GUID for the processor
     *
     */
    static FUID uid;

    std::array<float, EffectsParamOrder::NumEffectsParams> _params;
    Logger *logger;

// use this to dump 1 second of the wavetable signal to a file, could be used to output any signal. very useful for realtime debugging
//#define SIGDUMP
#ifdef SIGDUMP
    int counter3 = 0;
    std::vector<float> buf;
#endif
    std::thread _printThread;

    /**
     * @brief Process VST events (MIDI)
     */
    void processEvents(IEventList *events);

    /**
     * @brief
     *
     * @param param_changes
     * @return true
     * @return false
     */
    bool processParameterChanges(IParameterChanges *param_changes);
    /**
     * @brief Handle MIDI note ON event
     */
    void handleMidiNoteOnEvent(const MidiNote &midi_note);

    /**
     * @brief Handle MIDI note OFF event
     */
    void handleMidiNoteOffEvent(const MidiNote &midi_note);
    void processAudio(ProcessData &data);
    int counter_p = 0;
    void printDebugOutput();

    /**
     * @brief helper function to update all the parameter values if there has been
     * any changes
     */
    void _updateParams();

    std::vector<ParamID> _changed_param_ids;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
