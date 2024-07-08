/**
 * @file NinaParameters.h
 * @brief Specifies the base Nina parameters available to the processor.
 *
 * @copyright Copyright (c) 2022-2023 Melbourne Instruments
 */
#pragma once
#include "SynthMath.h"
#include "common.h"
#include "public.sdk/source/vst/vstparameters.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

// Various helper MACROs

class NinaParams {
  public:
    enum {

        // Current Layer is a special param, it must be first in this list
        CurrentLayer,

        // Global Params
        Morphing,
        MorphValue,
        MorphMode,
        MainOutMuteL,
        MainOutMuteR,
        TuningGain,
        Cv1Offset,
        Cv2Offset,
        Cv3Offset,
        Cv4Offset,
        Cv1Gain,
        Cv2Gain,
        Cv3Gain,
        Cv4Gain,
        Cv1InputMode,
        Cv2InputMode,
        Cv3InputMode,
        Cv4InputMode,
        MasterTune,
        MpeLowerZonePB,
        MpeUpperZonePB,
        MpeYBipolarMode,

        // Layer global params
        MpeLowerZoneNumChannels,
        MpeUpperZoneNumChannels,

        // Layer common params
        LayerState,
        LayerLoading,
        AllNotesOff,
        extInMode,
        extInGain,
        outputRouting,
        Layer1NumVoices,
        midiLowNote,
        midiHighNote,
        midiChannelFilter,
        midiSourceVelMode,
        octaveOffset,
        cvASelect,
        cvBSelect,
        PatchVolume,
        NumUnison,
        UnisonPan,
        PanNum,
        PanMode,
        Legato,
        MpeMode,
        MpeX,
        MpeY,
        MpeZ,
        MpeYFallTime,
        MpeZFallTime,
        Lfo1TempoSync,
        Lfo2TempoSync,
        WtInterpolateMode,
        MidiPitchBend,
        PitchBendRange,
        MidiModWheel,
        MidiAftertouch,
        MidiPolyAT,
        MidiExpression,
        MidiSource,
        VcfOverdrive,
        GlideMode,
        AmpEnvelopeDrone,
        Sustain,

        // morph mod parameters
        MorphEg1,
        MorphEg2,
        MorphLfo1,
        MorphWave,
        MorphKBD,
        MorphVel,
        MorphAfter,
        MorphMod,
        MorphExp,
        MorphPan,
        MorphTime,
        MorphMidi,
        MorphCvA,
        MorphCvB,
        MorphLfo2,
        MorphOffset,

        // The following params will be removed at some point
        AllRunTuning,
        RunMainVcaCal,
        RunMixVcaCal,
        RunFilterCal,
        writeTemp,
        reloadCal,
        vca_offset_1,
        vca_offset_2,
        vca_offset_3,
        vca_offset_4,
        vca_offset_5,
        main_vca_offset_1,
        main_vca_offset_2,
        filter_offset,
        filter_scale,
        filter_t_coeff,
        voice_cal_select,
        voice_cal_write,
        res_high,
        res_low,
        TriggerPrint,
        MiscScale,
        TimeRate,

        // Layer State base params
        // VCO 1
        Vco1TuneFine,
        Vco1TuneCoarse,
        Vco1TuneSemitone,
        // VCO 2
        Vco2TuneFine,
        Vco2TuneCoarse,
        Vco2TuneSemitone,
        // Wave
        WaveTuneFine,
        WaveTuneCoarse,
        WavetableSelect,
        // LFO
        LfoShape,
        Lfo2Shape,
        Lfo1SyncRate,
        Lfo2SyncRate,
        // VCF
        VcfDrive,
        VcfKeyTrack,
        VcfMode,
        // Others
        Glide,
        SubOscillator,
        HardSync,
        AmpEnvVelSense,
        FiltEnvVelSense,
        UnisonSpread,
        KeyPitchOffset,
        LfoSlew,
        LfoRetrigger,
        Lfo2Slew,
        Lfo2Retrigger,
        XorMode,
        VcaUnipolar,
        SpinReset,
        VcaEnvReset,
        VcfEnvReset,
        FineTuneRange,
        Osc3Low,

        // RO params
        CurrentMorphValue,

        // Mod Matrix entries start here
        ModMatrixEntries
    };

    // Mod Matrix Sources
    enum ModMatrixSrc {
        FilterEnvelope,
        AmpEnvelope,
        Lfo1,
        Wavetable,
        KeyPitch,
        KeyVelocity,
        Aftertouch,
        Modwheel,
        Expression,
        PanPosition,
        Time,
        Midi,
        CVA,
        CVB,
        Lfo2,
        Offset,
        Constant,
        NumSrcs
    };

    // Mod Matrix Destinations
    enum ModMatrixDst {
        // Destinations not the front panel
        Lfo1Rate,
        Lfo1Gain,
        Lfo2Rate,
        Lfo2Gain,
        VcaIn,
        Pan,

        // Destinations on the front panel
        Osc1Pitch,
        Osc1Width,
        Osc1Blend,
        Osc1Level,
        Osc2Pitch,
        Osc2Width,
        Osc2Blend,
        Osc2Level,
        Osc3Pitch,
        Osc3Shape,
        Osc3Level,
        XorLevel,
        FilterCutoff,
        FilterResonance,
        Drive,
        FilterEnvelopeAtt,
        FilterEnvelopeDec,
        FilterEnvelopeSus,
        FilterEnvelopeRel,
        FilterEnvelopeLevel,
        AmpEnvelopeAtt,
        AmpEnvelopeDec,
        AmpEnvelopeSus,
        AmpEnvelopeRel,
        AmpEnvelopeLevel,
        Spin,
        Morph,
        NumDsts
    };

    enum XorNoiseModes {
        Xor,
        WhiteNoise,
        PinkNoise,
        AuxIn
    };

    enum MpeModes {
        Off,
        Lower,
        Upper,
        NumMpeModes
    };

    // default value for the pitch bend range, this sets it to 48 semitones
    static constexpr float MpeDefStartValue = 0.5 - 1. / (2 * 96.);

    struct MpeChannelData {
        uint channel = 0;
        float mpe_x = 0.5;
        float mpe_y = 0.0;
        float mpe_z = 0.0;
    };

    enum class OutputRouting {
        One,
        Two,
        Three,
        Four,
        St_L_R,
        St_3_4,
        St_1_2_3_4,
        numModes
    };

    enum class CvInputMode {
        One,
        Two,
        Three,
        Four,
        numModes
    };

    struct TempoSyncMultipliers {
        static constexpr uint num_sync_tempos = 16;
        static constexpr float _0125 = .125f;
        static constexpr float _025 = .25f;
        static constexpr float _05 = .5f;
        static constexpr float _1D = 1.f / 1.5f;
        static constexpr float _1 = 1.f;
        static constexpr float _2 = 2.f;
        static constexpr float _2D = _2 / 1.5f;
        static constexpr float _4 = 4.f;
        static constexpr float _4D = _4 / 1.5f;
        static constexpr float _4T = _4 * (4.f / 3.f);
        static constexpr float _8 = 8.f;
        static constexpr float _8T = _8 * (4.f / 3.f);
        static constexpr float _16 = 16.f;
        static constexpr float _16T = _16 * (4.f / 3.f);
        static constexpr float _32 = 32.f;
        static constexpr float _32T = _32 * (4.f / 3.f);

        constexpr float getTempoSyncMultiplier(uint select) {
            switch (select) {
            case 0:
                return _0125 / 4.f;
                break;
            case 1:
                return _025 / 4.f;
                break;
            case 2:
                return _05 / 4.f;
                break;
            case 3:
                return _1D / 4.f;
                break;
            case 4:
                return _1 / 4.f;
                break;

            case 5:
                return _2D / 4.f;
                break;

            case 6:
                return _2 / 4.f;
                break;

            case 7:
                return _4D / 4.f;
                break;

            case 8:
                return _4 / 4.f;
                break;

            case 9:
                return _4T / 4.f;
                break;

            case 10:
                return _8 / 4.f;
                break;

            case 11:
                return _8T / 4.f;
                break;

            case 12:
                return _16 / 4.f;
                break;

            case 13:
                return _16T / 4.f;
                break;

            case 14:
                return _32 / 4.f;
                break;

            case 15:
                return _32T / 4.f;
                break;

            default:
                return 1.f / 4.f;
            }
        }
    };

    // matrix setup constants
    static constexpr uint NUM_FAST_SRC = 5;
    static constexpr uint NUM_FAST_DST = 17;
    // Parameter constants
    static constexpr uint FIRST_LAYER_COMMON_PARAM = LayerState;
    static constexpr uint FIRST_LAYER_STATE_PARAM = Vco1TuneFine;
    static constexpr uint NUM_GLOBAL_PARAMS = LayerState;
    static constexpr uint NUM_LAYER_COMMON_PARAMS = (Vco1TuneFine - LayerState);
    static constexpr uint NUM_LAYER_BASE_STATE_PARAMS = (ModMatrixEntries - Vco1TuneFine);
    static constexpr uint NUM_LAYER_MOD_MATRIX_STATE_PARAMS = (ModMatrixSrc::NumSrcs * ModMatrixDst::NumDsts);
    static constexpr uint NUM_LAYER_STATE_PARAMS = (NUM_LAYER_BASE_STATE_PARAMS + NUM_LAYER_MOD_MATRIX_STATE_PARAMS);
    static constexpr uint NUM_LAYER_PARAMS = (NUM_LAYER_COMMON_PARAMS + NUM_LAYER_STATE_PARAMS);
    static constexpr uint NUM_PARAMS = (NUM_GLOBAL_PARAMS + NUM_LAYER_PARAMS);

    // Aliases
    using GlobalParams = std::array<float, NUM_GLOBAL_PARAMS>;
    using LayerCommonParams = std::array<float, NUM_LAYER_COMMON_PARAMS>;
    using LayerStateParams = std::array<float, NUM_LAYER_STATE_PARAMS>;

    // Structures
    struct LayerParams {
        NinaParams::LayerCommonParams common_params;
        NinaParams::LayerStateParams state_params;
        bool mpe_mode_en = false;
        float mpe_pitchbend_gain = 0;
        float mpe_pitchbend_offset = 0;
        bool mute_out_1 = false;
        bool mute_out_2 = false;
        bool mute_out_3 = false;
        bool mute_out_4 = false;
        bool osc_3_slow_mode = false;
        bool release_vel_mode = false;
        float drive_factor = 1.f;
        float drive_compensation = 1.f;
        float pitchbend = 0;
        float expression = 0;
        float midi_source = 0;
        bool overDrive = false;
        bool vca_unipolar = true;
        bool spin_reset = false;
        bool lfo_reset = false;
        bool lfo_2_reset = false;
        bool amp_env_reset = false;
        bool filter_env_reset = false;
        bool vca_drone = false;
        float global_tempo = 10.f / 60.f;
        bool lfo_1_tempo_sync = false;
        bool lfo_2_tempo_sync = false;
        bool lfo_1_global = false;
        bool lfo_2_global = false;
        float global_lfo_1_phase = 0;
        float global_lfo_2_phase = 0;
        GlideModes glide_mode = GlideModes::LOG;
        float compression_signal = 0;
        XorNoiseModes xor_mode = Xor;
        bool filter_2_pole_mode = false;
        std::array<float, CV_BUFFER_SIZE> *_cv_a;
        std::array<float, CV_BUFFER_SIZE> *_cv_b;
        bool wave_interpolate = false;
        float master_detune = 0;
        bool mpe_y_bipolar = true;
        float mpe_y_fall_coeff = 1;
        float mpe_z_fall_coeff = 1;
        float morph_mod_fb = 0;
        float morph_pos = 0;
    };

    struct AudioInputBuffers {
        AudioInputBuffers(std::array<float, CV_BUFFER_SIZE> &cv_in_1,
            std::array<float, CV_BUFFER_SIZE> &cv_in_2,
            std::array<float, CV_BUFFER_SIZE> &cv_in_3,
            std::array<float, CV_BUFFER_SIZE> &cv_in_4) :
            _cv_in_1(cv_in_1),
            _cv_in_2(cv_in_2), _cv_in_3(cv_in_3), _cv_in_4(cv_in_4){};

        std::array<float, BUFFER_SIZE> *mix_left;
        std::array<float, BUFFER_SIZE> *mix_right;
        std::array<float, BUFFER_SIZE> *ex_in_1;
        std::array<float, BUFFER_SIZE> *ex_in_2;
        std::array<float, BUFFER_SIZE> *ex_in_3;
        std::array<float, BUFFER_SIZE> *ex_in_4;
        std::array<float, CV_BUFFER_SIZE> &_cv_in_1;
        std::array<float, CV_BUFFER_SIZE> &_cv_in_2;
        std::array<float, CV_BUFFER_SIZE> &_cv_in_3;
        std::array<float, CV_BUFFER_SIZE> &_cv_in_4;
    };

    // Public function
    static float getGlobalParam(ParamID param_id, const GlobalParams &params);
    static float getLayerCommonParam(ParamID param_id, const LayerCommonParams &params);
    static float getLayerStateParam(ParamID param_id, const LayerStateParams &params);
    static float *getLayerCommonParamAddr(ParamID param_id, LayerCommonParams &params);
    static float *getLayerStateParamAddr(ParamID param_id, LayerStateParams &params);
    static void setGlobalParam(ParamID param_id, float value, GlobalParams &params);
    static void setLayerCommonParam(ParamID param_id, float value, LayerCommonParams &params);
    static void setLayerStateParam(ParamID param_id, float value, LayerStateParams &params);
    static RangeParameter *toRangeParam(uint param_num, ParamValue minPlain = 0.0, ParamValue maxPlain = 1.0, ParamValue defaultValuePlain = 0.0);
    static RangeParameter *toRangeParam(uint param_num, uint mod_src, uint mod_dst);

  public:
    static const char *_paramTitles[];
    static const char *_modMatrixSrcTitles[];
    static const char *_modMatrixDstTitles[];
    static const std::unordered_map<ParamID, const char *> _modMatrixAltTitles;
};

/**
 * @brief Helper fn to return the array index used to access the parameter at ParamID in the common param array
 *
 * @param param_id
 * @return constexpr uint common param index
 */
constexpr uint LAYER_COMMON_PARAMID_TO_INDEX(ParamID param_id) { return (param_id - NinaParams::FIRST_LAYER_COMMON_PARAM); };

/**
 * @brief Helper fn used to return the array index to access the given param id in a layer state array
 *
 * @param param_id its type checked!
 * @return constexpr uint state array index
 */
constexpr uint LAYER_STATE_PARAMID_TO_INDEX(ParamID param_id) { return (param_id - NinaParams::FIRST_LAYER_STATE_PARAM); }

/**
 * @brief Helper fn used to return the param id of the gain connecting a given mod matrix source->dest pair. thank god its not a #define amiright
 *
 * @param src
 * @param dst
 * @return constexpr uint
 */
constexpr uint MAKE_MOD_MATRIX_PARAMID(NinaParams::ModMatrixSrc src, NinaParams::ModMatrixDst dst) { return (NinaParams::ModMatrixEntries + (src * NinaParams::ModMatrixDst::NumDsts) + dst); }

// returns the real param id after masking off the bits used to indicate the target layer

} // namespace Nina
} // namespace Vst
} // namespace Steinberg