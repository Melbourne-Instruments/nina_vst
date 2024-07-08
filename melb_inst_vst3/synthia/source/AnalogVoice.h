/**
 * @file AnalogVoice.h
 * @brief Definition of the analog voice. it takes the input streams and calibrates them, muxes the streams into the 2 96k streams that control the voice
 * @date 2022-07-07
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */

#pragma once

#include "AnalogFiltGen.h"
#include "AnalogOscGen.h"
#include "DsVca.h"
#include "NinaLogger.h"
#include "common.h"
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>

namespace Steinberg {
namespace Vst {
namespace Nina {

struct VoiceOutput {
    std::array<float, BUFFER_SIZE> *output_1;
    std::array<float, BUFFER_SIZE> *output_2;

    VoiceOutput(std::array<float, BUFFER_SIZE> *a1, std::array<float, BUFFER_SIZE> *a2) {
        output_1 = a1;
        output_2 = a2;
    }
};

struct VoiceInput {

    std::array<float, CV_BUFFER_SIZE> osc_1_pitch;
    std::array<float, CV_BUFFER_SIZE> osc_1_shape;
    std::array<float, CV_BUFFER_SIZE> osc_2_pitch;
    std::array<float, CV_BUFFER_SIZE> osc_2_shape;
    std::array<float, CV_BUFFER_SIZE> tri_1_lev;
    std::array<float, CV_BUFFER_SIZE> sqr_1_lev;
    std::array<float, CV_BUFFER_SIZE> tri_2_lev;
    std::array<float, CV_BUFFER_SIZE> sqr_2_lev;
    std::array<float, CV_BUFFER_SIZE> xor_lev;
    std::array<float, CV_BUFFER_SIZE> filt_cut;
    std::array<float, CV_BUFFER_SIZE> filt_res;
    std::array<float, CV_BUFFER_SIZE> vca_l;
    std::array<float, CV_BUFFER_SIZE> vca_r;
    std::array<bool, CV_BUFFER_SIZE> overdrive;
    std::array<bool, CV_BUFFER_SIZE> hard_sync;
    std::array<bool, CV_BUFFER_SIZE> sub_osc;
    std::array<bool, CV_BUFFER_SIZE> mute_1;
    std::array<bool, CV_BUFFER_SIZE> mute_2;
    std::array<bool, CV_BUFFER_SIZE> mute_3;
    std::array<bool, CV_BUFFER_SIZE> mute_4;
    std::array<bool, CV_BUFFER_SIZE> mix_mute_1;
    std::array<bool, CV_BUFFER_SIZE> mix_mute_2;

    float osc_1_freq_in_n;
    float osc_1_shape_in_n;
    float osc_2_freq_in_n;
    float osc_2_shape_in_n;
    float tri_1_lev_in_n;
    float sqr_1_lev_in_n;
    float tri_2_lev_in_n;
    float sqr_2_lev_in_n;
    float xor_lev_in_n;
    float filt_cut_in_n;
    float filt_res_in_n;
    float vca_l_in_n;
    float vca_r_in_n;
    bool overdrive_in_n;
    bool hard_sync_in_n;
    bool sub_osc_in_n;
    bool mute_1_in_n;
    bool mute_2_in_n;
    bool mute_3_in_n;
    bool mute_4_in_n;
    bool mix_mute_1_in_n;
    bool mix_mute_2_in_n;
    bool filter_2_pole = false;
    bool _last_allocated = false;

    float *osc_1_freq_in = &osc_1_freq_in_n;
    float *osc_1_shape_in = &osc_1_shape_in_n;
    float *osc_2_freq_in = &osc_2_freq_in_n;
    float *osc_2_shape_in = &osc_2_shape_in_n;
    float *tri_1_lev_in = &tri_1_lev_in_n;
    float *sqr_1_lev_in = &sqr_1_lev_in_n;
    float *tri_2_lev_in = &tri_2_lev_in_n;
    float *sqr_2_lev_in = &sqr_2_lev_in_n;
    float *xor_lev_in = &xor_lev_in_n;
    float *filt_cut_in = &filt_cut_in_n;
    float *filt_res_in = &filt_res_in_n;
    float *vca_l_in = &vca_l_in_n;
    float *vca_r_in = &vca_r_in_n;
    bool *overdrive_in = &overdrive_in_n;
    bool *hard_sync_in = &hard_sync_in_n;
    bool *sub_osc_in = &sub_osc_in_n;
    bool *filter_2_pole_in = &filter_2_pole;
    bool *mute_1_in = &mute_1_in_n;
    bool *mute_2_in = &mute_2_in_n;
    bool *mute_3_in = &mute_3_in_n;
    bool *mute_4_in = &mute_4_in_n;
    bool *mix_mute_1_in = &mix_mute_1_in_n;
    bool *mix_mute_2_in = &mix_mute_2_in_n;

    /**
     * @brief run when input signals are already set. Sets the sample counter to a new value, copies the current input values to the input array at the value passed by the counter parameter.
     *
     * @param counter
     */
    void setSampleCounter(const int counter) {
        // TODO: [NINA-279] test changing setSampleCOunter function to be nocopy. where it just changes a pointer
        osc_1_pitch[counter] = osc_1_freq_in_n;
        osc_1_shape[counter] = osc_1_shape_in_n;
        osc_2_pitch[counter] = osc_2_freq_in_n;
        osc_2_shape[counter] = osc_2_shape_in_n;
        tri_1_lev[counter] = tri_1_lev_in_n;
        sqr_1_lev[counter] = sqr_1_lev_in_n;
        tri_2_lev[counter] = tri_2_lev_in_n;
        sqr_2_lev[counter] = sqr_2_lev_in_n;
        xor_lev[counter] = xor_lev_in_n;
        filt_cut[counter] = filt_cut_in_n;
        filt_res[counter] = filt_res_in_n;
        vca_l[counter] = vca_l_in_n;
        vca_r[counter] = vca_r_in_n;
        overdrive[counter] = overdrive_in_n;
        hard_sync[counter] = hard_sync_in_n;
        sub_osc[counter] = sub_osc_in_n;
        mute_1[counter] = mute_1_in_n;
        mute_2[counter] = mute_2_in_n;
        mute_3[counter] = mute_3_in_n;
        mute_4[counter] = mute_4_in_n;
        mix_mute_1[counter] = mix_mute_1_in_n;
        mix_mute_2[counter] = mix_mute_2_in_n;
    }
};

struct VoiceCalOutput {

    std::array<float, CV_BUFFER_SIZE> osc_1_up;
    std::array<float, CV_BUFFER_SIZE> osc_1_down;
    std::array<float, CV_BUFFER_SIZE> osc_2_up;
    std::array<float, CV_BUFFER_SIZE> osc_2_down;
    std::array<float, CV_BUFFER_SIZE> tri_1_lev;
    std::array<float, CV_BUFFER_SIZE> sqr_1_lev;
    std::array<float, CV_BUFFER_SIZE> tri_2_lev;
    std::array<float, CV_BUFFER_SIZE> sqr_2_lev;
    std::array<float, CV_BUFFER_SIZE> xor_lev;
    std::array<float, CV_BUFFER_SIZE> filt_cut;
    std::array<float, CV_BUFFER_SIZE> filt_res;
    std::array<float, CV_BUFFER_SIZE> vca_l;
    std::array<float, CV_BUFFER_SIZE> vca_r;
    std::array<bool, CV_BUFFER_SIZE> overdrive;
    std::array<bool, CV_BUFFER_SIZE> hard_sync;
    std::array<bool, CV_BUFFER_SIZE> sub_osc;
    std::array<bool, CV_BUFFER_SIZE> mute_1;
    std::array<bool, CV_BUFFER_SIZE> mute_2;
    std::array<bool, CV_BUFFER_SIZE> mute_3;
    std::array<bool, CV_BUFFER_SIZE> mute_4;
    std::array<bool, CV_BUFFER_SIZE> mix_mute_1;
    std::array<bool, CV_BUFFER_SIZE> mix_mute_2;
};

class AnalogVoice {

  public:
    // Constants
    AnalogVoice(){};

    AnalogVoice(uint voice_num);
    ~AnalogVoice();
    AnalogVoice(const AnalogVoice &) = default;
    bool blacklisted();

    void setMuteDisable(bool mute_dis) {
        _disable_mutes = mute_dis;
    }

    void setBlacklist(bool bl) {
        _blacklisted = bl;
    }

    void reEvaluateMutes();
    void runTuning(float osc_1_up, float osc_1_down, float osc_2_up,
        float osc_2_down);
    void runOscTuning();
    void stopOscTuning();
    void loadCalibration();
    void saveCalibration();
    VoiceInput &getVoiceInputBuffers();
    void generateVoiceBuffers(VoiceOutput &output);

    void setUnitTestMode() {
        _osc_1.setUnitTestMode();
        _osc_0.setUnitTestMode();
    }

    void setOscTuningGain(float gain) {
        _osc_1.setTuningGain(gain);
        _osc_0.setTuningGain(gain);
    }

    void printStuff() {
        _osc_0.debugPrinting();
        _tri_0.print();
        _tri_1.print();
        _sqr_0.print();
        _sqr_1.print();
        _filter.print = true;
    }

    void dump() {
        _osc_0.dump();
    }

    void setOutMuteL(bool left) {
        _main_out_mute_l = left;
    }

    void setFilterMode(bool filter_2_pole_on) {
        _input.filter_2_pole = filter_2_pole_on;
    }

    void setOutMuteR(bool right) {
        _main_out_mute_r = right;
    }

    float getFilterCutOut() {
        return _filter.getLastCutoff();
    }

    void setFilterCal(FilterCalValues filter_cal) {
        _filter.setCal(filter_cal);
    }

    float getAveOscLevel() {
        auto osc_0 = _osc_0.queryOscModel();
        auto osc_1 = _osc_1.queryOscModel();
        float sum = get<0>(osc_0) + get<0>(osc_1);
        // printf("\n osc values: %f %f %f %f", get<0>(osc_0), get<1>(osc_0), get<0>(osc_1), get<1>(osc_1));
        return sum / 2.f;
    }

    void setAllocatedToVoice(bool val) {
        _voice_allocated_to_layer = val;
    }

    void setMainLVcaOffset(float offset) {
        _left.setZeroOffset(offset);
    }

    void setMainRVcaOffset(float offset) {
        _right.setZeroOffset(offset);
    }

    void setMixVcaOffset(float tri_0, float tri_1, float sqr_0, float sqr_1, float xor_l) {
        _tri_0.setZeroOffset(tri_0);
        _tri_1.setZeroOffset(-tri_1);
        _sqr_0.setZeroOffset(sqr_0);
        _sqr_1.setZeroOffset(-sqr_1);
        _xor.setZeroOffset(xor_l);
    }

  private:
    static constexpr int bit_array = (1 << VoiceBitMap::VOICE_MUTE_L) + (1 << VoiceBitMap::VOICE_MUTE_R) + (1 << VoiceBitMap::VOICE_MUTE_3) + (1 << VoiceBitMap::VOICE_MUTE_4);
    static constexpr float disable_voice_bits = (float)((((double)bit_array) + 0.1) / (double)((2 << 22) - 1));

    const uint _voice_num = 0;
    bool _blacklisted;
    float offset = 1.0;
    std::string path = "/udata/nina/calibration/";
    float _glide = 0;
    bool _hard_sync = false;
    bool _sub_oscillator = false;
    bool _drive_boost = false;
    bool _main_out_mute_l = false;
    bool _main_out_mute_r = false;
    bool _mute_allowed = false;
    bool _voice_allocated_to_layer = false;
    bool _disable_mutes = false;

    Logger *logger;
    VoiceInput _input;
    VoiceInput _output_store;
    VoiceCalOutput _cal_output;
    AnalogOscModel _osc_0{_voice_num, 0};
    AnalogOscModel _osc_1{_voice_num, 1};
    Vca<Cv0MixOsc1Tri, false> _tri_0;
    Vca<Cv0MixOsc2Tri, true> _tri_1;
    Vca<Cv0MixOsc1Sq, false> _sqr_0;
    Vca<Cv0MixOsc2Sq, true> _sqr_1;
    Vca<Cv1MixXor, false> _xor;
    Vca<Cv1AmpL, false> _left;
    Vca<Cv1AmpR, false> _right;
    AnalogFiltCal _filter = {};

    // when voices are disabled the osc will run at appx ~500hz and with a squarish shape so they can still be tracked
    std::array<float, CV_BUFFER_SIZE> _disable_pitch;
    std::array<float, CV_BUFFER_SIZE> _disable_shape;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
