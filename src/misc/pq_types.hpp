#ifndef PQ_TYPES_INCLUDED
#define PQ_TYPES_INCLUDED

//#include "sdsl/bit_vectors.hpp"
//#include "sdsl/int_vector.hpp"
//#include "sdsl/wavelet_trees.hpp"
//#include "sdsl/rrr_vector.hpp"
//#include "sdsl/sd_vector.hpp"
//#include "sdsl/rank_support_v5.hpp"
#include <cstdint>

namespace pq_types {
	/*typedef sdsl::bit_vector::size_type 	    size_type;
	typedef sdsl::bit_vector::size_type 	    node_type;
	typedef sdsl::int_vector<32>::value_type	value_type;
	//typedef sdsl::wt_int<> wt_int;
	//typedef sdsl::wt_int<sdsl::rrr_vector<63>> wt_int;
	typedef sdsl::wt_int<sdsl::rrr_vector<>> wt_int;
	//typedef sdsl::wt_int<sdsl::sd_vector<> > wt_int;
	//typedef sdsl::wt_int<sdsl::bit_vector_il<> > wt_int;
	 */

	// uint64_t is chosen so that
	// our library becomes compatible with sdsl
	// where everywhere int_vector::size_type, int_vector::value_type is used
	// so freedom from int-types is not total here
	// one solution is to introduced additional template
	// parameters in sdsl, e.g. for wt_int<>
	// note also that "node_type" is a type that is used
	// to encode node ids; in sdsl, node_type is a completely different
	// and unrelated thing
	// at the same time, we don't simply typedef int_vector<>::size_type size_type,
	// because we want to decouple our pointer-based data structures from sdsl
    using size_type= uint64_t;
    using node_type= uint32_t;
    using value_type= uint64_t;
    //typedef sdsl::wt_int<> wt_int;
    //typedef sdsl::wt_int<sdsl::rrr_vector<63>> wt_int;
    //typedef sdsl::wt_int<sdsl::rrr_vector<>> wt_int;
    //typedef sdsl::wt_int<sdsl::sd_vector<> > wt_int;
    //typedef sdsl::wt_int<sdsl::bit_vector_il<> > wt_int;
};
#endif
