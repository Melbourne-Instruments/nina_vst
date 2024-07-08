/**
 * @file NinaLfo.h
 * @brief Declaration of a simple LFO for nina
 * @date 2022-07-17
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */
#pragma once

#include "NinaParameters.h"
#include "NoiseOscillator.h"
#include "SynthMath.h"
#include "common.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

class NinaLfo {
  public:
    enum class LfoOscShape {
        SINE,
        TRIANGLE,
        SQUARE,
        SAWTOOTHUP,
        SAWTOOTHDOWN,
        RANDOM,
        NUMSHAPES
    };

    NinaLfo(float &shape_a, float &shape_b, float &morph, float &slew, bool &voice_trigger, bool &lfo_retrigger, float &tempo, bool &sync, bool &global, float &global_phase, bool set_global_phase) :
        _shape_a_f(shape_a), _shape_b_f(shape_b), _morph(morph), _slew_setting(slew), _voice_trig(voice_trigger), _lfo_retrigger(lfo_retrigger), _tempo(tempo), _tempo_sync(sync), _global(global), _global_phase(global_phase), _set_global_phase(set_global_phase) {
        _noise.setVolume(2.0);
    };

    ~NinaLfo();
    void run();
    void setSmoothing(float smooth_operator);
    void reCalculate();

    float *getOutput() {
        return &_lfo_out;
    }

    float *getPitchIn() {
        return &_freq;
    }

    float *getGainIn() { return &_gain; }

    bool dump = false;

  private:
    /**
     * @brief lfo slew rate control is defined by Fc = 2^(15x - 2) where x = (0,1)
     *
     */
    static constexpr float lfo_shape_calc = (float)LfoOscShape::NUMSHAPES;

    bool &_voice_trig;
    bool &_lfo_retrigger;
    bool &_global;
    float &_global_phase;
    const bool _set_global_phase;
    float _gain = 1.0;
    float _phase = 0.0f;
    float _freq = 0.0;
    float &_shape_a_f, &_shape_b_f;
    LfoOscShape _shape_a = LfoOscShape::SINE;
    LfoOscShape _shape_b = LfoOscShape::SINE;
    NoiseOscillator _noise;
    static NinaParams::TempoSyncMultipliers _tempo_calc;
    bool _phase_wrapped = false;
    float &_morph;
    float &_tempo;
    bool &_tempo_sync;
    float _sine;
    float _square;
    float _tri;
    float _saw_d;
    float _saw_u;
    float _rand;
    float _lfo_out = 0.0f;
    float _gain_a = 1.0f;
    float _gain_b = 0.0f;
    float _lfo_a = 0.0f;
    float _lfo_b = 0.0f;
    float _lfo_slew = 1.0f;
    float &_slew_setting;
    float _lfo_mix = 0;
    /**
     * @brief phase increment calculation = 2^(log(2,2*pi/fs) + freq_exponential)
     * so we precalculate the log(2,2*pi/fs factor) TODO: [NINA-268] investigate making the phase factor a constexpr
     *
     */
  public:
    const float phase_factor = exp2f(2.0f * M_PIf32 / CV_SAMPLE_RATE);

    /**
     * @brief calculate the output signal for the selected LFO shape and phase
     *
     * @param lfo_shape selected shape
     * @param phase current lfo phase
     * @param wrap if the phase has just wrapped
     * @param output current output. output signal is written to this variable
     * @return float
     */
    void _selectLFO(LfoOscShape lfo_shape, float &output) {
        switch (lfo_shape) {
        case LfoOscShape::SINE:
            output = _sine;
            break;

        case LfoOscShape::TRIANGLE:
            output = _tri;
            break;

        case LfoOscShape::SAWTOOTHUP:
            output = _saw_u;
            break;
        case LfoOscShape::SAWTOOTHDOWN:
            output = _saw_d;
            break;

        case LfoOscShape::SQUARE:
            output = _square;
            break;

        case LfoOscShape::RANDOM:
            output = _rand;
            break;

        default: {
        }
        }
    }
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg