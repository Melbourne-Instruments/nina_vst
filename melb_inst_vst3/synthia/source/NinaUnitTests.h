/**
 * @file NinaUnitTests.h
 * @brief A mis-named collection of Unit tests, integration tests of various classes, and test benches used to run/debug code primarily, and to sound the alarm if anything major stops working like class loading, setting params, triggering notes, getting output.
 * @date 2022-08-03
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */
#include "Layer.h"
#include "LayerManager.h"
#include "NinaDelay.h"
#include "NinaReverb.h"
#include "NinaVoice.h"
#include "SynthMath.h"
#include "WavetableOsc.h"
#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>

bool buffer_sanity_check(Steinberg::Vst::Nina::VoiceOutput out) {
    for (auto &item : *(out.output_1)) {
        if (item > 2.0f || item < -2.0f) {
            return false;
        }
    }
    for (auto &item : *(out.output_2)) {
        if (item > 2.0f || item < -2.0f) {
            return false;
        }
    }
    return true;
}

bool load_timbre_voice() {
    using namespace Steinberg::Vst::Nina;
    const char *test = "load timbre voice up";
    VoiceInput input;
    NinaParams::LayerParams layer_params;
    NinaParams::LayerStateParams state_p;
    WavetableLoader wt_loader_a;
    WavetableLoader wt_loader_b;
    std::array<float, BUFFER_SIZE> high_res_arra;
    std::array<float, BUFFER_SIZE> *high_res_arr = &high_res_arra;
    std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> high_res = {high_res_arr};
    high_res.fill(high_res_arr);
    float m = 0;
    LayerVoice voice{input, layer_params, state_p, state_p, wt_loader_a, wt_loader_b, high_res, high_res_arr, 0};
    voice.run();
    printf("\n%s", test);
    return true;
}

bool reverb_test() {
    using namespace Steinberg::Vst::Nina;
    printf("\nreverb test");

    NinaReverb reverb;
    reverb.setPreset(1);
    reverb.setDecay(0.5);
    reverb.setDistance(0.5);
    reverb.setShimmer(0.001);
    reverb.setTone(0.5);
    reverb.setInputLPF(100);
    reverb.setInputHPF(10000);

    std::vector<float> output_data;
    std::array<float, 128> input_l, input_r;
    std::array<float, 128> output_l, output_r;
    for (int i = 0; i < 128; i++) {
        input_l.at(i) = 0;
        input_r.at(i) = 0;
        output_l.at(i) = 0;
        output_r.at(i) = 0;
    }
    float *input_l_adr = input_l.data();
    float *input_r_adr = input_r.data();
    float *output_l_adr = output_l.data();
    float *output_r_adr = output_r.data();
    float *input_fp[2] = {input_l_adr, input_r_adr};
    float *output_fp[2] = {output_l_adr, output_r_adr};
    float phase = 0;
    float phase_inc = 1000.f / 96000.f;
    for (int i = 0; i < 1000; i++) {

        reverb.run(input_fp, output_fp, 128);
    }
    while (output_data.size() < 192000 * 4) {
        for (int i = 0; i < 128; i++) {
            input_l[i] = 0.1 * std::sin(2 * M_PI * phase);
            phase += phase_inc;
        }
        reverb.run(input_fp, output_fp, 128);

        for (int i = 0; i < 128; i++) {
            output_data.push_back(output_l[i]);
        }
    }
    printf("\nstart write");
    std::fstream file;
    file.open("/tmp/delay_dump.bin", std::ios::out | std::ios::trunc | std::ios::binary);
    file.write(reinterpret_cast<char *>(output_data.data()), (output_data.size()) * sizeof(float));
    file.close();

    return true;
}

bool delay_test() {
    using namespace Steinberg::Vst::Nina;
    printf("\ndelay test");

    BBDDelay delay;
    delay.setFB(0.1);
    delay.setFilter(0.5);
    delay.setLevel(1);
    delay.setLFO(0.4, 6);
    delay.setTime(0.3);

    std::vector<float> output_data;
    std::array<float, 128> input;
    std::array<float, 128> output;
    float phase = 0;
    float phase_inc = 1000.f / 96000.f;
    while (output_data.size() < 192000 * 4) {
        for (int i = 0; i < 128; i++) {
            input[i] = 0.1 * std::sin(2 * M_PI * phase);
            phase += phase_inc;
        }
        delay.run(input.data(), output.data());

        for (int i = 0; i < 128; i++) {
            output_data.push_back(output[i]);
        }
    }
    printf("\nstart write");
    std::fstream file;
    file.open("/tmp/delay_dump.bin", std::ios::out | std::ios::trunc | std::ios::binary);
    file.write(reinterpret_cast<char *>(output_data.data()), (output_data.size()) * sizeof(float));
    file.close();

    return true;
}

bool load_analog_voices() {
    using namespace Steinberg::Vst::Nina;
    std::array<AnalogVoice, 12> _voices = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    NinaParams::LayerParams layer_params;
    NinaParams::LayerStateParams a;
    WavetableLoader wt_loader_a;
    WavetableLoader wt_loader_b;
    std::array<float, BUFFER_SIZE> high_res_arra;
    std::array<float, BUFFER_SIZE> *high_res_arr = &high_res_arra;
    std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> high_res = {high_res_arr};
    float m = 0;
    _voices[0].getVoiceInputBuffers();
    std::array<LayerVoice, 12> _layer_voices = {
        LayerVoice(_voices[0].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 0),
        LayerVoice(_voices[1].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 1),
        LayerVoice(_voices[2].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 2),
        LayerVoice(_voices[3].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 3),
        LayerVoice(_voices[4].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 4),
        LayerVoice(_voices[5].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 5),
        LayerVoice(_voices[6].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 6),
        LayerVoice(_voices[7].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 7),
        LayerVoice(_voices[8].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 8),
        LayerVoice(_voices[9].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 9),
        LayerVoice(_voices[10].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 10),
        LayerVoice(_voices[11].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 11)};
    std::array<LayerVoice, 12> &_layer_voices2 = _layer_voices;
    return true;
}

bool run_osc_dump_to_file() {
    using namespace Steinberg::Vst::Nina;
    printf("run analog voice and dump osc output to file");
    AnalogVoice analog_voice{0};
    VoiceInput &input = analog_voice.getVoiceInputBuffers();
    NinaParams::LayerParams layer_params;
    NinaParams::LayerStateParams a;
    WavetableLoader wt_loader_a;
    WavetableLoader wt_loader_b;
    float m = 0;
    std::array<float, BUFFER_SIZE> high_res_arra;
    std::array<float, BUFFER_SIZE> *high_res_arr = &high_res_arra;
    std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> high_res = {high_res_arr};
    high_res.fill(high_res_arr);
    LayerVoice voice{input, layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 0};
    std::array<float, BUFFER_SIZE> out_1, out_2;
    VoiceOutput output{&out_1, &out_2};
    std::vector<float> osc_1_up, osc_1_down;
    for (int i = 0; i < 10000; i++) {
        analog_voice.generateVoiceBuffers(output);
        for (int i2 = 0; i2 < CV_BUFFER_SIZE; i2++) {
            int mux_itr = BUFFER_SIZE / CV_BUFFER_SIZE;
            osc_1_up.push_back(output.output_1->at(i2 * mux_itr + Cv0Order::Cv0Osc1Up));
            osc_1_down.push_back(output.output_1->at(i2 * mux_itr + Cv0Order::Cv0Osc1Down));
        }
    }
    std::fstream file_handler;
    std::stringstream path;
    file_handler.open("./osc_1_up.txt", std::ios::out | std::ios::trunc);
    if (file_handler.is_open()) {
        auto it_1 = osc_1_up.begin();
        while (it_1 != osc_1_up.end()) {
            file_handler << *it_1 << ", ";

            it_1++;
        }
        file_handler << std::endl;
        file_handler.close();
    }
    file_handler.open("./osc_1_down.txt", std::ios::out | std::ios::trunc);
    if (file_handler.is_open()) {
        auto it_1 = osc_1_up.begin();
        while (it_1 != osc_1_up.end()) {
            file_handler << *it_1 << ", ";

            it_1++;
        }
        file_handler << std::endl;
        file_handler.close();
    }
    return true;
}

bool run_voice_like_factory_vst() {
    using namespace Steinberg::Vst::Nina;
    printf("run the  analog voice like the fac vst and dump filter output to file");
    AnalogVoice analog_voice{0};
    std::array<float, BUFFER_SIZE> out_1, out_2;
    VoiceOutput output{&out_1, &out_2};
    std::vector<float> samples_1;
    std::vector<float> samples_2;
    int i = 0;
    auto &input = analog_voice.getVoiceInputBuffers();
    for (auto &item : input.filt_cut) {
        float sine_wave = 0.5f + sinf32((float)i / 3.0);
        i++;
        item = sine_wave;
    }
    for (auto &item : input.tri_2_lev) {
        float sine_wave = 0.5f + sinf32((float)i / 3.0);
        i++;
        item = sine_wave;
    }
    for (int i = 0; i < 10000; i++) {

        analog_voice.generateVoiceBuffers(output);
        for (int i2 = 0; i2 < CV_BUFFER_SIZE; i2++) {
            int mux_itr = BUFFER_SIZE / CV_BUFFER_SIZE;
            samples_1.push_back(output.output_2->at(i2 * mux_itr + Cv1Order::Cv1FilterCut));
            samples_2.push_back(output.output_1->at(i2 * mux_itr + Cv0Order::Cv0MixOsc2Tri));
        }
    }
    std::fstream file_handler;
    std::stringstream path;
    file_handler.open("./filter_dump.txt", std::ios::out | std::ios::trunc);
    if (file_handler.is_open()) {
        auto it_1 = samples_1.begin();
        while (it_1 != samples_1.end()) {
            file_handler << *it_1 << ", ";

            it_1++;
        }
        file_handler << std::endl;
        file_handler.close();
    }
    file_handler.open("./tri2_dump.txt", std::ios::out | std::ios::trunc);
    if (file_handler.is_open()) {
        auto it_1 = samples_2.begin();
        while (it_1 != samples_2.end()) {
            file_handler << *it_1 << ", ";

            it_1++;
        }
        file_handler << std::endl;
        file_handler.close();
    }
    return true;
}

bool analog_efficiency_test() {
    using namespace Steinberg::Vst::Nina;
    printf("time the analog run/muxing to get an idea of how efficient it is");
    std::array<AnalogVoice, 12> voices = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    std::array<float, BUFFER_SIZE> out_1, out_2;
    VoiceOutput output{&out_1, &out_2};
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < 10000; i++) {
        for (auto &voice : voices) {
            voice.runTuning(0.1, 0.1, 0.1, 0.1);
            voice.generateVoiceBuffers(output);
        }
    }
    auto finish = std::chrono::steady_clock::now();
    float tt = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    printf("\ntime taken = %d ms\n", (int)tt);
    return true;
}

bool voice_efficiency_test() {
    using namespace Steinberg::Vst::Nina;
    printf("time 12 layer voices and analog calibration to get an idea of how efficient it is");
    std::array<AnalogVoice, 12> voices = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    NinaParams::LayerParams layer_params;
    NinaParams::LayerStateParams a;
    WavetableLoader wt_loader_a;
    WavetableLoader wt_loader_b;
    std::array<float, BUFFER_SIZE> high_res_arra;
    std::array<float, BUFFER_SIZE> *high_res_arr = &high_res_arra;
    std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> high_res = {high_res_arr};
    high_res.fill(high_res_arr);
    float m = 0;
    std::array<LayerVoice, 12> layer_voices{
        LayerVoice(voices[0].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 0),
        LayerVoice(voices[1].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 1),
        LayerVoice(voices[2].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 2),
        LayerVoice(voices[3].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 3),
        LayerVoice(voices[4].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 4),
        LayerVoice(voices[5].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 5),
        LayerVoice(voices[6].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 6),
        LayerVoice(voices[7].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 7),
        LayerVoice(voices[8].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 8),
        LayerVoice(voices[9].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 9),
        LayerVoice(voices[10].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 10),
        LayerVoice(voices[11].getVoiceInputBuffers(), layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 11)};
    std::array<float, BUFFER_SIZE> out_1, out_2;
    VoiceOutput output{&out_1, &out_2};
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < 10000; ++i) {
        for (int i2 = 0; i2 < 12; ++i2) {
            layer_voices[i2].run();
            voices[i2].runTuning(0.1, 0.1, 0.1, 0.1);
            voices[i2].generateVoiceBuffers(output);
        }
    }
    auto finish = std::chrono::steady_clock::now();
    float tt = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    printf("\ntime taken = %d ms\n", (int)tt);
    return true;
}

bool run_timbre_check_filter() {
    using namespace Steinberg::Vst::Nina;
    printf("run analog voice and get output");
    AnalogVoice analog_voice{0};
    VoiceInput &input = analog_voice.getVoiceInputBuffers();
    NinaParams::LayerParams layer_params;
    NinaParams::LayerStateParams a;
    WavetableLoader wt_loader_a;
    WavetableLoader wt_loader_b;
    std::array<float, BUFFER_SIZE> high_res_arra;
    std::array<float, BUFFER_SIZE> *high_res_arr = &high_res_arra;
    std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> high_res = {high_res_arr};
    high_res.fill(high_res_arr);
    float m = 0;
    LayerVoice voice{input, layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 0};
    voice.run();
    voice.run();
    std::array<float, BUFFER_SIZE> out_1, out_2;

    VoiceOutput output{&out_1, &out_2};

    analog_voice.generateVoiceBuffers(output);
    bool res = buffer_sanity_check(output);
    return res;
}

bool run_bunch_of_buffers() {
    using namespace Steinberg::Vst::Nina;
    printf("run analog voice for a bynch of buffers");
    AnalogVoice analog_voice{0};
    VoiceInput &input = analog_voice.getVoiceInputBuffers();
    NinaParams::LayerParams layer_params;
    NinaParams::LayerStateParams a;
    WavetableLoader wt_loader_a;
    WavetableLoader wt_loader_b;
    float m = 0;
    std::array<float, BUFFER_SIZE> high_res_arra;
    std::array<float, BUFFER_SIZE> *high_res_arr = &high_res_arra;
    std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> high_res = {high_res_arr};
    high_res.fill(high_res_arr);
    LayerVoice voice{input, layer_params, a, a, wt_loader_a, wt_loader_b, high_res, high_res_arr, 0};
    std::array<float, BUFFER_SIZE> out_1, out_2;
    VoiceOutput output{&out_1, &out_2};
    for (int i = 0; i < 1000; i++) {
        analog_voice.generateVoiceBuffers(output);
        bool res = buffer_sanity_check(output);
        if (!res) {
            return res;
        }
    }
    return true;
}

bool num_test() {
    printf("\n numerical test");
    float p = 0.5;
    float offset = (p < 0) ? 1.0f : 0.0f;
    float clipp = (p < -126) ? -126.0f : p;
    int w = (int)clipp;
    printf("\n %d", w);
    float z = clipp - w + offset;

    union {
        uint32_t i;
        float f;
    } v = {cast_uint32_t((1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z))};

    printf("\n %f %f", p, v.f);
    return true;
}

bool run_layer() {
    using namespace Steinberg::Vst::Nina;
    printf("run a layer, allocate a note, change a param");
    std::array<float, CV_BUFFER_SIZE> cv;
    NinaParams::AudioInputBuffers input = NinaParams::AudioInputBuffers(cv, cv, cv, cv);
    std::array<float, BUFFER_SIZE> out_1, out_2;
    VoiceOutput output{&out_1, &out_2};
    std::array<float, BUFFER_SIZE> high_res_arra;
    std::array<float, BUFFER_SIZE> *high_res_arr = &high_res_arra;
    std::array<std::array<float, BUFFER_SIZE> *, NUM_VOICES> high_res = {high_res_arr};
    high_res.fill(high_res_arr);
    std::vector<ParamChange> changes;
    NinaParams::GlobalParams gp;
    std::array<AnalogVoice, 12> voices = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    Layer layer{12, 0, voices, high_res, input, changes, gp};
    layer.run();
    for (int i2 = 0; i2 < 12; ++i2) {
        voices[i2].setUnitTestMode();
    }
    Steinberg::Vst::NoteOnEvent note = {0, 50, 1.0, 1.0};
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    ParamChange change = ParamChange(60, 0.8);
    layer.updateParams(1, &change);
    change = ParamChange(222, 1);
    layer.updateParams(1, &change);
    change = ParamChange(226, 1);
    layer.updateParams(1, &change);
    change = ParamChange(46, 1);
    layer.updateParams(1, &change);
    change = ParamChange(49, 0.6);
    layer.updateParams(1, &change);
    for (int i2 = 0; i2 < 12; ++i2) {
        voices[i2].setUnitTestMode();
    }
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < 10; ++i) {
        layer.run();

        for (int i2 = 0; i2 < 1; ++i2) {
            voices[i2].generateVoiceBuffers(output);
        }
    }
    for (int i = 0; i < 16; i++) {
        printf("\n%f", output.output_2->at(i * 8 + Cv1FilterCut));
        printf("\t%f", output.output_2->at(i * 8 + Cv1AmpL));
        printf("\t%f", output.output_1->at(i * 8 + Cv0Osc1Down));
        printf("\t%f", output.output_1->at(i * 8 + Cv0Osc1Up));
        printf("\t%f", voices[0].getVoiceInputBuffers().osc_1_pitch.at(i));
    }

    note = {0, 55, 1.0, 1.0};
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    layer.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    change = ParamChange(60, 0.8);
    layer.updateParams(1, &change);
    for (int i = 0; i < 10; ++i) {
        layer.run();

        for (int i2 = 0; i2 < 1; ++i2) {
            voices[i2].generateVoiceBuffers(output);
        }
    }
    for (int i = 0; i < 16; i++) {
        printf("\n%f", output.output_2->at(i * 8 + Cv1FilterCut));
        printf("\t%f", output.output_2->at(i * 8 + Cv1AmpL));
        printf("\t%f", output.output_1->at(i * 8 + Cv0Osc1Down));
        printf("\t%f", output.output_1->at(i * 8 + Cv0Osc1Up));
        printf("\t%f", voices[0].getVoiceInputBuffers().osc_1_pitch.at(i));
    }

    auto finish = std::chrono::steady_clock::now();
    float tt = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    printf("\ntime taken = %d ms\n", (int)tt);

    return true;
}

bool run_layer_manager() {
    printf("\nrun layer manager");
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;

    ParamChange change;
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc1Pitch), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc2Pitch), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::VcaIn), 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::VcaIn), 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc1Width, 0.6);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc2Width, 0.6);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Legato, 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::NumUnison, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Level), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeSus), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeAtt), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeRel), 1);
    manager.updateParams(1, &change);
    manager.setLayerSmoothing(14);
    manager.setUnitTestMode();

    Steinberg::Vst::NoteOnEvent note = {0, 50, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    note = {0, 51, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.processAudio(data);

    for (int i = 0; i < 16; i++) {
        printf("\n%f", buffs[13][(i * 8 + Cv1FilterCut)]);
        printf("\t%f", buffs[13][(i * 8 + Cv1AmpL)]);
        printf("\t%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
        printf("\t%f", buffs[12][(i * 8 + Cv0Osc1Up)]);
    }
    Steinberg::Vst::NoteOffEvent note2 = {0, 50, 1.0, -1};
    manager.freeVoices(Steinberg::Vst::Nina::MidiNote(note2));
    note2 = {0, 51, 1.0, -1};
    manager.freeVoices(Steinberg::Vst::Nina::MidiNote(note2));
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
    };
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

void print_fail() {
    printf("\n \033[1;31mFailed\033[0m\n");
}

void print_pass() {

    printf("\n \033[1;32mPass \033[0m\n");
    // throw 20;
}

bool simple_poly_note_test() {
    printf("\n simple poly note test");
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;
    manager.setLayerSmoothing(14);

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;

    ParamChange change;
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc1Pitch), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc2Pitch), 1);
    manager.updateParams(1, &change);
    printf("\n\n%d\n\n", MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::VcaIn));
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::VcaIn), 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::VcaIn), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Width), 0.8);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc2Width), 0.8);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Legato, 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::NumUnison, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::AmpEnvVelSense, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Level), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeSus), .5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeAtt), 0.1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeRel), 0.0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeLevel), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterEnvelopeLevel), 1);
    manager.updateParams(1, &change);
    manager.setLayerSmoothing(14);

    for (int i = 0; i < 30; i++) {
        manager.processAudio(data);
        float res = (buffs[13][(Cv1AmpL)]);
        printf("\n %f", res);
    };
    Steinberg::Vst::NoteOnEvent note = {0, 50, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 30; i++) {
        manager.processAudio(data);
        float res = (buffs[13][(Cv1AmpL)]);
        printf("\n %f", res);
    };
    int i = 0;
    float res = abs(buffs[13 + 2 * i][(Cv1AmpL)]) + abs(buffs[13 + 2 * i][(Cv1AmpR)]);
    // assert(res > 0.1);
    note = {0, 51, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
    };
    i = 1;
    res = abs(buffs[13 + 2 * i][(Cv1AmpL)]) + abs(buffs[13 + 2 * i][(Cv1AmpR)]);
    // assert(res > 0.1);

    Steinberg::Vst::NoteOffEvent note2 = {0, 50, 1.0, -1};
    manager.freeVoices(Steinberg::Vst::Nina::MidiNote(note2));
    note2 = {0, 51, 1.0, -1};
    manager.freeVoices(Steinberg::Vst::Nina::MidiNote(note2));
    for (int i = 0; i < 20; i++) {
        manager.processAudio(data);
        res = (buffs[13][(Cv1AmpL)]);
        printf("\n %f", res);
    };
    i = 0;
    res = abs(buffs[13 + 2 * i][(Cv1AmpL)]) + abs(buffs[13 + 2 * i][(Cv1AmpR)]);
    // assert(res < 0.1);
    i = 1;
    res = abs(buffs[13 + 2 * i][(Cv1AmpL)]) + abs(buffs[13 + 2 * i][(Cv1AmpR)]);
    // assert(res < 0.1);

    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }

    return true;
}

bool wt_load_run_test() {
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    std::vector<float> audio_render;
    audio_render.reserve(96000 * 1);
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc1Pitch), 1);

    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc2Pitch), 1);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::VcaIn), 0.5);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc1Width, 0.6);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc2Width, 0.6);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Legato, 0);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::NumUnison, 0);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), .5);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::VcaIn), 0.5);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::FilterCutoff), 0.3);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo1Gain), 0.9);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange((NinaParams::LfoShape), 0.3);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo1Rate), 0.93);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::PanPosition, NinaParams::VcaIn), 0.5);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyVelocity, NinaParams::VcaIn), 0.5);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    manager.setLayerSmoothing(14);
    Steinberg::Vst::NoteOnEvent note = {0, 50, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    for (int i = 0; i < 20; i++) {
        manager.processAudio(data);
    }
    change = make_param_layer(ParamChange((NinaParams::WavetableSelect), 0.1), 1);
    change = make_param_layer(change, 0);
    manager.updateParams(1, &change);
    for (int i = 0; i < 20; i++) {
        manager.processAudio(data);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < (96000 / 128); i++) {
        manager.processAudio(data);
        for (int sp = 0; sp < BUFFER_SIZE; ++sp) {
            audio_render.push_back(data.outputs[0].channelBuffers32[0][sp]);
        }
    }
    auto finish = std::chrono::steady_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    printf("\n time taken %f ", time);

    AudioFile<float> audio_data;
    std::fstream file_handler;
    audio_data.setNumChannels(1);
    audio_data.setNumSamplesPerChannel(10 * 96000);
    int cont = 0;
    for (float s : audio_render) {

        audio_data.samples[0][cont++] = s;
    }

    audio_data.setSampleRate(96000);
    audio_data.save("/udata/wave_out.wav");
    audio_data.setBitDepth(24);

    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }

    // send a note on to check voice 0 allocated. check voice 1 not allocated
    return true;
}

bool wt_alloc_test() {

    using namespace Steinberg::Vst::Nina;
    std::array<WavetableLoader, 8> wave_loader_array;
    std::array<float, BUFFER_SIZE> output;
    std::array<float, BUFFER_SIZE> dummy;
    output.fill(0);
    auto *o_p = &output;
    auto *dummy_p = &dummy;
    float morph = 1;
    float drive = 0.5;
    bool interpolate = true;
    bool slow = false;
    std::array<WavetableOsc, 12> osc_array =
        {
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, o_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
        };
    bool first = false;
    std::array<float *, 12> pos;
    std::array<float *, 12> pitch;
    std::array<float *, 12> vol;
    for (int i = 0; i < 12; i++) {
        pos.at(i) = osc_array.at(i).getWtPosition();
        pitch.at(i) = osc_array.at(i).getWtPitch();
        vol.at(i) = osc_array.at(i).getWtVol();
        *vol.at(i) = 1.0;
    }
    float runtime = 1.1;
    std::vector<float> pos_v;
    std::vector<float> pitch_v;
    for (int i = 0; i < runtime * CV_SAMPLE_RATE; i++) {
        float phase_1 = ((float)i) / (float)CV_SAMPLE_RATE;
        phase_1 *= 2;
        if (phase_1 > 1)
            phase_1 = 2 - phase_1;
        pos_v.push_back((phase_1)*0 + 0.1);
        float phase_2 = ((2 * 2 * M_PI) * (float)i) / (float)CV_SAMPLE_RATE;

        pitch_v.push_back(1.5 + 0.1 * std::sin(phase_2));
        // printf("\n %f %f", pitch_v.at(i), pos_v.at(i));
    }

    std::vector<float> loads;
    loads.push_back(0.1);
    loads.push_back(0.2);
    loads.push_back(0.3);
    loads.push_back(0.4);

    loads.push_back(0.1);
    loads.push_back(0.2);
    loads.push_back(0.3);
    loads.push_back(0.4);
    loads.push_back(0.1);
    loads.push_back(0.2);
    loads.push_back(0.3);
    loads.push_back(0.4);

    loads.push_back(0.1);
    loads.push_back(0.2);
    loads.push_back(0.3);
    loads.push_back(0.4);
    for (float wt_load : loads) {
        printf("\nloading %f\n\n", wt_load);
        for (auto &loader : wave_loader_array) {
            loader.loadWavetable(wt_load);
        }
        osc_array.at(0).printstuff();
        while (1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            bool exit = true;
            for (auto &loader : wave_loader_array) {
                if (loader.isLoading()) {
                    exit = false;
                }
            }
            if (exit)
                break;
        }
    }

    auto pos_p = pos_v.begin();
    auto pitch_p = pitch_v.begin();
    std::vector<float> samples;
    samples.reserve(96000 * runtime);
    auto start = std::chrono::steady_clock::now();
    int in_i = 0;
    for (int i = 0; i < ((BUFFER_RATE * runtime) - 5); i++) {
        for (int osc_i = 0; osc_i < 12; osc_i++) {
            int in_i_osc = in_i;
            osc_array.at(osc_i).reCalculate();
            for (int bp = 0; bp < CV_BUFFER_SIZE; bp++) {
                *pos.at(osc_i) = pos_v.at(in_i);
                *pitch.at(osc_i) = pitch_v.at(in_i);
                osc_array.at(osc_i).run();
                if (osc_i == 0)
                    in_i_osc++;
            }
        }
        in_i += CV_BUFFER_SIZE;
        for (int sp = 0; sp < BUFFER_SIZE; sp++) {
            samples.push_back(output[sp]);
        }
    }
    auto finish = std::chrono::steady_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    printf("\n time taken %f ", time);
    return true;
}

bool run_single_wavetable_save_output() {

    using namespace Steinberg::Vst::Nina;
    std::array<WavetableLoader, 8> wave_loader_array;
    std::array<float, BUFFER_SIZE> output;
    std::array<float, BUFFER_SIZE> dummy;
    output.fill(0);
    auto *o_p = &output;
    auto *dummy_p = &dummy;
    float morph = 1;
    float drive = 0.5;
    bool interpolate = true;
    bool slow = false;
    std::array<WavetableOsc, 12> osc_array =
        {
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, o_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
            WavetableOsc(wave_loader_array.at(0), wave_loader_array.at(1), morph, dummy_p, drive, interpolate, slow),
        };
    bool first = false;
    std::array<float *, 12> pos;
    std::array<float *, 12> pitch;
    std::array<float *, 12> vol;
    for (int i = 0; i < 12; i++) {
        pos.at(i) = osc_array.at(i).getWtPosition();
        pitch.at(i) = osc_array.at(i).getWtPitch();
        vol.at(i) = osc_array.at(i).getWtVol();
        *vol.at(i) = 1.0;
    }
    float runtime = 1.1;
    std::vector<float> pos_v;
    std::vector<float> pitch_v;
    for (int i = 0; i < runtime * CV_SAMPLE_RATE; i++) {
        float phase_1 = ((float)i) / (float)CV_SAMPLE_RATE;
        phase_1 *= 2;
        if (phase_1 > 1)
            phase_1 = 2 - phase_1;
        pos_v.push_back((phase_1)*0 + 0.1);
        float phase_2 = ((2 * 2 * M_PI) * (float)i) / (float)CV_SAMPLE_RATE;

        pitch_v.push_back(1.5 + 0.1 * std::sin(phase_2));
        // printf("\n %f %f", pitch_v.at(i), pos_v.at(i));
    }
    for (auto &loader : wave_loader_array) {
        loader.loadWavetable(.08);
    }
    osc_array.at(0).printstuff();
    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto wt = (wave_loader_array.at(0).getCurrentWavetable());
        if (wt != nullptr)
            break;
    }

    AudioFile<float> audio_data;
    auto &samples_2 = wave_loader_array.at(0).getCurrentWavetable()->_samples;
    audio_data.setNumChannels(1);
    auto val = samples_2.size();
    printf("\n val %d\n\n\n", (int)val);
    audio_data.setNumSamplesPerChannel(samples_2.size());
    int cont = 0;
    int size = samples_2.size();
    printf("\n wt size %d\n", size);
    for (uint i = 0; i < size; i++) {
        audio_data.samples[0][cont++] = (float)samples_2.at(i) / (float)INT16_MAX;
    }

    audio_data.setSampleRate(96000);
    audio_data.setBitDepth(24);
    audio_data.save("/udata/wavetable.wav");

    auto pos_p = pos_v.begin();
    auto pitch_p = pitch_v.begin();
    std::vector<float> samples;
    samples.reserve(96000 * runtime);
    auto start = std::chrono::steady_clock::now();
    int in_i = 0;
    for (int i = 0; i < ((BUFFER_RATE * runtime) - 5); i++) {
        for (int osc_i = 0; osc_i < 12; osc_i++) {
            int in_i_osc = in_i;
            osc_array.at(osc_i).reCalculate();
            for (int bp = 0; bp < CV_BUFFER_SIZE; bp++) {
                *pos.at(osc_i) = pos_v.at(in_i);
                *pitch.at(osc_i) = pitch_v.at(in_i);
                osc_array.at(osc_i).run();
                if (osc_i == 0)
                    in_i_osc++;
            }
        }
        in_i += CV_BUFFER_SIZE;
        for (int sp = 0; sp < BUFFER_SIZE; sp++) {
            samples.push_back(output[sp]);
        }
    }
    auto finish = std::chrono::steady_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    printf("\n time taken %f ", time);
    samples.push_back(0);

    std::fstream file_handler;
    audio_data.shouldLogErrorsToConsole(true);
    audio_data.setNumChannels(1);
    audio_data.setNumSamplesPerChannel(samples.size());
    cont = 0;
    size = samples.size();
    printf("\n wt size %d\n", size);
    for (uint i = 0; i < size; i++) {
        audio_data.samples[0][cont++] = (float)samples.at(i) / 4.f;
    }

    audio_data.setSampleRate(96000);
    audio_data.setBitDepth(24);
    audio_data.save("/udata/wave_out.wav");

    return true;
}

bool filter_gen_test() {
    using namespace Steinberg::Vst::Nina;

    // scope trigger filter setup
    float cutoff = 60;
    float x = 2 * M_PI * cutoff / (float)SAMPLE_RATE;
    float p_lp = (2 - cosf32(x)) - sqrtf32((2 - cosf32(x)) * (2 - cosf32(x)) - 1);

    std::vector<float> input_data;

    for (int i = 0; i < 100000; i++) {
        float val = 0;
        if (i % 1000 >= 999) {
            val = 1.0;
        }
        input_data.push_back(val);
    }
    float old = 0;
    float old_2 = 0;
    for (float &item : input_data) {
        float tmp = item;
        tmp = (1 - p_lp) * item + p_lp * old;
        float tmp_2 = (1 - p_lp) * tmp + p_lp * old_2;
        old = tmp;
        old_2 = tmp_2;
        item = item - tmp_2;
    }

    AudioFile<float> audio_data;
    std::fstream file_handler;
    audio_data.shouldLogErrorsToConsole(true);
    audio_data.setNumChannels(1);
    audio_data.setNumSamplesPerChannel(input_data.size());
    int cont = 0;
    int size = input_data.size();
    printf("\n wt size %d\n", size);
    for (uint i = 0; i < size; i++) {
        audio_data.samples[0][cont++] = (float)input_data.at(i) / 4.f;
    }

    audio_data.setSampleRate(96000);
    audio_data.setBitDepth(24);
    audio_data.save("./filter.wav");

    return true;
}

bool lfo_test() {
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc1Pitch), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc2Pitch), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::VcaIn), 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc1Width, 0.6);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc2Width, 0.6);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Legato, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::NumUnison, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::LfoShape, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), .5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::VcaIn), 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::FilterCutoff), 0.3);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo1Gain), 0.9);
    manager.updateParams(1, &change);
    change = ParamChange((NinaParams::LfoShape), 0.0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo1Rate), .689);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::PanPosition, NinaParams::VcaIn), 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyVelocity, NinaParams::VcaIn), 0.5);
    manager.updateParams(1, &change);
    manager.setLayerSmoothing(14);

    for (int i = 0; i < 20; i++) {
        manager.processAudio(data);
    }
    manager.dump();

    // send a note on to check voice 0 allocated. check voice 1 not allocated
    for (int i = 0; i < 100; i++) {
        manager.processAudio(data);
        //  printf("\n");
        for (int i = 0; i < 16; i++) {
            printf("\n%f", buffs[13][(i * 8 + Cv1FilterCut)]);
        }
    };
    Steinberg::Vst::NoteOffEvent note2 = {0, 50, 1.0, -1};
    manager.freeVoices(Steinberg::Vst::Nina::MidiNote(note2));

    for (int i = 0; i < 100; i++) {
        manager.processAudio(data);
        // printf("\n");
        for (int i = 0; i < 16; i++) {
            // printf("\n%f", buffs[13][(i * 8 + Cv1AmpL)]);
        }
    };
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

bool env_test() {
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc1Pitch), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc2Pitch), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc3Pitch), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::VcaIn), 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::VcaIn), 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc1Width, 0.6);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc2Width, 0.6);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Legato, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::NumUnison, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Level), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeSus), .8);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeAtt), 0.01);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeDec), 0.1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeRel), 0.1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeLevel), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::VcaIn), 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo2, NinaParams::VcaIn), 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::FilterEnvelope, NinaParams::VcaIn), 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::VcaIn), 0.6);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::PanPosition, NinaParams::VcaIn), 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyVelocity, NinaParams::VcaIn), 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::VcaIn), 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Time, NinaParams::VcaIn), 0.5);
    manager.updateParams(1, &change);
    manager.setLayerSmoothing(14);

    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
    }
    // send a note on to check voice 0 allocated. check voice 1 not allocated
    Steinberg::Vst::NoteOnEvent note = {0, 50, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 20; i++) {
        manager.dump();
        change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeSus), .8 - 0.02 * i);
        manager.updateParams(1, &change);
        manager.processAudio(data);
        //  printf("\n");
        for (int i = 0; i < 16; i++) {
            // printf("\n%f", buffs[13][(i * 8 + Cv1AmpL)]);
        }
    };
    Steinberg::Vst::NoteOffEvent note2 = {0, 50, 1.0, -1};
    manager.freeVoices(Steinberg::Vst::Nina::MidiNote(note2));

    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
        // printf("\n");
        for (int i = 0; i < 16; i++) {
            // printf("\n%f", buffs[13][(i * 8 + Cv1AmpL)]);
        }
    };
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

bool simple_mono_note_test() {
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc1Pitch), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc2Pitch), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::VcaIn), 0.f);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::VcaIn), 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc1Width, 0.6);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc2Width, 0.6);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Legato, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::NumUnison, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Level), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeSus), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeAtt), 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeRel), 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeLevel), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterEnvelopeLevel), 1);
    manager.updateParams(1, &change);
    manager.setLayerSmoothing(14);

    // send a note on to check voice 0 allocated. check voice 1 not allocated
    Steinberg::Vst::NoteOnEvent note = {0, 50, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
    };
    printf("\n %f %f ", buffs[12][Cv0Osc1Up], buffs[12][Cv0Osc1Down]);
    // voice 0 on?
    int i = 0;
    float res = abs(buffs[13 + 2 * i][(Cv1AmpL)]) + abs(buffs[13 + 2 * i][(Cv1AmpR)]);
    // assert(res > 0.1);

    // voice 1 off?
    i = 1;
    res = abs(buffs[13 + 2 * i][(Cv1AmpL)]) + abs(buffs[13 + 2 * i][(Cv1AmpR)]);
    // assert(res < 0.1);

    // send 2nd note. verify only voice 1 used
    note = {0, 51, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 100; i++) {
        manager.processAudio(data);
    };
    printf("\n %f %f \n", buffs[12][Cv0Osc1Up], buffs[12][Cv0Osc1Down]);
    // voice 0 on?
    i = 0;
    res = abs(buffs[13 + 2 * i][(Cv1AmpL)]) + abs(buffs[13 + 2 * i][(Cv1AmpR)]);
    // assert(res > 0.1);

    // voice 1 off?
    i = 1;
    res = abs(buffs[13 + 2 * i][(Cv1AmpL)]) + abs(buffs[13 + 2 * i][(Cv1AmpR)]);
    // assert(res < 0.1);

    i = 0;
    float pitch = abs(buffs[12 + 2 * i][(Cv0Osc1Down)]) + abs(buffs[12 + 2 * i][(Cv0Osc1Up)]);
    // send note off for the 2nd note voice 0 should stay on
    Steinberg::Vst::NoteOffEvent note2 = {0, 51, 1.0, -1};
    manager.freeVoices(Steinberg::Vst::Nina::MidiNote(note2));
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
    };
    // voice 0 on?
    i = 0;
    res = abs(buffs[13 + 2 * i][(Cv1AmpL)]) + abs(buffs[13 + 2 * i][(Cv1AmpR)]);
    // assert(res > 0.1);
    float pitch_2 = abs(buffs[12 + 2 * i][(Cv0Osc1Down)]) + abs(buffs[12 + 2 * i][(Cv0Osc1Up)]);

    // send note off for the 1nd note voice 0 should go off also check pitch changes
    note2 = {0, 50, 1.0, -1};
    manager.freeVoices(Steinberg::Vst::Nina::MidiNote(note2));
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
    };
    // voice 0 off?
    i = 0;
    res = abs(buffs[13 + 2 * i][(Cv1AmpL)]) + abs(buffs[13 + 2 * i][(Cv1AmpR)]);
    // assert(res < 0.1);

    // is the pitch different?
    // assert(abs(pitch - pitch_2) > 0.0001);
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

bool velocity_test() {
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;
    manager.setLayerSmoothing(14);

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;
    change = ParamChange(NinaParams::AmpEnvVelSense, 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::VcaIn), 0.f);
    manager.updateParams(1, &change);

    // send note on with zero vel
    Steinberg::Vst::NoteOnEvent note = {0, 50, 1.0, 0.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 16; i++) {
            printf("\n%f", buffs[13][(i * 8 + Cv1AmpL)]);
        }
    };
    // vol should be 0 still
    int i = 0;
    float res = abs(buffs[13 + 2 * i][(Cv1AmpL)]) + abs(buffs[13 + 2 * i][(Cv1AmpR)]);
    // assert(res < 0.1);

    // send note off
    Steinberg::Vst::NoteOffEvent note2 = {0, 50, 1.0, -1};
    manager.freeVoices(Steinberg::Vst::Nina::MidiNote(note2));
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
    };

    // send note on with max vel
    for (int i = 0; i < 12; i++) {
        note = {0, 50, 1.0, 1.0};
        manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    }

    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
    };
    // voice 0 on?
    i = 0;
    res = abs(buffs[13 + 2 * i][(Cv1AmpL)]) + abs(buffs[13 + 2 * i][(Cv1AmpR)]);
    // assert(res > 0.1);
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

bool test_calibration_routine() {
    printf("\ntest cal ");
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;

    // test 5 octaves down
    change = ParamChange(NinaParams::AllRunTuning, 1);
    manager.updateParams(1, &change);
    manager.setLayerSmoothing(14);

    // send note on with zero vel
    for (int i = 0; i < 1000; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            // printf("\n%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
            // printf("\t%f", buffs[12][(i * 8 + Cv0Osc1Up)]);
            // printf("\t%f", buffs[12][(i * 8 + Cv0Osc2Down)]);
            // printf("\t%f", buffs[12][(i * 8 + Cv0Osc2Up)]);
        }
    }

    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

bool test_semitone_transpose() {
    printf("\ntest semitone transpose");
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;

    // test 5 octaves down
    change = ParamChange(NinaParams::AmpEnvVelSense, 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Vco1TuneCoarse, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Width), .5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc2Width), .5);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Vco1TuneSemitone, 0.1);
    manager.updateParams(1, &change);
    manager.setLayerSmoothing(14);

    // send note on with zero vel
    Steinberg::Vst::NoteOnEvent note = {0, 10, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            printf("\n%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
            printf("\t%f", buffs[12][(i * 8 + Cv0Osc1Up)]);
            printf("\t%f", buffs[12][(i * 8 + Cv0Osc2Down)]);
            printf("\t%f", buffs[12][(i * 8 + Cv0Osc2Up)]);
        }
    }
    float f1 = buffs[12][(0 * 8 + Cv0Osc1Up)];
    float f2 = buffs[12][(0 * 8 + Cv0Osc2Up)];
    // assert(f1 < (f2 - f2 * 0.001));
    //  vol should be 0 still

    change = ParamChange(NinaParams::Vco1TuneSemitone, 1);
    manager.updateParams(1, &change);
    // test 1 semitone down
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
        printf("1\n%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
        printf("1\t%f", buffs[12][(i * 8 + Cv0Osc1Up)]);
        printf("1\t%f", buffs[12][(i * 8 + Cv0Osc2Down)]);
        printf("1\t%f", buffs[12][(i * 8 + Cv0Osc2Up)]);
        float f1 = buffs[12][(0 * 8 + Cv0Osc1Down)];
        float f2 = buffs[12][(0 * 8 + Cv0Osc2Down)];
        // assert(f1 > (f2 - f2 * 0.001));
    }
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

bool test_coarse_pitch() {
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;

    // test 5 octaves down
    change = ParamChange(NinaParams::AmpEnvVelSense, 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Vco2TuneCoarse, .5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Width), .0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc2Width), .0);
    manager.setLayerSmoothing(14);

    // send note on with zero vel
    Steinberg::Vst::NoteOnEvent note = {0, 10, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);

        for (int i = 0; i < 1; i++) {
            printf("\n%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
            printf("\t%f", buffs[12][(i * 8 + Cv0Osc1Up)]);
            printf("\t%f", buffs[12][(i * 8 + Cv0Osc2Down)]);
            printf("\t%f", buffs[12][(i * 8 + Cv0Osc2Up)]);
        }
    }
    float f1 = buffs[12][(0 * 8 + Cv0Osc1Down)];
    float f2 = buffs[12][(0 * 8 + Cv0Osc2Down)];
    // assert(f1 < (f2 - f2 * 0.001));
    //  vol should be 0 still

    // test 1 semitone down
    change = ParamChange(NinaParams::Vco1TuneCoarse, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Vco1TuneFine, 0);
    manager.updateParams(1, &change);
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
        printf("\n%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
        printf("\t%f", buffs[12][(i * 8 + Cv0Osc1Up)]);
        printf("\t%f", buffs[12][(i * 8 + Cv0Osc2Down)]);
        printf("\t%f", buffs[12][(i * 8 + Cv0Osc2Up)]);
        float f1 = buffs[12][(0 * 8 + Cv0Osc1Down)];
        float f2 = buffs[12][(0 * 8 + Cv0Osc2Down)];
        // assert(f1 < (f2 - f2 * 0.001));
    }
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

bool test_od_comp() {
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;

    // test 5 octaves down
    change = ParamChange(NinaParams::AmpEnvVelSense, 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Vco1TuneCoarse, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc1Width, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc2Width, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Level), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Blend), .5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo1Gain), 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo2Gain), 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc2Level), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::XorLevel), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc3Level), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::VcaIn), 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::VcfDrive, 1);
    manager.updateParams(1, &change);
    manager.setLayerSmoothing(14);

    // send note on with zero vel
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            printf("\n%f", buffs[12][(i * 8 + Cv0MixOsc1Tri)]);
            printf("\t%f", buffs[12][(i * 8 + Cv0MixOsc1Sq)]);
            printf("\t%f", buffs[13][(i * 8 + Cv1AmpL)]);
        }
    }
    change = ParamChange(NinaParams::VcfDrive, 0.2);
    manager.updateParams(1, &change);
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
        /**
        for (int i = 0; i < 1; i++) {
            printf("\n%f", buffs[12][(i * 8 + Cv0MixOsc1Tri)]);
            printf("\t%f", buffs[12][(i * 8 + Cv0MixOsc2Tri)]);
            printf("\t%f", buffs[12][(i * 8 + Cv0MixOsc1Sq)]);
            printf("\t%f", buffs[12][(i * 8 + Cv0MixOsc2Sq)]);
        }
        **/
    }
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

bool test_analog_osc_delta() {

    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc1Pitch), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::KeyPitch, NinaParams::Osc2Pitch), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::VcaIn), 0.f);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::VcaIn), 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc1Width, 0.6);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Vco1TuneCoarse, 0.1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Vco2TuneCoarse, 0.1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc2Width, 0.6);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Legato, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::NumUnison, 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo1Rate), .9);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Lfo1Gain), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Lfo1, NinaParams::Osc1Pitch), .5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc1Level), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeSus), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeAtt), 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeRel), 0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeLevel), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterEnvelopeLevel), 1);
    manager.updateParams(1, &change);
    manager.setLayerSmoothing(14);
    for (int i = 0; i < 5; i++) {
        manager.processAudio(data);
        printf("\n %f %f", buffs[12][Cv0Osc1Up], buffs[12][Cv0Osc1Down]);
    };
    manager.setUnitTestMode();
    manager.dump();

    // send a note on to check voice 0 allocated. check voice 1 not allocated
    Steinberg::Vst::NoteOnEvent note = {0, 50, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 200; i++) {
        manager.processAudio(data);
        // printf("\n %f ", buffs[12][Cv0Osc1Up], buffs[12][Cv0Osc1Down]);
    };
    // voice 0 on?
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

bool test_analog_osc() {

    using namespace Steinberg::Vst::Nina;
    AnalogOscModel osc(0, 0);
    const float freq = std::log2(500);
    float shape = 0.5;
    const float shape2 = shape;
    float vup, vdown, fup, fdown;
    osc.generateOscSignals(freq, shape2, vup, vdown, fup, fdown);
    printf("\n %f %f %f %f ", freq, shape, fup, fdown);
    float fupl = std::exp2(fup);
    float fdownl = std::exp2(fdown);
    float fsum = 1. / (1. / fupl + 1. / fdownl);
    printf("\n f1 %f f2 %f  fsum: %f", fupl, fdownl, fsum);
    printf("\n v: %f %f ", vup, vdown);
    shape = 1;
    const float shape3 = shape;
    vup, vdown, fup, fdown;
    osc.generateOscSignals(freq, shape3, vup, vdown, fup, fdown);
    printf("\n %f %f %f %f ", freq, shape, fup, fdown);
    fupl = std::exp2(fup);
    fdownl = std::exp2(fdown);
    float fsum2 = 1. / (1. / fupl + 1. / fdownl);
    // assert(abs(fsum - fsum2) < 0.01);
    printf("\n f1 %f f2 %f  fsum: %f", fupl, fdownl, fsum2);
    printf("\n v: %f %f ", vup, vdown);
    return true;
}

bool test_smoothing() {
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;

    // test 5 octaves down
    change = ParamChange(NinaParams::AmpEnvVelSense, 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Vco1TuneCoarse, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc1Width, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc2Width, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 0.6);
    manager.updateParams(1, &change);

    manager.setLayerSmoothing(14);
    // send note on with zero vel
    Steinberg::Vst::NoteOnEvent note = {0, 10, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 1; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            // printf("\n%f", buffs[13][(i * 8 + Cv1FilterCut)]);
        }
    }
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 0.8);
    manager.updateParams(1, &change);
    for (int i = 0; i < 20; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            // printf("\n%f", buffs[13][(i * 8 + Cv1FilterCut)]);
        }
    }
    manager.setLayerSmoothing(1);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 0.6);
    manager.updateParams(1, &change);
    float old_val = 0;

    for (int i = 0; i < 200; i++) {
        manager.processAudio(data);
        // check the last 2 buffers and make sure the value of the output is still settling.
        if (i > 198) {
            float val = buffs[13][(i * 8 + Cv1FilterCut)];
            // assert(abs(val - old_val) > 0.0000001);
        }
    }
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

bool test_filter() {
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;

    // test 5 octaves down
    change = ParamChange(NinaParams::AmpEnvVelSense, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Vco1TuneCoarse, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc1Width, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc2Width, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 0.0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::FilterCutoff), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeAtt), .4);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeDec), .4);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeSus), 1);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::AmpEnvelopeLevel), 1);
    manager.updateParams(1, &change);

    manager.setLayerSmoothing(14);
    Steinberg::Vst::NoteOnEvent note = {0, 10, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 50; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            printf("\n%f", buffs[13][(i * 8 + Cv1FilterCut)]);
            // printf("amp: %f", buffs[13][(i * 8 + Cv1AmpL)]);
        }
    }
    // send note on with zero vel
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 16; i++) {
            //       printf("\nfilter: %f", buffs[13][(i * 8 + Cv1FilterCut)]);
            //     printf("\tamp: %f", buffs[13][(i * 8 + Cv1AmpL)]);
        }
    }
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

bool pitchbendTest() {
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;

    // test 5 octaves down
    change = ParamChange(NinaParams::AmpEnvVelSense, 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Vco1TuneCoarse, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc1Width, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc2Width, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 0.6);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Legato, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Glide, 0.0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::PitchBendRange, 0.2);
    manager.updateParams(1, &change);

    manager.setLayerSmoothing(14);
    manager.dump();
    // send note on with zero vel
    Steinberg::Vst::NoteOnEvent note = {0, 10, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            printf("\n%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
        }
    }
    change = ParamChange(NinaParams::MidiPitchBend, 1);
    manager.updateParams(1, &change);
    for (int i = 0; i < 20; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            printf("\n%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
            // printf("\n%f", buffs[13][(i * 8 + Cv1FilterCut)]);
        }
    }
    change = ParamChange(NinaParams::MidiPitchBend, 0.5);
    manager.updateParams(1, &change);
    for (int i = 0; i < 20; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            printf("\n%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
            // printf("\n%f", buffs[13][(i * 8 + Cv1FilterCut)]);
        }
    }
    change = ParamChange(NinaParams::MidiPitchBend, 0.0);
    manager.updateParams(1, &change);
    for (int i = 0; i < 20; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            printf("\n%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
            // printf("\n%f", buffs[13][(i * 8 + Cv1FilterCut)]);
        }
    }
    change = ParamChange(NinaParams::MidiPitchBend, 1);
    manager.updateParams(1, &change);
    for (int i = 0; i < 20; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            printf("\n%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
            // printf("\n%f", buffs[13][(i * 8 + Cv1FilterCut)]);
        }
    }
    change = ParamChange(NinaParams::MidiPitchBend, 0.0);
    manager.updateParams(1, &change);
    for (int i = 0; i < 20; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            printf("\n%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
            // printf("\n%f", buffs[13][(i * 8 + Cv1FilterCut)]);
        }
    }
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

bool wtTest() {
    printf("\n Wavetable Test");
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;

    // test 5 octaves down
    change = ParamChange(NinaParams::AmpEnvVelSense, 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Vco1TuneCoarse, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc1Width, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc2Width, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 0.6);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::Osc3Level), .5001);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Legato, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Glide, 0.0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::WavetableSelect, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::LayerState, 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::WavetableSelect, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::LayerState, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::WaveTuneCoarse, 0.0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::WaveTuneFine, 0.0);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::AmpEnvelope, NinaParams::Osc3Pitch), .50);
    manager.updateParams(1, &change);

    manager.setLayerSmoothing(14);
    // send note on with zero vel
    Steinberg::Vst::NoteOnEvent note = {0, 60, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 1000; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 128; i++) {
        }
    }
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 128; i++) {
        }
    }
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 128; i++) {
            // printf("\n%f", buffs[0][(i)]);
        }
    }
    auto finish = std::chrono::steady_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    printf("\n time: %f", time);
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

bool glide_test() {
    using namespace Steinberg::Vst::Nina;
    LayerManager manager;
    manager.setUnitTestMode();
    Steinberg::Vst::ProcessData data;
    Steinberg::Vst::ProcessContext context;
    context.tempo = 1.f;
    data.processContext = &context;
    Steinberg::Vst::AudioBusBuffers buffers;
    data.outputs = &buffers;
    Steinberg::Vst::AudioBusBuffers in;
    data.inputs = &in;

    float *buffs[36];

    for (int i = 0; i < 36; i++) {
        buffs[i] = new float[128];
    }
    float bufi0[128];
    float *chi0 = bufi0;
    float bufi1[128];
    float *chi1 = bufi1;
    float *inbuf[7] = {chi0, chi1, chi1, chi1, chi1, chi1, chi1};

    data.outputs[0].channelBuffers32 = buffs;
    data.inputs[0].channelBuffers32 = inbuf;
    data.outputs->channelBuffers32[1][127] = 0.1;
    ParamChange change;

    // test 5 octaves down
    change = ParamChange(NinaParams::AmpEnvVelSense, 1);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Vco1TuneCoarse, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc1Width, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Osc2Width, 0.5);
    manager.updateParams(1, &change);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 0.6);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Legato, 0);
    manager.updateParams(1, &change);
    change = ParamChange(NinaParams::Glide, 0.0);
    manager.updateParams(1, &change);

    manager.setLayerSmoothing(14);
    // send note on with zero vel
    Steinberg::Vst::NoteOnEvent note = {0, 10, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    for (int i = 0; i < 10; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            printf("\n%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
        }
    }
    note = {0, 20, 1.0, 1.0};
    manager.allocateVoices(Steinberg::Vst::Nina::MidiNote(note));
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 0.8);
    manager.updateParams(1, &change);
    for (int i = 0; i < 20; i++) {
        manager.processAudio(data);
        for (int i = 0; i < 1; i++) {
            printf("\n%f", buffs[12][(i * 8 + Cv0Osc1Down)]);
            // printf("\n%f", buffs[13][(i * 8 + Cv1FilterCut)]);
        }
    }
    manager.setLayerSmoothing(1);
    change = ParamChange(MAKE_MOD_MATRIX_PARAMID(NinaParams::Constant, NinaParams::FilterCutoff), 0.6);
    manager.updateParams(1, &change);
    float old_val = 0;

    for (int i = 0; i < 200; i++) {
        manager.processAudio(data);
        // check the last 2 buffers and make sure the value of the output is still settling.
        if (i > 198) {
            float val = buffs[13][(i * 8 + Cv1FilterCut)];
            // assert(abs(val - old_val) > 0.0000001);
        }
    }
    for (int i = 0; i < 36; i++) {
        delete[] buffs[i];
    }
    return true;
}

void code_test(int val) {
    using namespace Steinberg::Vst::Nina;
    float num = fastpow2(1);

    printf("\n%f", num);
    num = fastpow2(2);

    printf("\n%f", num);
    if (val > 10) {
        printf("here");
    }
    if (val > 11) {
        printf("here2");

    } else {
        printf("default");
    }
}

void tests_finish(int passes, int fails, float tt) {

    if (fails > 0) {
        printf("\n \033[1;31mFailed: %d\033[0m", fails);
    } else {
        printf("\n \033[1;32mFailed: %d\033[0m", fails);
    }
    printf("\n run tests time taken: %f ms", tt);
}

void print_res(bool res) {
    if (res) {
        print_pass();
    } else {
        print_fail();
    }
}

void run_test(bool res, int &pass, int &fail) {
    if (res) {
        pass++;
    } else {
        fail++;
    }
    print_res(res);
}