//
// Created by sj on 01/11/19.
//

#ifndef SPQ_WAVELET_TREE_HPP
#define SPQ_WAVELET_TREE_HPP
#include <vector>
#include <algorithm>
#include <cmath>
#include <queue>
#include <cassert>
#include <memory>
#include "simple_bitset.hpp"

template <typename size_type= int, typename value_type= int>
class wavelet_tree {
private:

    static std::pair<size_type,size_type> get_config( value_type n ) {
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

    struct state {
        size_type l,r,idx;
        value_type a,b;
        state() { l= r= idx= 0, a= b= 0; }
        state(
                size_type l, size_type r,
                value_type a, value_type b, size_type idx
        ) : l(l), r(r), a(a), b(b), idx(idx) {};
    };
    size_type n, num_nodes, length;
    size_type *backbone= nullptr;
    std::unique_ptr<size_type[]> shifts= nullptr;
    value_type sigma;
    std::unique_ptr<simple_bitset> valid_nodes= nullptr;

    void construct_im( std::unique_ptr<value_type[]> w ) ;
    inline size_type rank0( size_type i ) const ;
    inline size_type rank1( size_type i ) const ;
    // removed these, since we needed explicit instantiation
    // we don't need select queries anyway, unless
    // we want to recover stuff
    //template <typename Func>
    //size_type srch( size_type j, Func f ) ;
    //inline size_type select0( size_type j ) const ;
    //inline size_type select1( size_type j ) const ;

    value_type _range_quantile( size_type idx, size_type i, size_type j,
                                value_type a, value_type b, size_type k ) const ;
    value_type _range_quantile( size_type idx, std::vector<std::pair<size_type,size_type>> &vec,
                                value_type a, value_type b, size_type k ) const ;
    void mark_valid_nodes() ;

public:
    virtual double size_in_bytes() const ;
    // deleting copy-assignment and copy-constructors
    wavelet_tree<size_type,value_type> &operator = ( const wavelet_tree<size_type,value_type> &rhs ) = delete;
    wavelet_tree( const wavelet_tree<size_type,value_type> &rhs ) = delete ;

    wavelet_tree<size_type,value_type> &operator = ( wavelet_tree<size_type,value_type> &&rhs ) noexcept ;
    wavelet_tree( wavelet_tree<size_type,value_type> &&rhs ) noexcept ;
    explicit wavelet_tree( const std::vector<value_type> &w, bool make_power_of_two= true ) ;
    virtual ~wavelet_tree() ;
    virtual value_type range_quantile( size_type i, size_type j, size_type k ) const ;
    virtual value_type range_quantile(
            std::vector<std::pair<size_type,size_type>> &segments, size_type k
    ) const ;
};

template <typename size_type, typename value_type>
void wavelet_tree<size_type,value_type>::construct_im( std::unique_ptr<value_type[]> w ) {
    std::unique_ptr<size_type[]> len= std::make_unique<size_type[]>(2*sigma+7);
    auto* tw= new value_type[n];
    std::queue<state> q;
    auto tmp= state(0,n-1,0,sigma-1,0);
    auto highest_accessed= std::numeric_limits<size_type>::min();

    for ( shifts[tmp.idx]= 0, len[tmp.idx]= n, q.push(std::move(tmp)); not q.empty(); ) {
        auto tr= q.front(); q.pop();
        assert( shifts[tr.idx] < std::numeric_limits<size_type>::max() );
        assert( len[tr.idx] == tr.r-tr.l+1 );
        //std::cerr << "[" << tr.l << "," << tr.r << "]" << std::endl;
        if ( tr.a == tr.b ) {
            backbone[shifts[tr.idx]]= 0, len[tr.idx]= 1; //just some value; we are not going to access it anyway, since it is a leaf
            // highest_accessed= std::max(highest_accessed,shifts[tr.idx]);
            continue;
        }
        assert( tr.a < tr.b );
        auto mid= (tr.a+tr.b)>>1;

        auto ls= 2*tr.idx+1, rs= 2*tr.idx+2;
        len[ls]= len[rs]= 0;

        for ( auto l= tr.l; l <= tr.r; ++l )
            if ( w[l] <= mid ) ++len[ls];
            else ++len[rs];
        auto *lptr= tw;
        auto *rptr= tw+len[ls];
        //if ( tr.l <= tr.r ) {
        //    std::cerr << "[" << shifts[tr.idx] << "," << shifts[tr.idx]+tr.r-tr.l << "]" << std::endl;
        //}
        for ( auto l= tr.l; l <= tr.r; ++l ) {
            assert( shifts[tr.idx]+l-tr.l < length );
            backbone[shifts[tr.idx]+l-tr.l]= (w[l]<=mid) ? (*lptr++= w[l], 0) : (*rptr++= w[l], 1);
            highest_accessed= std::max(highest_accessed,shifts[tr.idx]+l-tr.l);
        }
        lptr= rptr= nullptr;

        for ( auto l= tr.l; l <= tr.r; w[l]= tw[l-tr.l], ++l ) ;

        shifts[ls]= shifts[ls-1]+len[ls-1];
        assert( ls < num_nodes );
        q.push({tr.l,tr.l+len[ls]-1,tr.a,mid,ls});

        assert( rs < num_nodes );
        shifts[rs]= shifts[ls]+len[ls];
        q.push({tr.l+len[ls],tr.r,mid+1,tr.b,rs});
    }
    delete[] tw;

    for ( auto l= 0; l <= highest_accessed; ++l ) {
        if ( not(backbone[l] < std::numeric_limits<size_type>::max()) ) {
            std::cerr << l << " " << length << " " << std::endl;
            std::cerr << length << " " << highest_accessed << std::endl;
        }
        assert( backbone[l] < std::numeric_limits<size_type>::max() );
    }

    value_type carry= backbone[0]; backbone[0]= 0;
    for ( auto l= 1; l <= highest_accessed; ++l ) {
        auto _tmp= carry; carry= backbone[l];
        backbone[l]= backbone[l-1]+_tmp;
    }
    backbone[highest_accessed+1]= backbone[highest_accessed]+carry;
}

template <typename size_type, typename value_type>
inline size_type wavelet_tree<size_type,value_type>::rank0( size_type i ) const {
    assert( backbone[i] < std::numeric_limits<size_type>::max() );
    return i-backbone[i];
}

template <typename size_type, typename value_type>
inline size_type wavelet_tree<size_type,value_type>::rank1( size_type i ) const {
    assert( backbone[i] < std::numeric_limits<size_type>::max() );
    return backbone[i];
}

/*
template <typename size_type,typename value_type>
template <typename Func>
size_type wavelet_tree<size_type,value_type>::srch( size_type j, Func f ) {
    size_type i= 0, low, high, mid, len= n*num_levels;
    for (;(1ll<<i) <= len and f(1ll<<i) < j; ++i ) ;
    if ( (1ll << i) > len ) {
        assert( i and (1ll<<(i-1)) <= len );
        assert( f(1ll<<(i-1)) < j and f(1ll<<i) >= j );
        low= (1ll<<(i-1)), high= len;
    }
    else {
        assert( (1ll<<i) < len and f(1ll<<i) >=j ) ;
        assert( i and f(1ll<<(i-1)) < j );
        low= (1ll<<(i-1)), high= (1ll<<i);
    }
    for (;low+1 < high; f(mid= (low+high)/2) < j ? (low= mid):(high= mid) ) ;
    return low;
}
template <typename size_type,typename value_type>
inline size_type wavelet_tree<size_type,value_type>::select0( size_type j ) const {
    return srch(j,this->rank0);
}
template <typename size_type,typename value_type>
inline size_type wavelet_tree<size_type,value_type>::select1( size_type j ) const {
    return srch(j,this->rank1);
}
*/

template <typename size_type,typename value_type>
value_type wavelet_tree<size_type,value_type>::_range_quantile(
        size_type idx, size_type i, size_type j, value_type a, value_type b, size_type k ) const {
    assert( j-i+1 > k );
    if ( a == b )
        return a;
    assert( a < b );
    auto mid= (a+b)>>1;
    // TODO: watch out for +-1 errors in mappings below
    if ( rank0(j+1)-rank0(i) > k ) {
        auto ni= shifts[idx*2+1]+rank0(i)-rank0(shifts[idx]);
        auto nj= shifts[idx*2+1]+rank0(j+1)-rank0(shifts[idx])-1;
        return _range_quantile(idx*2+1,ni,nj,a,mid,k);
    }
    k-= (rank0(j+1)-rank0(i));
    assert( rank1(j+1)-rank1(i) > k );
    auto ni= shifts[idx*2+2]+rank1(i)-rank1(shifts[idx]);
    auto nj= shifts[idx*2+2]+rank1(j+1)-rank1(shifts[idx])-1;
    return _range_quantile(idx*2+2,ni,nj,mid+1,b,k);
}

// ctor
template <typename size_type,typename value_type>
wavelet_tree<size_type,value_type>::wavelet_tree( const std::vector<value_type> &w, bool make_power_of_two ) {
    sigma= *(std::max_element(begin(w),end(w)))+1;
    for (;make_power_of_two and (sigma & (sigma-1)); ++sigma ) ;
    // making sigma to be closest power of two
    n= w.size();
    auto pr= get_config(sigma);
    num_nodes= pr.first+1;
    // valid_nodes= std::make_unique<simple_bitset>(num_nodes);
    auto num_levels= static_cast<size_type>(floor(log(sigma)/log(2)+1+(1e-7)));
    length= num_levels*n+1;
    // std::cerr << "n*num_levels+1 = " << n*num_levels+1 << ", sigma= " << sigma << std::endl;
    backbone= new size_type[length+1];
    for ( auto x= 0; x < length+1; backbone[x++]= std::numeric_limits<size_type>::max() ) ;
    shifts= std::make_unique<size_type[]>(num_nodes);
    for ( auto l= 0; l < num_nodes; shifts[l++]= std::numeric_limits<size_type>::max() ) ;
    // mark_valid_nodes();
    auto aw= std::make_unique<value_type[]>(w.size());
    for ( auto l= 0; l < w.size(); aw[l]= w[l], ++l ) ;
    construct_im(std::move(aw));
}

// destructor
template <typename size_type,typename value_type>
wavelet_tree<size_type,value_type>::~wavelet_tree() {
    delete[] backbone;
}

template <typename size_type,typename value_type>
value_type wavelet_tree<size_type,value_type>::range_quantile( size_type i, size_type j, size_type k ) const {
    return _range_quantile(0,i,j,0,sigma-1,k);
}

// copy assignment
/*
template<typename size_type, typename value_type>
wavelet_tree<size_type, value_type> &
wavelet_tree<size_type, value_type>::operator=(const wavelet_tree<size_type, value_type> &rhs) {
    std::cerr << "copy assignment" << endl;
    this->sigma= rhs.sigma, this->n= rhs.n, this->num_levels= rhs.num_levels;
    auto lshifts= ;
    auto *bb= new size_type[n*num_levels+1];
    memcpy(lshifts,rhs.shifts,(2*sigma+7)*sizeof(*(rhs.shifts)));
    memcpy(bb,rhs.backbone,(n*num_levels+1)*sizeof(*(rhs.backbone)));
    delete[] this->backbone, delete[] this->shifts;
    this->backbone= bb, this->shifts= lshifts;
    return *this;
}
 */

// move assignment
template<typename size_type, typename value_type>
wavelet_tree<size_type, value_type> &
wavelet_tree<size_type, value_type>::operator=(wavelet_tree<size_type, value_type> &&rhs) noexcept {
    if ( this != &rhs ) {
        this->sigma = rhs.sigma, this->n = rhs.n, this->num_nodes= rhs.num_nodes, this->length= rhs.length;
        this->shifts= nullptr, this->shifts = move(rhs.shifts), rhs.shifts = nullptr;
        delete[] this->backbone, this->backbone = rhs.backbone, rhs.backbone = nullptr;
    }
    return *this;
}

// copy ctor
/*
template<typename size_type, typename value_type>
wavelet_tree<size_type, value_type>::wavelet_tree(const wavelet_tree<size_type, value_type> &rhs) {
    std::cerr << "copy ctor" << endl;
    this->sigma= rhs.sigma, this->n= rhs.n, this->num_levels= rhs.num_levels;
    auto *lshifts= new size_type[2*sigma+7];
    auto *bb= new size_type[n*num_levels+1];
    memcpy(lshifts,rhs.shifts,(2*sigma+7)*sizeof(*(rhs.shifts)));
    memcpy(bb,rhs.backbone,(n*num_levels+1)*sizeof(*(rhs.backbone)));
    this->backbone= bb, this->shifts= lshifts;
}
*/

// move ctor
template<typename size_type, typename value_type>
wavelet_tree<size_type, value_type>::wavelet_tree(wavelet_tree<size_type, value_type> &&rhs) noexcept {
    std::cerr << "move ctor" << std::endl;
    this->sigma= rhs.sigma, this->n= rhs.n, this->num_nodes= rhs.num_nodes, this->length= rhs.length;
    this->shifts= move(rhs.shifts), rhs.shifts= nullptr;
    this->backbone= rhs.backbone, rhs.backbone= nullptr;
}

template<typename size_type, typename value_type>
value_type
wavelet_tree<size_type, value_type>::range_quantile( std::vector<std::pair<size_type,size_type>> &vec, size_type k ) const {
    return _range_quantile(0,vec,0,sigma-1,k);
}

// warning: modifies its own argument
template<typename size_type, typename value_type>
value_type
wavelet_tree<size_type, value_type>::_range_quantile(
        size_type idx, std::vector<std::pair<size_type, size_type>> &segments,
        value_type a, value_type b, size_type k) const {
    if ( a == b ) return a;
    size_type zeros= 0, ones= 0;
    for ( const auto &pr: segments ) {
        auto i= pr.first, j= pr.second;
        if ( i > j ) continue ;
        zeros+= rank0(j+1)-rank0(i), ones+= rank1(j+1)-rank1(i);
    }
    auto mid= (a+b)>>1;
    if ( k < zeros ) {
        for ( auto &pr: segments ) {
            auto i= pr.first, j= pr.second;
            if ( i > j ) continue ;
            auto ni= shifts[idx*2+1]+rank0(i)-rank0(shifts[idx]);
            auto nj= shifts[idx*2+1]+rank0(j+1)-rank0(shifts[idx])-1;
            pr= {ni,nj};
        }
        return _range_quantile(idx*2+1,segments,a,mid,k);
    }
    for ( auto &pr: segments ) {
        auto i= pr.first, j= pr.second;
        if ( i > j ) continue ;
        k-= (rank0(j+1)-rank0(i));
        auto ni= shifts[idx*2+2]+rank1(i)-rank1(shifts[idx]);
        auto nj= shifts[idx*2+2]+rank1(j+1)-rank1(shifts[idx])-1;
        pr= {ni,nj};
    }
    return _range_quantile(idx*2+2,segments,mid+1,b,k);
}

template<typename size_type, typename value_type>
double wavelet_tree<size_type, value_type>::size_in_bytes() const {
    double ans= sizeof(num_nodes)+sizeof(n)+sizeof(sigma)+sizeof(backbone);
    ans+= length*sizeof *backbone;
    ans+= num_nodes*sizeof *(shifts.get());
    return ans;
}

template<typename size_type, typename value_type>
void wavelet_tree<size_type, value_type>::mark_valid_nodes() {
    valid_nodes->clear_all();
    std::queue<std::tuple<size_t,size_t,size_t>> q;
    for ( q.push({0,0,sigma-1}); not q.empty(); ) {
        auto tpl= q.front(); q.pop();
        auto idx= std::get<0>(tpl), l= std::get<1>(tpl), r= std::get<2>(tpl);
        assert( l <= r );
        valid_nodes->set(idx);
        if ( l == r and 2*idx+1 < num_nodes )
            valid_nodes->clr(2*idx+1);
        if ( l == r and 2*idx+2 < num_nodes )
            valid_nodes->clr(2*idx+2);
        auto mid= (l+r)/2;
        if ( l < r ) {
            q.push({2*idx+1,l,mid}), q.push({2*idx+2,mid+1,r});
        }
    }
}
#endif //SPQ_WAVELET_TREE_HPP
