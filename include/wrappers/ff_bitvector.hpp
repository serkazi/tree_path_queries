//
// Created by sj on 17/03/19.
//
// ff_bitvector -- fully-functional bit-vector */
#ifndef SPQ_FF_BITVECTOR_HPP
#define SPQ_FF_BITVECTOR_HPP
#include "pq_types.hpp"
#include "sdsl/rrr_vector.hpp"
#include "sdsl/rank_support_v5.hpp"
#include "sdsl/select_support_mcl.hpp"
#include <memory>

template <
        typename size_type= pq_types::size_type,
        typename value_type= pq_types::value_type,
        typename t_bvector = sdsl::bit_vector,
        typename t_rank_1_support = sdsl::rank_support_v5<1,1>,
        typename t_select_0_support = sdsl::select_support_mcl<0,1>,
        typename t_select_1_support = sdsl::select_support_mcl<1,1>
        >
class ff_bitvector {

    std::unique_ptr<t_bvector> bv= nullptr;
    std::unique_ptr<t_rank_1_support> rank1= nullptr;
    std::unique_ptr<t_select_0_support> sel0= nullptr;
    std::unique_ptr<t_select_1_support> sel1= nullptr;

public:

	explicit ff_bitvector( const sdsl::bit_vector *cbv ) {
		bv = std::make_unique<t_bvector>(*cbv);
		rank1= std::make_unique<t_rank_1_support>(bv.get());
		sel0 = std::make_unique<t_select_0_support>(bv.get());
		sel1 = std::make_unique<t_select_1_support>(bv.get());
	}

	size_type rank( size_type x, value_type i ) const {
		return i?(*rank1)(x):x-(*rank1)(x);
	}

	size_type select( size_type x, size_type i ) const {
		return i?(*sel1)(x):(*sel0)(x);
	}

	value_type operator[] ( size_type i ) const {
		return static_cast<value_type>((*bv)[i]);
	}

	virtual ~ff_bitvector() = default;

	// FIXME: compute space correctly (i.e. unique_ptr)
	double size_in_bytes() const {
		return  sdsl::size_in_bytes(*bv)+\
				sdsl::size_in_bytes(*sel0)+\
				sdsl::size_in_bytes(*sel1)+\
				sdsl::size_in_bytes(*rank1);
	}
};
#endif //SPQ_FF_BITVECTOR_HPP
