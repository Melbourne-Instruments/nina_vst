/**
 * @file WavetableOsc.h
 * @brief Wavetable Oscillator class implementation.
 *
 * @copyright Copyright (c) 2022-2023 Melbourne Instruments, Australia
 */
#pragma once
#include "AudioFile.h"
#include "SynthMath.h"
#include "common.h"
#include <algorithm>
#include <atomic>
#include <filesystem>
#include <thread>

namespace Steinberg {
namespace Vst {
namespace Nina {

// Constants
constexpr uint WT_BUFFER_SIZE = SAMPLE_RATE / CV_SAMPLE_RATE;
constexpr uint MAX_NUM_WAVES = 256;
constexpr uint WAVE_LENGTH = (1024 * 2);
constexpr uint NUM_MIPMAPS = 8;
constexpr uint NUM_LOWPASS_IN_FILTERS = (NUM_MIPMAPS - 1);
constexpr float WT_POS_SMOOTHING_FREQ_HZ = 4000.0;
constexpr float WT_POS_SMOOTH_COEFF = 1.0 - std::exp((-1.0 / (float)(SAMPLE_RATE / WT_BUFFER_SIZE)) / (1.0 / (float)WT_POS_SMOOTHING_FREQ_HZ));
constexpr float SLOW_WAVE_SUB = -10.f;
;

class BiquadFilter {
  public:
    BiquadFilter(float a1, float a2, float b0, float b1, float b2);

    ~BiquadFilter() {}

    inline void reset();
    inline float process(float input);

  private:
    const float _a1, _a2, _b0, _b1, _b2;
    float _m1, _m2;
    float _dn;
};

inline int16_t _floatSampleToInt16(float sample) {
    sample = sample > 0.99 ? 0.99 : sample;
    sample = sample < -0.99 ? -0.99 : sample;
    int16_t val = std::round(sample * (std::numeric_limits<std::int16_t>::max()));
    return val;
}

class LowpassFilter {
  public:
    LowpassFilter() {}

    virtual ~LowpassFilter() = default;

    virtual inline void reset() {}

    virtual inline float process(float input) { return 0.0f; }
};

inline constexpr uint getMipmapOffset(uint mipmap) {
    switch (mipmap) {
    case 0:
        return 0;
    case 1:
        return 2048;
    case 2:
        return 1024 + 2048;
    case 3:
        return 512 + 1024 + 2048;
    case 4:
        return 256 + 512 + 1024 + 2048;
    case 5:
        return 128 + 256 + 512 + 1024 + 2048;
    case 6:
        return 128 + 128 + 256 + 512 + 1024 + 2048;
    case 7:
        return 128 + 128 + 128 + 256 + 512 + 1024 + 2048;
        break;
    default:
        break;
    }
    return 0;
};

class WavetableLowpassFilter {
  public:
    WavetableLowpassFilter(uint fc) :
        FC(fc) {

        // set the filter to bypass processing if the fc is over 20k
        if (fc > 20000) {
            bypass = true;
        }
    };

    ~WavetableLowpassFilter() {}

    inline void reset();

    inline float process(float input);

  private:
    bool bypass = false;
    const uint FC;
    float _q = 0.70f;
    float _w0 = (2 * M_PIf32 * FC) / SAMPLE_RATE;
    float _alpha = std::sin(_w0) / (2 * _q);
    float _b0 = (1 - std::cos(_w0)) / 2;
    float _b1 = 1 - std::cos(_w0);
    float _b2 = (1 - std::cos(_w0)) / 2;
    float _a0 = 1 + _alpha;
    float _a1 = -2 * std::cos(_w0);
    float _a2 = 1 - _alpha;
    BiquadFilter _biquad_1 = BiquadFilter{(_a1 / _a0), (_a2 / _a0), (_b0 / _a0), (_b1 / _a0), (_b2 / _a0)};
    BiquadFilter _biquad_2 = BiquadFilter{(_a1 / _a0), (_a2 / _a0), (_b0 / _a0), (_b1 / _a0), (_b2 / _a0)};
    BiquadFilter _biquad_3 = BiquadFilter{(_a1 / _a0), (_a2 / _a0), (_b0 / _a0), (_b1 / _a0), (_b2 / _a0)};
    BiquadFilter _biquad_4 = BiquadFilter{(_a1 / _a0), (_a2 / _a0), (_b0 / _a0), (_b1 / _a0), (_b2 / _a0)};
};

inline float _int16SampleToFloat(int16_t sample) {
    return static_cast<float>(sample) / static_cast<float>(std::numeric_limits<std::int16_t>::max());
};

inline const auto _compareFloats(float f1, float f2) -> float {
    return std::abs(f1 - f2) <= std::numeric_limits<float>::epsilon();
}

class WavetableWave {
  public:
    WavetableWave() {}

    ~WavetableWave() {}

    void reset();
    inline float getSample(uint sample_num, uint mipmap_index) const;
    inline const int16_t *getSampleAddr(uint sample_num, uint mipmap_index) const;
    void processWave(const float *samples, std::array<WavetableLowpassFilter, NUM_LOWPASS_IN_FILTERS> &_lowpass_in);
    void postProcessWave(std::array<std::unique_ptr<LowpassFilter>, NUM_LOWPASS_IN_FILTERS> &lowpass_in);

    float getIntSample(uint sample_num, uint mipmap_index) const {
        // Get the sample from the specified wave
        return (_wavetable_wave[((sample_num << 3) + mipmap_index)]);
    }

  private:
    std::array<int16_t, (WAVE_LENGTH * NUM_MIPMAPS)> _wavetable_wave;
};

class Wavetable {
  public:
    Wavetable() {
        // reserve the memory needed for a full length wavetable
        _samples.reserve((int)MAX_NUM_WAVES * (getMipmapOffset(7) + 128));
        for (int i = 0; i < (int)MAX_NUM_WAVES * (getMipmapOffset(7) + 128); i++) {
            _samples.push_back(0);
            _num_waves = 0;
        }
    }

    ~Wavetable() {
    }

    void reset();
    uint getNumWaves() const;
    void processWaves(uint num_waves, const float *samples);
    std::vector<int16_t> _samples;

  private:
    uint _num_waves;
};

class WavetableOsc;

class WavetableLoader {
  public:
    WavetableLoader();
    ~WavetableLoader();

    bool isLoading() {
        return _load_wavetable;
    }

    const Wavetable *getCurrentWavetable() const;
    void loadWavetable(float select);

  private:
    /**
     * @brief The current wavetable
     */
    std::atomic<const Wavetable *> _current_wavetable;
    Wavetable _wavetable_slot1;
    Wavetable _wavetable_slot2;
    std::thread _load_wavetable_thread;
    float _wt_select_num;
    std::atomic<bool> _load_wavetable;
    std::atomic<bool> _exit_thread;
    void _monitorWavetable();
};

class WavetableOsc {
  public:
    WavetableOsc(WavetableLoader &wavetable_loader_a, WavetableLoader &wavetable_loader_b, float &morph, std::array<float, BUFFER_SIZE> *&output, float &drive_comp, bool &interpolate, bool &slow_mode);

    ~WavetableOsc(){};

    void printstuff() {
    }

    void reCalculate();

    float *getWtPosition() {
        return &_position;
    }

    float *getWtPitch() {
        return &_pitch;
    }

    float *getWtCvOut() {
        return &_wt_cv_out;
    }

    float *getWtVol() {
        return &_wt_vol;
    }

    void setOutput(std::array<float, 128> *output) {
        _output = output;
    }

    float run();
    void reset();

  private:
    bool _wt_loaded = false;
    float _current_value;
    float _current_phase;
    float _phase_advance;
    uint _mipmap_index;
    uint _wave_position_a = 0;
    uint _wave_position_b = 0;
    uint _num_waves_a = 0;
    uint _num_waves_b = 0;
    float _wt_out_a = 0;
    float _wt_out_b = 0;
    uint _wave_pos_a = 0;
    uint _wave_pos_b = 0;

    const Wavetable *_current_wavetable_a = 0;
    const Wavetable *_current_wavetable_b = 0;
    WavetableLoader &_wavetable_loader_a;
    WavetableLoader &_wavetable_loader_b;
    std::array<float, BUFFER_SIZE> *&_output;
    std::array<float, BUFFER_SIZE> _audio_buffer;
    std::array<float, CV_BUFFER_SIZE> _pitch_cv_buffer;
    std::array<float, CV_BUFFER_SIZE> _shape_cv_buffer;
    uint _buffer_position = 0;
    uint _output_position = 0;
    float _position = 0.f;
    float _position_filter_state = 0.f;
    float _pitch = 0.f;
    float &_morph;
    float _wt_cv_out = 0.f;
    float _wt_vol = 0.f;
    float &_drive_comp;
    float _gain_a = 0;
    float _gain_b = 0;
    bool &_interpolate;
    bool &_slow_mode;

    void _setWavetableWave(const Wavetable *wavetable, float value);
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg
