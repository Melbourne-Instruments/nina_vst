/**
 * @file NinaExpander.h
 * @brief Expander used to improve the SNR of the final output when using fx at higher gains
 * @date 2023-03-21
 *
 * Copyright (c) 2021 Melbourne Instruments
 *
 */

#pragma once
#include "NinaParameters.h"
#include "SynthMath.h"
#include "common.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

constexpr float THRESHOLD = -11;
constexpr float LIN_THRESH = std::pow(10.f, THRESHOLD / 20.f);
constexpr float RATIO = 5.0;
constexpr float MAX_GR = -4.0f;
constexpr float RMS_REL = 0.1;
constexpr float RMS_SAMPLE = RMS_REL * (float)SAMPLE_RATE;
constexpr float RMS_TIME_CONST = std::pow(37.f / 100.f, 1.f / RMS_SAMPLE);
static_assert(RMS_TIME_CONST > 0);

class NinaExpander {
    float _rms_1 = 0;
    float _rms_2 = 0;
    int counter = 0;

  public:
    NinaExpander() {
    }

    void process(float *input_1, float *input_2) {
        for (uint i = 0; i < BUFFER_SIZE; ++i) {
            float &sample_1 = input_1[i];
            float &sample_2 = input_2[i];

            // calulate the square sum of the 2 channels, then integrate with a simple LPF and square, this approximates the response of a RMS detector
            const float ave = sample_1 * sample_1 + sample_2 * sample_2;
            _rms_1 = ave + RMS_TIME_CONST * _rms_1;
            float rms = sqrtf32(_rms_1);

            // calculate a dB based gain reduction depending on the amount the RMS level is lower than the threshold
            float db_under_thresh = fastlog2(rms / LIN_THRESH);
            db_under_thresh = db_under_thresh > 0 ? 0.f : db_under_thresh;
            db_under_thresh = db_under_thresh < MAX_GR ? MAX_GR : db_under_thresh;
            const float gain_reduction = db_under_thresh * (RATIO - 1.f) / RATIO;
            float gain_linear = fastpow2(gain_reduction);

            // apply the gain reduction
            sample_1 *= gain_linear;
            sample_2 *= gain_linear;
        }
    }
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg