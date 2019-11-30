#ifndef NAIVE_PROCESSOR
#define NAIVE_PROCESSOR
#include "succinct_tree.hpp"
#include "path_query_processor.hpp"
#include <iostream>
#include <algorithm>
#include <cassert>
#include <stack>
#include <vector>
#include <random>
#include <cmath>
#include <memory>
#include "pq_types.hpp"
#include <optional>

/**
 * //TODO format nicely in doxygen format
 * Randomized_Partition(A, l, u)
 * @\li Choose a random index i from [l..u]. Assign p ← A[i].
 * @\li Rearrange the elements of A[l..u] so that there exists an integer j s.t.
 *	    elements in A[l..j-1] are <= p,
 *			     A[j] = p (i.e. p is moved from A[i] to A[j]), and
 *						     elements in A[j+1..u] are >= p.
 * @\li return j
 * Randomized_Select(A, l, u, i) // find the ith smallest element from A[l..u]
 * if l = r then
 *  return A[l]
 *  p ← Randomized_Partition(A, l, u)
 *  k ← p - l + 1 // note that a[p] is the k-th smallest element in A[l..u]
 *  if k = i then
 *    return A[p]
 *    else if i < k then
 *           return Randomized_Select(A, l, p-1, i)
 *                else
 *                       return Randomized_Select(A, p+1, u, i-k)
 * @tparam node_type
 * @tparam size_type
 * @tparam value_type
 */
template<
        typename node_type= pq_types::node_type,
        typename size_type= pq_types::size_type,
        typename value_type= pq_types::value_type
        >
class naive_processor:
		public succinct_tree<node_type,size_type>,
		public path_query_processor<node_type,size_type,value_type> {
protected:

	enum {
		ROOT = 0
	};

	size_type n;
	std::unique_ptr<size_type[]> d= nullptr;
	std::unique_ptr<node_type[]> p = nullptr;
	std::unique_ptr<std::vector<pq_types::node_type>[]> adj = nullptr;
	std::vector<value_type> weights;

    int find_length(const node_type x, const node_type y) const {
	    auto z= lca(x,y);
	    int res= 0;
	    for (auto cur = x; cur != z; ++res, cur = parent(cur).value() );
		for (auto cur = y; cur != z; ++res, cur = parent(cur).value() );
		return 1+res;
	}

public:

	naive_processor() = default;

	naive_processor( const std::string &s, const std::vector<value_type> &w ) {
		int i, j, k, x, y;
		node_type V = 0;
		assert( not (s.size() & 1) );
		n = s.size() / 2;
		adj = std::make_unique<std::vector<node_type>[]>(n);
		p = std::make_unique<node_type[]>(n);
		d = std::make_unique<size_type[]>(n);
		assert(w.size() >= n);
		for (weights.clear(), weights.reserve(n), i = 0; i < n; weights.push_back(w[i]), adj[i++].clear());
		std::stack<node_type> st;
		assert(st.empty());
		for ( char ch: s ) {
			if ( ch == '(' ) {
				if ( not st.empty() )
					adj[st.top()].push_back(V), p[V] = st.top();
				d[V] = static_cast<size_type>(st.size()), st.push(V++);
				continue;
			}
			st.pop();
		}
		assert(V == n);
		this->m_sigma= *(std::max_element(w.begin(),w.end()))+1;
	}

	// tree info
	size_type size() const override {
		return n;
	}

	// navigation
	std::optional<node_type> parent(node_type x) const override {
		if ( x == 0 ) return std::nullopt;
		return std::optional<node_type>(p[x]);
	}

	// FIXME
	std::optional<node_type> ancestor(node_type x, size_type i) const override {
		return std::nullopt;
	}

	std::vector<node_type> children(node_type x) const override {
		return adj[x];
	}

	node_type lca(node_type cx, node_type cy) const override {
		node_type x = cx, y = cy;
    	for (;d[x] > d[y]; x= parent(x).value() );
		for (;d[x] < d[y]; y= parent(y).value() );
		assert(d[x] == d[y]);
		for (; x != y; x = parent(x).value(), y = parent(y).value());
		return x;
	}

	size_type depth(node_type x) const override {
		return d[x];
	}

	// predicates
	// FIXME
	bool is_ancestor(node_type p, node_type x) const override {
		//return in[p] <= in[x] && out[x] <= out[p];
		return false;
		//throw new NotImplemented();
	}

	bool is_leaf(node_type x) const override {
		return adj[x].empty();
	}

	value_type
	query(node_type x, node_type y) const override {
		auto z = lca(x, y);
		//std::vector<value_type> path(d[x]+d[y]+2-2*d[z]);
		//std::vector<value_type> path(d[x]+d[y]+2-2*d[z],0);
		std::vector<value_type> path;
		path.clear(), path.reserve(find_length(x,y));
		for (auto cur = x; cur != z; path.emplace_back(weight_of(cur)), cur = parent(cur).value());
		for (auto cur = y; cur != z; path.emplace_back(weight_of(cur)), cur = parent(cur).value());
		path.emplace_back(weight_of(z));
		/*std::sort(path.begin(),path.end());
		for ( auto p: path )
			std::cout << p << " ";
		std::cout << std::endl;
		return path[path.size()/2];*/
		std::nth_element(path.begin(),path.begin()+path.size()/2,path.end());
		return path[path.size()/2];
	}

	size_type count(node_type x,
					node_type y,
					value_type a,
					value_type b) const override {
		auto z = lca(x, y);
		size_type res = 0;
		for (auto cur = x; cur != z; res+= this->lies_inside(weight_of(cur), a, b) ? 1 : 0, cur = parent(cur).value() );
		for (auto cur = y; cur != z; res+= this->lies_inside(weight_of(cur), a, b) ? 1 : 0, cur = parent(cur).value() );
		if ( this->lies_inside(weight_of(z), a, b) ) ++res;
		return res;
	}

	void report(const node_type x,
				const node_type y,
				const value_type a,
				const value_type b,
				std::vector<std::pair<value_type, size_type>> &res) const override {
		auto z = this->lca(x, y);
		res.clear();
		for (auto cur = x; cur != z; cur = parent(cur).value()) {
			if (this->lies_inside(weight_of(cur), a, b))
				res.emplace_back(cur, weight_of(cur));
		}
		for (auto cur = y; cur != z; cur = parent(cur).value()) {
			if (this->lies_inside(weight_of(cur), a, b))
				res.emplace_back(cur, weight_of(cur));
		}
		if (this->lies_inside(weight_of(z), a, b))
			res.emplace_back(z, weight_of(z));
	}

	value_type selection( node_type x, node_type y, size_type qntl ) const override {
   		auto z = lca(x, y);
		std::vector<value_type> path;
		assert( path.empty() );
		path.reserve(find_length(x,y));
		for ( auto cur= x; cur != z; path.emplace_back(weight_of(cur)), cur= parent(cur).value() );
		for ( auto cur= y; cur != z; path.emplace_back(weight_of(cur)), cur= parent(cur).value() );
		path.emplace_back(weight_of(z));
		auto k= this->qntl2rnk(path.size(),qntl);
		std::nth_element(path.begin(),path.begin()+k,path.end());
		return path[k];
	}

	// TODO
	[[nodiscard]] double size_in_bytes() const override {
	    /*
		double ans = size() * (sizeof 0[p] + sizeof 0[d]) + \
                sizeof(std::vector<value_type>) + (sizeof(value_type) * weights.size());
		ans+= sizeof(std::vector<value_type> *);
		for ( auto i= 0; i < n; ++i )
			ans+= sizeof(std::vector<value_type>) + adj[i].size()*sizeof(value_type);
		return ans;
	     */
	    return 0;
	}

	value_type weight_of(const node_type x) const override { return weights[x]; }
	value_type weight(const node_type x) const override { return weight_of(x); }

	~naive_processor() override = default;
};

#endif
