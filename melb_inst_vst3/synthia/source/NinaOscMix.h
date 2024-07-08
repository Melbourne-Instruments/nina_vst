

#include "SynthMath.h"
#include "common.h"

#pragma once

namespace Steinberg {
namespace Vst {
namespace Nina {

class OscMixer {
  public:
    OscMixer(float *tri, float *sqr, float &_drive_comp) :
        _tri_lev(*tri), _sqr_lev(*sqr), _drive_comp(_drive_comp){};
    ~OscMixer(){};

    float *getTriOutput() { return &_tri_lev; };

    float *getSqrOutput() { return &_sqr_lev; };

    float *getBlendInput() {
        return &_blend;
    }

    float *getGainInput() {
        return &_gain;
    }

    void reCalculate() {
    }

    void print() {
        printb = true;
    }

    void run() {

        // transform the assymetric vca input range of +1,-3 to the range 1,-1
        float gain = _gain + 1.f;

        float m1 = fastSin((M_PIf32 / 2) * (_blend));
        float m2 = fastCos((M_PIf32 / 2) * (_blend));
        _tri_lev = m1 * _drive_comp * gain / 2;
        _sqr_lev = m2 * _drive_comp * gain / 2;
        if (printb) {

            printf("\n oscprint %f %f %f %f  ", _gain, m1, _tri_lev, _drive_comp);
            printb = false;
        }
    }

  private:
    static constexpr float max_vol = 0.5f;
    float _blend = 0.0f;
    float _gain = 1.0f;
    float &_drive_comp;
    float &_tri_lev;
    float &_sqr_lev;
    bool printb = false;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg