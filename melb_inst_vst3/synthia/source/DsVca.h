/**
 * @file DsVca.h
 * @brief Declaration of the VCA calibration class. it adjusts the offset of the vca signal so zero input --> minimum volume
 * @date 2022-06-30
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

template <int OFFSET, bool INVERT>
class Vca {
  public:
    Vca() = default;
    ~Vca() = default;
    ;

    /**
     * @brief Set the Zero Offset and precalc the gain values. this is used to caculate the final output levels
     *
     * @param offset
     */
    void setZeroOffset(float offset) {
        _offset = offset;
        _gain_up = (1 - _offset);
        _gain_down = _offset + 1;
    }

    float getZeroOffset() const {
        return _offset;
    }

    /**
     * @brief applies the vca calibration to the input data
     *
     * @param input_data reference to the data to calibrate
     */
    template <bool SUM>
    float runVca(const std::array<float, CV_BUFFER_SIZE> &input_data, std::array<float, BUFFER_SIZE> &out) const {

        float sum = 0;
        for (int i = 0; i < CV_BUFFER_SIZE; i++) {

            if (INVERT) {
                out[i * CV_MUX_INC + OFFSET] = (input_data[i]) - _offset;
            } else {
                out[i * CV_MUX_INC + OFFSET] = input_data[i] + _offset;
            }
            // if sum is enabled, then return the integrated vca level for the buffer
            if (SUM) {
                sum += input_data[i];
            }
        }
        return sum;
    }

    void print(){};

  private:
    float _offset = 0;
    float _gain_up = 1;
    float _gain_down = 1;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
