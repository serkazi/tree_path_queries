#ifndef SPQ_HEAVY_PATH_DECOMP_HPP
#define SPQ_HEAVY_PATH_DECOMP_HPP
#include "simple_tree.hpp"
#include "space_occupancy.cpp"
#include <istream>
#include <ostream>
#include <limits>
#include <algorithm>
#include <vector>
#include <string>
#include <stack>
#include <cassert>
#include <memory>
#include <queue>

template<typename node_type= int, typename size_type= int, typename value_type= int>
class heavy_path_decomp {
private:
#define is_integral_assert(T) static_assert( std::is_integral<T>::value,"Integral type required for "#T)
    is_integral_assert(node_type);
    is_integral_assert(size_type);
    is_integral_assert(value_type);

    std::shared_ptr<tree<node_type,size_type,value_type>> T;
    std::vector<node_type> head;
    std::vector<size_type> which, global_position;
    std::vector<value_type> permuted_weights;

    node_type head_of( size_type ch ) const ;
    node_type parent( node_type x ) const ;
    void perform_decomposition();
    void enlist( node_type x, value_type w, size_type chain_id ) ;

    size_t how_many_segments( node_type p, node_type x ) const ;

    void get_intervals( node_type p, node_type x,
                        std::vector<std::pair<size_type,size_type>> &res,
                        bool inclusive= false ) const ;

public:

    virtual std::vector<std::pair<size_type,size_type>> decompose_path( node_type x, node_type y ) ;
    // copy assignment
    heavy_path_decomp &operator = ( const heavy_path_decomp &other ) = delete;
    // comy constructor
    heavy_path_decomp( const heavy_path_decomp &other ) = delete;

    // move assignment
    heavy_path_decomp<node_type,size_type,value_type> &operator = ( heavy_path_decomp<node_type,size_type,value_type> &&other ) ;
    // move constructor
    heavy_path_decomp<node_type,size_type,value_type> ( heavy_path_decomp<node_type,size_type,value_type> &&other ) ;

    size_type which_chain( node_type x ) const ;
    explicit heavy_path_decomp<node_type,size_type,value_type>( std::istream &is ) ;
    heavy_path_decomp( std::shared_ptr<tree<node_type,size_type,value_type>> T ) ;
    size_type node2pos( node_type x ) const ;
    virtual const std::vector<value_type> &get_chain() const ;
    // virtual double size_in_bytes() const ;
    virtual ~heavy_path_decomp() ;
};

template<typename node_type, typename size_type, typename value_type>
std::ostream &operator << ( std::ostream &os, const heavy_path_decomp<node_type,size_type,value_type> &h ) ;

template<typename node_type, typename size_type, typename value_type>
size_type heavy_path_decomp<node_type, size_type, value_type>::which_chain(node_type x) const {
    return which[x];
}

template<typename node_type, typename size_type, typename value_type>
node_type heavy_path_decomp<node_type, size_type, value_type>::head_of(size_type ch) const {
    return head[ch];
}

template<typename node_type, typename size_type, typename value_type>
node_type heavy_path_decomp<node_type, size_type, value_type>::parent(node_type x) const {
    return T->parent(x);
}

// HPD using explicit stack
template<typename node_type, typename size_type, typename value_type>
void heavy_path_decomp<node_type, size_type, value_type>::perform_decomposition() {
    if ( T->n <= 1 ) return ;
    assert( T->n >= 2 );
    auto best_son= std::make_unique<size_type[]>(T->n);
    auto card= std::make_unique<size_type[]>(T->n);
    const size_type NONE= std::numeric_limits<size_type>::max();
    for ( auto x= T->root; x < T->n; card[x]= 1, best_son[x++]= NONE ) ;
    node_type x,y;
    size_type i,j,k;
    // precomputing heavy child
    std::stack<node_type> st;
    std::stack<size_type> idx;
    for (st.push(T->root), idx.push(0); not st.empty();) {
        x = st.top(), i = idx.top();
        st.pop(), idx.pop();
        const auto children = T->kinder(x);
        assert(i <= children.size());
        if (i == children.size()) {
            for (j = 0; j < children.size(); ++j) {
                card[x] += card[children[j]];
                if (best_son[x] == NONE or card[children[j]] > card[children[best_son[x]]])
                    best_son[x] = j;
            }
            continue;
        }
        st.push(x), idx.push(i + 1), st.push(children[i]), idx.push(0);
    }

    assert( st.empty() );
    assert( idx.empty() );
    std::stack<size_type> cnt;
    size_type chain_id= 0;

    for ( head.push_back(T->root), enlist(T->root,T->weight(T->root),chain_id++), st.push(T->root),
                  idx.push(best_son[T->root]), cnt.push(0); not st.empty(); ) {
        x= st.top(), i= idx.top(); st.pop(), idx.pop();
        k= cnt.top(), cnt.pop();
        auto children = T->kinder(x);
        if ( k == children.size() )
            continue ;
        y= children[i];
        enlist(y,T->weight(y),i==best_son[x]?which_chain(x):(head.push_back(y),chain_id++));
        st.push(x), idx.push((i+1)%children.size()), cnt.push(k+1);
        if ( best_son[y] != NONE )
            st.push(y), idx.push(best_son[y]), cnt.push(0);
        else {
            assert( T->is_leaf(y) );
        }
    }
}

template<typename node_type, typename size_type, typename value_type>
heavy_path_decomp<node_type, size_type, value_type>::heavy_path_decomp(std::istream &is) {
    T= std::make_shared<tree<node_type,size_type,value_type>>(is);
    global_position.resize(T->n), which.resize(T->n);
    perform_decomposition();
}

template<typename node_type, typename size_type, typename value_type>
size_type heavy_path_decomp<node_type, size_type, value_type>::node2pos(node_type x) const {
    return global_position[x];
}

template<typename node_type, typename size_type, typename value_type>
const std::vector<value_type>& heavy_path_decomp<node_type, size_type, value_type>::get_chain() const {
    return permuted_weights;
}

template<typename node_type, typename size_type, typename value_type>
void heavy_path_decomp<node_type, size_type, value_type>::enlist( node_type x, value_type w, size_type chain_id ) {
    which[x]= chain_id, global_position[x]= permuted_weights.size();
    permuted_weights.push_back(w);
}

template<typename node_type, typename size_type, typename value_type>
heavy_path_decomp<node_type,size_type,value_type>
&heavy_path_decomp<node_type, size_type, value_type>::operator=( heavy_path_decomp<node_type,size_type,value_type> &&other ) {
    if ( this != &other ) {
        permuted_weights.clear(), permuted_weights= move(other.permuted_weights);
        which.clear(), which= move(other.which);
        head.clear(), head= move(other.head);
        global_position.clear(), global_position= move(other.global_position);
        T= nullptr, T= move(other.T);
    }
    return *this;
}

template<typename node_type, typename size_type, typename value_type>
heavy_path_decomp<node_type, size_type, value_type>::heavy_path_decomp(heavy_path_decomp &&other) {
    permuted_weights= move(other.permuted_weights);
    which= move(other.which);
    head= move(other.head);
    global_position= move(other.global_position);
    T= move(other.T);
}

/*
template<typename node_type, typename size_type, typename value_type>
double heavy_path_decomp<node_type, size_type, value_type>::size_in_bytes() const {
    double ans= size_of_vector_in_bytes(permuted_weights)+
                size_of_vector_in_bytes(which)+size_of_vector_in_bytes(head)+
                size_of_vector_in_bytes(global_position)+T->size_in_bytes();
    return ans;
}
*/

template<typename node_type, typename size_type, typename value_type>
std::vector<std::pair<size_type, size_type>> heavy_path_decomp<node_type, size_type, value_type>
::decompose_path(node_type x, node_type y) {

    node_type z= T->lca(x,y);
    auto ik= how_many_segments(z,x)+1+how_many_segments(z,y);
    auto len= T->depth(x)+T->depth(y)+1-2*T->depth(z);
    std::vector<std::pair<size_type,size_type>> segments;
    segments.reserve(ik);
    assert( segments.empty() );
    get_intervals(z,x,segments), get_intervals(z,z,segments,true), get_intervals(z,y,segments);
    return move(segments);
}

template<typename node_type, typename size_type, typename value_type>
heavy_path_decomp<node_type, size_type, value_type>::heavy_path_decomp(std::shared_ptr<tree<node_type, size_type, value_type>> t) : T(t) {
    global_position.resize(T->n), which.resize(T->n);
    perform_decomposition();
}

template<typename node_type, typename size_type, typename value_type>
void heavy_path_decomp<node_type, size_type, value_type>
::get_intervals(node_type p, node_type x,
                std::vector<std::pair<size_type, size_type>> &res,
                bool inclusive) const {
    auto pp= head_of(which_chain(p));
    for ( auto px= head_of(which_chain(x)); px != pp; ) {
        res.emplace_back(node2pos(px),node2pos(x));
        x= parent(px), px= head_of(which_chain(x));
    }
    if ( node2pos(p)+(inclusive?0:1) <= node2pos(x) )
        res.emplace_back(node2pos(p)+(inclusive?0:1),node2pos(x));
}

template<typename node_type, typename size_type, typename value_type>
size_t heavy_path_decomp<node_type, size_type, value_type>::how_many_segments(node_type p, node_type x) const {
    auto pp= head_of(which_chain(p));
    size_t sz= 0;
    for ( auto px= head_of(which_chain(x)); px != pp; x= parent(px), px= head_of(which_chain(x)), ++sz ) ;
    return ++sz;
}

template<typename node_type, typename size_type, typename value_type>
heavy_path_decomp<node_type, size_type, value_type>::~heavy_path_decomp() = default ;

template<typename node_type, typename size_type, typename value_type>
std::ostream &operator << ( std::ostream &os, const heavy_path_decomp<node_type,size_type,value_type> &h ) {
    auto w= h.get_chain();
    for ( auto i= 0; i < w.size(); os << w[i++] << ' ' ) ;
    os << '\n';
    for ( auto i= 0; i < w.size(); os << h.which_chain(i++) << ' ' ) ;
    return os << '\n';
}

#endif //SPQ_HEAVY_PATH_DECOMP_HPP
