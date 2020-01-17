#ifndef NAIVE_SUCCINCT_RANDOMIZED_SELECT
#define NAIVE_SUCCINCT_RANDOMIZED_SELECT

#include <cstdio>
#include <cassert>
#include <cmath>
#include <vector>
#include <random>
#include "bp_trees.hpp"
#include "path_query_processor.hpp"
#include "succinct_tree.hpp"
#include "naive_processor_lca.hpp"
#include "pq_types.hpp"

/**
 * @brief nsrs stands for naive-succinct- with randomized-select
 * @tparam node_type
 * @tparam size_type
 * @tparam value_type
 * @tparam t_succinct_tree
 */
template<
        typename node_type= pq_types::node_type,
        typename size_type= pq_types::size_type,
        typename value_type= pq_types::value_type,
        typename t_succinct_tree= bp_tree_gg<node_type,size_type>>
class nsrs: public naive_processor_lca<node_type,size_type,value_type> {

private:

	std::unique_ptr<t_succinct_tree> T;
	std::unique_ptr<sdsl::int_vector<0>> weights;
    uint8_t t_width;

public:

	nsrs( const std::string &s, const std::vector<value_type> &w ) {
		this->m_sigma= *(std::max_element(w.begin(),w.end()))+1;
        t_width= static_cast<uint8_t>(std::ceil(log2(this->m_sigma+1e-7)+1e-3));
		weights= std::make_unique<sdsl::int_vector<>>(s.length()/2,0,t_width);
		this->n= (this->T= std::make_unique<t_succinct_tree>(s))->size();
		for ( auto i= 0; i < this->n; (*weights)[i]= w[i], ++i ) ;
	}
	
	std::optional<node_type> parent( node_type x ) const override {
		if ( x == 0 ) return std::nullopt;
		return std::optional<node_type>(T->parent(x));
	}
	std::vector<node_type> children( node_type x ) const override {
		return T->children(x);
	}
	std::optional<node_type> ancestor( node_type x, size_type i ) const override {
		return T->ancestor(x,i);
	}
	node_type lca( node_type cx, node_type cy ) const override {
		return T->lca(cx,cy);
	}
	size_type depth( node_type x ) const override {
		return T->depth(x);
	}
	bool is_ancestor( node_type p, node_type x ) const override {
		return T->is_ancestor(p,x);
	}
	bool is_leaf( node_type x ) const override {
		return T->is_leaf(x);
	}

	/*
	[[nodiscard]] double size_in_bytes() const override {
		return sizeof(t_succinct_tree *) + T->size_in_bytes()+sdsl::size_in_bytes(*weights);
	}
    */
	value_type weight_of(node_type x) const override { return (*weights)[x]; }
	value_type weight(node_type x) const override { return weight_of(x); }

	virtual ~nsrs() = default;
};
#endif //NAIVE_SUCCINCT_RANDOMIZED_SELECT
