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
#include <optional>
#include <functional>

template <typename size_type= int, typename value_type= int>
class wavelet_tree {
public:
    using point2d= std::pair<size_type,value_type>;
    using result_type= std::vector<std::pair<value_type,size_type>>;
private:

    std::vector<value_type> original_weights;

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
    value_type sigma;
    std::unique_ptr<size_type[]> shifts= nullptr;
	std::vector<size_type> precalc_select[2];

    void construct_im( std::unique_ptr<value_type[]> w ) ;
    inline size_type rank0( size_type i ) const ;
    inline size_type rank1( size_type i ) const ;
    size_type srch( size_type j, std::function<size_type(size_type)> f ) ;
    inline size_type select0( size_type j ) const ;
    inline size_type select1( size_type j ) const ;

    value_type _range_quantile( size_type idx, size_type i, size_type j,
                                value_type a, value_type b, size_type k ) const ;
    value_type _range_quantile( size_type idx, std::vector<std::pair<size_type,size_type>> &vec,
                                value_type a, value_type b, size_type k ) const ;
    void _range_2d_search( size_type idx,
                std::optional<std::vector<std::pair<size_type,size_type>> *> vec,
                value_type a, value_type b,
                std::optional<size_type> i, std::optional<size_type> j,
                value_type qa, value_type qb,
                size_type &cnt
            ) const ;

    std::pair<value_type,size_type> recover( size_type idx, size_type i ) const ;

public:
    virtual double size_in_bytes() const ;
    // deleting copy-assignment and copy-constructors
    wavelet_tree<size_type,value_type> &operator = ( const wavelet_tree<size_type,value_type> &rhs ) = delete;
    wavelet_tree( const wavelet_tree<size_type,value_type> &rhs ) = delete ;

    wavelet_tree<size_type,value_type> &operator = ( wavelet_tree<size_type,value_type> &&rhs ) noexcept ;
    wavelet_tree( wavelet_tree<size_type,value_type> &&rhs ) noexcept ;
    explicit wavelet_tree( const std::vector<value_type> &w, bool make_power_of_two= true ) ;
    virtual ~wavelet_tree() ;
    size_type range_2d_counting_query( size_type qi, size_type qj, value_type qa, value_type qb ) const ;
    void range_2d_reporting_query( size_type qi, size_type qj,
                                           value_type qa, value_type qb,
                                           std::optional<result_type*> result ) const ;
    value_type range_quantile( size_type i, size_type j, size_type k ) const ;
    value_type range_quantile(
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
        assert( ls < 2*sigma+7 );
        assert( rs < 2*sigma+7 );
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
    for ( auto l= highest_accessed+1; l <= length; ++l ) backbone[l]= 0;

	// for constant-time select we precalculate values
	auto ones = std::count_if(backbone,backbone+length+1,[]( auto x ) { return x == 1; });
	auto zeros= std::count_if(backbone,backbone+length+1,[]( auto x ) { return x == 0; });
	precalc_select[0].resize(zeros+1), precalc_select[1].resize(ones+1);
	ones= zeros= 0;
	for ( auto pos= 0; pos <= length; ++pos )
		if ( backbone[pos] == 0 )
			precalc_select[0][++zeros]= pos;
		else precalc_select[1][++ones]= pos;
	assert( precalc_select[0].size() == zeros+1 );
	assert( precalc_select[1].size() == ones+1 );

	// some checks, can be deleted
	// TODO: delete this afterwards
    for ( auto l= 0; l <= length; ++l ) {
        if ( not(backbone[l] < std::numeric_limits<size_type>::max()) ) {
            std::cerr << l << " " << length << " " << std::endl;
            std::cerr << length << " " << highest_accessed << std::endl;
        }
        assert( backbone[l] < std::numeric_limits<size_type>::max() );
    }

    value_type carry= backbone[0]; backbone[0]= 0;
    for ( auto l= 1; l < length; ++l ) {
        auto _tmp= carry; carry= backbone[l];
        backbone[l]= backbone[l-1]+_tmp;
    }
    backbone[length]= backbone[length-1]+carry;
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
    original_weights= w;
    sigma= *(std::max_element(begin(w),end(w)))+1;
    for (;make_power_of_two and (sigma & (sigma-1)); ++sigma ) ;
    // making sigma to be closest power of two
    n= w.size();
    auto pr= get_config(sigma);
    num_nodes= pr.first+1;
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
size_type wavelet_tree<size_type, value_type>::srch( size_type j, std::function<size_type(size_type)> f ) {
    size_type low= 0, high= length;
    assert( f(high) >= j );
    if ( f(low) == j )
        return low;
    assert( f(low) < j );
    for ( ;low+1 < high; ) {
        auto mid= (low+high)>>1;
        f(mid)<j?(low= mid):(high= mid);
    }
    assert( low+1 == high );
    assert( f(low) < j );
    assert( f(high) == j );
    return high;
}

// we'll first go for a logarithmic select
template<typename size_type, typename value_type>
size_type wavelet_tree<size_type, value_type>::select0( size_type j ) const {
	/*
    assert( j );
    // return the position i s.t. rank_0[i] == j, and rank_0[i-1] == j-1
    size_type low= 0, high= length;
    assert( rank0(high) >= j );
    assert( rank0(low) < j );
    for ( ;low+1 < high; ) {
        auto mid= (low+high)>>1;
        rank0(mid)<j?(low= mid):(high= mid);
    }
    assert( low+1 == high );
    assert( rank0(low) < j );
    assert( rank0(high) == j );
    return low;
	*/
	assert( 1 <= j and j < precalc_select[0].size() );
	return precalc_select[0][j];
}

template<typename size_type, typename value_type>
size_type wavelet_tree<size_type, value_type>::select1(size_type j) const {
	/*
    assert( j );
    size_type low= 0, high= length;
    assert( rank1(high) >= j );
    assert( rank1(low) < j );
    for ( ;low+1 < high; ) {
        auto mid= (low+high)>>1;
        rank1(mid)<j?(low= mid):(high= mid);
    }
    assert( low+1 == high );
    assert( rank1(low) < j );
    assert( rank1(high) == j );
    return low;
	*/
	assert( 1 <= j and j < precalc_select[1].size() );
	return precalc_select[1][j];
    // return srch(j,rank1());
}

template<typename size_type, typename value_type>
void
wavelet_tree<size_type, value_type>::_range_2d_search(
        size_type idx,
        std::optional<std::vector<std::pair<size_type, size_type>> *> vec,
        value_type a, value_type b,
        std::optional<size_type> qi, std::optional<size_type> qj,
        value_type qa, value_type qb,
        size_type &cnt ) const {

    if ( (not qi) or (not qj) or *qi > *qj ) return ;
    if ( qb < a or b < qa ) return ;

    // base case
    if ( qa <= a and b <= qb ) {
        cnt+= (*qj)-(*qi)+1;
        for ( auto t= *qi; vec and t <= *qj; (*vec)->push_back(recover(idx,t++)) )
            ;
        return ;
    }

    auto mid= (a+b)>>1;

    auto _left= 2*idx+1, _right= _left+1;
    std::optional<size_type> li= (rank0(*qi)-rank0(shifts[idx]))+shifts[2*idx+1];
    std::optional<size_type> lj= [&]() {
        auto x= (rank0(*qj+1)-rank0(shifts[idx]))+shifts[2*idx+1];
        return x<1?std::nullopt:std::optional<size_type>(x-1);
    }();
    std::optional<size_type> ri= (rank1(*qi)-rank1(shifts[idx]))+shifts[2*idx+2];
    std::optional<size_type> rj= [&]() {
        auto x= (rank1(*qj+1)-rank1(shifts[idx]))+shifts[2*idx+2];
        return x<1?std::nullopt:std::optional<size_type>(x-1);
    }();

    _range_2d_search(2*idx+1,vec,a,mid,li,lj,qa,qb,cnt);
    _range_2d_search(2*idx+2,vec,mid+1,b,ri,rj,qa,qb,cnt);

}

template<typename size_type, typename value_type>
size_type wavelet_tree<size_type, value_type>::range_2d_counting_query(
        size_type qi, size_type qj, value_type qa, value_type qb) const {
    size_type cnt= 0;
    _range_2d_search(0,std::nullopt,0,sigma-1,
            std::optional<size_type>(qi),std::optional<size_type>(qj),
                    qa,qb,cnt);
    return cnt;
}

template<typename size_type, typename value_type>
std::pair<value_type,size_type> wavelet_tree<size_type, value_type>::recover(size_type idx, size_type i) const {
    if ( idx == 0 ) {
        assert( 0 <= i and i < original_weights.size() );
        return {i,original_weights[i]};
    };
    auto parent_id= (idx-1)>>1;
    auto pos= i-shifts[idx];
    return (idx&1)?\
           recover(parent_id,select0(rank0(shifts[parent_id])+pos+1)):\
           recover(parent_id,select1(rank1(shifts[parent_id])+pos+1));
}

template<typename size_type, typename value_type>
void wavelet_tree<size_type, value_type>::range_2d_reporting_query( size_type qi, size_type qj,
                               value_type qa, value_type qb,
                               std::optional<std::vector<std::pair<value_type,size_type>> *> result ) const {
    size_type cnt= 0;
    _range_2d_search(0,result,0,sigma-1,qi,qj,qa,qb,cnt);
}

#endif //SPQ_WAVELET_TREE_HPP
