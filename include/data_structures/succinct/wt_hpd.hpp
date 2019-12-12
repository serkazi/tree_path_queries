#ifndef WT_HPD
#define WT_HPD

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "sdsl_types.hpp"
#include "bp_tree_sada.hpp"
#include "succinct_tree.hpp"
#include "path_query_processor.hpp"
#include "sdsl/int_vector.hpp"
#include "sdsl/wt_int.hpp"
#include <cassert>
#include <vector>
#include <sstream>
#include "path_decomposer.hpp"
#include "hpd_preprocessor.hpp"

/**
 * @tparam node_type
 * @tparam size_type
 * @tparam value_type
 * @tparam t_succinct_tree
 * @tparam t_bvector
 * @tparam t_rank_support
 * @tparam t_select_1_support
 * @tparam t_select_0_support
 */
// wt_hpd: implements the path_query_processor interface
template <
        typename node_type= pq_types::node_type,
        typename size_type= pq_types::size_type,
        typename value_type= pq_types::value_type,
        typename t_succinct_tree= bp_tree_sada<node_type,size_type>,
        typename t_bvector = sdsl::bit_vector,
        typename t_rank_support= typename t_bvector::rank_1_type,
        typename t_select_1_support = typename t_bvector::select_1_type,
        typename t_select_0_support = typename t_bvector::select_0_type
        >
class wt_hpd:
		public path_query_processor<node_type,size_type,value_type>,
	 	public succinct_tree<node_type,size_type>,
	 	public path_decomposer<node_type,size_type> {
public:

	using cwt_int= sdsl::wt_int<t_bvector,t_rank_support,t_select_1_support,t_select_0_support>;

protected:
    //rank_space_reducer<value_type> *reducer= nullptr;
	size_type m= 0;
	std::unique_ptr<succinct_tree<node_type,size_type>> original= nullptr,
	                                                    condensed= nullptr;
	std::unique_ptr<cwt_int> wavelet_tree= nullptr;
	std::unique_ptr<t_bvector> B;
	std::unique_ptr<t_select_1_support> BS;

	[[nodiscard]] size_type ref_count( node_type x ) const {
		auto j = BS->select(x+1)+1;
        auto i = (x+2<=condensed->size())?BS->select(x+2):j;
		assert( i >= j );
		return i-j;
	}

	[[nodiscard]] node_type ref( node_type x ) const {
		return is_head_of_chain(x) ? x : condensed->parent(x).value();
	}

	[[nodiscard]] bool is_head_of_chain( node_type x ) const {
		return ref_count(x) > 0;
	}

	/*
	size_type position_in_chain( const node_type x ) const {
		node_type px= ref(x);
		assert( original->is_ancestor(px,x) );
		return B->select(px+1) - px + (original->depth(x) - original->depth(px));
	}*/

	[[nodiscard]] size_type how_many_segments( node_type p, node_type x ) const {
  		auto pp= ref(p);
		size_type sz= 0;
		for ( auto px= ref(x); px != pp; x= original->parent(px).value(), px= ref(x), ++sz ) ;
		return ++sz;
	}

	// half-open segments exclusive of "p" itself
	void get_intervals( node_type p, node_type x, 
						std::vector<std::pair<size_type,size_type>> &res, 
						bool inclusive= false ) const {
		assert( original->is_ancestor(p,x) );
		auto pp= ref(p);
		for ( auto px= ref(x); px != pp; ) {
			res.emplace_back(position_in_chain(px),position_in_chain(x)+1);
			x= original->parent(px).value(), px= ref(x);
		}
		res.emplace_back(position_in_chain(p)+(inclusive?0:1),position_in_chain(x)+1);
	}

	value_type query( std::vector<std::pair<size_type,size_type>> &vec, size_type k ) const {
		//return reducer->recover(static_cast<value_type>(wavelet_tree->range_quantile(vec,k)));
        return static_cast<value_type>(wavelet_tree->range_quantile(vec,k));
	}

	[[nodiscard]] size_type total_length( const std::vector<std::pair<size_type,size_type>> &res ) const {
		size_type ax = 0;
		for ( const auto &p: res ) {
			assert( p.second >= p.first );
			ax += p.second-p.first;
		}
		return ax;
	}

	// TODO: need to add a test to assert
	// that position_in_chain(head_of_chain) == 0
	[[nodiscard]] size_type position_in_chain( node_type x ) const {
		auto px= ref(x);
		assert( original->is_ancestor(px,x) );
		return BS->select(px+1) - px + (original->depth(x) - original->depth(px));
	}

public:

    [[nodiscard]] size_type get_decomposition_length( node_type x, node_type y ) const override {
  		auto z = original->lca(x,y);
		return how_many_segments(z,x)+1+how_many_segments(z,y);
    }

	value_type weight_of( node_type x ) const override {
	    //if ( reducer )
		//    return static_cast<value_type>(reducer->recover((*wavelet_tree)[position_in_chain(x)]));
        return static_cast<value_type>((*wavelet_tree)[position_in_chain(x)]);
	}
    value_type weight( node_type x ) const override { return weight_of(x); }

	[[nodiscard]] size_type size() const override { return m; }

	wt_hpd() {};

	wt_hpd( const std::string &s, const std::vector<value_type> &w ) {
        original= std::make_unique<t_succinct_tree>(s);
		auto bundle= hpd_preprocessor<node_type,size_type>(original.get())();
		auto bv= std::move(std::get<0>(bundle));
        condensed= std::make_unique<t_succinct_tree>(&bv);
		assert( condensed->size() == s.length()/2 );
		B= std::make_unique<t_bvector>(std::get<1>(bundle));
        BS= std::make_unique<t_select_1_support>(B.get());
		std::vector<node_type> chain= std::move(std::get<2>(bundle));
		auto weights= sdsl::int_vector<>(this->m= original->size());
		this->m_sigma= 0;
		for ( auto l= 0; l < original->size();\
		this->m_sigma= std::max(this->m_sigma,(value_type)(weights[l]= w[chain[l]])), ++l ) ;
		++this->m_sigma, wavelet_tree= std::make_unique<cwt_int>();
        //int_vector reduced_weights= int_vector(this->m= original->size());
        //reducer= new rank_space_reducer<>(w,reduced_weights);
		//construct_im(*wavelet_tree,reduced_weights);
        construct_im(*wavelet_tree,weights);
		//m_sigma= static_cast<value_type >(wavelet_tree->sigma);
	}

	value_type
	query( node_type x, node_type y ) const override {
		auto z = original->lca(x,y);
		auto k = original->depth(x)+original->depth(y)+1-2*original->depth(z);
		auto ik= how_many_segments(z,x)+1+how_many_segments(z,y);
		std::vector<std::pair<size_type,size_type>> segments;
		segments.reserve(ik);
		assert( segments.empty() );
		get_intervals(z,x,segments), get_intervals(z,z,segments,true), get_intervals(z,y,segments);
		return query(segments,k>>1);
	}

	size_type
	count( node_type x,  node_type y,
		   value_type a, value_type b ) const override {
		auto z = original->lca(x,y);
        auto ik= how_many_segments(z,x)+1+how_many_segments(z,y);
		std::vector<std::pair<size_type,size_type>> segments;
		segments.reserve(ik);
		assert( segments.empty() );
		get_intervals(z,x,segments), get_intervals(z,z,segments,true), get_intervals(z,y,segments);
		size_type result= 0;
		/*
		auto ra= reducer->successor(a),
		     rb= reducer->predecessor(b);*/
		for ( auto pr: segments ) {
			auto res= wavelet_tree->range_search_2d(pr.first,pr.second-1,a,b,false);
			result+= res.first;
		}
		return result ;
	}

	void
	report( node_type x,  node_type y,
			value_type a, value_type b,
			std::vector<std::pair<value_type,size_type>> &result ) const override {
		auto z = original->lca(x,y);
        auto ik= how_many_segments(z,x)+1+how_many_segments(z,y);
		std::vector<std::pair<size_type,size_type>> segments;
		segments.reserve(ik);
		assert( segments.empty() );
		get_intervals(z,x,segments), get_intervals(z,z,segments,true), get_intervals(z,y,segments);
		result.clear(), result.reserve(count(x,y,a,b));
		/*
		auto ra= reducer->successor(a),
		     rb= reducer->predecessor(b);*/
		for ( auto pr: segments ) {
            auto res= wavelet_tree->range_search_2d(pr.first,pr.second-1,a,b,true);
			//auto res= wavelet_tree->range_search_2d(pr.first,pr.second-1,ra,rb,true);
			// points is a pair "position" and "value" at that position
			// NOTE: in sdsl, the fact that this is pair<value_type,size_type> is confusing
			// it should be other way around. Probably it is a bug.
			for ( auto point: res.second )
				//result.emplace_back(point.first,reducer->recover(point.second));
                result.emplace_back(point.first,point.second);
		}
	}

	value_type selection( node_type x, node_type y, size_type qntl ) const override {
		auto z = original->lca(x,y);
		auto k = original->depth(x)+original->depth(y)+1-2*original->depth(z);
        auto ik= how_many_segments(z,x)+1+how_many_segments(z,y);
		std::vector<std::pair<size_type,size_type>> segments;
		segments.reserve(ik);
		assert( segments.empty() );
		get_intervals(z,x,segments), get_intervals(z,z,segments,true), get_intervals(z,y,segments);
		return query(segments,this->qntl2rnk(k,qntl));
	}

	//succinct_tree implementation
	/*
	[[nodiscard]] double size_in_bytes() const override {
	    return original->size_in_bytes()+condensed->size_in_bytes()+\
				  sdsl::size_in_bytes(*B)+sdsl::size_in_bytes(*BS)+sdsl::size_in_bytes(*wavelet_tree)+\
				  sizeof original + sizeof condensed + sizeof wavelet_tree + sizeof B;
	}
	*/

	[[nodiscard]] std::optional<node_type> parent(node_type x) const override {
		return original->parent(x);
	}

	[[nodiscard]] std::optional<node_type> ancestor(node_type x, size_type i) const override {
		return original->ancestor(x,i);
	}

	[[nodiscard]] std::vector<node_type> children(node_type x) const override {
		return original->children(x);
	}

	[[nodiscard]] node_type lca(node_type x, node_type y) const override {
		return original->lca(x,y);
	}

	[[nodiscard]] size_type depth(node_type x) const override {
		return original->depth(x);
	}

	[[nodiscard]] bool is_ancestor(node_type p, node_type x) const override {
		return original->is_ancestor(p,x);
	}

	[[nodiscard]] bool is_leaf(node_type x) const override {
		return original->is_leaf(x);
	}

	// a method to check the sanity of HPD
	size_type num_segments( node_type x, node_type y ) const {
        auto z = original->lca(x,y);
        return how_many_segments(z,x)+1+how_many_segments(z,y);
	}
};
#endif
