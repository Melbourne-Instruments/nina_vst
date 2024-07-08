

#include "NinaParameters.h"
#include "NoiseOscillator.h"
#include "SynthMath.h"
#include "common.h"

#pragma once

namespace Steinberg {
namespace Vst {
namespace Nina {

class NoiseXorMix {
  public:
    NoiseXorMix(float *&noise_xor_level, NinaParams::XorNoiseModes &mode, std::array<float, BUFFER_SIZE> *&input, std::array<float, BUFFER_SIZE> *&output, float &_drive_comp, float &ex_in_gain) :
        _xor_lev(*noise_xor_level), _mode(mode), _input(input), _output(output), _drive_mix_comp(_drive_comp), _ex_in_gain(ex_in_gain) {
        _noise_gen.setVolume(1.f);
    };

    ~NoiseXorMix(){};

    float *getXorOut() { return &_xor_lev; };

    float *getGainInput() {
        return &_gain;
    }

    void print() {
        printf("\n xor: %f %d", _gain, _mode);
        printb = true;
    }

    void reCalculate() {
        _buffer_counter = 0;
    }

    void run() {

        switch (_mode) {
        case NinaParams::XorNoiseModes::Xor: {
            // transform the assymetric vca input range of +1,-3 to the range 1,-1
            _xor_lev = (_drive_mix_comp * (1.f + _gain));
        } break;
        case NinaParams::XorNoiseModes::WhiteNoise: {
            _xor_lev = 0;
            float noise_gain = (_drive_mix_comp * (1.f + _gain));
            for (uint buff_i = 0; buff_i < AUDIO_BUFFER_FILL; ++buff_i) {
                float noise_sample = _noise_gen.getSample() - 0.5;
                (*_output)[(_buffer_counter++)] += noise_sample * noise_gain;
                /**
                if (printb) {
                    printb = false;
                    printf("\n %f %f %f %f %f", noise_sample * noise_gain, noise_gain, _drive_mix_comp);
                }**/
            }
        } break;
        case NinaParams::XorNoiseModes::PinkNoise: {
            float noise_gain = (_drive_mix_comp * (1.f + _gain));
            _xor_lev = 0;
            for (uint buff_i = 0; buff_i < AUDIO_BUFFER_FILL; ++buff_i) {
                float noise_sample = _pink_noise_gen.tick() - 0.5;
                (*_output)[(_buffer_counter++)] += noise_sample * noise_gain;
            }
        } break;

        case NinaParams::XorNoiseModes::AuxIn: {
            constexpr float dc_block_tc = 0.1;
            constexpr float dc_filter = -1.0 / (dc_block_tc * (float)SAMPLE_RATE) + 1.0;

            // printf("\n ex in gain %f", _ex_in_gain);

            _xor_lev = 0;
            const float noise_gain = (_drive_mix_comp * (1.f + _gain)) * (15 * _ex_in_gain + 1);
            for (uint buff_i = 0; buff_i < AUDIO_BUFFER_FILL; ++buff_i) {
                float sample = (*_input)[(_buffer_counter)];
                _y_t1 = sample - _x_t1 + dc_filter * _y_t1;
                _x_t1 = sample;
                (*_output)[(_buffer_counter)] += _y_t1 * noise_gain;
                _buffer_counter++;
            }
        } break;

        default:
            break;
        }
    }

  private:
    static constexpr uint AUDIO_BUFFER_FILL = 8;
    static constexpr float max_vol = 0.5f;
    float _blend = 0.0f;
    float _gain = 1.0f;
    float &_xor_lev;
    float &_drive_mix_comp;
    float &_ex_in_gain;
    float _x_t1 = 0;
    float _y_t1 = 0;
    float _b0 = 0;
    float _b1 = 0;
    float _b2 = 0;
    bool printb = false;

    std::array<float, BUFFER_SIZE> *&_output;
    std::array<float, BUFFER_SIZE> *&_input;
    NinaParams::XorNoiseModes &_mode;
    NoiseOscillator _noise_gen;
    PinkNoiseGen _pink_noise_gen;
    uint _buffer_counter = 0;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg