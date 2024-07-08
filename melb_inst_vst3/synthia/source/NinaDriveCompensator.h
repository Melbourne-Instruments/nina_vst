

#include "SynthMath.h"
#include "common.h"

#pragma once

namespace Steinberg {
namespace Vst {
namespace Nina {

class DriveCompensator {
  public:
    DriveCompensator(bool &overdrive, float &compresson_signal, float &main_out_max_level, float &patch_vol) :
        _overdrive(overdrive), _compression_signal(compresson_signal), _main_out_max_level(main_out_max_level), _patch_volume(patch_vol) {
    }

    ~DriveCompensator(){};

    float &getMixbusCompensation() {
        return _main_vca_gain;
    }

    float &getMixerVcaCompensation() {
        return _mix_gain;
    }

    float *getdriveInput() {
        return &_drive_level;
    }

    void reCalculate() {
        float drive_level = cv_clip(_drive_level) / 2 + 0.5;

        float total_gain = min_drive_gain + od_gain * (int)_overdrive + (drive_range) * (drive_level);
        _mix_gain = (min_drive_gain + (1 - min_drive_gain) * drive_level);

        // calculate the level reduction for main out mix muting
        constexpr float thresh = 0.01;
        float mix_mult = 1.0;
        if (_main_out_max_level < thresh) {
            mix_mult = mix_mult * (1.f / thresh) * _main_out_max_level;
        }

        _pre_compression_vca_gain = (((min_drive_gain / total_gain) * compensation_factor) + (1 - compensation_factor)) / 2;
        _pre_compression_vca_gain *= _patch_volume;
        _mix_gain *= mix_mult;
        if (_print) {
            printf("\ncomp: %f %f ", min_drive_gain, _mix_gain);
            _print = false;
        }
    }

    void print() {
        // printf("\n drive comp %f %f %f %f ", _mix_gain, _main_vca_gain, _drive_level);
        _print = true;
    }

    void run() {
        _main_vca_gain = _pre_compression_vca_gain - _compression_signal;

        // transform the assymetric vca input range of +1,-3 to the range 1,-1
    }

  private:
    // amount of gain applied by the overdrive button
    static constexpr float od_gain_db = 14.f;

    // range of the drive knob
    static constexpr float drive_range_db = 12.f;
    static constexpr float od_gain = std::pow(10.f, od_gain_db / 20.f);
    static constexpr float drive_range = std::pow(10.f, drive_range_db / 20.f) / 2;

    // experimentally found drive comp factor for main output vca
    static constexpr float compensation_factor = .55;
    static constexpr float min_drive_gain = std::pow(10.f, (-drive_range_db) / 20.f);
    static constexpr float max_vol = 0.6f;
    float _pre_compression_vca_gain = 1;

    float _main_vca_gain = 1;
    float _mix_gain = 1;
    float _drive_level = 0;
    float &_compression_signal;
    float &_main_out_max_level;
    bool &_overdrive;
    float &_patch_volume;
    bool _print = false;
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg