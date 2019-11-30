#ifndef TREE_PATH_QUERIES_SIMPLE_TREE_HPP
#define TREE_PATH_QUERIES_SIMPLE_TREE_HPP

#include <memory>
#include <queue>
#include <stack>
#include <cassert>

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
struct tree {
    static const node_type NO_ANC= std::numeric_limits<node_type>::max();
    size_type n,K;
    const node_type root= 0;
    std::unique_ptr<std::vector<node_type>[]> adj;
    std::unique_ptr<node_type[]> p;
    std::unique_ptr<value_type[]> w;
    std::unique_ptr<size_type[]> d;
    std::unique_ptr<std::unique_ptr<node_type[]>[]> anc;

    // as we are traversing the tree upwards only, after
    // finding the HPD we no longer need children info, hence we delete it
    // we also don't need parents, since they are effectively served by anc[x][0]
    void shed_redudancy() ;
    node_type parent( node_type x ) const ;
    size_type depth( node_type x ) const ;
    value_type weight( node_type x ) const ;

    node_type up( node_type x, unsigned int u ) const ;
    node_type lca( node_type x, node_type y ) const ;
    bool is_leaf( node_type x ) const ;

    void preprocess() ;

    size_t size() const ;

    void init( const std::string &s, const std::vector<value_type> &weights ) ;

    explicit tree( std::istream &is ) ;

    tree( const std::string &s, const std::vector<value_type> &weights ) ;

    virtual ~tree() ;
    virtual const std::vector<node_type> &kinder( node_type x ) const ;

    mutable double sz{};
    mutable bool flag{};

    virtual double size_in_bytes() const ;

    // delete the copy constructor
    tree<node_type,size_type,value_type>( const tree<node_type,size_type,value_type> &other ) = delete;
    // delete the copy assignment
    tree<node_type,size_type,value_type>& operator = ( const tree<node_type,size_type,value_type> &other ) = delete;

    // move-constructor
    tree<node_type,size_type,value_type>( tree<node_type,size_type,value_type> &&other ) noexcept = delete ;

    // move-assignment
    tree<node_type,size_type,value_type>& operator = ( tree<node_type,size_type,value_type> &&other ) noexcept ;
};

template<typename node_type, typename size_type, typename value_type>
node_type tree<node_type, size_type, value_type>::parent(node_type x) const {
    return anc[x][0];
}

template<typename node_type, typename size_type, typename value_type>
size_type tree<node_type, size_type, value_type>::depth(node_type x) const {
    return d[x];
}

template<typename node_type, typename size_type, typename value_type>
value_type tree<node_type, size_type, value_type>::weight(node_type x) const {
    return w[x];
}

template<typename node_type, typename size_type, typename value_type>
node_type tree<node_type, size_type, value_type>::up(node_type x, unsigned int u) const {
    for ( size_type k= 0; k < K and u; u>>= 1, ++k )
        if ( u & 1 )
            x= anc[x][k];
    return x;
}

template<typename node_type, typename size_type, typename value_type>
node_type tree<node_type, size_type, value_type>::lca(node_type x, node_type y) const {
    if ( d[x] > d[y] )
        return lca(up(x,d[x]-d[y]),y);
    if ( d[x] < d[y] )
        return lca(y,x);
    if ( x == y )
        return x;
    for ( size_type k= K-1; k; --k ) {
        assert( anc[x][k] == anc[y][k] );
        if ( anc[x][k-1] != anc[y][k-1] )
            x= anc[x][k-1], y= anc[y][k-1];
    }
    return anc[x][0];
}

template<typename node_type, typename size_type, typename value_type>
bool tree<node_type, size_type, value_type>::is_leaf(node_type x) const {
    return adj[x].empty();
}

template<typename node_type, typename size_type, typename value_type>
void tree<node_type, size_type, value_type>::shed_redudancy() {
    adj= nullptr, p= nullptr;
}

template<typename node_type, typename size_type, typename value_type>
void tree<node_type, size_type, value_type>::preprocess() {
    for ( node_type x= 0; x < n; ++x )
        for ( size_type k= 0; k < K; ++k )
            anc[x][k]= NO_ANC;
    std::queue<node_type> q;
    for ( node_type x= 0; x < n; d[x++]= std::numeric_limits<size_type>::max() ) ;
    for ( d[root]= 0, q.push(root); not q.empty(); ) {
        node_type x= q.front(); q.pop();
        const auto &kind= kinder(x);
        for ( auto y: kind )
            if ( d[y] > d[x]+1 ) {
                d[y]= d[anc[y][0]= x]+1;
                for ( size_type k= 1; k < K and anc[y][k-1] != NO_ANC; anc[y][k]= anc[anc[y][k-1]][k-1], ++k ) ;
                q.push(y);
            }
    }
    for ( auto x= 0; x < n; ++x )
        assert( d[x] < std::numeric_limits<size_type>::max() );
}

template<typename node_type, typename size_type, typename value_type>
void tree<node_type, size_type, value_type>::init(const std::string &s, const std::vector<value_type> &weights) {
    for ( n= s.size()/2, K= 0; (1<<K)<=n; ++K ) ;
    adj= std::make_unique<std::vector<node_type>[]>(n);
    p= std::make_unique<node_type[]>(n);
    d= std::make_unique<size_type[]>(n);
    w= std::make_unique<value_type[]>(n);
    anc= std::make_unique<std::unique_ptr<node_type[]>[]>(n);
    for ( auto x= 0; x < n; ++x )
        anc[x]= std::make_unique<node_type[]>(K);
    std::stack<node_type> st;
    size_type V= root;
    for ( auto ch: s ) {
        if ( ch == '(' ) {
            if ( not st.empty() )
                adj[p[V]= st.top()].push_back(V);
            st.push(V++);
            continue ;
        }
        assert( ch == ')' );
        st.pop();
    }
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
const std::vector<node_type> &tree<node_type, size_type, value_type>::kinder(node_type x) const {
    return adj[x];
}

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

template<typename node_type, typename size_type, typename value_type>
tree<node_type, size_type, value_type> &
tree<node_type, size_type, value_type>::operator=(tree<node_type, size_type, value_type> &&other) noexcept {
    if ( this != &other ) {
        this->n= other.n, this->K= other.K;
        this->p= nullptr, this->p= std::move(other.p);
        this->w= nullptr, this->w= std::move(other.w);
        this->d= nullptr, this->d= std::move(other.d);
        for ( auto x= 0; x < n; ++x )
            this->anc[x]= nullptr, (void)(this->adj[x]);
        this->anc= nullptr, this->anc= std::move(other.anc);
        this->adj= nullptr, this->adj= std::move(other.adj);
    }
    return *this;
}

template<typename node_type, typename size_type, typename value_type>
size_t tree<node_type, size_type, value_type>::size() const {
    return n;
}

template<typename node_type, typename size_type, typename value_type>
tree<node_type, size_type, value_type>::~tree() = default;

#endif //TREE_PATH_QUERIES_SIMPLE_TREE_HPP
