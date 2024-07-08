
#ifndef JMXNOISEOSC
#define JMXNOISEOSC
#pragma once
#include "SynthMath.h"
#include "math.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

class NoiseOscillator {
  public:
    NoiseOscillator() { _noise_state = rand(); }

    ~NoiseOscillator() {}

    /**
     * @brief Return a random float from 0*volume to 1*volume
     *
     * @return float
     */
    float getSample() {
        float sample =
            int(juicy_hash(_noise_state++) & 0x7fffffff) * (1.f / 0x7fffffff);
        return sample * _volume;
    }

    void setVolume(float volume) { _volume = volume; }

    /**
     * @brief mate, thats a noice juicy hash
     *
     * @param x
     * @return u_int32_t
     */

  private:
    u_int32_t _noise_state = 0;
    float _volume = 0;
};

// Technique by Larry "RidgeRat" Trammell 3/2006
// http://home.earthlink.net/~ltrammell/tech/pinkalg.htm
// implementation and optimization by David Lowenfels

#include <cstdlib>
#include <ctime>

#define PINK_NOISE_NUM_STAGES 3

class PinkNoiseGen {
  public:
    PinkNoiseGen() {
        clear();
    }

    void clear() {
        for (size_t i = 0; i < PINK_NOISE_NUM_STAGES; i++)
            state[i] = 0.0;
    }

    float tick() {
        static const float RMI2 = 2.0 / float(RAND_MAX); // + 1.0; // change for range [0,1)
        static const float offset = A[0] + A[1] + A[2];

        // unrolled loop
        float temp = float(juicy_hash(_noise_state++));
        state[0] = P[0] * (state[0] - temp) + temp;
        temp = float(juicy_hash(_noise_state++));
        state[1] = P[1] * (state[1] - temp) + temp;
        temp = float(juicy_hash(_noise_state++));
        state[2] = P[2] * (state[2] - temp) + temp;
        return (A[0] * state[0] + A[1] * state[1] + A[2] * state[2]) * RMI2 - offset;
    }

  protected:
    float state[PINK_NOISE_NUM_STAGES];
    static constexpr const float A[] = {0.02109238, 0.07113478, 0.68873558}; // rescaled by (1+P)/(1-P)
    static constexpr float P[] = {0.3190, 0.7756, 0.9613};
    u_int32_t _noise_state = 0;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
#endif