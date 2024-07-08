#include "FactoryTestNinaProcessor.h"
#include "FactoryTestNinaController.h"
#include "NinaParameters.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/base/futils.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include <chrono>
#include <math.h>

namespace Steinberg {
namespace Vst {
namespace Nina {

// Constants
constexpr int MAX_MIDI_NOTES = 12;

#define PRINTSIGNALS

FUID FactoryTestProcessor::uid(0xC1A4E445, 0x720643C4, 0x9152F102, 0xFCE80559);

FactoryTestProcessor::FactoryTestProcessor() {
    auto voice = _voices.begin();
    for (auto &item : _voice_inputs) {
        item = &(voice->getVoiceInputBuffers());
        voice++;
    }
    setControllerClass(FactoryTestController::uid);
#ifdef PRINTSIGNALS
    _exit_print_thread = false;
    _printThread = new std::thread(&FactoryTestProcessor::debugPrinting, this, print_data1, print_data2, print_data3);
#endif
    logger = logger->getInstance();
}

FactoryTestProcessor::~FactoryTestProcessor() {
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
    printf("\nwrite logger");
    logger->writeFile();
}

tresult PLUGIN_API FactoryTestProcessor::initialize(FUnknown *context) {
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
        addAudioInput(STR16("Diagnostic Input"), SpeakerArr::kMono);
        addAudioInput(STR16("Tuning Input"), SpeakerArr::kMono);

        // MIDI event input bus, one channel
        addEventInput(STR16("Event Input"), 16);
        for (int i = 0; i < 12; i++) {
            std::for_each(_voice_inputs.at(i)->osc_1_pitch.begin(), _voice_inputs.at(i)->osc_1_pitch.end(), [](float &f) { f = 7.9658 / noteGain; });
            std::for_each(_voice_inputs.at(i)->osc_2_pitch.begin(), _voice_inputs.at(i)->osc_2_pitch.end(), [](float &f) { f = 7.9658 / noteGain; });
            std::fill_n(_voice_inputs.at(i)->osc_1_shape.begin(), CV_BUFFER_SIZE, 0.5);
            std::fill_n(_voice_inputs.at(i)->osc_2_shape.begin(), CV_BUFFER_SIZE, 0.5);
            _params[2 + i * num_voice_params + 17] = 0.5;
            _params[2 + i * num_voice_params + 18] = 0.5;
        }

#ifdef SIGDUMP
        buf.reserve(96000);
        for (int i = 0; i < 96000; i++) {
            buf.push_back(-2.0);
        }
#endif
    }

    return result;
}

tresult PLUGIN_API FactoryTestProcessor::setState(IBStream *fileStream) {

    return kResultTrue;
}

tresult PLUGIN_API FactoryTestProcessor::getState(IBStream *fileStream) {

    return kResultTrue;
}

tresult PLUGIN_API FactoryTestProcessor::setBusArrangements(SpeakerArrangement *inputs,
    int32 numIns,
    SpeakerArrangement *outputs,
    int32 numOuts) {

    if (numIns == 2 && numOuts == 24 && outputs[0] == SpeakerArr::kMono) {
        return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
    }
    return kResultFalse;
}

tresult PLUGIN_API FactoryTestProcessor::canProcessSampleSize(int32 symbolicSampleSize) {

    // --- currently 32 bit only
    if (symbolicSampleSize == kSample32) {
        return kResultTrue;
    }
    return kResultFalse;
}

tresult PLUGIN_API FactoryTestProcessor::setActive(TBool state) {
    if (state) {
    } else {
    }

    // call base class method
    return AudioEffect::setActive(state);
}

tresult PLUGIN_API FactoryTestProcessor::process(ProcessData &data) {
    // Process all events
    processEvents(data.inputEvents);
    processParameterChanges(data.inputParameterChanges);
    processAudio(data);

    return kResultTrue;
}

int buf_counter = 0;

bool FactoryTestProcessor::processParameterChanges(IParameterChanges *param_changes) {
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

                // Get the last (latest) point value in the queue, any previous values
                // are not processed
                int32 sampleOffset;
                ParamValue value;
                queue->getPoint((queue->getPointCount() - 1), sampleOffset, value);

                // Set the param value and add to the vector of param IDs changed
                _params.at(paramId) = value;
                changed_param_ids.push_back(paramId);
            }

            // Update the parameters
            _updateParams(changed_param_ids);
            return true;
        }
    }
    return false;
};

std::array<float, NUM_VOICES> sin_phases = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void FactoryTestProcessor::processAudio(ProcessData &data) {
    float *high_res_audio_outputs[12];
    float *cv_ch_0_outputs[12];
    float *cv_ch_1_outputs[12];
    float *diag_input = data.inputs->channelBuffers32[0];
    float *tuning_input = data.inputs->channelBuffers32[1];
    for (int i = 0; i < NUM_VOICES; i++) {
        high_res_audio_outputs[i] = data.outputs[0].channelBuffers32[i];
        cv_ch_0_outputs[i] = data.outputs[0].channelBuffers32[i * 2 + 12];
        cv_ch_1_outputs[i] = data.outputs[0].channelBuffers32[i * 2 + 13];
        _voices.at(i).runTuning(tuning_input[i * 4 + 1], tuning_input[i * 4 + 0], tuning_input[i * 4 + 3], tuning_input[i * 4 + 2]);
    }
    int voice_num = 0;
    _noise_osc.setVolume(1);
    buf_counter++;
    if ((buf_counter % 1000) == 0) {
        _print_data = true;
    }
    auto input_itr = _voice_inputs.begin();
    int i = 0;
    int voicen = 0;
    for (auto &voice : _voices) {

        // dodgy test of this method sending the actual pointers to the voice output. data is written directly into the output buffer like this
        VoiceOutput output = {reinterpret_cast<std::array<float, BUFFER_SIZE> *>(cv_ch_0_outputs[voice_num]), reinterpret_cast<std::array<float, BUFFER_SIZE> *>(cv_ch_1_outputs[voice_num])};

        voice.generateVoiceBuffers(output);

        for (int i = 0; i < BUFFER_SIZE; i++) {
            const float _noise_osc_src = _noise_osc.getSample();
            sin_phases[voice_num] += 500.f / 96000.f;
            if (sin_phases[voice_num] > 1) {
                sin_phases[voice_num] -= 1;
            }
            high_res_audio_outputs[voice_num][i] = _noise_osc_src * _voice_settings[voice_num].noise + std::sin(2 * M_PI * sin_phases[voice_num]) * _voice_settings[voice_num].sine;
            print_data1[voice_num][i] = cv_ch_0_outputs[voice_num][i];
            print_data2[voice_num][i] = cv_ch_1_outputs[voice_num][i];
            print_data3[i] = tuning_input[i];
            if (voice_num == 0) {
                float tmp = std::sin(2 * M_PI * sin_phases[voice_num]);
                // printf("\n %f",tmp);
            }
        }
        voice_num++;
    }
};

void FactoryTestProcessor::debugPrinting(float data1[12][BUFFER_SIZE], float data2[12][BUFFER_SIZE], float data3[BUFFER_SIZE]) {
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

            printf("\n osc 1 up\n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = data1[i][Cv0Osc1Down];
                printf("%1.2f\t", tmp);
            }
            printf("\n osc 1 down\n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = data1[i][Cv0Osc1Up];
                printf("%1.2f\t", tmp);
            }
            printf("\n osc 2 down\n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = data1[i][Cv0Osc2Down];
                printf("%1.2f\t", tmp);
            }
            printf("\n osc 2 up\n");
            for (int i = 0; i < printvoices; i++) {
                float tmp = data1[i][Cv0Osc2Up];
                printf("%1.2f\t", tmp);
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

void FactoryTestProcessor::processEvents(IEventList *events) {
    int midi_note_count = 0;

    // If events are specified
    if (events) {
        // Process the events
        auto event_count = events->getEventCount();
        for (int i = 0; i < event_count; i++) {
            // If the maximum number of notes have not been processed
            if (midi_note_count < MAX_MIDI_NOTES) {
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

void FactoryTestProcessor::handleMidiNoteOnEvent(const MidiNote &midi_note) {
    // Allocate voices for this note on
    if (midi_note.velocity > 0) {
    } else {
    }
}

void FactoryTestProcessor::handleMidiNoteOffEvent(const MidiNote &midi_note) {
    // Free the voices associated with this note off
}

void FactoryTestProcessor::_updateParams(const std::vector<ParamID> &changed_param_ids) {

    for (int i = 0; i < 12; i++) {
        _voices.at(i).setOscTuningGain(5);
        bool octave_down_osc2 = _params.at(2 + num_voice_params * i + 21);
        float oct_osc2 = octave_down_osc2 ? -1.5 : 0;
        std::for_each(_voice_inputs.at(i)->osc_1_pitch.begin(), _voice_inputs.at(i)->osc_1_pitch.end(), [](float &f) { f = 8.9658 / noteGain; });
        std::for_each(_voice_inputs.at(i)->osc_2_pitch.begin(), _voice_inputs.at(i)->osc_2_pitch.end(), [oct_osc2](float &f) { f = (8.9658 + oct_osc2) / noteGain; });
        std::fill_n(_voice_inputs.at(i)->osc_1_shape.begin(), CV_BUFFER_SIZE, _params[2 + num_voice_params * i + 17] * 2 - 1);
        std::fill_n(_voice_inputs.at(i)->osc_2_shape.begin(), CV_BUFFER_SIZE, _params[2 + num_voice_params * i + 18] * 2 - 1);
        std::fill_n(_voice_inputs.at(i)->sqr_1_lev.begin(), CV_BUFFER_SIZE, _params[2 + num_voice_params * i + 0]);
        std::fill_n(_voice_inputs.at(i)->tri_1_lev.begin(), CV_BUFFER_SIZE, _params[2 + num_voice_params * i + 1]);
        std::fill_n(_voice_inputs.at(i)->sqr_2_lev.begin(), CV_BUFFER_SIZE, _params[2 + num_voice_params * i + 2]);
        std::fill_n(_voice_inputs.at(i)->tri_2_lev.begin(), CV_BUFFER_SIZE, _params[2 + num_voice_params * i + 3]);
        std::fill_n(_voice_inputs.at(i)->xor_lev.begin(), CV_BUFFER_SIZE, _params[2 + num_voice_params * i + 4]);
        _voice_settings.at(i).noise = _params[2 + num_voice_params * i + 5];
        // std::fill_n(_voice_inputs.at(i).xor_lev.begin(), CV_BUFFER_SIZE, _params[2 + num_voice_params * i + 5]);
        std::fill_n(_voice_inputs.at(i)->filt_res.begin(), CV_BUFFER_SIZE, _params[2 + num_voice_params * i + 6]);
        std::fill_n(_voice_inputs.at(i)->filt_cut.begin(), CV_BUFFER_SIZE, _params[2 + num_voice_params * i + 7]);
        std::fill_n(_voice_inputs.at(i)->overdrive.begin(), CV_BUFFER_SIZE, (bool)(0.5 < _params[2 + num_voice_params * i + 8]));
        std::fill_n(_voice_inputs.at(i)->vca_l.begin(), CV_BUFFER_SIZE, _params[2 + num_voice_params * i + 9]);
        std::fill_n(_voice_inputs.at(i)->vca_r.begin(), CV_BUFFER_SIZE, _params[2 + num_voice_params * i + 10]);

        std::fill_n(_voice_inputs.at(i)->sub_osc.begin(), CV_BUFFER_SIZE, 0.5 < _params[2 + num_voice_params * i + 11]);
        std::fill_n(_voice_inputs.at(i)->hard_sync.begin(), CV_BUFFER_SIZE, 0.5 < _params[2 + num_voice_params * i + 12]);

        std::fill_n(_voice_inputs.at(i)->mute_1.begin(), CV_BUFFER_SIZE, 0.5 < _params[2 + num_voice_params * i + 13]);
        std::fill_n(_voice_inputs.at(i)->mute_2.begin(), CV_BUFFER_SIZE, 0.5 < _params[2 + num_voice_params * i + 14]);
        std::fill_n(_voice_inputs.at(i)->mute_3.begin(), CV_BUFFER_SIZE, 0.5 < _params[2 + num_voice_params * i + 15]);
        std::fill_n(_voice_inputs.at(i)->mute_4.begin(), CV_BUFFER_SIZE, 0.5 < _params[2 + num_voice_params * i + 16]);

        _voices.at(i).setFilterMode(0.5 < _params[2 + num_voice_params * i + 22]);

        _voices.at(i).setOutMuteL(0.5 < _params[0]);
        _voices.at(i).setOutMuteR(0.5 < _params[1]);

        _voice_settings.at(i).sine = _params[2 + num_voice_params * i + 19];
        std::fill_n(_voice_inputs.at(i)->hard_sync.begin(), CV_BUFFER_SIZE, 0.5 < _params[2 + num_voice_params * i + 20]);
    }
}

void FactoryTestProcessor::_processGuiMsg() {
}

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
