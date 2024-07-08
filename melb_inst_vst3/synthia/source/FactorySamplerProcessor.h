#pragma once

#include "FactorySampleMsgQueue.h"
#include "NinaLogger.h"
#include "common.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "public.sdk/source/vst/vstparameters.h"
#include <atomic>
#include <fstream>
#include <thread>

namespace Steinberg {
namespace Vst {
namespace Nina {

//#define DEBUGPRINTING

// Nina Processor class
class FactorySamplerProcessor : public AudioEffect {
  public:
    FactorySamplerProcessor();
    ~FactorySamplerProcessor();

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
        return (IAudioProcessor *)new FactorySamplerProcessor();
    }

    /**
     * @brief GUID for the processor
     *
     */
    static FUID uid;

    std::array<float, 2> _params;
    Logger *logger;

// use this to dump 1 second of the wavetable signal to a file, could be used to output any signal. very useful for realtime debugging
//#define SIGDUMP
#ifdef SIGDUMP
    int counter3 = 0;
    std::vector<float> buf;
#endif
    std::thread *_smplr_msg_thread;
    std::atomic<bool> _exit_smplr_msg_thread;
    uint _smplr_buffers_count;
    int _sample_counter = 0;
    float _msg_samples_1[SAMPLER_MSG_SIZE];
    float _msg_samples_2[SAMPLER_MSG_SIZE];
    std::atomic<float *> _current_msg_samples;
    std::atomic<bool> _msg_samples_ready;
    bool _en_sampling = false;
    bool _run_sampling = false;
    bool _done_write = true;
    float _sine_gain = .1;
    int _capture_length = 1;
    int _captured_length = 0;
    float _sine_phase = 0;
    const float _sine_inc = 500.f / (float)SAMPLE_RATE;

    static constexpr int samples = CHANNELS * SAMPLE_RATE * 1.0f;
    std::array<float, (int)(samples)> *sample_store;
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
    void _updateParams(const std::vector<ParamID> &changed_param_ids);

    /**
     * @brief Process GUI message thread function
     */
    void _processSamplerMsg();
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
