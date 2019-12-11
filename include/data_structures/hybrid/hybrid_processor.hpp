//
// Created by sj on 15/11/19.
//
#ifndef SPQ_HYBRID_PROCESSOR_HPP
#define SPQ_HYBRID_PROCESSOR_HPP
#include "pq_types.hpp"
#include "heavy_path_decomp.hpp"
#include "range_tree_simple.hpp"
#include "wavelet_tree.hpp"
#include "bender_farach_colton.hpp"
// #include "succinct_container.hpp" <-- we'll worry about memory later
#include "path_query_processor.hpp"
#include <memory>

template <
        typename node_type= pq_types::node_type,
        typename size_type= pq_types::size_type,
        typename value_type= pq_types::value_type
        >
class hybrid_processor:
        public path_query_processor<node_type,size_type,value_type> {
private:
    std::unique_ptr<heavy_path_decomp<node_type,size_type,value_type>> h= nullptr;
    std::unique_ptr<wavelet_tree<size_type,value_type>> wt= nullptr;
    std::unique_ptr<range_tree_simple<size_type,value_type>> rt= nullptr;
    std::shared_ptr<tree<node_type,size_type,value_type>> T= nullptr;
    std::shared_ptr<lca_processor<node_type,size_type>> lca_proc= nullptr;
    void init() ;
public:

    hybrid_processor( const std::string &s, const std::vector<value_type> &w ) ;
    explicit hybrid_processor( std::istream &is ) ;

    // copy constructor deleted
    hybrid_processor( const hybrid_processor &other ) = delete;
    // copy assignment deleted
    hybrid_processor& operator = ( const hybrid_processor &other ) = delete;

    // move constructor
    hybrid_processor( hybrid_processor &&other ) noexcept ;
    // move assignment
    hybrid_processor &operator = ( hybrid_processor &&other ) noexcept ;

    value_type weight(node_type x) const override;

    value_type query(node_type x, node_type y) const override;

    value_type selection(node_type x, node_type y, size_type qntl) const override;

    value_type weight_of(node_type x) const override;

    size_type count(node_type x, node_type y, value_type a, value_type b) const override;

    void report(node_type x, node_type y, value_type a, value_type b,
                std::vector<std::pair<value_type, size_type>> &res) const override;

    //double bits_per_node() const override;
    virtual ~hybrid_processor() = default;
};

// move ctor
template <typename node_type,typename size_type,typename value_type>
hybrid_processor<node_type,size_type,value_type>::hybrid_processor(hybrid_processor &&other) noexcept {
    h= move(other.h), wt= move(other.wt), rt= move(other.rt);
}

// move assignment
template <typename node_type,typename size_type,typename value_type>
hybrid_processor<node_type,size_type,value_type>
&hybrid_processor<node_type,size_type,value_type>::operator=(hybrid_processor &&other) noexcept {
    if ( this != &other ) {
        h= nullptr, wt= nullptr, rt= nullptr;
        h= move(other.h), wt= move(other.wt), rt= move(other.rt);
    }
    return *this;
}

template <typename node_type,typename size_type,typename value_type>
value_type hybrid_processor<node_type,size_type,value_type>::weight(const node_type x) const {
    return T->weight(x);
}

template <typename node_type,typename size_type,typename value_type>
value_type hybrid_processor<node_type,size_type,value_type>::weight_of(const node_type x) const {
    return T->weight(x);
}

template <typename node_type,typename size_type,typename value_type>
value_type hybrid_processor<node_type,size_type,value_type>::query(const node_type x, const node_type y) const {
    auto z= (*lca_proc)(x,y);
    auto len= T->depth(x)+T->depth(y)+1-2*T->depth(z);
    auto path= h->decompose_path(x,y);
    return wt->range_quantile(path,len>>1);
}

template <typename node_type,typename size_type,typename value_type>
value_type hybrid_processor<node_type,size_type,value_type>::selection(
        const node_type x, const node_type y, const size_type qntl) const {
    auto z= (*lca_proc)(x,y);
    auto len= T->depth(x)+T->depth(y)+1-2*T->depth(z);
    auto path= h->decompose_path(x,y);
    return wt->range_quantile(
            path,
            path_query_processor<node_type,size_type,value_type>::qntl2rnk(len,qntl)
    );
}

template <typename node_type,typename size_type,typename value_type>
size_type hybrid_processor<node_type,size_type,value_type>
::count(node_type x,node_type y,value_type a,value_type b) const {
    size_type res= 0;
    auto path= h->decompose_path(x,y);
    for ( const auto &pr: path )
        res+= rt->range_2d_counting_query(pr.first,pr.second,a,b);
    return res;
}

template <typename node_type,typename size_type,typename value_type>
void hybrid_processor<node_type,size_type,value_type>
::report(node_type x,node_type y,value_type a,value_type b,
         std::vector<std::pair<value_type, size_type>> &res ) const {
    res.clear();
    auto path= h->decompose_path(x,y);
    for ( const auto &pr: path ) {
        std::vector<std::pair<value_type,size_type>> tmp;
        rt->range_2d_reporting_query(pr.first,pr.second,a,b,&tmp);
        res.insert(res.end(),tmp.begin(),tmp.end());
    }
}

/*
double hybrid_processor::bits_per_node() const {
    return (h->size_in_bytes()+rt->size_in_bytes()+wt->size_in_bytes()+T->size_in_bytes())*8.00/T->n;
}
 */

// ctor
template <typename node_type,typename size_type,typename value_type>
hybrid_processor<node_type,size_type,value_type>::hybrid_processor(std::istream &is) {
    T= std::make_shared<tree<node_type,size_type,value_type>>(is);
    init();
}
// ctor
template <typename node_type,typename size_type,typename value_type>
hybrid_processor<node_type,size_type,value_type>
::hybrid_processor( const std::string &s, const std::vector<value_type> &w ) {
    T= std::make_shared<tree<node_type,size_type,value_type>>(s,w);
    init();
}

template <typename node_type,typename size_type,typename value_type>
void hybrid_processor<node_type,size_type,value_type>::init() {
    assert( T != nullptr );
    lca_proc= std::make_shared<lca_processor<node_type,size_type>>(T.get());
    h= std::make_unique<heavy_path_decomp<node_type,size_type,value_type>>(T,lca_proc);
    T->shed_redundancy(); //delete the child-information, since we are going to traverse upwards only
    const auto &src= h->get_chain();
    wt= std::make_unique<wavelet_tree<size_type,value_type>>(src);
    std::vector<typename range_tree_simple<size_type,value_type>::point2d> points;
    points.reserve(src.size());
    for ( auto l= 0; l < src.size(); ++l )
        points.emplace_back(l,src[l]);
    rt= std::make_unique<range_tree_simple<size_type,value_type>>(points);
    // delete also the O(nlog(n))-word LCA answering structure, since we have lca_proc object now
}
#endif //SPQ_HYBRID_PROCESSOR_HPP
