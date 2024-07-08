
#include "FactorySamplerProcessor.h"
#include "FactorySampleMsgQueue.h"
#include "FactorySamplerController.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/base/futils.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include <chrono>
#include <math.h>

// --- VST2 wrapper
namespace Steinberg {
namespace Vst {
namespace Nina {

// Constants
#define SAMPLERTHREAD

FUID FactorySamplerProcessor::uid(0x84BA0B17, 0xE7A14FAB, 0x933B3C7D, 0x0149BF8A);

FactorySamplerProcessor::FactorySamplerProcessor() {
    setControllerClass(FactorySamplerController::uid);
    // Create a wavetable oscillator and start the GUI message thread

#ifdef SAMPLERTHREAD
    _exit_smplr_msg_thread = false;
    _smplr_buffers_count = 0;
    _current_msg_samples.store(_msg_samples_1);
    _smplr_msg_thread = new std::thread(&FactorySamplerProcessor::_processSamplerMsg, this);
    sample_store = new std::array<float, (int)(CHANNELS * SAMPLE_RATE * 1.0f)>;
    for (auto &item : *sample_store) {
        item = 0;
    }
#endif
}

FactorySamplerProcessor::~FactorySamplerProcessor() {
// If the GUI message thread is running
#ifdef SAMPLERTHREAD
    if (_smplr_msg_thread) {
        // Stop the GUI message thread
        _exit_smplr_msg_thread = true;
        _smplr_msg_thread->join();
    }
#endif
}

tresult PLUGIN_API FactorySamplerProcessor::initialize(FUnknown *context) {
    tresult result = AudioEffect::initialize(context);
    if (result == kResultTrue) {
        printf("\nhere %d", result);
        addAudioInput(STR16("In 0"), SpeakerArr::kMono);
        addAudioInput(STR16("In 1"), SpeakerArr::kMono);
        addAudioInput(STR16("In 2"), SpeakerArr::kMono);
        addAudioInput(STR16("In 3"), SpeakerArr::kMono);
        addAudioInput(STR16("In 4"), SpeakerArr::kMono);
        addAudioInput(STR16("In 5"), SpeakerArr::kMono);

        addAudioOutput(STR16("out 0"), SpeakerArr::kMono);
        addAudioOutput(STR16("out 1"), SpeakerArr::kMono);
        // MIDI event input bus, one channel
    }
    return result;
}

tresult PLUGIN_API FactorySamplerProcessor::setState(IBStream *fileStream) {

    return kResultTrue;
}

tresult PLUGIN_API FactorySamplerProcessor::getState(IBStream *fileStream) {

    return kResultTrue;
}

tresult PLUGIN_API FactorySamplerProcessor::setBusArrangements(SpeakerArrangement *inputs,
    int32 numIns,
    SpeakerArrangement *outputs,
    int32 numOuts) {

    if (numIns == 6 && numOuts == 2 && outputs[0] == SpeakerArr::kMono) {
        auto val = AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
        return val;
    }
    return kResultFalse;
}

tresult PLUGIN_API FactorySamplerProcessor::canProcessSampleSize(int32 symbolicSampleSize) {

    // --- currently 32 bit only
    if (symbolicSampleSize == kSample32) {
        return kResultTrue;
    }
    return kResultFalse;
}

tresult PLUGIN_API FactorySamplerProcessor::setActive(TBool state) {
    if (state) {
    } else {
    }

    // call base class method
    return AudioEffect::setActive(state);
}

tresult PLUGIN_API FactorySamplerProcessor::process(ProcessData &data) {
    // Process all events
    processEvents(data.inputEvents);
    processParameterChanges(data.inputParameterChanges);
    processAudio(data);

    return kResultTrue;
}

bool FactorySamplerProcessor::processParameterChanges(IParameterChanges *param_changes) {
    // Is the param changes pointer specified?
    if (param_changes) {
        // Get the number of changes, and check if there are any to process
        int32 count = param_changes->getParameterCount();
        if (count > 0) {
            std::vector<ParamID> changed_param_ids(count);

            // Process each param change
            for (int32 i = 0; i < count; i++) {
                // Get the queue of changes for this parameter, if no queue is
                // returned then skip this parameter change
                IParamValueQueue *queue = param_changes->getParameterData(i);
                if (!queue)
                    continue;

                // Get the param ID and if valid process
                ParamID paramId = queue->getParameterId();
                if (paramId < 2) {
                    // Get the last (latest) point value in the queue, any previous values
                    // are not processed
                    int32 sampleOffset;
                    ParamValue value;
                    queue->getPoint((queue->getPointCount() - 1), sampleOffset, value);

                    // Set the param value and add to the vector of param IDs changed
                    _params[paramId] = value;
                    changed_param_ids.push_back(paramId);
                }
            }

            // Update the parameters
            _updateParams(changed_param_ids);
            return true;
        }
    }
    return false;
}

void FactorySamplerProcessor::processAudio(ProcessData &data) {
    float *inputs[CHANNELS];
    float *left_out = data.outputs->channelBuffers32[0];
    float *right_out = data.outputs->channelBuffers32[1];

    for (uint j = 0; j < BUFFER_SIZE; j++) {
        _sine_phase += _sine_inc;
        if (_sine_phase > 1) {
            _sine_phase -= 1;
        }
        float sine = std::sin(2 * M_PI * _sine_phase) * _sine_gain;
        left_out[j] = sine;
        right_out[j] = sine;
    }

    if (_en_sampling && _done_write) {
        for (uint i = 0; i < CHANNELS; i++) {
            inputs[i] = data.inputs->channelBuffers32[i];
        }
        // Get the GUI samples for testing the scope
        for (uint i = 0; i < BUFFER_SIZE; i++) {
            for (uint j = 0; j < CHANNELS; j++) {
                sample_store->at(_sample_counter++) = inputs[j][i];
            }
        }
        _captured_length = _sample_counter - 1;

        // All GUI buffers processed and ready to send samples?
        if (_sample_counter > _capture_length - CHANNELS * BUFFER_SIZE) {
            printf("\n done sampling. trigger write of %d samples ", _captured_length);
            if (!_msg_samples_ready) {
                _msg_samples_ready = true;
                _done_write = false;
            }
            _sample_counter = 0;
            _en_sampling = false;
        }
        for (int i = 0; i < BUFFER_SIZE; i++) {
        }
    }
}

void FactorySamplerProcessor::processEvents(IEventList *events) {
    int midi_note_count = 0;

    // If events are specified
    if (events) {
        // Process the events
        auto event_count = events->getEventCount();
        for (int i = 0; i < event_count; i++) {
            // If the maximum number of notes have not been processed
            if (midi_note_count < 12) {
                Event event;

                // Get the event
                auto res = events->getEvent(i, event);
                if (res == kResultOk) {
                    // Parse the event type
                    switch (event.type) {
                    case Event::kNoteOnEvent: {
                        // Handle the MIDI note on event
                        handleMidiNoteOnEvent(MidiNote(event.noteOn));
                        midi_note_count++;
                        break;
                    }

                    case Event::kNoteOffEvent: {
                        // Handle the MIDI note off event
                        handleMidiNoteOffEvent(MidiNote(event.noteOff));
                        midi_note_count++;
                        break;
                    }

                    default: {
                        // Ignore all other events
                    }
                    }
                }
            }
        }
    }
}

void FactorySamplerProcessor::handleMidiNoteOnEvent(const MidiNote &midi_note) {
    // Allocate voices for this note on
}

void FactorySamplerProcessor::handleMidiNoteOffEvent(const MidiNote &midi_note) {
    // Free the voices associated with this note off
}

void FactorySamplerProcessor::_updateParams(const std::vector<ParamID> &changed_param_ids) {
    float trigger = _params[0];
    if ((trigger > 0.0999) && (!_en_sampling)) {
        _en_sampling = true;
        _capture_length = CHANNELS * (int)std::round(trigger * SAMPLE_RATE);
        printf("\ntrigger samps for %d samples", _capture_length);
        _params[0] = 0;
    }
    _sine_gain = _params[1];
}

void FactorySamplerProcessor::_processSamplerMsg() {
    SamplerMsgQueue msg_queue;
    printf("\nopen queue");

    // Open the GUI message queue
    if (msg_queue.open()) {
        printf("\nopen queue");
        // Run the thread in a loop processing messages until stopped
        while (!_exit_smplr_msg_thread) {
            // Wait for a message to be received
            if (_msg_samples_ready) {
                printf("\nstart write");
                std::fstream file;
                file.open("/udata/bist/sampler_dump.bin", std::ios::out | std::ios::trunc | std::ios::binary);
                file.write(reinterpret_cast<char *>(sample_store), (_captured_length) * sizeof(float));
                file.close();
                _done_write = true;

                printf("\nfinish write");
                auto msg = SamplerMsg();

                // Get a pointer to the samples just processed

                // Send the message
                bool res = msg_queue.post_msg();
                printf("\n\nsnd msg: %d", res);
                _msg_samples_ready = false;
            }

            // Sleep 1ms and check again
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // Close the GUI message queue
        msg_queue.close();
    } else {
        printf("\n failed to open msg queue");
    }
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
