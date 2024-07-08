/**
 * @file NinaProcessor.cpp
 * @brief Nina Processor implementation.
 *
 * @copyright Copyright (c) 2022-2023 Melbourne Instruments, Australia
 */
#include "NinaProcessor.h"

#include "NinaController.h"
#include "NinaParameters.h"
#include "NinaVoice.h"
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
constexpr int MAX_MIDI_NOTES = 12;

//#define PRINTSIGNALS

FUID Processor::uid(0xB9A3DB09, 0xBD554308, 0xACA1EF6F, 0xB02E9187);

Processor::Processor() {
    setControllerClass(Controller::uid);

#ifdef PRINTSIGNALS
    _exit_print_thread = false;
    _printThread = new std::thread(&Processor::debugPrinting, this, print_data1, print_data2, print_data3);
#endif
    // start the GUI message thread & init scope vars/note storage
    _exit_gui_msg_thread = false;
    _gui_buffer_complete = false;
    _current_gui_samples.store(_gui_samples_1);
    _gui_msg_thread = new std::thread(&Processor::_processGuiMsg, this);

    // preallocate max notes * max midi channels
    _held_midi_notes.reserve(127 * 16);
}

Processor::~Processor() {
    printf("\n exit Nina vst");
#ifdef PRINTSIGNALS
    // If the print thread is running
    if (_printThread) {
        // Stop the print thread
        _exit_print_thread = true;
        _printThread->join();
        delete _printThread;
    }
#endif
    // exit the scope thread
    if (_gui_msg_thread) {
        // Stop the GUI message thread
        _exit_gui_msg_thread = true;
        _gui_msg_thread->join();
    }
}

tresult PLUGIN_API Processor::initialize(FUnknown *context) {
    tresult result = AudioEffect::initialize(context);
    if (result == kResultTrue) {
        // 12 mono outputs for High Res audio
        addAudioOutput(STR16("Audio Voice 0"), SpeakerArr::kMono);
        addAudioOutput(STR16("Audio Voice 1"), SpeakerArr::kMono);
        addAudioOutput(STR16("Audio Voice 2"), SpeakerArr::kMono);
        addAudioOutput(STR16("Audio Voice 3"), SpeakerArr::kMono);
        addAudioOutput(STR16("Audio Voice 4"), SpeakerArr::kMono);
        addAudioOutput(STR16("Audio Voice 5"), SpeakerArr::kMono);
        addAudioOutput(STR16("Audio Voice 6"), SpeakerArr::kMono);
        addAudioOutput(STR16("Audio Voice 7"), SpeakerArr::kMono);
        addAudioOutput(STR16("Audio Voice 8"), SpeakerArr::kMono);
        addAudioOutput(STR16("Audio Voice 9"), SpeakerArr::kMono);
        addAudioOutput(STR16("Audio Voice 10"), SpeakerArr::kMono);
        addAudioOutput(STR16("Audio Voice 11"), SpeakerArr::kMono);

        // 12 stereo pairs for control data
        addAudioOutput(STR16("Control Voice 0"), SpeakerArr::kStereo);
        addAudioOutput(STR16("Control Voice 1"), SpeakerArr::kStereo);
        addAudioOutput(STR16("Control Voice 2"), SpeakerArr::kStereo);
        addAudioOutput(STR16("Control Voice 3"), SpeakerArr::kStereo);
        addAudioOutput(STR16("Control Voice 4"), SpeakerArr::kStereo);
        addAudioOutput(STR16("Control Voice 5"), SpeakerArr::kStereo);
        addAudioOutput(STR16("Control Voice 6"), SpeakerArr::kStereo);
        addAudioOutput(STR16("Control Voice 7"), SpeakerArr::kStereo);
        addAudioOutput(STR16("Control Voice 8"), SpeakerArr::kStereo);
        addAudioOutput(STR16("Control Voice 9"), SpeakerArr::kStereo);
        addAudioOutput(STR16("Control Voice 10"), SpeakerArr::kStereo);
        addAudioOutput(STR16("Control Voice 11"), SpeakerArr::kStereo);

        // audio input for diagnostic audio and tuning measurements
        addAudioInput(STR16("Mix Left Input"), SpeakerArr::kMono);
        addAudioInput(STR16("Mix Right Input"), SpeakerArr::kMono);
        addAudioInput(STR16("External In 1"), SpeakerArr::kMono);
        addAudioInput(STR16("External In 2"), SpeakerArr::kMono);
        addAudioInput(STR16("External In 3"), SpeakerArr::kMono);
        addAudioInput(STR16("External In 4"), SpeakerArr::kMono);
        addAudioInput(STR16("Tuning Input"), SpeakerArr::kMono);

        // MIDI event input bus, one channel
        addEventInput(STR16("Event Input"), 16);
#ifdef SIGDUMP
        buf.reserve(96000);
        for (int i = 0; i < 96000; i++) {
            buf.push_back(-2.0);
        }
#endif
    }

    return result;
}

tresult PLUGIN_API Processor::setState(IBStream *fileStream) {

    return kResultTrue;
}

tresult PLUGIN_API Processor::getState(IBStream *fileStream) {

    return kResultTrue;
}

tresult PLUGIN_API Processor::setBusArrangements(SpeakerArrangement *inputs,
    int32 numIns,
    SpeakerArrangement *outputs,
    int32 numOuts) {

    if (numIns == 7 && numOuts == 24 && outputs[0] == SpeakerArr::kMono) {
        return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
    }
    return kResultFalse;
}

tresult PLUGIN_API Processor::canProcessSampleSize(int32 symbolicSampleSize) {

    // --- currently 32 bit only
    if (symbolicSampleSize == kSample32) {
        return kResultTrue;
    }
    return kResultFalse;
}

tresult PLUGIN_API Processor::setActive(TBool state) {
    if (state) {
    } else {
    }

    // call base class method
    return AudioEffect::setActive(state);
}

tresult PLUGIN_API Processor::process(ProcessData &data) {
    // Process all events
    processEvents(data.inputEvents);
    processParameterChanges(data.inputParameterChanges);
    processAudio(data);

    return kResultTrue;
}

bool Processor::processParameterChanges(IParameterChanges *param_changes) {
    // Is the param changes pointer specified?
    if (param_changes) {
        // Get the number of changes, and check if there are any to process
        int32 count = param_changes->getParameterCount();
        if (count > 0) {
            // Process each param change
            _num_changed_params = 0;
            for (int32 i = 0; i < count; i++) {
                // Get the queue of changes for this parameter, if no queue is
                // returned then skip this parameter change
                IParamValueQueue *queue = param_changes->getParameterData(i);
                if (!queue)
                    continue;

                // Get the param ID and if valid process
                ParamID paramId = queue->getParameterId();

                // Get the last (latest) point value in the queue, any previous values
                // are not processed
                int32 sampleOffset;
                ParamValue value;
                queue->getPoint((queue->getPointCount() - 1), sampleOffset, value);

                // Set the param value and add to the vector of param IDs changed
                _changed_params[i] = ParamChange(paramId, value);
                _num_changed_params++;
                // printf("\n change: %d %f ", paramId, value);
            }

            // Update the parameters
            _updateParams();
            return true;
        }
    }
    return false;
}

int ninabufcounter = 0;

void Processor::processAudio(ProcessData &data) {
    // Process audio via the Layer Manager
    _layer_manager.processAudio(data);
    float *high_res_audio_outputs[NUM_VOICES];
    float *cv_ch_0_outputs[NUM_VOICES];
    float *cv_ch_1_outputs[NUM_VOICES];
    float *tuning_input = data.inputs->channelBuffers32[6];

    /**
    for (int i = 0; i < 12; i++) {
        high_res_audio_outputs[i] = data.outputs[0].channelBuffers32[i];
        cv_ch_0_outputs[i] = data.outputs[0].channelBuffers32[i * 2 + 12];
        cv_ch_1_outputs[i] = data.outputs[0].channelBuffers32[i * 2 + 13];
    }
    // save the data for display
    if (ninabufcounter++ % 1000 == 0) {
        for (int voice_num = 0; voice_num < 12; voice_num++) {
            for (int i = 0; i < BUFFER_SIZE; i++) {
                print_data1[voice_num][i] = cv_ch_0_outputs[voice_num][i];
                print_data2[voice_num][i] = cv_ch_1_outputs[voice_num][i];
                print_data3[i] = tuning_input[i];
            }
        }
    }
    _print_data = true;
    **/

    // Get the GUI samples for the scope
    float *left = data.inputs->channelBuffers32[0];
    float *right = data.inputs->channelBuffers32[1];
    float *current_gui_samples = _current_gui_samples.load();

    // scope trigger filter setup
    static constexpr float x = 2 * M_PI * (SCOPE_HP_FREQ) / (float)SAMPLE_RATE;
    static constexpr float p_lp = (2 - std::cos(x)) - std::sqrt((2 - std::cos(x)) * (2 - std::cos(x)) - 1);

    if (!_gui_buffer_complete) {
        while (phase < (float)BUFFER_SIZE) {

            // calc time step for current window based on midi note length
            const float window = 1.0 / (GUI_NUM_SAMPLES * midiNoteToFreq(_current_low_midi_note.note));
            float phase_inc = (float)SAMPLE_RATE / (0.5 / window);

            // current sample to add to buffer
            const int samp = std::floor(phase);
            phase += phase_inc;

            // filter the LR samples
            float current_sample_l = 8 * (left[samp]);
            dc_filter_l = (1 - p_lp) * current_sample_l + p_lp * dc_filter_l;
            current_sample_l = current_sample_l - dc_filter_l;
            float current_sample_r = 8 * (right[samp]);
            dc_filter_r = (1 - p_lp) * current_sample_r + p_lp * dc_filter_r;
            current_sample_r = current_sample_r - dc_filter_r;
            float current_sample_sum = current_sample_l + current_sample_r;
            trigger_out = current_sample_sum;

            // if we have written at least 1/2 a buffer worth of data, then we can start looking for zero crossings
            if (!_zero_crossing_detect && (_current_samples_written > (uint)(GUI_NUM_SAMPLES / 2))) {

                const bool sign_1 = current_sample_sum < 0.f;
                const bool n_sign_2 = _previous_scope_trigger_sample > 0.f;
                const bool zero_x = sign_1 && n_sign_2;

                // if we find a zero crossing or the level is very low, then trigger a capture
                bool auto_trig = (_scope_auto_counter > GUI_NUM_SAMPLES / 2);
                if (zero_x || auto_trig) {
                    _zero_crossing_detect = true;
                    _scope_ring_end = _scope_ring_start + (GUI_NUM_SAMPLES / 2);
                    if (_scope_ring_end >= GUI_NUM_SAMPLES) {
                        _scope_ring_end -= GUI_NUM_SAMPLES;
                    }
                    // reset the auto counter
                    _scope_auto_counter = 0;
                }
                _scope_auto_counter++;
            }
            _previous_scope_trigger_sample = trigger_out;

            // write the samples into the ring buffer
            const int offset = _gui_buffer_counter++;
            _scope_ringbuffer_l[(_scope_ring_start)] = current_sample_l * _scope_dynamic_gain;
            _scope_ringbuffer_r[(_scope_ring_start)] = current_sample_r * _scope_dynamic_gain;
            _current_samples_written++;
            _scope_ring_start++;
            if (_scope_ring_start == GUI_NUM_SAMPLES) {
                _scope_ring_start = 0;
            }

            // if we have filled the ring buffer, then trigger the gui message transfer
            if (_zero_crossing_detect && (_scope_ring_start == _scope_ring_end)) {
                _zero_crossing_detect = false;
                _gui_buffer_complete = true;
                break;
            }
        }

        // reset the sample phase
        if (!_gui_buffer_complete) {
            phase -= 128.0;
        }
    }

    // All GUI buffers processed and ready to send samples?
    if (_gui_buffer_complete) {
        // printf("\nbuff send");
        //  If we haven't processed the previous GUI buffer yet, skip this one
        if (!_gui_samples_ready) {
            // Load the ring buffer samples into the GUI buffer in the ringbuffer order
            float *current_samples = _current_gui_samples;
            uint ring_counter = _scope_ring_end;
            for (uint i = 0; i < GUI_NUM_SAMPLES; i++) {
                current_samples[i * 2] = _scope_ringbuffer_l[(ring_counter)];
                current_samples[i * 2 + 1] = _scope_ringbuffer_r[(ring_counter++)];
                if (ring_counter == GUI_NUM_SAMPLES) {
                    ring_counter -= GUI_NUM_SAMPLES;
                }
            }

            // Swap the pointer to the other buffer to process the next samples
            if (_current_gui_samples == _gui_samples_1)
                _current_gui_samples = _gui_samples_2;
            else
                _current_gui_samples = _gui_samples_1;
            _gui_samples_ready = true;
            _zero_crossing_detect = false;
            _gui_buffer_complete = false;
            _current_samples_written = 0;
        } else {
        }
    }
}

void Processor::processEvents(IEventList *events) {

    // If events are specified
    if (events) {
        // Process the events
        auto event_count = events->getEventCount();
        for (int i = 0; i < event_count; i++) {
            // If the maximum number of notes have not been processed
            Event event;

            // Get the event
            auto res = events->getEvent(i, event);
            if (res == kResultOk) {
                // Parse the event type
                switch (event.type) {
                case Event::kNoteOnEvent: {
                    // Handle the MIDI note on event
                    const MidiNote note = MidiNote(event.noteOn);
                    handleMidiNoteOnEvent(note);

                    // update scope note vars
                    _held_midi_notes.push_back(note.pitch);
                    if (note.pitch < _current_low_midi_note.note)
                        _current_low_midi_note.note = note.pitch;
                    _updateScopeGain();
                    if (!_current_low_midi_note.on) {
                        _current_low_midi_note.on = true;
                        _current_low_midi_note.note = note.pitch;
                        _updateScopeGain();
                    }
                    break;
                }

                case Event::kNoteOffEvent: {
                    // Handle the MIDI note off event
                    const MidiNote note = MidiNote(event.noteOff);
                    uint i = 0;
                    while (i < _held_midi_notes.size()) {
                        if (_held_midi_notes.at(i) == note.pitch) {
                            _held_midi_notes.erase(_held_midi_notes.begin() + i);
                        } else {
                            i++;
                        }
                    }
                    handleMidiNoteOffEvent(MidiNote(event.noteOff));
                    if (note.pitch == _current_low_midi_note.note)
                        _current_low_midi_note.on = false;
                    break;
                }

                case Event::kPolyPressureEvent: {
                    _layer_manager.polyPressureEvent(event.polyPressure);
                    break;
                }

                default:
                    // Ignore all other events
                    break;
                }
            }
        }
    }
}

void Processor::debugPrinting(float data1[12][BUFFER_SIZE], float data2[12][BUFFER_SIZE], float data3[BUFFER_SIZE]) {
    printf("\nstart printing thread\n");
    while (!_exit_print_thread) {
        if (_print_data) {
            _print_data = false;
            static const int printvoices = 12;

            printf("\n\n");
            for (int i = 0; i < printvoices; i++) {
                printf("v:%d\t", i);
            }

            printf("\n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = data2[i][Cv1Order::Cv1AmpL];
                printf("%1.2f\t", tmp);
            }
            printf("\n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = data2[i][Cv1Order::Cv1AmpR];
                printf("%1.2f\t", tmp);
            }

            printf("\n sq 1 \n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = data1[i][Cv0MixOsc1Sq];
                printf("%0.4f\t", tmp);
            }
            printf("\n tri 1 \n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = data1[i][Cv0MixOsc1Tri];
                printf("%0.4f\t", tmp);
            }
            printf("\n sqr 1 \n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = data1[i][Cv0MixOsc2Sq];
                printf("%0.4f\t", tmp);
            }
            printf("\n tri 2 \n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = data1[i][Cv0MixOsc2Tri];
                printf("%0.4f\t", tmp);
            }
            printf("\n xor \n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = data2[i][Cv1MixXor];
                printf("%0.4f\t", tmp);
            }
            printf("\n bit array\n");
            for (int i = 0; i < printvoices; i++) {
                int tmp = data2[i][Cv1Order::Cv1BitArray + 10 * 8] * (1 / 1.19209303761637659268e-7f);
                printf("%x\t", tmp);
            }
            const float count_scale = (2 << 22) / (73.75e6 / 2.0); //
            printf("\n freq\n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = 1 / (data3[i * 4] * count_scale);
                printf("%1.0f\t", tmp);
            }
            printf("\n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = 1 / (count_scale * data3[i * 4 + 1]);
                printf("%1.0f\t", tmp);
            }
            printf("\n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = 1 / (count_scale * data3[i * 4 + 2]);
                printf("%1.0f\t", tmp);
            }
            printf("\n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = 1 / (count_scale * data3[i * 4 + 3]);
                printf("%1.0f\t", tmp);
            }

            printf("\n");
        }
        struct timespec remaining, request = {1, 500 * 1000 * 1000};
        nanosleep(&request, &remaining);
    }
    printf("\n exit printing thread\n");
}

void Processor::handleMidiNoteOnEvent(const MidiNote &midi_note) {
    // Allocate voices for this note on
    if (midi_note.velocity > 0) {
        _layer_manager.allocateVoices(midi_note);
    } else {
        _layer_manager.freeVoices(midi_note);
    }
}

void Processor::handleMidiNoteOffEvent(const MidiNote &midi_note) {
    // Free the voices associated with this note off
    _layer_manager.freeVoices(midi_note);
}

void Processor::_updateParams() {
    // Apply the param updates to the Layer Manager
    _layer_manager.updateParams(_num_changed_params, _changed_params);
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
