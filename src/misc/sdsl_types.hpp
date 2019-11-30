//
// Created by sj on 16/11/19.
//
#ifndef SPQ_SDSL_TYPES_HPP
#define SPQ_SDSL_TYPES_HPP

#include "sdsl/bit_vectors.hpp"
#include "sdsl/int_vector.hpp"
#include "sdsl/wavelet_trees.hpp"
#include "sdsl/rrr_vector.hpp"
#include "sdsl/sd_vector.hpp"
#include "sdsl/rank_support_v5.hpp"

namespace sdsl_types {
    using wt_int= sdsl::wt_int<sdsl::rrr_vector<>>;
}
#endif //SPQ_SDSL_TYPES_HPP
