/**
 * @file Matrix.h
 * @brief Mod matrix Declaration for the NINA synth
 * @date 2022-06-29
 *
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */

#pragma once

namespace Steinberg {
namespace Vst {
namespace Nina {
#include "common.h"

enum MtxSrc {

    LFO_1,
    LFO_2,
    ENV_1,
    ENV_2,
    KEY_VEL,
    KEY_PITCH,
    CONSTANT,
    NUM_SRC
};

enum MtxDst {
    OSC_1_PITCH,
    OSC_1_SHAPE,
    OSC_2_PITCH,
    OSC_2_SHAPE,
    PAN,
    SPIN,
    NUM_DSTS
};

class Matrix {
    Matrix();
    ~Matrix();

  public:
    /**
     * @brief change the gain on a particular source/dest pair
     *
     * @param source
     * @param destintion
     * @param gain
     */
    void updateGain(MtxSrc source, MtxDst destintion, float gain);

    /**
     * @brief Set a source in the mod matrix
     *
     * @param source
     * @param src_val
     */
    void setSource(MtxSrc source, float &src_val);

    /**
     * @brief Set a destination in the mod matrix
     *
     * @param dest
     * @param dest_val
     */
    void setDest(MtxDst dest, float &dest_val);

    /**
     * @brief calculate and cache the sum of all the src dst pairs marked as slow
     *
     */
    void runSlowSlots();

    /**
     * @brief sums all the fast source dst pairs and a cached value for the slow slots to output a final mod result
     *
     */
    void runFastSlots();
};
} // namespace Nina
} // namespace Vst
} // namespace Steinberg