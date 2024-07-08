/**
 * @file NinaMatrix.h
 * @brief Declaration of Nina's matrix
 * @date 2022-07-18
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */
#pragma once

#include "NinaParameters.h"
#include "common.h"

namespace Steinberg {
namespace Vst {
namespace Nina {
constexpr int matrix_destinations = NinaParams::ModMatrixDst::NumDsts;
constexpr int matrix_sources = NinaParams::ModMatrixSrc::NumSrcs;

// template <uint N_SRC_F, uint N_DST_F, uint N_SRC_S, uint N_DST_S>
class NinaMatrix {
  public:
    int voicen = 0;
    bool dump = false;
    NinaMatrix(std::array<float *, NinaParams::NUM_FAST_DST> &fast_dst,
        std::array<float *, NinaParams::NUM_FAST_SRC> &fast_src,
        std::array<float *, NinaParams::NUM_FAST_SRC * NinaParams::NUM_FAST_DST> &fast_gains,
        std::array<float *, matrix_destinations> &matrix_dests,
        std::array<float *, matrix_sources> &matrix_srcs,
        std::array<float *, matrix_destinations * matrix_sources> &gains) :
        _fast_dst(fast_dst),
        _fast_src(fast_src),
        _fast_gains(fast_gains),
        _matrix_dests(matrix_dests),
        _matrix_srcs(matrix_srcs),
        _gains(gains)

            {};

    ~NinaMatrix();

    void add_source(int source_num, const float *const source, bool is_fast = false);
    void add_dst(int destination_num, float *const destination, bool is_fast = false);

    void set_gain_addr(int source_num, int dest_num, float *gain);

    /**
     * @brief calculate the sum of mod_gain * slot_dst_pairs for each dst and apply. also add the cached slow sum
     *
     */
    void run_fast_slots() const;

    /**
     * @brief calculate the sum of all slow gain * slot_dst_pairs for each dst and cache the value for the fast slot function to use
     *
     */
    void run_slow_slots();
    bool print = false;

  private:
    float zero = 0.0f;

    const std::array<float *, NinaParams::NUM_FAST_DST> &_fast_dst;
    const std::array<float *, NinaParams::NUM_FAST_SRC> &_fast_src;
    const std::array<float *, NinaParams::NUM_FAST_SRC * NinaParams::NUM_FAST_DST> &_fast_gains;
    const std::array<float *, matrix_destinations> &_matrix_dests;
    const std::array<float *, matrix_sources> &_matrix_srcs;
    const std::array<float *, matrix_destinations * matrix_sources> &_gains;
    std::array<float, NinaParams::NUM_FAST_DST> _dst_cache = {0.0};
};

} // namespace Nina
} // namespace Vst
} // namespace Steinberg