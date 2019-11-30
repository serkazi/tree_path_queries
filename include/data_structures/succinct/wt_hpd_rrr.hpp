//
// Created by sj on 07/06/19.
//
#ifndef SPQ_WT_HPD_RRR_HPP
#define SPQ_WT_HPD_RRR_HPP
#include "wt_hpd.hpp"
/**
 * @brief specialization of the wt_int, using compressed bit-vectors
 * @details as wt_hpd<> uses the type of the bit-vector by default to initialize the support structures,
 * it suffices to supply the bit-vector type only
 * rrr does not have other support structures, anyway
 * @tparam node_type
 * @tparam size_type
 * @tparam value_type
 * @tparam t_succinct_tree
 */
 // TODO: remove the constructor altogether
template<
        typename node_type,
        typename size_type,
        typename value_type,
        class t_succinct_tree= bp_tree_sada<node_type,size_type>
        >
class wt_hpd_rrr: public wt_hpd<node_type,size_type,value_type,t_succinct_tree,sdsl::rrr_vector<>> {
public:
    /*
    wt_hpd_rrr( const std::string &s, const std::vector<value_type> &w ) {
        using cwt_int= sdsl::wt_int<sdsl::rrr_vector<>>;
	    auto H= hpd(this->original= new t_succinct_tree(s));
		auto bundle= H();
		auto bv= std::move(std::get<0>(bundle));
		//condensed= new bp_tree_sada<>(&bv);
        this->condensed= std::make_unique<t_succinct_tree>(&bv);
		assert( this->condensed->size() == s.length()/2 );
		//B= new rs_bitvector(std::get<1>(bundle));
		this->B= std::make_unique<sdsl::rrr_vector<>>(std::get<1>(bundle));
        this->BS= std::make_unique<sdsl::rrr_vector<>::select_1_type>(this->B.get());
		std::vector<node_type> chain= std::move(std::get<2>(bundle));
		sdsl::int_vector weights= int_vector(this->m= this->original->size());
		this->m_sigma= 0;
		for ( auto l= 0; l < this->original->size(); \
		this->m_sigma= std::max(this->m_sigma,(value_type)(weights[l]= w[chain[l]])), ++l ) ;
		++this->m_sigma, this->wavelet_tree= std::make_unique<cwt_int>();
        //int_vector reduced_weights= int_vector(this->m= original->size());
        //reducer= new rank_space_reducer<>(w,reduced_weights);
		//construct_im(*wavelet_tree,reduced_weights);
        construct_im(*(this->wavelet_tree),weights);
    }
    */
};
#endif //SPQ_WT_HPD_RRR_HPP
