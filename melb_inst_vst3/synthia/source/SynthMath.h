#include "common.h"
#include "fastapprox.h"
#include <array>
#include <cmath>

#pragma once

namespace Steinberg {
namespace Vst {
namespace Nina {

inline float pitchShiftMultiplier(float dPitchShiftSemitones) {
    if (dPitchShiftSemitones == 0)
        return 1.0f;

    // 2^(N/12)
    //	return fastPow(2.0, dPitchShiftSemitones/12.0);
    return std::exp2f(dPitchShiftSemitones / 12.0f);
}

inline float midiNoteToCv(int midi_n) {
    constexpr float cv_scale = 10.0 / 120.0;
    constexpr float cv_offset = std::log2(440.f) - (69.f * cv_scale);
    return ((float)(midi_n)) * cv_scale + cv_offset;
}

inline float midiNoteToFreq(int midi_n) {
    return 440.f * fastpow2(((float)midi_n - 69.0f) / 12.f);
}

inline float midiSourceScaleBipolar(float value) {
    return value * 2.0f - 1.0f;
}

inline float midiSourceScaleUnipolar(float value) {
    return value * 2.0f;
}

// Special smoothing function for MPE data, it has a very low cutoff but will speed up with a bigger delta to avoid 'lag'
inline float mpeParamSmooth(float value, float &prev_value) {
    constexpr float filter_rate = 50.f / 750.f;
    constexpr float nonlinear_rate = 0.02;
    const float diff = value - prev_value;
    prev_value += ((diff) * (filter_rate + nonlinear_rate * std::fabs(diff)));
    return prev_value;
}

inline float mpeParamFall(float value, float &prev_value, float fall_time) {

    // if value is falling, then decay the value according to fall time
    if (((value) < (prev_value))) {
        prev_value += fall_time * (value - prev_value);
    } else {
        prev_value = value;
    }
    return value;
}

constexpr float noteGain = 5.0;

inline float mpePbRangeConvert(uint semitones) {
    return 2.f * (((float)semitones) / 12.0) / noteGain;
}

constexpr float MASTER_DETUNE_RANGE_SEMI_T = 2.f;

inline float calcMasterDetune(float param) {
    const float detune_semi_tones = (param - 0.5) * 2.0 * MASTER_DETUNE_RANGE_SEMI_T;
    return (detune_semi_tones / 12.0) / noteGain;
}

inline float fastLog2(float x) {
    return fastlog2(x);
    // return std::log2(x);
    // return x;
}

inline float fastlog10(float num) { return std::log10(num); }

inline float volume_knob_cal(float input) {
    float output;
    output = (10.0f / 9.0f) * (exp10f32(input - 1.0f) - 1.0f / 10.0f);
    return output;
};

inline float fastCos(float input) { return std::cos(input); }

inline float fastSin(float input) { return std::sin(input); }

/**
 * @brief WORLDS WORST REALLY UNSAFE RINGBUFFER pls dont use. has an offset (for
 * filter with delay) and inits all values to 0
 *
 * @tparam N size of the ringbuffer
 * @tparam O starting offset point
 */
template <int N, int O>
class delayLineBuffer {
  public:
    delayLineBuffer() {
        for (float val : _array) {
            val = 0;
        }
    };

    ~delayLineBuffer() = default;
    ;

    void run(float number) {
        _array[_tail++] = number;
        _head++;
        _head = (_head > _max_size) ? _head : 0;
        _tail = (_tail >= _max_size) ? _tail : 0;
    }

    float read() {
        float val = _array[_head];

        return val;
    }

    int getSize() {
        int size;
        if (_head >= _tail) {
            size = _head - _tail;
        } else {
            size = _max_size + _head - _tail;
        }

        return size;
    }

  private:
    static constexpr const size_t _max_size = N;
    std::array<float, _max_size> _array;
    size_t _head = 0;
    size_t _tail = 0;
};

inline u_int32_t juicy_hash(u_int32_t x) {
    x ^= x >> 17;
    x *= 0xed5ad4bb;
    x ^= x >> 11;
    x *= 0xac4c1b51;
    x ^= x >> 15;
    x *= 0x31848bab;
    x ^= x >> 14;

    return x;
};

inline float param_smooth(float input, float filter_state) {
    const float diff = input - filter_state;
    filter_state += (diff * (PARAM_SMOOTH_COEFF + (DYN_COEFF * std::fabs(diff))));
    return filter_state;
}

/**
 * @brief Clip the signal to the range 1.0->-1.0
 *
 * @param cv
 * @return float
 */
inline float cv_clip(float cv) {
    cv = (cv > 1.0f) ? 1.0f : cv;
    cv = (cv < -1.0f) ? -1.0f : cv;
    return cv;
};

inline float unipolarClip(float cv) {
    cv = (cv > 1.0f) ? 1.0f : cv;
    cv = (cv < -.0f) ? -.0f : cv;
    return cv;
};

// returns true if the two floats are close within an audible threshold of -80dB
inline constexpr bool audioFloatEq(const float val, const float comp) {
    constexpr float DB_THRESH = -80.f;
    constexpr float LIN_THRESH = pow(10, DB_THRESH / 20.f);
    return abs(val - comp) < LIN_THRESH;
}

;
#ifndef __x86_64__
inline void setFpStatusRegister(intptr_t fpsr) noexcept {

    asm volatile("msr fpcr, %0"
                 :
                 : "ri"(fpsr));
}

inline intptr_t getFpStatusRegister() noexcept {
    intptr_t fpsr = 0;

    asm volatile("mrs %0, fpcr"
                 : "=r"(fpsr));

    return fpsr;
}

inline void enableFlushToZeroMode(bool shouldEnable) noexcept {

    intptr_t mask = (1 << 24 /* FZ */);

    setFpStatusRegister((getFpStatusRegister() & (~mask)) | (shouldEnable ? mask : 0));
}
#else
inline void enableFlushToZeroMode(bool shouldEnable) {}
#endif

} // namespace Nina
} // namespace Vst
} // namespace Steinberg