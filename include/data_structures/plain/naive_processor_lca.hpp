#ifndef NAIVE_TREE_LCA_INCLUDED
#define NAIVE_TREE_LCA_INCLUDED
#include "naive_processor.hpp"
#include "bender_farach_colton.hpp"

/**
 * @details this class is identical to naive_processor, but
 * has a fast LCA built-in
 * @tparam node_type
 * @tparam size_type
 * @tparam value_type
 */
template<
        typename node_type= pq_types::node_type,
        typename size_type= pq_types::size_type,
        typename value_type= pq_types::value_type
>
class naive_processor_lca: public naive_processor<node_type,size_type,value_type> {

private:

	std::unique_ptr<lca_processor<node_type,size_type>> prc= nullptr;
	using naive_processor<node_type,size_type,value_type> :: p;
	using naive_processor<node_type,size_type,value_type> :: d;
	using naive_processor<node_type,size_type,value_type> :: adj;

public:
	naive_processor_lca() = default;
    naive_processor_lca( const std::string &s, const std::vector<value_type> &w )
    : naive_processor<node_type,size_type,value_type>(s,w) {
        prc= std::make_unique<lca_processor<node_type,size_type>>(this);
    }
	node_type lca( node_type cx, node_type cy ) const override {
		return (*prc)(cx,cy);
	}
	/*
	[[nodiscard]] double size_in_bytes() const override {
		auto ans= naive_processor<node_type,size_type,value_type>::size_in_bytes();
		ans+= prc->size_in_bytes();
		return ans;
	}
	*/
	~naive_processor_lca() override = default;
};
#endif
