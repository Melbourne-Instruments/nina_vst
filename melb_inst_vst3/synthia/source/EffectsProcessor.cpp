/**
 * @file EffectsProcessor.cpp
 * @brief
 * @version 0.1
 * @date 2021-09-30
 *
 * @copyright Copyright (c) 2023 Melbourne Instruments
 */

#include "EffectsProcessor.h"
#include "EffectsController.h"
#include "GuiMsgQueue.h"
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

FUID EffectsProcessor::uid(0xE19B5E2C, 0xB84B403E, 0xA466809E, 0x3D96323A);

EffectsProcessor::EffectsProcessor() {
    setControllerClass(EffectsController::uid);
    _changed_param_ids.reserve(NumEffectsParams * 10);
    for (auto &item : _delay_buffer) {
        item = 0.0;
    }

    _reverb.setPreset(0);
    _delay_left.setLevel(0.7);
    _delay_left.setLFO(0.4, 6);
    _delay_right.setLevel(0.7);
    _delay_right.setLFO(0.65, 6);
    cengine.setChorus1LfoRate(.3);
    cengine.setChorus2LfoRate(.7);
    _reverb.setInGain(0.9);
    // audio_samples.reserve(SAMPLE_RATE*2);
    _reverb.setInGain(0.9);
}

EffectsProcessor::~EffectsProcessor() {
    // If the GUI message thread is running
}

tresult PLUGIN_API EffectsProcessor::initialize(FUnknown *context) {
    tresult result = AudioEffect::initialize(context);
    if (result == kResultTrue) {

        addAudioOutput(STR16("In 0"), SpeakerArr::kMono);
        addAudioOutput(STR16("In 1"), SpeakerArr::kMono);

        addAudioInput(STR16("Out 0"), SpeakerArr::kMono);
        addAudioInput(STR16("Out 1"), SpeakerArr::kMono);

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

tresult PLUGIN_API EffectsProcessor::setState(IBStream *fileStream) {

    return kResultTrue;
}

tresult PLUGIN_API EffectsProcessor::getState(IBStream *fileStream) {

    return kResultTrue;
}

tresult PLUGIN_API EffectsProcessor::setBusArrangements(SpeakerArrangement *inputs,
    int32 numIns,
    SpeakerArrangement *outputs,
    int32 numOuts) {

    if (numIns == 2 && numOuts == 2 && outputs[0] == SpeakerArr::kMono) {
        return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
    }
    return kResultFalse;
}

tresult PLUGIN_API EffectsProcessor::canProcessSampleSize(int32 symbolicSampleSize) {

    // --- currently 32 bit only
    if (symbolicSampleSize == kSample32) {
        return kResultTrue;
    }
    return kResultFalse;
}

tresult PLUGIN_API EffectsProcessor::setActive(TBool state) {
    if (state) {
    } else {
    }

    // call base class method
    return AudioEffect::setActive(state);
}

tresult PLUGIN_API EffectsProcessor::process(ProcessData &data) {
    // Process all events
    _changed_param_ids.clear();
    processEvents(data.inputEvents);
    processParameterChanges(data.inputParameterChanges);
    processAudio(data);
    _global_tempo = (float)(data.processContext->tempo) / 60.f;
    _delay_left.setTempo(_global_tempo);
    _delay_right.setTempo(_global_tempo);
    return kResultTrue;
}

bool EffectsProcessor::processParameterChanges(IParameterChanges *param_changes) {
    _changed_param_ids.clear();
    // Is the param changes pointer specified?
    if (param_changes) {
        // Get the number of changes, and check if there are any to process
        int32 count = param_changes->getParameterCount();
        if (count > 0) {

            // Process each param change
            for (int32 i = 0; i < count; i++) {
                // Get the queue of changes for this parameter, if no queue is
                // returned then skip this parameter change
                IParamValueQueue *queue = param_changes->getParameterData(i);
                if (!queue)
                    continue;

                // Get the param ID and if valid process
                ParamID paramId = mask_param_id(queue->getParameterId());
                if (paramId < EffectsParamOrder::NumEffectsParams) {
                    // Get the last (latest) point value in the queue, any previous values
                    // are not processed
                    int32 sampleOffset;
                    ParamValue value;
                    queue->getPoint((queue->getPointCount() - 1), sampleOffset, value);

                    // Set the param value and add to the vector of param IDs changed
                    _params[paramId] = value;
                    _changed_param_ids.push_back(paramId);
                }
            }

            // Update the parameters
            _updateParams();
            return true;
        }
    }
    return false;
}
#ifdef SIGDUMP
bool note_onoff = false;
bool oldstate = false;
#endif
float servo_offsetl = 0;
float servo_offsetr = 0;
float max_out = 0;

void EffectsProcessor::processAudio(ProcessData &data) {
    float *(left_in) = data.inputs->channelBuffers32[0];
    float *(right_in) = data.inputs->channelBuffers32[1];

    float *left_out = data.outputs->channelBuffers32[0];
    float *right_out = data.outputs->channelBuffers32[1];

    const float chorus_level = _chorus_level;
    const float delay_level = _delay_level;
    const float reverb_level = _reverb_level;

    // to avoid clicks and pops, we duck the output when configuring fx
    if (_output_duck) {
        vol_smooth = 0.f;
        _duck_counter++;
        for (int i = 0; i < BUFFER_SIZE; i++) {
            left_out[i] = 0.f;
            right_out[i] = 0.f;
            left_in[i] = 0.f;
            right_in[i] = 0;
        }
        if (_duck_counter == 1) {

            _reverb._hard_mute = true;
        }

        if (_duck_counter > (0.2 * BUFFER_RATE)) {
            _duck_counter = 0;
            _output_duck = false;
        }
        if (_duck_counter < 10) {
            return;
        }
        return;
    }

    for (int i = 0; i < BUFFER_SIZE; ++i) {

        // will sort out later, but there seems to be a DC offset on the output, so i've added this servo to cope with it.
        servo_offsetl += ((left_in[i] - servo_offsetl) * SERVO_RATE);
        servo_offsetr += ((right_in[i] - servo_offsetr) * SERVO_RATE);

        // invert the effects output to fix the phase inversion circuit issue in NINA rev A&B boards
        left_in[i] = -left_in[i] + servo_offsetl;
        right_in[i] = -right_in[i] + servo_offsetr;

        // zero the output buffers so we can just sum into them
        left_out[i] = 0.f;
        right_out[i] = 0.f;
    }

    // run the limiter inplace on the arrays
    _limiter.run(left_in, right_in, left_in, right_in);

    // run the expander which improves the final fx SNR
    _expander.process(left_in, right_in);

    float *fx_in_l;
    float *fx_in_r;
    float *fx_sum_l;
    float *fx_sum_r;
    fx_in_l = left_in;
    fx_in_r = right_in;
    fx_sum_l = left_out;
    fx_sum_r = right_out;
    uint last_slot = 0;
    if (last_slot < _delay_slot) {
        last_slot = _delay_slot;
    }
    if (last_slot < _chorus_slot) {
        last_slot = _chorus_slot;
    }
    if (last_slot < _reverb_slot) {
        last_slot = _reverb_slot;
    }
    bool prev_slot_processing = false;
    for (uint fx_row = 0; fx_row < (last_slot + 1); fx_row++) {

        bool slot_has_processing = (_chorus_slot == fx_row) || (_delay_slot == fx_row) || (_reverb_slot == fx_row);

        bool final_slot = (last_slot == fx_row);
        float dry_passthough;
        float level_sum = 0;
        if (_chorus_slot == fx_row) {
            level_sum += _chorus_level;
        }
        if (_reverb_slot == fx_row) {
            level_sum += _reverb_level;
        }
        if (_delay_slot == fx_row) {
            level_sum += delay_level;
        }
        dry_passthough = level_sum < 1.0f ? 1.0 - level_sum : 0;
        if (final_slot && !(prev_slot_processing)) {
            dry_passthough = 0.f;
        }

        _dry_level_filtered = param_smooth(dry_passthough, _dry_level_filtered);
        prev_slot_processing = slot_has_processing;

        // clear the output buffer
        for (int i = 0; i < BUFFER_SIZE; i++) {
            fx_sum_l[i] = dry_passthough * fx_in_l[i];
            fx_sum_r[i] = dry_passthough * fx_in_r[i];
        }

        // chorus processing
        if ((fx_row == _chorus_slot)) {
            cengine.setVol(chorus_level);
            for (int i = 0; i < BUFFER_SIZE; i++) {
                cengine.process(&fx_in_l[i], &fx_in_r[i], &fx_sum_l[i], &fx_sum_r[i]);
            }
        }
        // delay processing
        if ((fx_row == _delay_slot)) {
            _delay_left.run(fx_in_l, fx_sum_l);
            _delay_right.run(fx_in_r, fx_sum_r);
        }

        // reverb processing
        if ((fx_row == _reverb_slot)) {

            _limiter_2.run(fx_in_l, fx_in_r, fx_in_l, fx_in_r);
            float *in[2] = {fx_in_l, fx_in_r};
            float *out[2] = {fx_sum_l, fx_sum_r};
            _reverb.run(in, out, BUFFER_SIZE);
        }
        for (int i = 0; i < BUFFER_SIZE; i++) {

            fx_in_l[i] = fx_sum_l[i];
            fx_in_r[i] = fx_sum_r[i];
        }
    }
    _output_limiter.run(fx_in_l, fx_in_r, left_out, right_out);
}

void EffectsProcessor::processEvents(IEventList *events) {

    // If events are specified
    if (events) {
        // Process the events
        auto event_count = events->getEventCount();
        for (int i = 0; i < event_count; i++) {
            Event event;

            // Get the event
            auto res = events->getEvent(i, event);
            if (res == kResultOk) {
                // Parse the event type
                switch (event.type) {
                case Event::kNoteOnEvent: {
                    // Handle the MIDI note on event
                    handleMidiNoteOnEvent(MidiNote(event.noteOn));

                    break;
                }

                case Event::kNoteOffEvent: {
                    // Handle the MIDI note off event
                    handleMidiNoteOffEvent(MidiNote(event.noteOff));
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

void EffectsProcessor::handleMidiNoteOnEvent(const MidiNote &midi_note) {
    // Allocate voices for this note on
}

void EffectsProcessor::handleMidiNoteOffEvent(const MidiNote &midi_note) {
    // Free the voices associated with this note off
}

void EffectsProcessor::_updateParams() {

    if (_volume != _params[EffectsParamOrder::volume]) {
    }
    _volume = _params[EffectsParamOrder::volume];
    int fx_mode = (int)(std::floor(3.99 * _params[effectsMode]));

    // reset the fx if the mode has changed to avoid noise etc

    for (auto &change : _changed_param_ids) {
        switch (change) {
        case EffectsParamOrder::reverbTone:
            _reverb.setTone(_params[EffectsParamOrder::reverbTone]);
            break;
        case EffectsParamOrder::reverbDecay:
            _reverb.setDecay(_params[reverbDecay]);
            _output_duck = true;
            break;
        case EffectsParamOrder::reverbShimmer:
            _reverb.setShimmer(_params[reverbShimmer]);
            break;
        case EffectsParamOrder::tempoSync:
            _tempo_sync = _params[tempoSync] > 0.25;
            _delay_left.setTempoSync(_tempo_sync);
            _delay_right.setTempoSync(_tempo_sync);
            break;
        case EffectsParamOrder::reverbEarlyMix:
            _reverb.setDistance(_params[EffectsParamOrder::reverbEarlyMix]);
            break;
        case EffectsParamOrder::reverbPreDelay:
            _reverb.setPreDelay(_params[reverbPreDelay]);
            _output_duck = true;
            break;
        case EffectsParamOrder::reverbPreset:
            _reverb.setPreset(_params[reverbPreset]);
            _output_duck = true;
            break;
        case EffectsParamOrder::volume:
            _limiter.setVolume(_volume);
            break;
        case EffectsParamOrder::chorusLevel:
            _chorus_level = _params[chorusLevel];
            break;
        case EffectsParamOrder::delaySlot: {
            float slot = std::round(_params[delaySlot] * FX_SLOTS);
            _delay_slot = (uint)slot;
        }
        case EffectsParamOrder::chorusSlot: {
            float slot = std::round(_params[chorusSlot] * FX_SLOTS);
            _chorus_slot = (uint)slot;
        }
        case EffectsParamOrder::reverbSlot: {
            float slot = std::round(_params[reverbSlot] * FX_SLOTS);
            _reverb_slot = (uint)slot;
        }

        case EffectsParamOrder::delayLevel:
            _delay_left.setDelayMixAmount(_params[delayLevel]);
            _delay_right.setDelayMixAmount(_params[delayLevel]);
            _delay_level = _params[delayLevel];
        case EffectsParamOrder::ReverbLevel:
            _reverb.setwetDry(_params[ReverbLevel]);
            _reverb_level = _params[ReverbLevel];

        case EffectsParamOrder::delayFeedback:
            _delay_left.setFB(_params[EffectsParamOrder::delayFeedback]);
            _delay_right.setFB(_params[EffectsParamOrder::delayFeedback]);
            break;
        case EffectsParamOrder::delayTime:
            _delay_left.setTime(_params[EffectsParamOrder::delayTime]);
            _delay_right.setTime(_params[EffectsParamOrder::delayTime]);
            break;
        case EffectsParamOrder::delayTimeSync:
            _delay_left.setTimeSync(_params[EffectsParamOrder::delayTimeSync]);
            _delay_right.setTimeSync(_params[EffectsParamOrder::delayTimeSync]);
            break;
        case EffectsParamOrder::delayTone:
            _delay_left.setFilter(_params[EffectsParamOrder::delayTone]);
            _delay_right.setFilter(_params[EffectsParamOrder::delayTone]);
            break;
        case EffectsParamOrder::chorusmode: {
            const float cmode = _params[chorusmode];
            if (cmode < 1.f / 3.f) {
                _chorus_mode = I;
            } else if (cmode < 2.f / 3.f) {
                _chorus_mode = II;
            } else {
                _chorus_mode = I_II;
            }
            cengine.setEnablesChorus((_chorus_mode == I) || (_chorus_mode == I_II), (_chorus_mode == II) || (_chorus_mode == I_II));
        } break;
        default:
            break;
        }
    }
}
} // namespace Nina
} // namespace Vst
} // namespace Steinberg
