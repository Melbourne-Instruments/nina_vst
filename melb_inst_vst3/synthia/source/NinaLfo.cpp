/**
 * @file NinaLfo.cpp
 * @brief Implementation of our simple LFO module.
 * @date 2022-07-17
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */
#include "NinaLfo.h"
#include <algorithm>
#include <cmath>

namespace Steinberg {
namespace Vst {
namespace Nina {

NinaLfo::~NinaLfo(){};

void NinaLfo::reCalculate() {

    // we have to stop the phase value overflowing. but it can go over 1.0. so we only do this here in recalculate, rather than in the run func
    _gain_a = 1 - _morph;
    _gain_b = _morph;
    if (_lfo_retrigger) {
        _phase = _voice_trig ? 0.f : _phase;
    }

    // set the LFO shape;

    _shape_a = (LfoOscShape)(int)(lfo_shape_calc * _shape_a_f + 0.5);
    _shape_b = (LfoOscShape)(int)(lfo_shape_calc * _shape_b_f + 0.5);
}

void NinaLfo::run() {

    // Clip the LFO rate so it can't run backwards or go excessively fast
    _freq = fmin(fmax(_freq, -0.0001f), 5.f);
    float phase_inc;
    float local_phase;

    if (_tempo_sync) {

        int time_multiplier_set = roundf32((_tempo_calc.num_sync_tempos) * (_freq));
        float time_multiplier = _tempo_calc.getTempoSyncMultiplier(time_multiplier_set);
        float time = (_tempo * time_multiplier);
        float lfo_rate = time;
        phase_inc = 2.0f * M_PI * lfo_rate / (float)CV_SAMPLE_RATE;
    } else {
        phase_inc = (2 * M_PI * (0.03 + 30. * _freq * _freq)) / (float)CV_SAMPLE_RATE;
    }

    _phase += phase_inc;
    _phase = _phase > 2 * M_PIf32 ? _phase - 2.f * M_PIf32 : _phase;
    local_phase = _phase;
    if (_set_global_phase) {
        _global_phase = local_phase;
    }
    if (_global) {
        local_phase = _global_phase;
    }

    _sine = sinf32(local_phase);
    float square = _sine > 0 ? 1.f : -1.f;
    bool rand_step = (square > 0.f) != (_square > 0.f);
    _square = square;
    _saw_u = (local_phase / M_PIf32) - 1.f;
    _saw_d = 1.0 - (local_phase / M_PIf32);
    _tri = _saw_u > 0 ? 2.f * (.5f - _saw_u) : 2.f * (_saw_u + .5f);
    _rand = rand_step ? _noise.getSample() - 1.f : _rand;

    // Calculate the LFO value at the current phase
    _selectLFO(_shape_a, _lfo_a);
    _selectLFO(_shape_b, _lfo_b);

    float lfo = (_gain_a * _lfo_a + _gain_b * _lfo_b);

    // we apply a first order filter to the output for the lfo slew control.
    _lfo_mix += ((lfo - _lfo_mix) * _slew_setting);
    _lfo_out = cv_clip(_lfo_mix * _gain);
}
} // namespace Nina
} // namespace Vst
} // namespace Steinberg
