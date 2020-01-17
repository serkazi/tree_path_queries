//
// Created by sj on 16/12/18.
//

#ifndef SPQ_BP_TREE_GG_H
#define SPQ_BP_TREE_GG_H

#include "sdsl/bp_support_gg.hpp"
#include "succinct_tree.hpp"
#include <stack>
#include <memory>
#include "pq_types.hpp"
#include <optional>

template <
		  typename node_type= pq_types::node_type,
          typename size_type= pq_types::size_type,
		  typename t_nnd= sdsl::nearest_neighbour_dictionary<30>,
          typename t_rank= sdsl::rank_support_v5<>,
          typename t_select= sdsl::select_support_mcl<>,
          uint32_t t_bs= 840
		 >
class bp_tree_gg: public succinct_tree<node_type,size_type> {

public:
private:

    sdsl::bp_support_gg<t_nnd,t_rank,t_select,t_bs> bpsupport;
	std::unique_ptr<sdsl::bit_vector> m_bv= nullptr;

    /*! The position of the opening parenthesis of the node $x$
	 * \param x the pre-order number of the node
	 */
	size_type node2position( const node_type x ) const {
        return bpsupport.select(x+1);
	}
	/*! The pre-order number of the node, whose opening parenthesis is at position i
	 * \param i position of the opening parenthesis
	 */
	node_type position2node( const size_type i ) const {
		return bpsupport.rank(i)-1;
	}

	std::pair<size_type,size_type> interval( const node_type x ) const {
		size_type i = node2position(x);
		return {i,bpsupport.find_close(i)};
	}

public:

	bp_tree_gg( const std::string &s ) {
		auto k= s.size();
		assert( !(k&1) );
		m_bv= std::make_unique<sdsl::bit_vector>(k,0);
		for ( auto i = 0; i < k; ++i )
			if ( s[i] == '(' )
				(*m_bv)[i]= 1;
		bpsupport = sdsl::bp_support_gg<t_nnd,t_rank,t_select,t_bs>(m_bv.get());
	}

	bp_tree_gg( const sdsl::bit_vector *bp ) {
		m_bv= std::make_unique<sdsl::bit_vector>(*bp);
		bpsupport = sdsl::bp_support_gg<t_nnd,t_rank,t_select,t_bs>(m_bv.get());
	}

	node_type lca( node_type x, node_type y ) const override {
		if ( is_ancestor(x,y) )
			return x;
		if ( is_ancestor(y,x) )
			return y;
		auto ix = interval(x),
			 iy = interval(y);
		assert( ix.second < iy.first || iy.second < ix.first );
		if ( ix.second < iy.first ) {
	label01:
			return position2node(bpsupport.double_enclose(ix.first,iy.first));
		}
		swap(ix,iy);
		goto label01;
	}

	// tree info
	size_type size() const override {
		return bpsupport.size()>>1;
	}

	// size_in_bytes(bpsupport) does not take into consideration the backbone bitvector
	// now I fixed it
	/*
	[[nodiscard]] double size_in_bytes() const {
		return sdsl::size_in_bytes(bpsupport)+sdsl::size_in_bytes(*m_bv);
	}
	*/

	// navigation
	std::optional<node_type> parent( node_type x ) const override {
		auto pos = node2position(x);
		if ( pos == 0 )
			return std::nullopt;
		//assert( bpsupport.is_opening(pos) );
		return position2node( bpsupport.enclose( node2position(x) ) );
	}

	// FIXME: we never call this level ancestor; probably remove? we keep it around since
	//  it can be supported out of the box
	// besides, not all other succinct_tree implementations support this at no additional cost as here
	std::optional<node_type> ancestor( node_type x, size_type i ) const override {
		/*
		if ( x == 0 ) return i == 0 ? std::optional<node_type>(x) : std::nullopt;
		return position2node(bpsupport.enclose(node2position(x),i));
		*/
		//TODO: check if bp_support_gg indeed does not have enclose with two parameters, as bp_support_sada
		return std::nullopt;
	}

	std::vector<node_type> children( node_type x ) const override {
		std::vector<node_type> res;
		auto ix= interval(x);
		assert (res.empty());
		for ( auto i= ix.first; i+1 < bpsupport.size() && bpsupport.is_opening(i+1); i= bpsupport.find_close(i+1) )
			res.push_back(position2node(i+1));
		assert( is_leaf(x) || !res.empty() );
		return res;
	}

	size_type depth( node_type x ) const override {
		auto pos = node2position(x);
		if ( pos == 0 )
			return 0;
		return static_cast<size_type>(bpsupport.excess(pos)-1);
	}

	// predicates
	bool is_leaf( node_type x ) const override {
		auto i = node2position(x);
		return bpsupport.find_close(i) == i+1;
	}

	bool is_ancestor( node_type x, node_type y ) const override {
		auto ix = interval(x),
			 iy = interval(y);
		return ix.first <= iy.first and iy.second <= ix.second;
	}

	virtual ~bp_tree_gg() = default;

	// bp_tree ----> balanced parenthesis string
	// we want to do this using explicit stack
	// ostringstream derives from ostream
	friend std::ostream &operator << ( std::ostream &ofs, const bp_tree_gg &t ) {
	    std::stack<std::pair<node_type,bool>> st;
	    for ( st.push({0,true}); !st.empty(); ) {
	    	auto pr= st.top(); st.pop();
	    	if ( pr.second ) {
	    		ofs << "(";
	    		auto children= t.children(pr.first);
	    		st.push({pr.first,false});
	    		for ( int l= ((int)children.size())-1; l >= 0; --l )
	    			st.push({children[l],true});
	    	}
	    	else {
	    		ofs << ")";
	    	}
	    }
	    return ofs;
	}
};
#endif //SPQ_BP_TREE_GG_H
