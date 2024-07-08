/**
 * @file NinaEnvelope.h
 * @brief Definition of the ADSR envelope.
 * @date 2022-07-15
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */
#pragma once

#include "SynthMath.h"
#include "common.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

enum class AdsrState { ATT,
    DEC,
    REL
};

class GenAdsrEnvelope {
  public:
    GenAdsrEnvelope(float &velocity, float &vel_sense, float &scale, bool &reset, bool &drone) :
        _velocity(velocity),
        _velocity_sense(vel_sense),
        misc_scale(scale),
        _reset(reset),
        _drone(drone) {}

    ~GenAdsrEnvelope() {}

    /**
     * @brief set if the envelope should be looping or not TODO: [NINA-267] implement envelope looping
     *
     * @param should_loop enable for loop on
     */
    void setloop(bool should_loop);

    /**
     * @brief retrigger the ADSR
     *
     */
    void trigger() {
        _trigger = true;
    }

    /**
     * @brief force the envelope to zero
     *
     */
    void forceReset() {
        _output = 0;
        _env_signal = 0;
    }

    /**
     * @brief set the gate input on
     *
     */
    void gateOn() {
        _gate_on = true;
    }

    /**
     * @brief set the gate input off
     *
     */
    void gateOff() {
        _gate_on = false;
    }

    /**
     * @brief generate 1 sample of the env output. run at the CV rate
     *
     */
    void run();

    /**
     * @brief restarts envelope, recalculates the adsr coeffs etc. run at the slow (buffer) rate
     *
     */
    void reCalculate();

    bool inReleaseState() {
        return _env_state == AdsrState::REL;
    }

    float *getOutput() { return &_output; }

    float *getAttIn() { return &_att_input; }

    float *getDecIn() { return &_dec_input; }

    float *getSusIn() { return &_sus_input; }

    float *getRelIn() { return &_rel_input; }

    float *getGainIn() { return &_gain; }

    bool dump = false;
    bool print = false;

  private:
    static constexpr float env_control_clip_min = 0.f;
    static constexpr float env_control_clip_max = 1.f;
    static constexpr float att_trigger = 1.0f;
    static constexpr float att_asymtote = att_trigger / (1 - 1 / M_Ef32);
    static constexpr float idle_level = 0.f;
    static constexpr int time_const_translate = 19;
    const float time_const_offset = (exp2f32(time_const_mult * 0 - time_const_translate));
    float time_const_mult = 14;
    float &misc_scale;

    /**
     * @brief Coeff for env 6db rolloff filter.
     *
     */
    static constexpr float ENV_SMOOTH = 650 / (float)CV_SAMPLE_RATE;

    AdsrState _env_state = AdsrState::REL;
    float _smoothing = 1;
    float _env_signal = 0;
    float _att_coeff = 1;
    float _dec_coeff = 1;
    float _sus_level = 0.5;
    float _rel_coeff = 1;
    bool _gate_on = false;
    bool _trigger = false;
    float _output = 0;
    float _gain = 1.0;
    float _output_gain = 0;
    float &_velocity;
    float &_velocity_sense;
    bool &_reset;
    bool &_drone;

    float _att_input = 0;
    float _dec_input = 0;
    float _sus_input = 0;
    float _rel_input = 0;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
