//
// 24/12/19.
//
#ifndef TREE_PATH_QUERIES_WT_HPD_PTR_HPP
#define TREE_PATH_QUERIES_WT_HPD_PTR_HPP

#include "pq_types.hpp"
#include "heavy_path_decomp.hpp"
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
class wt_hpd_ptr:
        public path_query_processor<node_type,size_type,value_type>,
		public path_decomposer<node_type,size_type> {
private:
    std::unique_ptr<heavy_path_decomp<node_type,size_type,value_type>> h= nullptr;
    std::unique_ptr<wavelet_tree<size_type,value_type>> wt= nullptr;
    std::shared_ptr<tree<node_type,size_type,value_type>> T= nullptr;
    std::shared_ptr<lca_processor<node_type,size_type>> lca_proc= nullptr;
    void init() ;
public:

	// implementing path decomposer
    size_type get_decomposition_length( node_type x, node_type y ) const override ;

    wt_hpd_ptr( const std::string &s, const std::vector<value_type> &w ) ;
    explicit wt_hpd_ptr( std::istream &is ) ;

    // copy constructor deleted
    wt_hpd_ptr( const wt_hpd_ptr &other ) = delete;
    // copy assignment deleted
    wt_hpd_ptr& operator = ( const wt_hpd_ptr &other ) = delete;

    // move constructor
    wt_hpd_ptr( wt_hpd_ptr &&other ) noexcept ;
    // move assignment
    wt_hpd_ptr &operator = ( wt_hpd_ptr &&other ) noexcept ;

    value_type weight(node_type x) const override;

    value_type query(node_type x, node_type y) const override;

    value_type selection(node_type x, node_type y, size_type qntl) const override;

    value_type weight_of(node_type x) const override;

    size_type count(node_type x, node_type y, value_type a, value_type b) const override;

    void report(node_type x, node_type y, value_type a, value_type b,
                std::vector<std::pair<value_type, size_type>> &res) const override;

    //double bits_per_node() const override;
    virtual ~wt_hpd_ptr() = default;

    // a few methods to check the sanity of HPD
    size_type num_segments( node_type x, node_type y ) const ;
};

// move ctor
template <typename node_type,typename size_type,typename value_type>
wt_hpd_ptr<node_type,size_type,value_type>::wt_hpd_ptr(wt_hpd_ptr &&other) noexcept {
    h= move(other.h), wt= move(other.wt);
}

// move assignment
template <typename node_type,typename size_type,typename value_type>
wt_hpd_ptr<node_type,size_type,value_type>
&wt_hpd_ptr<node_type,size_type,value_type>::operator=(wt_hpd_ptr &&other) noexcept {
    if ( this != &other ) {
        h= nullptr, wt= nullptr;
        h= move(other.h), wt= move(other.wt);
    }
    return *this;
}

template <typename node_type,typename size_type,typename value_type>
value_type wt_hpd_ptr<node_type,size_type,value_type>::weight(const node_type x) const {
    return T->weight(x);
}

template <typename node_type,typename size_type,typename value_type>
value_type wt_hpd_ptr<node_type,size_type,value_type>::weight_of(const node_type x) const {
    return T->weight(x);
}

template <typename node_type,typename size_type,typename value_type>
value_type wt_hpd_ptr<node_type,size_type,value_type>::query(const node_type x, const node_type y) const {
    auto z= (*lca_proc)(x,y);
    auto len= T->depth(x)+T->depth(y)+1-2*T->depth(z);
    auto path= h->decompose_path(x,y);
    return wt->range_quantile(path,len>>1);
}

template <typename node_type,typename size_type,typename value_type>
value_type wt_hpd_ptr<node_type,size_type,value_type>::selection(
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
size_type wt_hpd_ptr<node_type,size_type,value_type>
::count(node_type x,node_type y,value_type a,value_type b) const {
    size_type res= 0;
    auto path= h->decompose_path(x,y);
    for ( const auto &pr: path )
        res+= wt->range_2d_counting_query(pr.first,pr.second,a,b);
    return res;
}

template <typename node_type,typename size_type,typename value_type>
void wt_hpd_ptr<node_type,size_type,value_type>
::report(node_type x,node_type y,value_type a,value_type b,
         std::vector<std::pair<value_type, size_type>> &res ) const {
    res.clear();
    auto path= h->decompose_path(x,y);
    for ( const auto &pr: path ) {
        std::vector<std::pair<value_type,size_type>> tmp;
        wt->range_2d_reporting_query(pr.first,pr.second,a,b,&tmp);
        res.insert(res.end(),tmp.begin(),tmp.end());
    }
}

/*
double wt_hpd_ptr::bits_per_node() const {
    return (h->size_in_bytes()+rt->size_in_bytes()+wt->size_in_bytes()+T->size_in_bytes())*8.00/T->n;
}
*/

// ctor
template <typename node_type,typename size_type,typename value_type>
wt_hpd_ptr<node_type,size_type,value_type>::wt_hpd_ptr(std::istream &is) {
    T= std::make_shared<tree<node_type,size_type,value_type>>(is);
    init();
}
// ctor
template <typename node_type,typename size_type,typename value_type>
wt_hpd_ptr<node_type,size_type,value_type>
::wt_hpd_ptr( const std::string &s, const std::vector<value_type> &w ) {
    T= std::make_shared<tree<node_type,size_type,value_type>>(s,w);
    init();
}

template <typename node_type,typename size_type,typename value_type>
void wt_hpd_ptr<node_type,size_type,value_type>::init() {
    assert( T != nullptr );
    lca_proc= std::make_shared<lca_processor<node_type,size_type>>(T.get());
    h= std::make_unique<heavy_path_decomp<node_type,size_type,value_type>>(T,lca_proc);
    T->shed_redundancy(); //delete the child-information, since we are going to traverse upwards only
    const auto &src= h->get_chain();
    wt= std::make_unique<wavelet_tree<size_type,value_type>>(src);
    std::vector<std::pair<size_type,value_type>> points;
    points.reserve(src.size());
    for ( auto l= 0; l < src.size(); ++l )
        points.emplace_back(l,src[l]);
}

template<typename node_type, typename size_type, typename value_type>
size_type wt_hpd_ptr<node_type, size_type, value_type>::num_segments( node_type x, node_type y ) const {
    return h->num_segments(x,y);
}

// decompose_path returns the actual path, but we only need the size of the path
template<typename node_type, typename size_type, typename value_type>
size_type wt_hpd_ptr<node_type, size_type, value_type>::get_decomposition_length( node_type x, node_type y ) const {
    return h->num_segments(x,y);
}

#endif //TREE_PATH_QUERIES_WT_HPD_PTR_HPP
