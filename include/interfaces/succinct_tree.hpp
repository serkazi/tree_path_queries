#ifndef SUCCINCT_TREE
#define SUCCINCT_TREE
#include "pq_types.hpp"

// removed these so that we won't have
// dependency on sdsl
/*#include "sdsl/int_vector.hpp"
#include "sdsl/rank_support.hpp"
#include "sdsl/select_support.hpp"
#include "sdsl/bp_support_algorithm.hpp"
#include "sdsl/fast_cache.hpp"*/
#include <vector>
#include <stack>
#include <map>
#include <set>
#include <utility>
#include <stdexcept>
#ifndef NDEBUG
#include <algorithm>
#endif
#include <iostream>
#include "pq_types.hpp"
#include <optional>

/**
 * @brief the tree does not have to be succinct per se; represents a tree topology with navigation
 * @details @code parent(), ancestor() @endcode methods can return @code std::nullopt @endcode
 * @tparam node_type
 * @tparam size_type
 */
template<typename node_type= pq_types::node_type ,typename size_type= pq_types::size_type>
class succinct_tree {
public:

	// tree info
	virtual size_type size() const = 0;
	[[nodiscard]] virtual double size_in_bytes() const = 0;

	// navigation
	virtual std::optional<node_type> parent( node_type x ) const = 0;
	virtual std::optional<node_type> ancestor( node_type x, size_type i ) const = 0;
	/**
	 * @note we don't make this to return const-reference, because
	 * most of the time the list of children is created from scratch anyway
	 * @param x
	 * @return
	 */
	virtual std::vector<node_type> children( node_type x ) const = 0;
    virtual node_type lca( node_type x, node_type y ) const = 0;
	virtual size_type depth( node_type x ) const = 0;

	// predicates
	virtual bool is_ancestor( node_type p, node_type x ) const = 0;
	virtual bool is_leaf( node_type x ) const = 0;

	virtual ~succinct_tree() = default;
};

#endif
