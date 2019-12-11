//
// Created by kazi on 2019-12-09.
//
#ifndef TREE_PATH_QUERIES_RANGE_TREE_SIMPLE_HPP
#define TREE_PATH_QUERIES_RANGE_TREE_SIMPLE_HPP
#include <vector>
#include <queue>
#include <tuple>
#include <limits>
#include <algorithm>
#include <cassert>
#include <memory>
#include <optional>
// #include "simple_bitset.hpp"
#include <cmath>
#include <iostream>
#include "pq_types.hpp"

/**
 * the interface must be such that we accept a vector of ints only,
 * rather than 2d points. I.e. we assume that the first coordinates
 * correspond to indices to the array
 * When using range tree with the heavy paths, we thus going to return
 * positions in the concatenated input array,
 * which is sufficient to recover the ids
 * This means we can actually return only an array of positions,
 * as the 2d-query result. This does not matter much though,
 * as we explicitly store the points anyway
 */
/**
 * @details we remove fractional cascading bridges in order to save space
 * For ease of copy/move constructors, we concatenate
 * all the levels into a single backbone
 * @tparam size_type
 * @tparam value_type
 */
template<typename size_type= int, typename value_type= int>
class range_tree_simple {
public:
    using point2d= std::pair<size_type,value_type>;
    using result_type= std::vector<std::pair<value_type,size_type>>;
private:
    static const int _left= 0, _right= 1;
    static const int _pred= 0, _succ= 1;
    std::vector<point2d> original;
    /**
     * offset records the various
     * regions of the backbone, saying where each section of the concatenated
     * range tree starts
     */
    std::vector<size_type> sorted_list;
    std::vector<size_type> offset;
    size_type n, num_nodes, len;

    std::optional<size_type> bridge( int t, int j, size_type idx, size_type i ) const {
        if ( idx >= num_nodes ) return std::nullopt;
        assert( j == _pred or j == _succ );
        assert( t == _left or t == _right );
        if ( not(offset[idx] <= i and i < (idx+1==num_nodes?len:offset[idx+1])) ) {
            std::cerr << idx << " " << offset[idx] << " " << i << " " << (idx+1==num_nodes?len:offset[idx+1]) << std::endl;
        }
        assert ( offset[idx] <= i and i < (idx+1==num_nodes?len:offset[idx+1]) );
        size_type sons[2]= {2*idx+1,2*idx+2};
        if ( sons[t] < num_nodes ) {
            if ( j == _pred ) {
                return predecessor_of(
                        offset[sons[t]], sons[t]+1==num_nodes?len-1:offset[sons[t]+1]-1,
                        original[sorted_list[i]].second);
            }
            return successor_of(
                    offset[sons[t]], sons[t]+1==num_nodes?len-1:offset[sons[t]+1]-1,
                    original[sorted_list[i]].second);
        }
        return std::nullopt;
    }
    //std::unique_ptr<simple_bitset> valid_nodes= nullptr;

    /**
     * simulates the construction of range tree over [0,n-1]
     * and calculates how many nodes and cells there will be
     * will be needed when optimizing for space
     * @param n
     * @return
     */
    static std::pair<size_type,size_type> get_config( size_type n ) {
        auto max_node_id= std::numeric_limits<size_type>::min();
        size_type span= 0;
        std::queue<std::tuple<size_type,size_type,size_type>> q;
        for ( q.push(std::make_tuple(0,0,n-1)); not q.empty(); ) {
            auto tpl= q.front(); q.pop();
            auto l= std::get<1>(tpl), r= std::get<2>(tpl), idx= std::get<0>(tpl);
            assert( l <= r );
            max_node_id= std::max(max_node_id,idx), span+= (r-l+1);
            if ( l < r ) {
                auto mid= (l+r)/2;
                q.push(std::make_tuple(2*idx+1,l,mid)), q.push(std::make_tuple(2*idx+2,mid+1,r));
            }
        }
        return {max_node_id,span};
    }

    // void mark_valid_nodes() ;

    /**
     * assumes that sorted_list[l]..sorted_list[r] is a range sorted by y-coordinate
     * @param l
     * @param r
     * @param x
     * @return : the position t, l <= t <= r, such that backbone[t].second is the successor of "x"
     */
    std::optional<size_type> successor_of( size_type l, size_type r, value_type x ) const {
        if ( l > r ) return std::nullopt;
        if ( original[sorted_list[l]].second >= x ) return std::optional<size_type>(l);
        assert( original[sorted_list[l]].second < x );
        if ( original[sorted_list[r]].second < x )
            return std::nullopt;
        assert( original[sorted_list[r]].second >= x );
        for ( auto m= l; l+1 < r; original[sorted_list[m= (l+r)/2]].second < x? (l= m):(r= m) );
        return std::optional<size_type>(r);
    }

    std::optional<size_type> predecessor_of( size_type l, size_type r, value_type x ) const {
        if ( l > r ) return std::nullopt;
        if ( original[sorted_list[l]].second > x )
            return std::nullopt;
        assert( original[sorted_list[l]].second <= x );
        if ( original[sorted_list[r]].second <= x ) return std::optional<size_type>(r);
        assert( original[sorted_list[r]].second > x );
        for ( auto m= l; l+1 < r; original[sorted_list[m= (l+r)/2]].second <= x? (l= m):(r= m) );
        return std::optional<size_type>(l);
    }

    void search_along_left_ridge(
            size_type idx, size_type l, size_type r, size_type trg,
            std::optional<size_type> sc, std::optional<size_type> pr,
            std::optional<result_type*> result,
            size_type &cnt ) const ;
    void search_along_right_ridge(
            size_type idx, size_type l, size_type r, size_type trg,
            std::optional<size_type> sc, std::optional<size_type> pr,
            std::optional<result_type*> result,
            size_type &cnt ) const ;
    void search_2d_range( size_type qi, size_type qj,
                          value_type a, value_type b,
                          std::optional<result_type*> result,
                          size_type &cnt
    ) const ;
    void construct_im( const std::vector<point2d> &pts ) ;
    void presort_lists() ;
public:

    // remove copy-assignment
    range_tree_simple& operator = ( const range_tree_simple& other ) = delete;
    // remove copy-constructor
    range_tree_simple( const range_tree_simple &other ) = delete;

    // move-assignment
    range_tree_simple &operator = ( range_tree_simple &&other ) noexcept;
    // move-constructor
    range_tree_simple( range_tree_simple &&other ) noexcept;

    virtual size_t size() const { return n; }
    virtual size_type range_2d_counting_query( size_type qi, size_type qj, value_type a, value_type b ) const ;
    virtual void range_2d_reporting_query( size_type qi, size_type qj,
                                           value_type a, value_type b,
                                           std::optional<result_type*> result ) const ;
    /*
    virtual double size_in_bytes() const ;
    */
    explicit range_tree_simple( const std::vector<typename range_tree_simple<size_type,value_type>::point2d> &points ) ;
    virtual ~range_tree_simple() ;
};

template<typename size_type, typename value_type>
void
range_tree_simple<size_type, value_type>::search_2d_range(
        size_type qi, size_type qj,
        value_type a, value_type b,
        std::optional<range_tree_simple::result_type*> result,
        size_type &cnt) const
{
    /**
     * find the splitting point first
     */
    cnt= 0;
    std::tuple<size_type,size_type,size_type> lca_segment= [qi,qj,n=this->n](){
        size_type cur_node= 0, l= 0, r= n-1, mid;
        assert( l <= qi and qj <= r );
        for ( ;l < r; ) {
            assert( l <= qi and qj <= r );
            if ( qj <= (mid= (l+r)/2) ) {
                r= mid, cur_node= 2*cur_node+1;
                continue ;
            }
            if ( mid+1 <= qi ) {
                l= mid+1, cur_node= 2*cur_node+2;
                continue ;
            }
            assert( qi <= mid and mid+1 <= qj );
            break ;
        }
        return std::make_tuple(cur_node,l,r);
    }();
    auto idx= std::get<0>(lca_segment),
            l= std::get<1>(lca_segment),
            r= std::get<2>(lca_segment);
    assert( l <= qi and qj <= r );
    assert( l == r or (qi <= (l+r)/2 and (l+r)/2 < qj) );
    if ( l == r ) {
        if ( a <= original[sorted_list[offset[idx]]].second and original[sorted_list[offset[idx]]].second <= b ) {
            ++cnt;
            if ( result ) (*result)->push_back(original[sorted_list[offset[idx]]]);
        }
        return ;
    }
    auto sc= successor_of(offset[idx],idx+1==num_nodes?len-1:offset[idx+1]-1,a),
         pr= predecessor_of(offset[idx],idx+1==num_nodes?len-1:offset[idx+1]-1,b);
    if ( (not pr) or (not sc) or *sc > *pr ) { // there can be no points with a <= xxx <= b
        return ;
    }
    assert( offset[idx] <= *sc and *pr <= (idx+1==num_nodes?len-1:offset[idx+1]-1) );

    auto lsc= bridge(_left,_succ,idx,*sc),
         lpr= bridge(_left,_pred,idx,*pr),
         rsc= bridge(_right,_succ,idx,*sc),
         rpr= bridge(_right,_pred,idx,*pr);

    search_along_left_ridge(2*idx+1,l,(l+r)/2,qi,lsc,lpr,result,cnt);
    search_along_right_ridge(2*idx+2,(l+r)/2+1,r,qj,rsc,rpr,result,cnt);
}

template<typename size_type, typename value_type>
size_type range_tree_simple<size_type, value_type>
::range_2d_counting_query(size_type qi, size_type qj,
                          value_type a, value_type b)
const {
    size_type cnt= 0;
    search_2d_range(qi,qj,a,b,std::nullopt,cnt);
    return cnt;
}

template<typename size_type, typename value_type>
void range_tree_simple<size_type, value_type>
::range_2d_reporting_query(size_type qi, size_type qj,
                           value_type a, value_type b,
                           std::optional<range_tree_simple::result_type*> result)
const {
    size_type cnt= 0;
    search_2d_range(qi,qj,a,b,result,cnt);
}

// destructor
template<typename size_type, typename value_type>
range_tree_simple<size_type, value_type>::~range_tree_simple() {
    // this is essentially a No-Op, since we have smart pointers all round
}

// constructor
template<typename size_type, typename value_type>
range_tree_simple<size_type, value_type>
::range_tree_simple( const std::vector<typename range_tree_simple<size_type,value_type>::point2d> &input )
{
    for ( this->n= input.size(); (this->n & (this->n-1)); ++this->n ) ;
    original= std::vector<point2d>(input);
    for ( size_type idx= input.size(); original.size() < this->n; original.emplace_back(idx++,0) ) ;
    auto pr= get_config(this->n);
    num_nodes= pr.first+1, len= pr.second;
    offset.resize(num_nodes), sorted_list.resize(len);
    // mark_valid_nodes();
    construct_im(original);
    presort_lists();
}

template<typename size_type, typename value_type>
void range_tree_simple<size_type, value_type>
::construct_im( const std::vector<range_tree_simple::point2d> &pts ) {
    std::queue<std::tuple<size_type,size_type,size_type>> q;
    std::unique_ptr<size_type[]> length= std::make_unique<size_type[]>(num_nodes+1);
    for ( auto it= 0; it < num_nodes; offset[it++]= std::numeric_limits<size_type>::max() ) ;
    for ( offset[0]= 0, length[0]= n, q.push(std::make_tuple(0,0,n-1)); not q.empty(); ) {
        auto tpl= q.front(); q.pop();
        auto l= std::get<1>(tpl), r= std::get<2>(tpl), idx= std::get<0>(tpl);
        for ( auto iit= l; iit <= r; ++iit ) {
            assert( offset[idx]+iit-l < len );
            assert( iit < pts.size() );
            sorted_list[offset[idx]+iit-l]= iit;
        }
        if ( l < r ) {
            auto mid= (l+r)/2;
            assert( 2*idx+1 < num_nodes );
            assert( offset[2*idx] != std::numeric_limits<size_type>::max() );
            offset[2*idx+1]= offset[2*idx]+length[2*idx], length[2*idx+1]= mid-l+1;
            q.push(std::make_tuple(2*idx+1,l,mid));
            assert( mid+1 <= r );
            if ( 2*idx+2 < num_nodes ) {
                offset[2 * idx + 2] = offset[2 * idx + 1] + length[2 * idx + 1], length[2 * idx + 2] = r - mid;
                q.push(std::make_tuple(2 * idx + 2, mid + 1, r));
            }
        }
        else {
            // in an un-balanced tree, left and right children may be non-existent
            // we however still need to define their offsets
            if ( 2*idx+1 < num_nodes )
                offset[2*idx+1]= offset[2*idx]+length[2*idx], length[2*idx+1]= 0;
            if ( 2*idx+2 < num_nodes )
                offset[2*idx+2]= offset[2*idx+1]+length[2*idx+1], length[2*idx+2]= 0;
        }
    }
}

template<typename size_type, typename value_type>
void range_tree_simple<size_type, value_type>
::search_along_left_ridge( size_type idx, size_type l, size_type r,
                          size_type trg,
                          std::optional<size_type> sc, std::optional<size_type> pr,
                          std::optional<range_tree_simple::result_type*> result, size_type &cnt)
const {
    if ( not(pr.has_value() and sc.has_value()) or *sc > *pr )
        return ;
    assert( offset[idx] <= *sc and *sc <= *pr and *pr <= (idx+1==num_nodes?len-1:offset[idx+1]-1) );
    for ( auto ll= offset[idx]; ll < (idx+1==num_nodes?len:offset[idx+1]); ++ll )
        assert( l <= original[sorted_list[ll]].first and original[sorted_list[ll]].first <= r );
    assert( l <= trg and trg <= r );
    for (;idx < num_nodes;) {
        // assert( valid_nodes->test(idx) );
        if ( l == r ) {
            auto it = offset[idx], jt = offset[idx];
            for ( cnt+= (jt-it+1); it <= jt and result; (*result)->push_back(original[sorted_list[it++]]) );
            return ;
        }
        auto mid= (l+r)/2;
        size_type where_to_descend_next= _left;
        if ( l <= trg and trg <= mid ) {
            // explore the right subtree
            auto it = bridge(_right,_succ,idx,*sc),
                 jt = bridge(_right,_pred,idx,*pr);
            if ( it.has_value() and jt.has_value() ) {
                auto kt= it.value();
                for ( cnt+= (jt.value()-kt+1); kt <= jt.value() and result; (*result)->push_back(original[sorted_list[kt++]]) ) ;
            }
        }
        else where_to_descend_next= _right;
        l= where_to_descend_next==_left?l:mid+1, r= where_to_descend_next==_left?mid:r;
        auto it= bridge(where_to_descend_next,_succ,idx,*sc), jt= bridge(where_to_descend_next,_pred,idx,*pr);
        idx= 2*idx+where_to_descend_next, ++idx;
        if ( (not it.has_value()) or (not jt.has_value()) )
            return ;
        sc= it, pr= jt;
    }
}

template<typename size_type, typename value_type>
void range_tree_simple<size_type, value_type>
::search_along_right_ridge(size_type idx, size_type l, size_type r,
                           size_type trg,
                           std::optional<size_type> sc, std::optional<size_type> pr,
                           std::optional<range_tree_simple::result_type*> result, size_type &cnt)
const {
    //std::cerr << "Entering" << std::endl;
    if ( not(pr.has_value() and sc.has_value()) or *sc > *pr )
        return ;
    assert( l <= trg and trg <= r );
    for (;idx < num_nodes;) {
        // assert( valid_nodes->test(idx) );
        if ( l == r ) {
            auto it = offset[idx], jt = offset[idx];
            for ( cnt += (jt - it + 1); it <= jt and result; (*result)->push_back(original[sorted_list[it++]]) ) ;
            return ;
        }
        auto mid= (l+r)/2;
        size_type where_to_descend_next= _right;
        if ( mid+1 <= trg and trg <= r ) {
            // explore the left subtree
            auto it = bridge(_left,_succ,idx,*sc), jt = bridge(_left,_pred,idx,*pr);
            if ( it.has_value() and jt.has_value() ) {
                auto kt= it.value();
                for ( cnt += (jt.value()-kt+1); kt <= jt.value() and result; (*result)->push_back(original[sorted_list[kt++]]) );
            }
        }
        else where_to_descend_next= _left;
        l= where_to_descend_next==_left?l:mid+1, r= where_to_descend_next==_left?mid:r;
        auto it= bridge(where_to_descend_next,_succ,idx,*sc), jt= bridge(where_to_descend_next,_pred,idx,*pr);
        idx= 2*idx+where_to_descend_next, ++idx;
        if ( (not it.has_value()) or (not jt.has_value()) )
            return ;
        sc= it, pr= jt;
    }
}

template<typename size_type, typename value_type>
void range_tree_simple<size_type, value_type>::presort_lists() {
    for ( size_type idx= 0; idx < num_nodes; ++idx )
        //if ( valid_nodes->test(idx) )
        std::sort(
                sorted_list.begin()+offset[idx],sorted_list.begin()+(idx+1==num_nodes?len:offset[idx+1]),
                [&](auto a, auto b){return original[a].second < original[b].second;}
        );
}

template<typename size_type, typename value_type>
range_tree_simple<size_type,value_type>
&range_tree_simple<size_type, value_type>::operator=(range_tree_simple &&other) noexcept {
    if ( this != &other ) {
        original= std::move(other.original);
        offset = nullptr, offset = std::move(other.offset), other.offset = nullptr;
        sorted_list = std::move(other.sorted_list);
        n = other.n, num_nodes = other.num_nodes, len = other.len;
        //valid_nodes= nullptr, valid_nodes= std::move(other.valid_nodes), other.valid_nodes= nullptr;
    }
    return *this;
}

template<typename size_type, typename value_type>
range_tree_simple<size_type, value_type>::range_tree_simple(range_tree_simple &&other) noexcept {
    original= std::move(other.original);
    offset= std::move(other.offset), other.offset= nullptr;
    sorted_list= std::move(other.sorted_list);
    n= other.n, num_nodes= other.num_nodes, len= other.len;
    //valid_nodes= std::move(other.valid_nodes), other.valid_nodes= nullptr;
}

/*
template<typename size_type, typename value_type>
double range_tree_simple<size_type, value_type>::size_in_bytes() const {
    double ans= sizeof(n)+sizeof(num_nodes)+sizeof(len)+\
                sizeof(offset)+sizeof(bridge)+sizeof(sorted_list);
    if ( offset )
        ans+= num_nodes*sizeof (offset[0]);
    if ( bridge )
        ans+= 4*num_nodes* sizeof ****bridge;
    if ( sorted_list )
        ans+= len*sizeof *sorted_list;
    if ( valid_nodes )
        ans+= valid_nodes->size_in_bytes();
    return ans;
}
*/

/*
template<typename size_type, typename value_type>
void range_tree_simple<size_type, value_type>::mark_valid_nodes() {
    valid_nodes= std::make_unique<simple_bitset>(num_nodes);
    std::queue<std::tuple<size_t,size_t,size_t>> q;
    for ( q.push({0,0,n-1}); not q.empty(); ) {
        auto tpl= q.front(); q.pop();
        auto idx= std::get<0>(tpl), l= std::get<1>(tpl), r= std::get<2>(tpl);
        assert( l <= r );
        valid_nodes->set(idx);
        if ( l == r and 2*idx+1 < num_nodes ) {
            valid_nodes->clr(2*idx+1);
        }
        if ( l == r and 2*idx+2 < num_nodes ) {
            valid_nodes->clr(2*idx+2);
        }
        auto mid= (l+r)/2;
        if ( l < r ) {
            q.push({2*idx+1,l,mid}), q.push({2*idx+2,mid+1,r});
        }
    }
}
*/
#endif //TREE_PATH_QUERIES_RANGE_TREE_SIMPLE_HPP
