#ifndef PATH_QUERY_PROCESSOR_INCLUDED
#define PATH_QUERY_PROCESSOR_INCLUDED

#include "pq_types.hpp"
#include <cassert>
#include <cmath>
#include <vector>

template<
        typename node_type= pq_types::node_type,
        typename size_type= pq_types::size_type,
        typename value_type= pq_types::value_type
        >
class path_query_processor {
protected:
	static bool lies_inside(
			const pq_types::value_type x,
			const pq_types::value_type a,
			const pq_types::value_type b ) {
		return a <= x and x <= b;
	}
	static size_type qntl2rnk( size_type n, size_type q ) {
		auto lower= static_cast<size_type>(std::ceil(n*(q/100.00))-1),
		  	 upper= static_cast<size_type>(std::floor(n*(q/100.00)));
		assert( lower <= upper );
		assert( 0 <= lower );
		return std::min(lower,n-1);
	}
    value_type m_sigma;
public:
	path_query_processor() {}
	virtual value_type query( node_type x, node_type y ) const = 0;
	virtual value_type selection( node_type x, node_type y, size_type qntl ) const = 0;
	virtual value_type weight_of( node_type x ) const = 0;
    virtual value_type weight( node_type x ) const = 0;
	virtual size_type count( node_type x,
							 node_type y,
				   			 value_type a,
							 value_type b	) const= 0;
	virtual void report( node_type x,
						 node_type y,
						 value_type a,
						 value_type b,
						 std::vector<std::pair<value_type,size_type>> &res ) const= 0;
	virtual ~path_query_processor() {};
};
#endif
