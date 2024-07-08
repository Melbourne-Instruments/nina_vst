
#include "NinaReverb.h"

namespace Steinberg {
namespace Vst {
namespace Nina {

NinaReverb::NinaReverb(/* args */) {
    input_lpf_0.mute();
    input_lpf_1.mute();
    input_hpf_0.mute();
    input_hpf_1.mute();

    early.loadPresetReflection(FV3_EARLYREF_PRESET_1);
    early.setMuteOnChange(false);
    early.setdryr(0); // mute dry signal
    early.setwet(0);  // 0dB
    early.setwidth(0.8);
    early.setLRDelay(0.3);
    early.setLRCrossApFreq(750, 4);
    early.setDiffusionApFreq(150, 4);
    early.setSampleRate(sampleRate);
    early_send = 0.20;

    late.setMuteOnChange(false);
    late.setwet(0);      // 0dB
    late.setdryr(0.001); // mute dry signal
    late.setwidth(1.0);
    early.setSampleRate(sampleRate);
    late.setSampleRate(sampleRate);
    nrev.setdryr(0);
    nrev.setwetr(1);
    nrev.setMuteOnChange(false);
    nrev.setSampleRate(sampleRate);

    nrevb.setdryr(0);
    nrevb.setwetr(1);
    nrevb.setMuteOnChange(false);
    nrevb.setSampleRate(sampleRate);

    strev.setdryr(0);
    strev.setwetr(1);
    strev.setMuteOnChange(false);
    strev.setdccutfreq(6);
    strev.setspinlimit(12);
    strev.setspindiff(0.15);
    strev.setSampleRate(sampleRate);
    early_hall.loadPresetReflection(FV3_EARLYREF_PRESET_1);
    early_hall.setMuteOnChange(false);
    early_hall.setdryr(0); // mute dry signal
    early_hall.setwet(0);  // 0dB
    early_hall.setwidth(0.8);
    early_hall.setLRDelay(0.3);
    early_hall.setLRCrossApFreq(750, 4);
    early_hall.setDiffusionApFreq(150, 4);
    early_hall.setSampleRate(sampleRate);

    late_hall.setMuteOnChange(false);
    late_hall.setwet(0);  // 0dB
    late_hall.setdryr(0); // mute dry signal
    late_hall.setwidth(1.0);
    late_hall.setSampleRate(sampleRate);
    model = &nrevb;

    setInputLPF(20000);
    setInputHPF(20);
};

NinaReverb::~NinaReverb() {
}

void NinaReverb::run(float **inputs, float **final_outputs, uint32_t frames) {
    if (_hard_mute) {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            outputs[0][i] = 0.f;
            outputs[1][i] = 0.f;
        }
        _pitch_shift.reset();

        input_hpf_0.mute();
        input_hpf_1.mute();
        input_lpf_0.mute();
        input_lpf_1.mute();
        early_hall.mute();
        late_hall.mute();
        early.mute();
        late.mute();
        nrev.mute();
        nrevb.mute();
        strev.mute();
    }
    for (uint32_t index = 0; index < paramCount; index++) {
        if (_invalidated_params[index]) {
            oldParams[index] = newParams[index];
            float value = newParams[index];

            switch (index) {
            case paramWet:
                // do something?
                break;
            case paramDry:
                break;
            case paramEarly:
                early_level = (_early_setting * 1.5) * (value / 100.0);
                break;
            case paramEarlySend:
                early_send = (value / 100.0);
                break;
            case paramLate:
                late_level = (value / 100.0);
                break;
            case paramSize:
                early.setRSFactor(value / 10.0);
                late.setRSFactor(value / 10.0);
                early_hall.setRSFactor(value / 10.0);
                late_hall.setRSFactor(value / 80.0);
                late.setbassboost(newParams[paramBoost] / 20.0 / pow(newParams[paramDecay], 1.5) * (newParams[paramSize] / 10.0));
                break;
            case paramWidth:
                strev.setwidth(value / 120.0);
                nrev.setwidth(value / 120.0);
                nrevb.setwidth(value / 120.0);
                early.setwidth(value / 120.0);
                late.setwidth(value / 100.0);
                early_hall.setwidth(value / 100.0);
                late_hall.setwidth(value / 100.0);
                break;
            case paramPredelay:

                strev.setPreDelay(value);
                nrev.setPreDelay(value);
                nrevb.setPreDelay(value);
                late.setPreDelay(value);
                late_hall.setPreDelay(value);
                // printf("\n set predelayu %f", value);
                break;
            case paramDecay:
                late_hall.setrt60(value);
                strev.setrt60(value);
                nrev.setrt60(value);
                nrevb.setrt60(value);
                late.setrt60(value);
                late.setbassboost(newParams[paramBoost] / 20.0 / pow(newParams[paramDecay], 1.5) * (newParams[paramSize] / 10.0));
                break;
            case paramDiffuse:
                late_hall.setidiffusion1(value / 140.0);
                late_hall.setapfeedback(value / 140.0);
                late.setidiffusion1(value / 120.0);
                late.setodiffusion1(value / 120.0);
                break;
            case paramSpin:
                late_hall.setspin(value);
                late.setspin(value);
                late.setspin2(std::sqrt(100.0 - (10.0 - value) * (10.0 - value)) / 2.0);
                strev.setspin(value);

                break;
            case paramModulation: {
                float mod = value + 0.001;
                late_hall.setspinfactor(mod);
                late_hall.setlfofactor(mod);
            } break;
            case paramWander:
                late_hall.setwander(value);
                strev.setwander(value / 200.0 + 0.1);
                late.setwander(value / 200.0 + 0.1);
                late.setwander2(value / 200.0 + 0.1);
                strev.setspindiff(value / 1000.f);
                strev.setwander(value / 200.f + 0.00);
                strev.setmodulationnoise1(value / 200.f);
                strev.setmodulationnoise2(value / 200.f);

                break;
            case paramHighXover:
                late_hall.setxover_high(value);
                break;
            case paramHighMult: {
                float mult_high = _tone_setting + 0.5;
                late_hall.setrt60_factor_high(mult_high * value);
            } break;
            case paramLowXover:
                late_hall.setxover_low(value);
                break;
            case paramLowMult: {
                float mult_low = 1.5 - _tone_setting;
                late_hall.setrt60_factor_low(mult_low * value);
            } break;
            case paramInHighCut: {
                float mult_high = _tone_setting + 0.5;
                late_hall.setoutputlpf(mult_high * value);
                early_hall.setoutputlpf(mult_high * value);
                setInputLPF(value);
            } break;
            case paramEarlyDamp: {
                float mult_high = _tone_setting + 0.5;
                nrev.setDampLpf(mult_high * value);
                nrevb.setDampLpf(mult_high * value);
                strev.setdamp(mult_high * value);
                strev.setoutputdamp(mult_high * value);
                early.setoutputlpf(value * mult_high);
                // printf("\n early damp: %f", value * mult_high);
            } break;
            case paramLateDamp: {
                float mult_high = _tone_setting + 0.5;
                late.setdamp(mult_high * value);
                late.setoutputdamp(mult_high * value);
            } break;
            case paramBoost:
                late.setbassboost(newParams[paramBoost] / 20.0 / pow(newParams[paramDecay], 1.5) * (newParams[paramSize] / 10.0));
                break;
            case paramBoostLPF:
                late.setdamp2(newParams[paramBoostLPF]);
                break;
            case paramInLowCut: {
                float mult_low = 1.5 - _tone_setting;
                early_hall.setoutputhpf(value);
                late_hall.setoutputhpf(value);
                strev.setdccutfreq(mult_low * value);
                early.setoutputhpf(mult_low * value);
                setInputHPF(value);
            } break;
            case paramTone:
                break;
            }
            if (index == paramInternalAlgo) {
                fv3::revbase_f *previous = model;
                int algorithm = value;
                if (algorithm == ALGORITHM_NREV) {
                    model = &nrev;
                } else if (algorithm == ALGORITHM_NREV_B) {
                    model = &nrevb;
                } else if (algorithm == ALGORITHM_STREV) {
                    model = &strev;
                }
                if (model != previous) {
                    previous->mute();
                }
            }
        }
    }
    _invalidated_params.fill(false);

    if (_hard_mute) {
        _hard_mute = false;
        return;
    }
    // run the input filtering for all reverb algos
    for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
        filtered_input_buffer[0][i] = in_gain * input_lpf_0.process(input_hpf_0.process(inputs[0][i]));
        filtered_input_buffer[1][i] = in_gain * input_lpf_1.process(input_hpf_1.process(inputs[1][i]));
    }

    bufc++;
    if (bufc % 700 == 0) {
        // printf("\n mode: %d %f %f wander stuff %f %f %f ", _mode, early_level, late_level, strev.getwander(), strev.getmodulationnoise1(), strev.getmodulationnoise2());
    }
    const auto tmp = _mode;
    switch (tmp) {
    case ReverbModes::room:
        /* code */
        {
            early.processreplace(
                const_cast<float *>(filtered_input_buffer[0]),
                const_cast<float *>(filtered_input_buffer[1]),
                early_out_buffer[0],
                early_out_buffer[1],
                BUFFER_SIZE);

            for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
                late_in_buffer[0][i] = early_send * early_out_buffer[0][i] + filtered_input_buffer[0][i] + _shimmer_gain * _pitch_shift_left[i];
                late_in_buffer[1][i] = early_send * early_out_buffer[1][i] + filtered_input_buffer[1][i] + _shimmer_gain * _pitch_shift_right[i];
            }
            late.processreplace(
                const_cast<float *>(late_in_buffer[0]),
                const_cast<float *>(late_in_buffer[1]),
                late_out_buffer[0],
                late_out_buffer[1],
                BUFFER_SIZE);
        }
        if (bufc % 700 == 0) {
            // printf("\n");
            for (int i = 0; i < BUFFER_SIZE; i++) {
                // printf(" %f", late_out_buffer[0][i]);
            }
        }
        break;

    case ReverbModes::plate: {

        for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
            filtered_input_buffer[0][i] += _shimmer_gain * _pitch_shift_left[i];
            filtered_input_buffer[1][i] += _shimmer_gain * _pitch_shift_right[i];
        }
        early_level = 0.f;
        strev.processreplace(
            const_cast<float *>(filtered_input_buffer[0]),
            const_cast<float *>(filtered_input_buffer[1]),
            late_out_buffer[0],
            late_out_buffer[1],
            BUFFER_SIZE);

    } break;

    case ReverbModes::hall: {
        early_hall.processreplace(
            const_cast<float *>(filtered_input_buffer[0]),
            const_cast<float *>(filtered_input_buffer[1]),
            early_out_buffer[0],
            early_out_buffer[1],
            BUFFER_SIZE);
        for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
            late_in_buffer[0][i] = early_send * early_out_buffer[0][i] + filtered_input_buffer[0][i] + _shimmer_gain * _pitch_shift_left[i];
            late_in_buffer[1][i] = early_send * early_out_buffer[1][i] + filtered_input_buffer[1][i] + _shimmer_gain * _pitch_shift_right[i];
        }
        late_hall.processreplace(
            const_cast<float *>(late_in_buffer[0]),
            const_cast<float *>(late_in_buffer[1]),
            late_out_buffer[0],
            late_out_buffer[1],
            BUFFER_SIZE);
    } break;
    default:
        break;
    }
    float local_early_level = early_level;
    float local_late_level = late_level;
    if (_hard_mute) {
        local_early_level = 0.f;
        local_late_level = 0.f;
        _hard_mute = false;
    }
    local_early_level = 0;

    for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
        outputs[0][i] = local_early_level * early_out_buffer[0][i];
        outputs[1][i] = local_early_level * early_out_buffer[1][i];
    }

    _mix_smooth = Nina::param_smooth(_wet_dry_mix, _mix_smooth);
    const float wet = _mix_smooth;
    for (uint32_t i = 0; i < BUFFER_SIZE; i++) {

        outputs[0][i] = (outputs[0][i] + local_late_level * late_out_buffer[0][i]) * wet;
        outputs[1][i] = (outputs[1][i] + local_late_level * late_out_buffer[1][i]) * wet;
    }
    _pitch_shift.run(late_out_buffer[0], late_out_buffer[1]);

    constexpr float ms_filter_cutoff = 400.f;
    constexpr float om3 = (ms_filter_cutoff * M_PI * 2.0) / ((float)SAMPLE_RATE);
    constexpr float f_a = std::cos(om3) - 1 + std::sqrt(std::cos(om3) * std::cos(om3) - 4 * std::cos(om3) + 3);
    constexpr float f_b = 1 - f_a;
    for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
        float side = outputs[0][i] - outputs[1][i];
        float mid = outputs[0][i] + outputs[1][i];
        _ms_eq_filter_state = f_b * _ms_eq_filter_state + f_a * side;
        side = side - _ms_eq_filter_state;
        final_outputs[0][i] += mid + side;
        final_outputs[1][i] += mid - side;
    }
}

NRev::NRev() :
    fv3::nrev_f() {}

void NRev::setDampLpf(float value) {
    dampLpf = limFs2(value);
    dampLpfL.setLPF_BW(dampLpf, getTotalSampleRate());
    dampLpfR.setLPF_BW(dampLpf, getTotalSampleRate());
}

void NRev::mute() {
    fv3::nrev_f::mute();
    dampLpfL.mute();
    dampLpfR.mute();
}

void NRev::setFsFactors() {
    fv3::nrev_f::setFsFactors();
    setDampLpf(dampLpf);
}

void NRev::processloop2(long count, float *inputL, float *inputR, float *outputL, float *outputR) {
    float outL, outR;
    while (count-- > 0) {
        outL = outR = 0;
        hpf = damp3_1 * inDCC(*inputL + *inputR) - damp3 * hpf;
        UNDENORMAL(hpf);

        hpf *= FV3_NREV_SCALE_WET;

        for (long i = 0; i < FV3_NREV_NUM_COMB; i++)
            outL += combL[i]._process(hpf);
        for (long i = 0; i < 3; i++)
            outL = allpassL[i]._process_ov(outL);
        lpfL = dampLpfL(damp2 * lpfL + damp2_1 * outL);
        UNDENORMAL(lpfL);
        outL = allpassL[3]._process_ov(lpfL);
        outL = allpassL[5]._process_ov(outL);
        outL = delayWL(lLDCC(outL));

        for (long i = 0; i < FV3_NREV_NUM_COMB; i++)
            outR += combR[i]._process(hpf);
        for (long i = 0; i < 3; i++)
            outR = allpassR[i]._process_ov(outR);
        lpfR = dampLpfR(damp2 * lpfR + damp2_1 * outR);
        UNDENORMAL(lpfR);
        outR = allpassR[3]._process_ov(lpfR);
        outR = allpassR[6]._process_ov(outR);
        outR = delayWR(lRDCC(outR));

        *outputL = outL * wet1 + outR * wet2 + delayL(*inputL) * dry;
        *outputR = outR * wet1 + outL * wet2 + delayR(*inputR) * dry;
        inputL++;
        inputR++;
        outputL++;
        outputR++;
    }
}

NRevB::NRevB() :
    fv3::nrevb_f() {}

void NRevB::setDampLpf(float value) {
    dampLpf = limFs2(value);
    dampLpfL.setLPF_BW(dampLpf, getTotalSampleRate());
    dampLpfR.setLPF_BW(dampLpf, getTotalSampleRate());
}

void NRevB::mute() {
    fv3::nrevb_f::mute();
    dampLpfL.mute();
    dampLpfR.mute();
}

void NRevB::setFsFactors() {
    fv3::nrevb_f::setFsFactors();
    setDampLpf(dampLpf);
}

void NRevB::processloop2(long count, float *inputL, float *inputR, float *outputL, float *outputR) {
    float outL, outR, tmpL, tmpR;
    while (count-- > 0) {
        hpf = damp3_1 * inDCC.process(*inputL + *inputR) - damp3 * hpf;
        UNDENORMAL(hpf);
        outL = outR = tmpL = tmpR = hpf;

        outL += apfeedback * lastL;
        lastL += -1 * apfeedback * outL;

        for (long i = 0; i < FV3_NREV_NUM_COMB; i++)
            outL += combL[i]._process(tmpL);
        for (long i = 0; i < FV3_NREVB_NUM_COMB_2; i++)
            outL += comb2L[i]._process(tmpL);
        for (long i = 0; i < 3; i++)
            outL = allpassL[i]._process(outL);
        for (long i = 0; i < FV3_NREVB_NUM_ALLPASS_2; i++)
            outL = allpass2L[i]._process(outL);
        lpfL = dampLpfL(damp2 * lpfL + damp2_1 * outL);
        UNDENORMAL(lpfL);
        outL = allpassL[3]._process(lpfL);
        outL = allpassL[5]._process(outL);
        outL = lLDCC(outL);

        outR += apfeedback * lastR;
        lastR += -1 * apfeedback * outR;
        for (long i = 0; i < FV3_NREV_NUM_COMB; i++)
            outR += combR[i]._process(tmpR);
        for (long i = 0; i < FV3_NREVB_NUM_COMB_2; i++)
            outR += comb2R[i]._process(tmpR);
        for (long i = 0; i < 3; i++)
            outR = allpassR[i]._process(outR);
        for (long i = 0; i < FV3_NREVB_NUM_ALLPASS_2; i++)
            outR = allpass2R[i]._process(outR);
        lpfR = dampLpfR(damp2 * lpfR + damp2_1 * outR);
        UNDENORMAL(lpfR);
        outR = allpassR[3]._process(lpfR);
        outR = allpassL[6]._process(outR);
        outR = lRDCC(outR);

        lastL = FV3_NREVB_SCALE_WET * delayWL(lastL);
        lastR = FV3_NREVB_SCALE_WET * delayWR(lastR);
        *outputL = lastL * wet1 + lastR * wet2 + delayL(*inputL) * dry;
        *outputR = lastR * wet1 + lastL * wet2 + delayR(*inputR) * dry;
        lastL = outL;
        lastR = outR;
        inputL++;
        inputR++;
        outputL++;
        outputR++;
    }
}
} // namespace Nina
} // namespace Vst
} // namespace Steinberg