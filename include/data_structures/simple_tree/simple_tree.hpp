#ifndef TREE_PATH_QUERIES_SIMPLE_TREE_HPP
#define TREE_PATH_QUERIES_SIMPLE_TREE_HPP

#include <memory>
#include <queue>
#include <stack>
#include <cassert>
#include <optional>
#include <succinct_tree.hpp>

/**
 * @brief simple tree that supports all basic queries in \f$O(1)\f$ time, and LCA in \f$ O(\log{n}) \f$ time
 * @details space \f$ O(n\log{n})\f$ words, since we use the well-known power-of-two ancestors method; traversing
 * children are not supported once @code shed_redundancy() @endcode is called
 * @tparam node_type
 * @tparam size_type
 * @tparam value_type
 */
template<
        typename node_type,
        typename size_type,
        typename value_type
        >
struct tree: public succinct_tree<node_type,size_type> {

    size_type n,K,E= 0;
    const node_type root= 0;

	std::vector<std::optional<size_type>> last,next_;
	std::vector<node_type> to;

    std::vector<std::optional<node_type>> p;
    std::unique_ptr<value_type[]> w;
    std::vector<size_type> d;
    std::vector<std::vector<std::optional<node_type>>> anc;

    // as we are traversing the tree upwards only, after
    // finding the HPD we no longer need children info, hence we delete it
    // we also don't need parents, since they are effectively served by anc[x][0]
    void shed_redundancy() ;
    std::optional<node_type> parent( node_type x ) const override;
    size_type depth( node_type x ) const override;
    value_type weight( node_type x ) const ;

    std::optional<node_type> up( node_type x, unsigned int u ) const ;
    node_type lca( node_type x, node_type y ) const override;
    bool is_leaf( node_type x ) const override;

    void preprocess() ;

    size_t size() const override;

    void init( const std::string &s, const std::vector<value_type> &weights ) ;

    explicit tree( std::istream &is ) ;

    tree( const std::string &s, const std::vector<value_type> &weights ) ;

    virtual ~tree() ;
    virtual std::vector<node_type> kinder( node_type x ) const ;

    // delete the copy constructor
    tree<node_type,size_type,value_type>( const tree<node_type,size_type,value_type> &other ) = default;
    // delete the copy assignment
    tree<node_type,size_type,value_type>& operator = ( const tree<node_type,size_type,value_type> &other ) = delete;
    // move-constructor
    tree<node_type,size_type,value_type>( tree<node_type,size_type,value_type> &&other ) noexcept = delete ;
    // move-assignment
    tree<node_type,size_type,value_type>& operator = ( tree<node_type,size_type,value_type> &&other ) noexcept = delete;

    std::optional<node_type> ancestor(node_type x, size_type i) const override;

    std::vector<node_type> children(node_type x) const override;

    bool is_ancestor(node_type p, node_type x) const override;

	void add_arc( node_type p, node_type x ) ;
};

template<typename node_type, typename size_type, typename value_type>
void tree<node_type, size_type, value_type>::add_arc( node_type p, node_type x ) {
	auto i= E++;
	assert( E < n );
	to[i]= x, next_[i]= last[p], last[p]= i;
}

template<typename node_type, typename size_type, typename value_type>
std::optional<node_type> tree<node_type, size_type, value_type>::parent(node_type x) const {
    assert( 0 <= x and x < n );
    return p[x];
}

template<typename node_type, typename size_type, typename value_type>
size_type tree<node_type, size_type, value_type>::depth(node_type x) const {
    assert( 0 <= x and x < n );
    return d[x];
}

template<typename node_type, typename size_type, typename value_type>
value_type tree<node_type, size_type, value_type>::weight(node_type x) const {
    assert( 0 <= x and x < n );
    return w[x];
}

template<typename node_type, typename size_type, typename value_type>
std::optional<node_type> tree<node_type, size_type, value_type>::up(node_type x, unsigned int u) const {
    assert( 0 <= x and x < n );
    if ( u > d[x] ) return std::nullopt;
    for ( size_type k= 0; k < K and u; u>>= 1, ++k )
        if ( u & 1 ) {
            assert( k < anc[x].size() );
            assert( anc[x][k] );
            x = *anc[x][k];
        }
    return x;
}

template<typename node_type, typename size_type, typename value_type>
node_type tree<node_type, size_type, value_type>::lca(node_type x, node_type y) const {
    // since up() is an internal use function, we know that up() exists
    // so we take its value
    if ( d[x] > d[y] ) {
        auto z= up(x,d[x]-d[y]);
        assert( z.has_value() );
        return lca(*z,y);
    }
    if ( d[x] < d[y] )
        return lca(y,x);
    if ( x == y )
        return x;
    for ( size_type k= K-1; k; --k ) {
        assert( anc[x][k] == anc[y][k] ); //both exist or nullopts
        assert( *anc[x][k] == *anc[y][k] );
        if ( *anc[x][k-1] != *anc[y][k-1] )
            x= *anc[x][k-1], y= *anc[y][k-1];
    }
    return *anc[x][0];
}

template<typename node_type, typename size_type, typename value_type>
bool tree<node_type, size_type, value_type>::is_leaf(node_type x) const {
    return false;
}

template<typename node_type, typename size_type, typename value_type>
void tree<node_type, size_type, value_type>::shed_redundancy() {
	to.clear(), last.clear(), next_.clear(), anc.clear();
}

template<typename node_type, typename size_type, typename value_type>
void tree<node_type, size_type, value_type>::preprocess() {
    for ( node_type x= 0; x < n; ++x )
        for ( auto &v: anc[x] )
            v= std::nullopt;
    std::queue<node_type> q;
    std::fill(d.begin(),d.end(),std::numeric_limits<size_type>::max());
    for ( d[root]= 0, q.push(root); not q.empty(); ) {
        auto x= q.front(); q.pop();
        auto kind= kinder(x);
        for ( auto y: kind )
            if ( d[y] > d[x]+1 ) {
                d[y]= d[*(anc[y][0]= x)]+1;
                for ( size_type k= 1; k < K and k < anc[y].size() and anc[y][k-1];\
                anc[y][k]= anc[*anc[y][k-1]][k-1], ++k ) ;
                q.push(y);
            }
    }
    assert( std::find_if(d.begin(),d.end(),[]( auto v ) { return v == std::numeric_limits<size_type>::max(); }) == d.end() );
}

template<typename node_type, typename size_type, typename value_type>
void tree<node_type, size_type, value_type>::init(const std::string &s, const std::vector<value_type> &weights) {
    for ( E= 0, n= s.size()/2, K= 0; (1<<K)<=n; ++K ) ;
	to.resize(n==0?0:n-1), last.resize(n), next_.resize(n==0?0:n-1);
	std::fill(last.begin(),last.end(),std::nullopt), std::fill(next_.begin(),next_.end(),std::nullopt);
    p.resize(n);
    std::fill(p.begin(),p.end(),std::nullopt);
    d.resize(n);
    w= std::make_unique<value_type[]>(n);
    anc.resize(n);
    std::stack<node_type> st;
    size_type V= root;
    for ( auto ch: s ) {
        if ( ch == '(' ) {
            d[V]= st.size();
            if ( not st.empty() ) {
				p[V]= st.top();
				add_arc(*p[V],V);
			}
            st.push(V++);
            continue ;
        }
        assert( ch == ')' );
        st.pop();
    }
    for ( auto &v: anc ) v.resize(K+1);
    for ( auto x= 0; x < n; ++x )
        assert( anc[x].size() >= K );
    for ( auto i= 0; i < n; w[i]= weights[i], ++i ) ;
    preprocess();
}

template<typename node_type, typename size_type, typename value_type>
tree<node_type, size_type, value_type>::tree(std::istream &is) {
    std::string s;
    is >> s, n= s.size()/2;
    std::vector<value_type> weights(n);
    for ( auto &x: weights )
        is >> x;
    init(s,weights);
}

template<typename node_type, typename size_type, typename value_type>
tree<node_type, size_type, value_type>::tree(const std::string &s, const std::vector<value_type> &weights) {
    init(s,weights);
}

template<typename node_type, typename size_type, typename value_type>
std::vector<node_type> tree<node_type, size_type, value_type>::kinder(node_type x) const {
	std::vector<node_type> res{};
	for ( auto i= last[x]; i; i= next_[*i] )
		res.push_back(to[*i]);
	return res;
}

/*
template<typename node_type, typename size_type, typename value_type>
double tree<node_type, size_type, value_type>::size_in_bytes() const {
    if ( not flag ) {
        double ans = sizeof(n) + sizeof(K) + sizeof(adj) +
                     sizeof(p) + sizeof(w) + sizeof(d) + sizeof(anc);
        if ( p != nullptr )
            ans += n * sizeof *(p.get());
        ans += n * sizeof *(w.get());
        ans += n * sizeof *(d.get());
        for (auto x = 0; adj != nullptr and x < n; ++x) {
            ans += adj[x].size() * sizeof(node_type), ans+= sizeof(adj[x]);
            ans += K * sizeof *(anc[x].get());
        }
        flag= true;
        return sz= ans;
    }
    return sz;
}
*/

template<typename node_type, typename size_type, typename value_type>
size_t tree<node_type, size_type, value_type>::size() const {
    return n;
}

template<typename node_type, typename size_type, typename value_type>
std::optional<node_type> tree<node_type, size_type, value_type>::ancestor(node_type x, size_type i) const {
    return std::nullopt;
}

template<typename node_type, typename size_type, typename value_type>
std::vector<node_type> tree<node_type, size_type, value_type>::children(node_type x) const {
	std::vector<node_type> res{};
	for ( auto i= last[x]; i; i= next_[*i] )
		res.push_back(to[*i]);
	return res;
}

template<typename node_type, typename size_type, typename value_type>
bool tree<node_type, size_type, value_type>::is_ancestor(node_type p, node_type x) const {
    return false;
}

template<typename node_type, typename size_type, typename value_type>
tree<node_type, size_type, value_type>::~tree() = default;

#endif //TREE_PATH_QUERIES_SIMPLE_TREE_HPP
