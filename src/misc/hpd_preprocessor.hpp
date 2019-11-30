/*
 */
#ifndef PLAIN_TREE
#define PLAIN_TREE
#include <cassert>
#include "sdsl/int_vector.hpp"
#include "succinct_tree.hpp"
#include "pq_types.hpp"
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <stack>
#include <memory>

template<
        typename node_type,
        typename size_type
        >
class hpd_preprocessor {
private:
    //"dynamically-growing" tree, needed to store
    // a tree that is being constructed,
    // with subsequently obtaining its BP-sequence
    class plain_tree {
    private:
        std::map<node_type,std::set<node_type>> adj;
        bool str_repr= false;
        size_type num_of_edges= 0;
        sdsl::bit_vector bv;
        void dfs( node_type x, size_type &cur ) {
            bv[cur++]= true;
            for ( auto y: adj[x] )
                dfs(y,cur);
            bv[cur++]= false;
        }
    public:
        plain_tree() { adj.clear(); }
        inline void add_arc( node_type x, node_type y ) {
            if ( not adj.count(x) )
                adj[x].clear();
            assert( not adj[x].count(y) );
            adj[x].insert(y), ++num_of_edges;
        }
        explicit operator sdsl::bit_vector() {
            if ( !str_repr ) {
                bv= sdsl::bit_vector(2*(nedges()+1),0);
                size_type cur= 0;
                dfs(0,cur);
                assert( cur == 2*size() );
                str_repr= true ;
            }
            return bv;
        }
        inline size_type size() const {
            return adj.size();
        }
        inline size_type nedges() const {
            return num_of_edges;
        }
        virtual ~plain_tree() = default;
    };

	const succinct_tree<node_type,size_type> *T;
	mutable std::unique_ptr<size_type[]> which_chain;
	std::unique_ptr<node_type[]> head_of_chain;
	std::unique_ptr<size_type[]> len;
	std::unique_ptr<std::optional<node_type>[]> best_son;
	size_type n,num_chains;

	size_type dfs( node_type x ) {
		size_type c= 1, cc, bc= 0;
		assert( T );
		assert( T->size() > x );
		auto children= T->children(x);
		best_son[x]= n;
		for ( auto y: children ) {
			c+= (cc= dfs(y));
			if ( cc > bc )
				bc= cc, best_son[x]= y;
		}
		return c;
	}

	void dfs_explicit_stack( const node_type src ) {
	    node_type x,y,z;
	    size_type i,j,k,processed_nodes= 1;
	    std::stack<node_type> node_stack;
	    std::stack<size_type> child_idx_stack;

	    for (;not node_stack.empty(); node_stack.pop() ) ;
        for (;not child_idx_stack.empty(); child_idx_stack.pop() ) ;

        std::unique_ptr<bool[]> seen= std::make_unique<bool[]>(n);
        for ( x= 0; x < n; seen[x++]= false ) ;
        std::unique_ptr<size_type[]> card= std::make_unique<size_type[]>(n);
        std::unique_ptr<node_type[]> papa= std::make_unique<node_type[]>(n);

        for ( x= 0; x < n; card[x]= 1, best_son[x++]= std::nullopt ) ;

        for ( seen[src]= true, node_stack.push(src), child_idx_stack.push(0); not node_stack.empty(); ) {
            x= node_stack.top(), node_stack.pop(), i= child_idx_stack.top(), child_idx_stack.pop();
            auto children= T->children(x); //we rely on the property that ``children'' are always returned in the same order
            for ( ;i < (int)children.size() and seen[children[i]]; ++i ) ;
            if ( i == (int)children.size() ) {
                if ( x != src ) {
                    if ( (not best_son[z= papa[x]].has_value()) or card[best_son[z].value()] < card[x] )
                        best_son[z]= std::optional<node_type>(x);
                    card[z]+= card[x];
                }
                continue;
            }
            papa[y= children[i]]= x, seen[y]= true, ++processed_nodes;
            node_stack.push(x), child_idx_stack.push(i+1), node_stack.push(y), child_idx_stack.push(0);
        }
        assert( processed_nodes == n );
	}

	void hld( node_type x, bool is_start= false ) {
		which_chain[x]= num_chains-1;
		if ( is_start )
			head_of_chain[num_chains-1]= x;
		if ( best_son[x] < n ) 
			hld(best_son[x]);
		auto children= T->children(x);
		for ( auto y: children )
			if ( y != best_son[x] )
				++num_chains, hld(y,true);
	}

	std::optional<size_type> find_best_son_idx( const node_type x ) {
	    if ( not best_son[x].has_value() )
	        return std::nullopt;
	    auto children= T->children(x);
	    for ( auto i= 0; i < children.size(); ++i )
	        if ( children[i] == best_son[x].value() )
	            return std::optional<size_type>(i);
        return std::nullopt;
	}

	void hld_explicit_stack( const node_type src ) {
	    node_type x,y,z;
	    size_type i,j,k,l,processed_nodes= 0;
	    std::stack<node_type> node_stack;
	    std::stack<size_type> child_idx_stack;
	    std::stack<bool> is_start;

	    for (;not node_stack.empty(); node_stack.pop() ) ;
        for (;not child_idx_stack.empty(); child_idx_stack.pop() ) ;
        for (;not is_start.empty(); is_start.pop() ) ;

	    auto seen= std::make_unique<bool[]>(n);
	    for ( x= 0; x < n; seen[x++]= false ) ;

	    which_chain[src]= num_chains-1, seen[src]= true, node_stack.push(src), is_start.push(true);
	    auto best_son_id= find_best_son_idx(src);
	    child_idx_stack.push(best_son_id.has_value()?best_son_id.value():0);
	    ++processed_nodes;

	    while ( not node_stack.empty() ) {
	        x= node_stack.top(), node_stack.pop(), i= child_idx_stack.top(), child_idx_stack.pop();
	        auto flag= is_start.top(); is_start.pop();
	        if ( flag )
	            head_of_chain[which_chain[x]]= x;

	        auto children= T->children(x);
	        auto sz= children.size();

	        for ( l= 0; l < (int)children.size() and seen[y= children[i]]; ++l, ++i, i%= sz ) ;
            if ( l == (int)children.size() ) continue ;

            node_stack.push(x), child_idx_stack.push(0), is_start.push(false);
            seen[y]= true, node_stack.push(y), best_son_id= find_best_son_idx(y);
	        child_idx_stack.push(best_son_id.has_value()?best_son_id.value():0);
            ++processed_nodes;

	        if ( best_son[x].has_value() and y == best_son[x].value() )
	            which_chain[y]= which_chain[x], is_start.push(false);
	        else
                which_chain[y]= (++num_chains)-1, is_start.push(true);
	    }
	    assert( is_start.empty() );
	    assert( child_idx_stack.empty() );
	    assert( processed_nodes == n );
	}

	node_type ref( node_type x ) const {
		return head_of_chain[which_chain[x]];
	}

public:

	explicit hpd_preprocessor( const succinct_tree<node_type,size_type> *t ) {
		assert( t );
		n= (T= t)->size();
		best_son= std::make_unique<std::optional<node_type>[]>(n);
		which_chain= std::make_unique<size_type[]>(n);
		head_of_chain= std::make_unique<node_type[]>(n);
	};

	std::tuple<sdsl::bit_vector, sdsl::bit_vector, std::vector<node_type>>
	operator()() {

		//dfs(0), num_chains= 1, hld(0,true);
        dfs_explicit_stack(0), num_chains= 1, hld_explicit_stack(0);
        //dfs(0), num_chains= 1, hld_explicit_stack(0);

		size_type i,j,k,ch;
		node_type x,y;

		len= std::make_unique<size_type[]>(num_chains);
		for ( ch= 0; ch < num_chains; len[ch++]= 0 ) ;
		for ( x= 0; x < n; ++len[which_chain[x++]] ) ;

		plain_tree pt{};
		for ( x= 1; x < n; pt.add_arc(ref(T->parent(x).value()),x), ++x ) ;
		assert( pt.nedges()+1 == n );

		auto B= sdsl::bit_vector(2*n,0);
		for ( k= 0, x= 0; x < n; ++x ) {
			B[k++]= 1, ch= which_chain[x];
			if ( head_of_chain[ch] == x and (k+= len[ch]) ) ;
		}

		assert( k == 2*n );
		std::vector<node_type> rchain(n,0);
		auto counts= std::make_unique<size_type[]>(n);
		for ( x= 0; x < n; counts[x++]= 0 ) ;
		for ( x= 0; x < n; ++counts[ref(x++)] ) ;
		for ( x= 1; x < n; counts[x]+= counts[x-1], ++x ) ;
		for ( int z= n-1; z >= 0; --z ) {
			auto nz= static_cast<node_type>(z);
			assert( 0 <= nz && nz < n );
			assert( counts[ref(nz)] );
			rchain[--counts[ref(nz)]]= nz;
		}
		return std::make_tuple(sdsl::bit_vector(pt),B,rchain);
	}

	virtual ~hpd_preprocessor() = default;
};
#endif
