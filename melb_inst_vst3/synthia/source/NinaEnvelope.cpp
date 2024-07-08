/**
 * @file NinaEnvelope.cpp
 * @brief Implementation of the ADSR envelope
 * @date 2022-07-15
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */

#include "NinaEnvelope.h"
#include <cmath>

namespace Steinberg {
namespace Vst {
namespace Nina {

void GenAdsrEnvelope::run() {

    // each state has an exp decay or release. only att->rel transition is triggered here so its more efficient
    float old = _env_signal;
    switch (_env_state) {
    case AdsrState::ATT: {
        _env_signal += _att_coeff * (att_asymtote - _env_signal);
        if (_env_signal > att_trigger) {
            _env_signal = att_trigger;
            _env_state = AdsrState::DEC;
        }
        _output = _env_signal;
        break;
    }
    case AdsrState::DEC: {
        _env_signal += _dec_coeff * (-_env_signal);
        _output = _env_signal + (1 - _env_signal) * _sus_level;
        break;
    }
    case AdsrState::REL: {
        _env_signal += _rel_coeff * (idle_level - _env_signal);
        _output = _env_signal;
        break;
    }
    default:
        break;
    }
    if (_drone) {
        _output = _sus_input;
    }
    _output = _output * _output_gain;
}

void GenAdsrEnvelope::reCalculate() {
    // we remove these state transitions out of the run function so that they arn't called for every sample since it seems to have little impact on sound
    switch (_env_state) {
    case AdsrState::ATT: {
        if (!_gate_on) {
            _env_state = AdsrState::REL;
        }
        break;
    }
    case AdsrState::DEC: {
        if (!_gate_on) {
            _env_state = AdsrState::REL;
            _env_signal = _env_signal + (1 - _env_signal) * _sus_level;
        }
        if (_trigger) {
            _env_state = AdsrState::ATT;
            if (_reset) {
                _env_signal = 0;
                // set the env signal to current output level to avoid clicks
            } else {
                _env_signal = _env_signal + (1 - _env_signal) * _sus_level;
            }
        }
        break;
    }
    case AdsrState::REL: {
        if (_gate_on) {
            _env_state = AdsrState::ATT;
        }
        if (_trigger) {
            _env_state = AdsrState::ATT;
            if (_reset) {
                _env_signal = 0;
            }
        }
        break;
    }
    }
    if (_trigger) {
        _trigger = false;
    }

    // clip each input signal so we dont have any numerical issues
    _att_input = _att_input < env_control_clip_min ? env_control_clip_min : _att_input;
    _dec_input = _dec_input < env_control_clip_min ? env_control_clip_min : _dec_input;
    _sus_input = _sus_input > env_control_clip_max ? env_control_clip_max : _sus_input;
    _rel_input = _rel_input < env_control_clip_min ? env_control_clip_min : _rel_input;

    // transform the input coefficients to give exponential control over the time periods of the adsr envelope
    // TODO: optimise env coeff generation to use constants for the divider
    const float at3 = _att_input * _att_input * _att_input;
    const float dec3 = _dec_input * _dec_input * _dec_input;
    const float rel3 = _rel_input * _rel_input * _rel_input;
    _att_coeff = (1 - fastpow2(-(1.4 / ((float)CV_SAMPLE_RATE * 5. * at3))));
    _dec_coeff = (1 - fastpow2(-(1.4 / ((float)CV_SAMPLE_RATE * 5. * dec3))));
    constexpr float b = 6.7;
    constexpr float a = 10;
    constexpr float c = 10;
    if (print) {
        printf("\nenv: %f %f %f   %f %f %f", _att_input, _dec_input, _rel_input, _att_coeff, _dec_coeff, _rel_coeff);
    }

    //_dec_coeff = (fastpow2(b * (0.6 - _dec_input) + 10) + a * 100 * _dec_input + a * c) / (fastpow2(4 * b) * _dec_input * _dec_input);
    _rel_coeff = (1 - fastpow2(-(1.4 / (CV_SAMPLE_RATE * 5. * rel3))));
    _sus_level = _sus_input * _sus_input;

    // calculate velocity gain which is a blend of a static gain and velocity
    float vel_gain;
    if (_velocity_sense > 0.f) {
        vel_gain = 2 * (1 - _velocity_sense) + _velocity_sense * _velocity;
    } else {
        vel_gain = 2 * (1 + _velocity_sense) + _velocity_sense * (_velocity - 2);
    }

    if (print) {
        printf("\n vel sense %f %f ", _velocity_sense, vel_gain);
        print = false;
    }

    // final gain
    _output_gain = _gain * vel_gain;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
