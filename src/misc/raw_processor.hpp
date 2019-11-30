#ifndef RAW_TREE_INCLUDED
#define RAW_TREE_INCLUDED

#include "path_query_processor.hpp"
#include "simple_tree.hpp"
#include <algorithm>
#include <cassert>
#include <vector>
#include <memory>

/**
 * @brief raw path query processor, for the purpose of testing other processors
 * @details stores the original weighted tree internally, and answers the queries in the most naive way
 * @tparam node_type
 * @tparam size_type
 * @tparam value_type
 */
template<typename node_type,typename size_type,typename value_type>
class raw_processor: public path_query_processor<node_type,size_type,value_type> {
private:
    std::unique_ptr<tree<node_type,size_type,value_type>> T;
public:
    raw_processor(const std::string &s, const std::vector<value_type> &w ) {
        T= std::make_unique<tree<node_type,size_type,value_type>>(s,w);
    }
    virtual ~raw_processor() = default;
    value_type query( node_type x, node_type y ) const {
		std::vector<value_type> path;
		path.clear();
		auto z = T->lca(x,y);
		for ( auto cur= x; cur != z; path.push_back(T->weight(cur)), cur= T->parent(cur) );
		for ( auto cur= y; cur != z; path.push_back(T->weight(cur)), cur= T->parent(cur) );
		path.push_back(T->weight(z));
		std::sort(path.begin(),path.end());
		return path[path.size()/2];
	}
	value_type selection( node_type x, node_type y, size_type qntl ) const override {
		std::vector<value_type> path;
		assert( path.empty() );
		auto z = T->lca(x,y);
		for ( auto cur= x; cur != z; path.push_back(T->weight(cur)), cur= T->parent(cur) );
		for ( auto cur= y; cur != z; path.push_back(T->weight(cur)), cur= T->parent(cur) );
		path.push_back(T->weight(z));
		std::sort(path.begin(),path.end());
		return path[this->qntl2rnk(path.size(),qntl)];
	}
	size_type
	count( const node_type x, const node_type y, 
		   const value_type a, const value_type b ) const {
		std::vector<value_type> path;
		path.clear();
		auto z= T->lca(x,y);
		size_type res= 0;
		for ( auto cur= x; cur != z; path.push_back(T->weight(cur)), cur= T->parent(cur) );
		for ( auto cur= y; cur != z; path.push_back(T->weight(cur)), cur= T->parent(cur) );
		path.push_back(T->weight(z));
		for ( auto val: path )
		    if ( path_query_processor<node_type,size_type,value_type>::lies_inside(val,a,b) )
				++res;
		return res;
	}
	void
	report( node_type x, node_type y,
			value_type a, value_type b,
			std::vector<std::pair<value_type,size_type>> &result ) const {
		std::vector<size_type> path;
		auto z = T->lca(x,y);
		for ( auto cur = x; cur != z; path.push_back(cur), cur = T->parent(cur) );
		for ( auto cur = y; cur != z; path.push_back(cur), cur = T->parent(cur) );
		path.push_back(z);
		for ( auto idx: path )
		    if ( path_query_processor<node_type,size_type,value_type>::lies_inside(T->weight(idx),a,b) )
				result.emplace_back(idx,T->weight(idx));
	}

    value_type weight_of(node_type x) const override {
        return T->weight(x);
    }

    value_type weight(node_type x) const override {
        return T->weight(x);
    }
};
#endif
