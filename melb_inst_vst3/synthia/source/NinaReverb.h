
#pragma once
#include "SynthMath.h"
#include "common.h"
#include "freeverb/earlyref.hpp"
#include "freeverb/nrev.hpp"
#include "freeverb/nrevb.hpp"
#include "freeverb/progenitor2.hpp"
#include "freeverb/strev.hpp"
#include "freeverb/zrev2.hpp"

namespace Steinberg {
namespace Vst {
namespace Nina {

/**
 * @brief this is a ported version of the octave pitch shifter from REAPER, its used for the shimmer effect
 *
 */
class JsPitchAnOctaveUp {
    // This effect Copyright (C) 2004 and later Cockos Incorporated
    // License: LGPL - http://www.gnu.org/licenses/lgpl.html
  public:
    JsPitchAnOctaveUp(std::array<float, BUFFER_SIZE> &output_l, std::array<float, BUFFER_SIZE> &output_r) :
        _output_l(output_l), _output_r(output_r) {}

    ~JsPitchAnOctaveUp() {}

    void reset() {
        _rec_pos = 0;
        _play_pos = hbsz;
        _rec_buffer = &_buffer_1;
        _play_buffer = &_buffer_2;
        _buffer_1.fill(0.f);
        _buffer_2.fill(0.f);
    }

    void run(float left[BUFFER_SIZE], float right[BUFFER_SIZE]) {
        for (uint buf_i = 0; buf_i < BUFFER_SIZE; buf_i++) {

            (*_rec_buffer)[_rec_pos * 2] = left[buf_i];
            (*_rec_buffer)[_rec_pos * 2 + 1] = right[buf_i];

            if (_play_pos < _o_start) {
                _output_l[buf_i] = (*_play_buffer)[_play_pos * 4];
                _output_r[buf_i] = (*_play_buffer)[_play_pos * 4 + 1];

            } else if (_play_pos < hbsz) {
                float sc = (float)(_play_pos - _o_start) * _fade_sc;
                _output_l[buf_i] = (*_play_buffer)[_play_pos * 4] * (1 - sc) + (*_play_buffer)[(_play_pos - qbsz) * 4] * sc;
                _output_r[buf_i] = (*_play_buffer)[_play_pos * 4 + 1] * (1 - sc) + (*_play_buffer)[(_play_pos - qbsz) * 4 + 1] * sc;
            } else if (_play_pos < _o_end) {
                float sc = (float)(_play_pos - hbsz) * _fade_sc;
                _output_l[buf_i] = (*_play_buffer)[(_play_pos - qbsz) * 4] * (1 - sc) + (*_play_buffer)[(_play_pos - hbsz) * 4] * sc;
                _output_r[buf_i] = (*_play_buffer)[(_play_pos - qbsz) * 4 + 1] * (1 - sc) + (*_play_buffer)[(_play_pos - hbsz) * 4 + 1] * sc;
            } else {
                _output_l[buf_i] = (*_play_buffer)[(_play_pos - hbsz) * 4];
                _output_r[buf_i] = (*_play_buffer)[(_play_pos - hbsz) * 4 + 1];
            }
            _rec_pos++;
            if (_rec_pos > BUFLENGTH) {
                _rec_pos = 0;
                if (_rec_buffer == &_buffer_1) {
                    _rec_buffer = &_buffer_2;
                } else {
                    _rec_buffer = &_buffer_1;
                }
            }
            _play_pos++;
            if (_play_pos > BUFLENGTH) {
                _play_pos = 0;
                if (_play_buffer == &_buffer_1) {
                    _play_buffer = &_buffer_2;
                } else {
                    _play_buffer = &_buffer_1;
                }
            }
        }
    }

  private:
    static constexpr uint BUFLENGTH = 162 * 0.001 * SAMPLE_RATE;
    static constexpr uint hbsz = BUFLENGTH * 0.5;
    static constexpr uint qbsz = BUFLENGTH * 0.25;
    static constexpr float OVERLAP = 0.18;
    uint _rec_pos = 0;
    uint _play_pos = hbsz;
    static constexpr uint _o_start = (int)qbsz * (2 - OVERLAP);
    static constexpr uint _o_end = (int)qbsz * (2 + OVERLAP);
    static constexpr float _fade_sc = 1. / (float)(hbsz - _o_start);

    std::array<float, BUFLENGTH * 4 + 1> _buffer_1;
    std::array<float, BUFLENGTH * 4 + 1> _buffer_2;

    std::array<float, BUFLENGTH * 4 + 1> *_rec_buffer = &_buffer_1;
    std::array<float, BUFLENGTH * 4 + 1> *_play_buffer = &_buffer_2;
    std::array<float, BUFFER_SIZE> &_output_l;
    std::array<float, BUFFER_SIZE> &_output_r;
};

enum ReverbModes {
    hall,
    plate,
    room,
    shimmer
};

enum PlateAlgos {
    ALGORITHM_NREV = 0,
    ALGORITHM_NREV_B,
    ALGORITHM_STREV,
    ALGORITHM_COUNT
};

enum Parameters {
    paramDry = 0,
    paramEarly,
    paramEarlySend,

    paramLate,
    paramInternalAlgo,
    paramWet,
    paramSize,

    paramWidth,
    paramPredelay,
    paramDecay,
    paramDiffuse,
    paramSpin,
    paramWander,
    paramInHighCut,
    paramHighMult,
    paramHighXover,
    paramInLowCut,
    paramLowXover,
    paramLowMult,
    paramEarlyDamp,
    paramLateDamp,
    paramBoost,
    paramBoostLPF,
    paramTone,
    paramModulation,
    paramCount
};

class NRev : public fv3::nrev_f {
  public:
    NRev();
    void setDampLpf(float value);
    virtual void mute();
    virtual void setFsFactors();
    virtual void processloop2(long count, float *inputL, float *inputR, float *outputL, float *outputR);

  private:
    float dampLpf = 20000;
    fv3::iir_1st_f dampLpfL, dampLpfR;
};

class NRevB : public fv3::nrevb_f {
  public:
    NRevB();
    void setDampLpf(float value);
    virtual void mute();
    virtual void setFsFactors();
    virtual void processloop2(long count, float *inputL, float *inputR, float *outputL, float *outputR);

  private:
    float dampLpf = 20000;
    fv3::iir_1st_f dampLpfL, dampLpfR;
};

template <typename T>
static inline bool d_isNotEqual(const T &v1, const T &v2) {
    return std::abs(v1 - v2) >= std::numeric_limits<T>::epsilon();
}

class NinaReverb {
  private:
    float _tone_setting = 0.5;
    float _early_setting = 0.5;
    std::array<float, BUFFER_SIZE> _pitch_shift_left;
    std::array<float, BUFFER_SIZE> _pitch_shift_right;
    fv3::iir_1st_f input_lpf_0, input_lpf_1, input_hpf_0, input_hpf_1;
    JsPitchAnOctaveUp _pitch_shift = JsPitchAnOctaveUp(_pitch_shift_left, _pitch_shift_right);
    fv3::earlyref_f early;
    fv3::progenitor2_f late;
    fv3::revbase_f *model; // points to one of the following:
    NRev nrev;
    NRevB nrevb;
    fv3::strev_f strev;
    fv3::earlyref_f early_hall;
    fv3::zrev2_f late_hall;
    int bufc = 0;
    float _wet_dry_mix = 0.f;
    float _mix_smooth = 0;

    float filtered_input_buffer[2][BUFFER_SIZE];
    float early_out_buffer[2][BUFFER_SIZE];
    float late_in_buffer[2][BUFFER_SIZE];
    float late_out_buffer[2][BUFFER_SIZE];
    static constexpr float sampleRate = SAMPLE_RATE;
    ReverbModes _mode2;
    uint _mode;
    float dry_level = 0.0;
    float early_level = 0.0;
    float early_send = 0.0;
    float late_level = 0.0;
    std::array<float, paramCount> oldParams;
    std::array<float, paramCount> newParams;
    std::array<bool, paramCount> _invalidated_params;
    float in_gain = 0;
    float _shimmer_gain = 0;
    uint _selected_preset = 10000;
    float _ms_eq_filter_state = 0;
    float outputs[2][BUFFER_SIZE];

  public:
    bool _hard_mute = false;
    NinaReverb(/* args */);
    ~NinaReverb();

    void reset() {
        /**
        strev.mute();
        late_hall.mute();
        late.mute();
        early.mute();
        early_hall.mute();
        **/

        _pitch_shift.reset();
    };

    void run(float **in, float **out, uint frames);

    void setInputLPF(float freq) {
        if (freq < 0) {
            freq = 0;
        } else if (freq > sampleRate / 2.0) {
            freq = sampleRate / 2.0;
        }

        input_lpf_0.setLPF_BW(freq, sampleRate);
        input_lpf_1.setLPF_BW(freq, sampleRate);
    }

    void setInputHPF(float freq) {
        if (freq < 0) {
            freq = 0;
        } else if (freq > sampleRate / 2.0) {
            freq = sampleRate / 2.0;
        }

        input_hpf_0.setHPF_BW(freq, sampleRate);
        input_hpf_1.setHPF_BW(freq, sampleRate);
    }

    void setInGain(float val) {
        in_gain = val;
    }

    void setShimmer(float val) {
        _shimmer_gain = val / 4.f;
    }

    void setwetDry(float val) {
        _wet_dry_mix = val;
    }

    void setPreDelay(float val) {
        newParams[Parameters::paramPredelay] = val * 600;
        _invalidated_params[Parameters::paramPredelay] = true;
    }

    void setTone(float val) {
        _tone_setting = val;
        _invalidated_params[Parameters::paramHighMult] = true;
        _invalidated_params[Parameters::paramLowMult] = true;
        _invalidated_params[Parameters::paramEarlyDamp] = true;
        _invalidated_params[Parameters::paramLateDamp] = true;
    }

    void setDistance(float val) {
        _early_setting = val;
        _invalidated_params[Parameters::paramEarly] = true;
        ;
    }

    void setDecay(float val) {
        newParams[Parameters::paramDecay] = 0.2 + 10 * (val * val);
        _invalidated_params[Parameters::paramDecay] = true;
    }

    void setPreset(float preset) {
        constexpr uint num_presets = 4;
        uint selection = (int)(preset * (float)num_presets);
        if (selection == _selected_preset) {
            return;
        }
        _selected_preset = selection;
        _hard_mute = true;

        // invalidate all params so theey are recalculated, execpt a few that we want to be left as is
        _invalidated_params.fill(true);
        _invalidated_params[paramPredelay] = false;

        // mute all the reverb processors to avoid noises on changes

        switch (selection) {
        case 0:
            /* code */

            _mode = room;
            _mode2 = room;
            early.mute();
            late.mute();
            newParams[Parameters::paramHighXover] = 4000;
            newParams[Parameters::paramHighMult] = .5;
            newParams[Parameters::paramLowMult] = 1;
            newParams[Parameters::paramLowXover] = 800;
            newParams[Parameters::paramBoost] = 50;
            newParams[Parameters::paramBoostLPF] = 50;
            newParams[Parameters::paramDiffuse] = 100;
            newParams[Parameters::paramEarly] = 50;
            newParams[Parameters::paramEarlyDamp] = 8990;
            newParams[Parameters::paramEarlySend] = 100;
            newParams[Parameters::paramInHighCut] = 10000;
            newParams[Parameters::paramInLowCut] = 150;
            newParams[Parameters::paramLate] = 100;
            newParams[Parameters::paramLateDamp] = 8000;
            newParams[Parameters::paramSize] = 10;
            newParams[Parameters::paramSpin] = .4;
            newParams[Parameters::paramWander] = 100;
            newParams[Parameters::paramWidth] = 80;
            newParams[Parameters::paramModulation] = 1;

            break;

        case 1:
            _mode = plate;
            _mode2 = plate;
            strev.mute();
            newParams[Parameters::paramBoost] = 50;
            newParams[Parameters::paramBoostLPF] = 50;
            newParams[Parameters::paramDiffuse] = 90;
            newParams[Parameters::paramEarly] = 50;
            newParams[Parameters::paramEarlyDamp] = 0;
            newParams[Parameters::paramEarlySend] = 40;
            newParams[Parameters::paramInHighCut] = 4000;
            newParams[Parameters::paramInLowCut] = 50;
            newParams[Parameters::paramLate] = 100;
            newParams[Parameters::paramLateDamp] = 0;
            newParams[Parameters::paramSize] = 50;
            newParams[Parameters::paramWidth] = 100;
            newParams[Parameters::paramHighXover] = 4000;
            newParams[Parameters::paramHighMult] = .5;
            newParams[Parameters::paramLowXover] = 800;
            newParams[Parameters::paramBoost] = 50;
            newParams[Parameters::paramBoostLPF] = 50;
            newParams[Parameters::paramDiffuse] = 100;
            newParams[Parameters::paramEarlyDamp] = 1990;
            newParams[Parameters::paramEarlySend] = 100;
            newParams[Parameters::paramInHighCut] = 4000;
            newParams[Parameters::paramInLowCut] = 150;
            newParams[Parameters::paramLate] = 100;
            newParams[Parameters::paramLateDamp] = 0;
            newParams[Parameters::paramSize] = 10;
            newParams[Parameters::paramSpin] = .2;
            newParams[Parameters::paramWander] = 150;
            newParams[Parameters::paramWidth] = 80;
            newParams[Parameters::paramInternalAlgo] = 2;
            break;

        case 2:
            strev.mute();
            _mode = plate;
            _mode2 = plate;
            for (uint i = 0; i < BUFFER_SIZE; i++) {
                filtered_input_buffer[0][i] = 0.f;
                filtered_input_buffer[1][i] = 0.f;
            }
            newParams[Parameters::paramBoost] = 50;
            newParams[Parameters::paramBoostLPF] = 50;
            newParams[Parameters::paramDiffuse] = 90;
            newParams[Parameters::paramEarly] = 50;
            newParams[Parameters::paramEarlyDamp] = 0;
            newParams[Parameters::paramEarlySend] = 40;
            newParams[Parameters::paramInHighCut] = 4000;
            newParams[Parameters::paramInLowCut] = 50;
            newParams[Parameters::paramLate] = 100;
            newParams[Parameters::paramLateDamp] = 0;
            newParams[Parameters::paramSize] = 50;
            newParams[Parameters::paramWidth] = 100;
            newParams[Parameters::paramHighXover] = 4000;
            newParams[Parameters::paramHighMult] = .5;
            newParams[Parameters::paramLowXover] = 800;
            newParams[Parameters::paramBoost] = 50;
            newParams[Parameters::paramBoostLPF] = 50;
            newParams[Parameters::paramDiffuse] = 100;
            newParams[Parameters::paramEarlyDamp] = 1990;
            newParams[Parameters::paramInHighCut] = 4000;
            newParams[Parameters::paramInLowCut] = 150;
            newParams[Parameters::paramLate] = 100;
            newParams[Parameters::paramLateDamp] = 0;
            newParams[Parameters::paramSize] = 10;
            newParams[Parameters::paramSpin] = 0.00;
            newParams[Parameters::paramWander] = 0.00;
            newParams[Parameters::paramWidth] = 80;
            newParams[Parameters::paramInternalAlgo] = 2;
            break;

        case 3:
            _mode = hall;
            _mode2 = hall;
            early_hall.mute();
            late_hall.mute();

            newParams[Parameters::paramBoost] = 50;
            newParams[Parameters::paramBoostLPF] = 50;
            newParams[Parameters::paramDiffuse] = 90;
            newParams[Parameters::paramEarly] = 50;
            newParams[Parameters::paramEarlyDamp] = 0;
            newParams[Parameters::paramEarlySend] = 40;
            newParams[Parameters::paramInHighCut] = 4000;
            newParams[Parameters::paramInLowCut] = 20;
            newParams[Parameters::paramLate] = 100;
            newParams[Parameters::paramLateDamp] = 0;
            newParams[Parameters::paramSize] = 50;
            newParams[Parameters::paramSpin] = .1;
            newParams[Parameters::paramWander] = 10;
            newParams[Parameters::paramWidth] = 100;
            newParams[Parameters::paramHighXover] = 4000;
            newParams[Parameters::paramHighMult] = .5;
            newParams[Parameters::paramLowXover] = 800;
            newParams[Parameters::paramLowMult] = 2.5;
            newParams[Parameters::paramModulation] = 0.1;
            break;

        case 4:
            _mode = hall;
            _mode2 = hall;
            early_hall.mute();
            late_hall.mute();

            newParams[Parameters::paramBoost] = 50;
            newParams[Parameters::paramBoostLPF] = 50;
            newParams[Parameters::paramDiffuse] = 90;
            newParams[Parameters::paramEarly] = 50;
            newParams[Parameters::paramEarlyDamp] = 0;
            newParams[Parameters::paramEarlySend] = 40;
            newParams[Parameters::paramInHighCut] = 4000;
            newParams[Parameters::paramInLowCut] = 20;
            newParams[Parameters::paramLate] = 100;
            newParams[Parameters::paramLateDamp] = 0;
            newParams[Parameters::paramSize] = 50;
            newParams[Parameters::paramSpin] = .4;
            newParams[Parameters::paramWander] = 50;
            newParams[Parameters::paramWidth] = 100;
            newParams[Parameters::paramHighXover] = 4000;
            newParams[Parameters::paramHighMult] = .5;
            newParams[Parameters::paramLowXover] = 800;
            newParams[Parameters::paramLowMult] = 2.5;
            newParams[Parameters::paramModulation] = 0.5;

            break;
        default:
            break;
        }
    }
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg