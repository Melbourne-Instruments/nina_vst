/**
 * @file NinaMatrix.cpp
 * @brief Implementation of Nina's matrix
 * @date 2022-07-18
 *
 * Copyright (c) 2023 Melbourne Instruments
 *
 */
#include "NinaMatrix.h"
#include <cmath>

namespace Steinberg {
namespace Vst {
namespace Nina {

//#define MATRIX_DBG
NinaMatrix::~NinaMatrix(){};

float dummy = 0;

void NinaMatrix::run_slow_slots() {
    int fast_dst_counter = 0;
    for (int i = 0; i < matrix_destinations; ++i) {
#ifdef MATRIX_DBG
        bool printing = (i == 4);
        printing = true;
#endif
        bool fast_dst = false;
        if (_matrix_dests[i] == _fast_dst[fast_dst_counter]) {
            fast_dst = true;
        }
        float sum = 0.0f;
        for (int j = 0; j < matrix_sources; ++j) {
            sum += ((*(_matrix_srcs[(j)])) * (*_gains[(j * matrix_destinations + i)]));
#ifdef MATRIX_DBG
            if (print && printing) {
                printf("\n");
                printf(NinaParams::_modMatrixSrcTitles[(j)]);
                printf("\t\t\t");
                printf(NinaParams::_modMatrixDstTitles[i]);
                printf("\t\t%f * %f  %f ", *_matrix_srcs.at(j), *_gains.at(j * matrix_destinations + i), sum);
            }
#endif
        }

        if (fast_dst) {
            _dst_cache[fast_dst_counter] = sum;
            if (fast_dst_counter < (NinaParams::NUM_FAST_DST - 1)) {
                fast_dst_counter++;
            }
        } else {
            *(_matrix_dests[i]) = sum;
        }
    }
    print = false;
}

void NinaMatrix::run_fast_slots() const {
    for (uint dst = 0; dst < NinaParams::NUM_FAST_DST; ++dst) {
        float sum = 0;
        for (uint src = 0; src < NinaParams::NUM_FAST_SRC; ++src) {
            sum += *(_fast_src[src]) * (*_fast_gains[(src * NinaParams::NUM_FAST_DST + dst)]);
        }
        *(_fast_dst[dst]) = sum + _dst_cache[(dst)];
    }
}
} // namespace Nina
} // namespace Vst
} // namespace Steinberg
