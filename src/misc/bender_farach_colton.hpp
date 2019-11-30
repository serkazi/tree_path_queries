#ifndef BENDER_FARACH_LCA
#define BENDER_FARACH_LCA

#include <algorithm>
#include <cassert>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include "succinct_tree.hpp"
#include "pq_types.hpp"

template<typename size_type= pq_types::size_type>
class small_block_rmq {
	size_type BS;
	std::unique_ptr<std::unique_ptr<std::unique_ptr<size_type[]>[]>[]> M= nullptr;
	void preprocess() {
		for ( auto u= 0; u < (1<<(BS-1)); ++u )
			for ( auto i= 0; i < BS; M[u][i][i]= i, ++i ) ;
		for ( auto k= 2; k <= BS; ++k )
			for ( auto u= 0; u < (1<<(BS-1)); ++u ) 
				for ( auto i= 0, j=i+k-1; (j=i+k-1) < BS; ++i ) {
					M[u][i][j]= i;
					for ( auto curmin= 0, cursum= 0, t= i+1; t <= j; ++t )
						if ( (cursum+= ((u>>(t-1))&1)?1:-1) < curmin )
							curmin= cursum,  M[u][i][j]= t;
				}
	}
public:
	small_block_rmq( size_type bs ) : BS{bs} {
		M= std::make_unique<std::unique_ptr<std::unique_ptr<size_type[]>[]>[]>((1<<(BS-1)));
		for ( auto u= 0; u < (1<<(BS-1)); ++u ) {
			M[u]= std::make_unique<std::unique_ptr<size_type[]>[]>(BS);
			for ( auto i= 0; i < BS; M[u][i++]= std::make_unique<size_type[]>(BS) ) ;
		}
		preprocess();
	}
	virtual ~small_block_rmq() = default;
	size_type operator () ( size_type u, size_type i, size_type j ) const {
		if ( i > j ) std::swap(i,j);
		assert( u < (1<<(BS-1)) );
		assert( j-i+1 <= BS );
		assert( i < BS && j < BS );
		return M[u][i][j];
	}
	// TODO
	[[nodiscard]] double size_in_bytes() const {
		return (1<<(BS-1))*BS*BS*sizeof ***M;
	}
};

template<typename size_type= pq_types::size_type>
class sparse_table_rmq {
	const size_type *A= nullptr;
	std::unique_ptr<std::unique_ptr<size_type[]>[]> M= nullptr;
	size_type n, K;
public:
	sparse_table_rmq( const size_type *AA, size_type nn ) : A(AA), n{nn} {
		for ( K= 0; (1<<K) <= n; ++K ) ;
		M= std::make_unique<std::unique_ptr<size_type[]>[]>(n);
		for ( auto i= 0; i < n; M[i++]= std::make_unique<size_type[]>(K) ) ;
		for ( auto i= 0; i < n; M[i][0]= i, ++i ) ;
		for ( auto k= 1; k < K; ++k )
			for ( auto i= 0; i+(1<<k) < n; ++i ) {
				assert( M[i][k-1] < n );
				assert( M[i+(1<<(k-1))][k-1] < n );
				M[i][k]= A[M[i][k-1]]<A[M[i+(1<<(k-1))][k-1]]?M[i][k-1]:M[i+(1<<(k-1))][k-1];
			}
	}

	virtual ~sparse_table_rmq() = default;

	size_type operator() ( size_type i, size_type j ) const {
		assert( i <= j );
		if ( i == j ) 
			return i;
		assert( i < j );
		size_type k,l,rgt;
		for ( k= 0; i+(1<<k) <= j; ++k ) ;
		assert( i+(1<<(k-1)) <= j );
		return A[l=M[i][k-1]] < A[rgt=M[j-(1<<(k-1))+1][k-1]] ? l:rgt;
	}
	[[nodiscard]] double size_in_bytes() const {
		return n*K*sizeof **M;
	}
};

/*
 * ±1RMQ 
 */
template<typename size_type= pq_types::size_type>
class pm_one_rmq {
	size_type n,K,BS,border;
    std::unique_ptr<size_type[]> A= nullptr;
    std::unique_ptr<size_type[]> L= nullptr;
	std::unique_ptr<size_type[]> min_for_block= nullptr;
	std::unique_ptr<size_type[]> B= nullptr;
	std::unique_ptr<sparse_table_rmq<size_type>> st= nullptr;
	std::unique_ptr<small_block_rmq<size_type>> smbl= nullptr;
	std::unique_ptr<size_type[]> block_type= nullptr;

	[[nodiscard]] size_type _rmq( size_type i, size_type j ) const {
		if ( i > j ) std::swap(i,j);
		size_type ans, bi= i/BS, bj= j/BS;
		if ( bi == bj )
			return bi*BS+(*smbl)(block_type[bi],i-bi*BS,j-bj*BS);
		auto idxi = bi*BS+(*smbl)(block_type[bi],i-bi*BS,BS-1),
			 idxj = bj*BS+(*smbl)(block_type[bj],0,j-bj*BS);
		ans= L[idxi]<L[idxj]?idxi:idxj;
		if ( bi+1 <= bj-1 )
			if ( auto t= (*st)(bi+1,bj-1); A[t] < L[ans] )
				ans= t*BS+B[t];
		return ans;
	}

public:

	pm_one_rmq( std::unique_ptr<size_type[]> a, pq_types::size_type nn ) : L{std::move(a)}, n{nn} {
		for ( K= 0; (1<<K) <= n; ++K ) ;
		for ( BS= (K>>1), border= n; n%BS; ++n ) ;
		A= std::make_unique<size_type[]>(n/BS+3);
		B= std::make_unique<size_type[]>(n/BS+3);
		smbl= std::make_unique<small_block_rmq<size_type>>(BS);
		for ( auto t= 0; t < n/BS; ++t ) {
			A[t]= L[t*BS], B[t]= 0;
			for ( auto i= t*BS+1; i < t*BS+BS && i < border; ++i ) 
				if ( L[i] < A[t] )
					A[t]= L[i], B[t]= i-t*BS;
		}
		// you move A and then call it in line 112... Too bad
		st= std::make_unique<sparse_table_rmq<size_type>>(A.get(),n/BS);
		block_type= std::make_unique<size_type[]>(n/BS+1);
		for ( auto t= 0; t < n/BS; ++t ) {
			size_type u= (1<<(BS-1))-1;
			for ( auto i= t*BS+1; i < t*BS+BS && i < border; ++i ) 
				if ( L[i] != L[i-1]+1 ) 
					u &= ~(1<<(i-t*BS-1));
			block_type[t]= u;
		}
	}

	size_type operator ()( size_type i, size_type j ) const {
		return _rmq(i,j);
	}

	virtual ~pm_one_rmq() = default;

	// TODO: account for "a" as well
	[[nodiscard]] double size_in_bytes() const {
	    return 0;
		//return (n/BS+3)*(sizeof *A+sizeof *B+sizeof *block_type)+st->size_in_bytes()+smbl->size_in_bytes();
	}
};

template<typename node_type= pq_types::node_type, typename size_type= pq_types::size_type>
class lca_processor {
	std::unique_ptr<size_type[]> L;
	std::unique_ptr<size_type[]> R;
	std::unique_ptr<node_type[]> E;
	const succinct_tree<node_type,size_type> *T;
	size_type n;
	void euler_tour( node_type x, size_type d, size_type &cur ) {
		L[R[E[cur]= x]= cur]= d, ++cur;
		auto children= T->children(x);
		for ( auto y: children )
			euler_tour(y,1+d,cur), L[cur]= d, E[cur++]= x;
	}

	void euler_tour_with_explicit_stack( node_type src, size_type &cur ) {
	    node_type *vst,*vtop,x,y;
	    size_type *ist,*itop,i,j,*dst,*dtop;

	    vtop= vst= new node_type[2*n+7];
	    itop= ist= new size_type[2*n+7];
        dtop= dst= new size_type[2*n+7];

	    for ( cur= 0, L[R[E[cur]= *++vtop= src]= cur]= *++dtop= 0, ++cur, *++itop= 0; vtop > vst; ) {
	        x= *vtop--, i= *itop--;
	        auto d= *dtop--;
	        //TODO: source of inefficiency; keeps creating vector all the time
	        // possible fix: keep the children in some place, i.e. pre-calc the vectors
	        // instead of creating them anew
	        // another option: implement i-th child operation in succinct tree
            auto children= T->children(x);
            if ( i == children.size() ) {
                if ( x != src ) {
                    auto papa= T->parent(x);
                    L[cur]= (d-1), E[cur++]= papa.value();
                }
                continue ;
            }
            assert( i < children.size() );
            y= children[i++], *++vtop= x, *++itop= i, *++dtop= d;
            L[R[E[cur]= *++vtop= y]= cur]= *++dtop= (d+1), ++cur, *++itop= 0;
	    }

	    delete[] vst, delete[] ist, delete[] dst;
	}

	std::unique_ptr<pm_one_rmq<size_type>> rmq;

public:

	lca_processor( const succinct_tree<node_type,size_type> *t ): T(t), n(t->size()) {
		L= std::make_unique<size_type[]>(2*n);
		R= std::make_unique<size_type[]>(2*n);
		E= std::make_unique<node_type[]>(2*n);
		pq_types::size_type cur= 0;
		//euler_tour(0,0,cur);
        euler_tour_with_explicit_stack(0,cur);
		assert( cur == 2*n-1 );
		rmq= std::make_unique<pm_one_rmq<size_type>>(std::move(L),2*n-1);
		L= nullptr;
	}
	virtual ~lca_processor() = default;
	node_type operator ()( node_type x, node_type y ) const {
		return E[(*rmq)(R[x],R[y])];
	}
	[[nodiscard]] double size_in_bytes() const {
		//return 2*n*(sizeof *R+sizeof *E)+rmq->size_in_bytes();
		return 0;
	}
};
#endif //BENDER_FARACH_LCA
