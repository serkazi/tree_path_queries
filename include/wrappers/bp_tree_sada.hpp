//
// Created by sj on 16/12/18.
//
#ifndef SPQ_BP_TREE_SADA_H
#define SPQ_BP_TREE_SADA_H

#include "sdsl/bp_support_sada.hpp"
#include "succinct_tree.hpp"
#include <stack>
#include <memory>
#include "pq_types.hpp"
#include <optional>

template <
          typename node_type= pq_types::node_type,
          typename size_type= pq_types::size_type,
          uint32_t t_sml_blk= 256,
          uint32_t t_med_deg= 32,
          typename t_rank= sdsl::rank_support_v5<>,
          typename t_select= sdsl::select_support_mcl<>
         >
class bp_tree_sada : public succinct_tree<node_type,size_type> {

public:
private:

    sdsl::bp_support_sada<t_sml_blk,t_med_deg,t_rank,t_select> bp_sada;
	std::unique_ptr<sdsl::bit_vector> m_bv= nullptr;

    /*! The position of the opening parenthesis of the node $x$
	 * \param x the pre-order number of the node
	 */
	size_type node2position( const node_type x ) const {
		return bp_sada.select(x+1);
	}
	/*! The pre-order number of the node, whose opening parenthesis is at position i
	 * \param i position of the opening parenthesis
	 */
	node_type position2node( const size_type i ) const {
		return bp_sada.rank(i)-1;
	}

	std::pair<size_type,size_type> interval( const node_type x ) const {
		size_type i = node2position(x);
		return {i,bp_sada.find_close(i)};
	}

public:

	bp_tree_sada( const std::string &s ) {
		auto k= s.size();
		//_sz= 0;
		assert( !(k&1) );
		m_bv= std::make_unique<sdsl::bit_vector>(k,0);
		for ( auto i = 0; i < k; ++i )
			if ( s[i] == '(' )
				(*m_bv)[i]= 1;
		bp_sada = sdsl::bp_support_sada<t_sml_blk,t_med_deg,t_rank,t_select>(m_bv.get());
	}

	bp_tree_sada( sdsl::bit_vector *bp ) {
		//_sz= 0;
		m_bv= std::make_unique<sdsl::bit_vector>(*bp);
		bp_sada = sdsl::bp_support_sada<t_sml_blk,t_med_deg,t_rank,t_select>(m_bv.get());
	}

	node_type lca( node_type x, node_type y ) const {
		if ( is_ancestor(x,y) )
			return x;
		if ( is_ancestor(y,x) )
			return y;
		auto ix = interval(x),
			 iy = interval(y);
		assert( ix.second < iy.first || iy.second < ix.first );
		if ( ix.second < iy.first ) {
	label01:
			return position2node(bp_sada.double_enclose(ix.first,iy.first));
		}
		swap(ix,iy);
		goto label01;
	}

	// tree info
	size_type size() const {
		return bp_sada.size()>>1;
	}

	// size_in_bytes(bp_sada) does not take into consideration the backbone bitvector
	// now I fixed it
	[[nodiscard]] double size_in_bytes() const {
		return sdsl::size_in_bytes(bp_sada)+sdsl::size_in_bytes(*m_bv);
	}

	// navigation
	std::optional<node_type> parent( node_type x ) const override {
		auto pos = node2position(x);
		if ( pos == 0 )
			return std::nullopt;
		//assert( bp_sada.is_opening(pos) );
		return position2node( bp_sada.enclose( node2position(x) ) );
	}

	// FIXME: we never call this level ancestor; probably remove?
	std::optional<node_type> ancestor( node_type x, size_type i ) const override {
		if ( x == 0 ) return i == 0 ? std::optional<node_type>(x) : std::nullopt;
		return position2node(bp_sada.enclose(node2position(x),i));
	}

	std::vector<node_type> children( node_type x ) const override {
		std::vector<node_type> res;
		auto ix= interval(x);
		assert (res.empty());
		for ( auto i= ix.first; i+1 < bp_sada.size() && bp_sada.is_opening(i+1); i= bp_sada.find_close(i+1) )
			res.push_back(position2node(i+1));
		assert( is_leaf(x) || !res.empty() );
		return res;
	}

	size_type depth( node_type x ) const override {
		auto pos = node2position(x);
		if ( pos == 0 )
			return 0;
		return static_cast<size_type>(bp_sada.excess(pos)-1);
	}

	// predicates
	bool is_leaf( node_type x ) const override {
		auto i = node2position(x);
		return bp_sada.find_close(i) == i+1;
	}

	bool is_ancestor( node_type x, node_type y ) const override {
		auto ix = interval(x),
			 iy = interval(y);
		return ix.first <= iy.first and iy.second <= ix.second;
	}

	virtual ~bp_tree_sada() = default;

	// bp_tree ----> balanced parenthesis string
	// we want to do this using explicit stack
	// ostringstream derives from ostream
	friend std::ostream &operator << ( std::ostream &ofs, const bp_tree_sada &t ) {
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
#endif //SPQ_BP_TREE_SADA_H
