/**
 * @file AnalogVoice.cpp
 * @brief
 *
 * @copyright Copyright (c) 2022-2023 Melbourne Instruments, Australia
 */
#include "AnalogVoice.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

// Constants

AnalogVoice::AnalogVoice(uint voice_num) :
    _voice_num(voice_num) {
    loadCalibration();
}

AnalogVoice::~AnalogVoice(){};

bool AnalogVoice::blacklisted() { return _blacklisted; };

VoiceInput &AnalogVoice::getVoiceInputBuffers() {
    return _input;
}

int buffcount = 0;

void AnalogVoice::runTuning(float osc_1_up, float osc_1_down, float osc_2_up, float osc_2_down) {
    _filter.setTemp(getAveOscLevel());
    _osc_0.tuningFeedback(osc_1_up, osc_1_down);
    _osc_1.tuningFeedback(osc_2_up, osc_2_down);
}

void AnalogVoice::generateVoiceBuffers(VoiceOutput &output) {
    const auto &input = _input;
    if (!_blacklisted && _voice_allocated_to_layer) {

        // osc can only be in sync mode when its running normally
        _osc_1.OscSynced((input.hard_sync[0] && _osc_1.isNormal()));
        _filter.run(input.filt_cut, input.filt_res, *output.output_2);
        _tri_0.runVca<false>(input.tri_1_lev, *output.output_1);
        _tri_1.runVca<false>(input.tri_2_lev, *output.output_1);
        _sqr_0.runVca<false>(input.sqr_1_lev, *output.output_1);
        _sqr_1.runVca<false>(input.sqr_2_lev, *output.output_1);
        const float left_sum = _left.runVca<true>(input.vca_l, *output.output_2);
        const float right_sum = _right.runVca<true>(input.vca_r, *output.output_2);
        _xor.runVca<false>(input.xor_lev, *output.output_2);
        _osc_0.run(input.osc_1_pitch, input.osc_1_shape, *output.output_1);
        _osc_1.run(input.osc_2_pitch, input.osc_2_shape, *output.output_1);
        // generate the bit array for the switched signals. also see FPGA buffer config

        bool mute_l = false;
        bool mute_r = false;
        _mute_allowed = (_mute_allowed || _input._last_allocated) && (!_disable_mutes);

        // if the LR level of the voice is below the thresh for the whole buffer, then mute the output
        mute_l = (std::abs(left_sum) + std::abs(right_sum) < 0.0001) && _mute_allowed;
        mute_r = (std::abs(left_sum) + std::abs(right_sum) < 0.0001) && _mute_allowed;
        _mute_allowed = (mute_l) && (mute_r);

        for (int i = 0; i < CV_BUFFER_SIZE; i++) {
            int bit_array =
                ((int)(input.mute_1[i] || mute_l) << VoiceBitMap::VOICE_MUTE_L) +
                ((int)(input.mute_2[i] || mute_r) << VoiceBitMap::VOICE_MUTE_R) +
                ((int)(input.mute_3[i] || mute_l) << VoiceBitMap::VOICE_MUTE_3) +
                ((int)(input.mute_4[i] || mute_r) << VoiceBitMap::VOICE_MUTE_4) +
                ((int)input.hard_sync[i] << VoiceBitMap::HARD_SYNC) +
                ((int)(!input.sub_osc[i]) << VoiceBitMap::SUB_OSC_EN_N) +
                ((int)input.overdrive[i] << VoiceBitMap::DRIVE_EN_N) +
                ((int)false << VoiceBitMap::FILTER_TYPE) +
                ((int)_main_out_mute_l << VoiceBitMap::MIX_MUTE_L) +
                ((int)_main_out_mute_r << VoiceBitMap::MIX_MUTE_R);
            const auto float_bit_array = (float)((((double)bit_array) + 0.1) / (double)((2 << 22) - 1));
            (*(output.output_2))[i * CV_MUX_INC + Cv1BitArray] = float_bit_array;
            // if the osc are not both in normal mode yet, then disable the voice outputs
            if ((!_osc_0.isNormal()) || (!_osc_1.isNormal()))

            {
                (*(output.output_2))[i * CV_MUX_INC + Cv1BitArray] = disable_voice_bits;
            }
        }

    } else {

        // when voice is disabled, mute the output, but still run the oscillator at a lowish freq and 50% shape
        _disable_pitch.fill(1.2);
        _disable_shape.fill(0);
        _osc_0.run(_disable_pitch, _disable_shape, *output.output_1);
        _osc_1.run(_disable_pitch, _disable_shape, *output.output_1);
        for (int i = 0; i < CV_BUFFER_SIZE; i++) {

            (*(output.output_2))[i * CV_MUX_INC + Cv1BitArray] = disable_voice_bits;
        }
    }
}

void AnalogVoice::runOscTuning() {
    _osc_0.runTuning();
    _osc_1.runTuning();
};

void AnalogVoice::stopOscTuning() {
    _osc_0.stopTuning();
    _osc_1.stopTuning();
};

void AnalogVoice::loadCalibration() {
    std::fstream file_handler;
    std::stringstream fpath;
    fpath << path << "voice_" << _voice_num << ".cal";

    file_handler.open(fpath.str(), std::ios::in);
    if (file_handler.is_open()) {
        std::string line;
        std::getline(file_handler, line);
        std::istringstream iss(line);
        float vca_cal_1, vca_cal_2, vca_cal_3, vca_cal_4, vca_cal_5, main_vca_cal_1, main_vca_cal_2, blacklist;

        FilterCalValues f;
        if (!(iss >> vca_cal_1 >> vca_cal_2 >> vca_cal_3 >> vca_cal_4 >>
                vca_cal_5 >> main_vca_cal_1 >> main_vca_cal_2 >>
                f.fc_low_clip >> f.fc_high_clip >> f.fc_gain >>
                f.fc_offset >> f.fc_temp_track >> f.res_low_clip >>
                f.res_high_clip >> f.res_gain >> f.res_zero_offset >> blacklist)) {
            printf("\nfailed voice cal load %d ", _voice_num);
        }

        _tri_0.setZeroOffset(vca_cal_1);
        _sqr_0.setZeroOffset(vca_cal_2);
        _tri_1.setZeroOffset(vca_cal_3);
        _sqr_1.setZeroOffset(vca_cal_4);
        _xor.setZeroOffset(vca_cal_5);
        _left.setZeroOffset(main_vca_cal_1);
        _right.setZeroOffset(main_vca_cal_2);
        file_handler.close();

        std::stringstream fpath_filter;
        fpath_filter << path << "voice_" << _voice_num << "_filter.model";
        file_handler.open(fpath_filter.str(), std::ios::in);
        if (file_handler.is_open()) {
            std::getline(file_handler, line);
            std::istringstream iss2(line);
            if (!(iss2 >> f.a >> f.c >> f.base_temp)) {
                printf("\nfailed filter cal load %d ", _voice_num);
            }
        }

        _filter.setCal(f);
        if (blacklist > 0.5) {
            _blacklisted = true;
            _osc_0.osc_disable = true;
            _osc_1.osc_disable = true;
            printf("\n\n BLACKLIST VOICE %d", _voice_num);
        }
        printf("\nload %d success", _voice_num);
    } else {
        printf("\nIO error\n");
    }
    _osc_0.loadCalibration();
    _osc_1.loadCalibration();
};

void AnalogVoice::reEvaluateMutes() {
    _mute_allowed = true;
}

void AnalogVoice::saveCalibration() {
    std::fstream file_handler;
    std::stringstream fpath;
    fpath << path << "voice_" << _voice_num << ".cal";
    file_handler.open(fpath.str(), std::ios::out | std::ios::trunc);
    if (file_handler.is_open()) {
        float vca_cal_1, vca_cal_2, vca_cal_3, vca_cal_4, vca_cal_5, main_vca_cal_1,
            main_vca_cal_2, filter_offset, filter_scale, filter_t_coff;
        vca_cal_1 = _tri_0.getZeroOffset();
        vca_cal_2 = _sqr_0.getZeroOffset();
        vca_cal_3 = _tri_1.getZeroOffset();
        vca_cal_4 = _sqr_1.getZeroOffset();
        vca_cal_5 = _xor.getZeroOffset();
        main_vca_cal_1 = _left.getZeroOffset();
        main_vca_cal_2 = _right.getZeroOffset();
        float blacklist = (float)((int)_blacklisted);
        FilterCalValues f = _filter.getCal();

        file_handler << " " << vca_cal_1 << " " << vca_cal_2 << " " << vca_cal_3 << " " << vca_cal_4 << " " << vca_cal_5 << " " << main_vca_cal_1 << " " << main_vca_cal_2 << " " << f.fc_low_clip << " " << f.fc_high_clip << " " << f.fc_gain << " " << f.fc_offset << " " << f.fc_temp_track << " " << f.res_low_clip << " " << f.res_high_clip << " " << f.res_gain << " " << f.res_zero_offset << " " << blacklist;
        file_handler.close();
        printf("\nsaved cal to file");
    } else {
        printf("\nfileIOerror");
    }
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
