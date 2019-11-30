//
// Created by sj on 10/04/19.
//
#ifndef SPQ_EXT_PTR_CPP
#define SPQ_EXT_PTR_CPP
#include <sdsl/rank_support.hpp>
#include <sdsl/int_vector.hpp>
#include "interfaces/path_query_processor.hpp"
#include "interfaces/succinct_container.hpp"
#include "interfaces/succinct_tree.hpp"
#include "custom_types/rs_bitvector.hpp"
#include "custom_types/rs_bitvector01.hpp"
#include "custom_types/ff_bitvector_v01.hpp"
#include "utils/pq_types.hpp"

class ext_ptr: public succinct_tree,
               public path_query_processor,
               public succinct_container {
public:
    using node_type= pq_types::node_type;
    using size_type= pq_types::size_type;
    using value_type= pq_types::value_type;

protected:
    size_type n,K;
    node_type **precomputed_image= nullptr, *original_node= nullptr, *prnt= nullptr;
    mutable node_type **anc= nullptr;
    value_type *wgt= nullptr, lower, upper;
    size_type *d= nullptr;
    ext_ptr *subtree[2];

    node_type up( node_type x, size_type u ) const {
        assert( anc );
        for ( size_type k= 0; u; ++k, u >>= 1 )
            if ( u & 1 ) {
                assert( k < K );
                x = anc[x][k];
            }
        return x;
    }

    void init( value_type a, value_type b, const tree_extraction<> *compressed_tree ) {
        for ( auto t= 0; t <= 1; subtree[t++] = nullptr ) ;
        if ( (this->lower= a) == (this->upper= b) or not compressed_tree )
            return ;
        tree_extraction<> *tr[]= {compressed_tree->t[0],compressed_tree->t[1]};
        auto mid= (a+b)>>1;
        precomputed_image= new node_type*[2];
        for ( auto t= 0; t <= 1; precomputed_image[t++]= nullptr ) ;
        for ( value_type t= 0; t <= 1; ++t ) {
            if ( not tr[t] ) continue ;
            precomputed_image[t]= new node_type[n];
            for (node_type x = 0; x < n; ++x)
                precomputed_image[t][x] = compressed_tree->image_of(x, t);
            auto na= (t==0?a:mid+1), nb= (t==0?mid:b);
            subtree[t]= new ext_ptr(na,nb,tr[t]);
        }
    }

    ext_ptr( value_type a, value_type b, const tree_extraction<> *compressed_tree ) {
        d= new size_type[n= compressed_tree->size()], original_node= new node_type[n];
        prnt= new node_type[n];
        for ( node_type x= 0; x < n; original_node[x]= compressed_tree->pre_image(x),\
        d[x]= compressed_tree->depth(x), prnt[x]= compressed_tree->parent(x), ++x ) ;
        init(a,b,compressed_tree);
    }

    virtual value_type
	_query( node_type x, node_type y, node_type z, const value_type wc, size_type k,
			const value_type a, const value_type b ) const {
		if ( a == b ) {
			assert( k < this->size() );
			return a;
		}
		//assert( this->a < this->b );
		auto mid= (a+b)>>1;

		size_type accum= 0;
		for ( value_type i= 0; i < 2; ++i ) {
			if ( not subtree[i] ) continue ;
			node_type ix,iy,iz;
			size_type dw,dx,dy,dz;
			assert( subtree[i]->size() >= 1 );
			ix= (x<+OO?precomputed_image[i][x]:x), iy= (y<+OO?precomputed_image[i][y]:y), iz= (z<+OO?precomputed_image[i][z]:z);
			dx= (ix<+OO?subtree[i]->depth(ix)+1:0), dy= (iy<+OO?subtree[i]->depth(iy)+1:0), dz= (iz<+OO?subtree[i]->depth(iz)+1:0);
			//dw= dx+dy+(lies_inside(wc,this->t[i]->a,this->t[i]->b)?1:0)-2*dz;
			auto na= (i==0?a:mid+1), nb= (i==0?mid:b);
			dw= dx+dy+(lies_inside(wc,na,nb)?1:0)-2*dz;
			if ( accum+dw > k )
				return subtree[i]->_query(ix,iy,iz,wc,k-accum,na,nb);
			accum+= dw;
		}
		assert( false );
		return 0;
	}

	size_type
	search(
	        const ext_ptr &root_element,
			node_type x, node_type y, node_type z,
			const value_type p, const value_type q,
			const value_type wc,
		 	std::vector<std::pair<value_type,size_type>> &vec,
		 	bool rprt,
		 	const value_type a, const value_type b ) const {

		node_type ix,iy,iz;
		size_type dw,dx,dy,dz;

		if ( p <= a && b <= q ) {
			if ( rprt ) {
				for ( auto cx= x; cx != z; cx= this->parent(cx) ) {
					auto original_cx= original_node[cx];
					vec.emplace_back(original_cx,root_element.weight_of(original_cx));
				}
				for ( auto cy= y; cy != z; cy= this->parent(cy) ) {
					auto original_cy= original_node[cy];
					vec.emplace_back(original_cy,root_element.weight_of(original_cy));
				}
				if ( lies_inside(wc,a,b) ) {
					auto original_z= original_node[z];
					vec.emplace_back(original_z,root_element.weight_of(original_z));
				}
			}
			dx= (x<+OO?depth(x)+1:0), dy= (y<+OO?depth(y)+1:0), dz= (z<+OO?depth(z)+1:0);
			dw= dx+dy+(lies_inside(wc,a,b)?1:0)-2*dz;
			return dw;
		}

		size_type res= 0;
		if ( a > q || b < p ) return res;
		auto mid= (a+b)>>1;
		for ( auto i= 0; i < 2; ++i )
			if ( subtree[i] and subtree[i]->size() >= 1 ) {
				ix= (x<+OO?precomputed_image[i][x]:x), iy= (y<+OO?precomputed_image[i][y]:y), iz= (z<+OO?precomputed_image[i][z]:z);
				res+= subtree[i]->search(root_element,ix,iy,iz,p,q,wc,vec,rprt,(i==0?a:mid+1),(i==0?mid:b));
			}
		return res;
	}

	ext_ptr() {};

public:

    value_type weight(const node_type x) const override { return wgt[x]; }
    value_type weight_of(const node_type x) const override { return weight(x); }

    /* the idea is to first build the tree extraction
     * in succinct form, and then uncompress it */
    ext_ptr( const std::string &s, const std::vector<value_type> &w ) {
        method_name= "{\\texttt{ext.ptr}}";
        auto compressed_version= new tree_extraction<>(s,w);
        auto topology= dynamic_cast<succinct_tree *>(compressed_version);
        wgt= new value_type[n= compressed_version->size()];
        d= new size_type[n], prnt= new node_type[n];
        for ( node_type x= 0; x < n; wgt[x]= w[x], d[x]= compressed_version->depth(x),\
        prnt[x]= compressed_version->parent(x), ++x ) ;
        value_type minw= std::numeric_limits<value_type>::max(),
                   maxw= std::numeric_limits<value_type>::min();
        std::for_each(wgt,wgt+n,[&](auto x){minw= std::min(minw,x), maxw= std::max(maxw,x);});
        for ( K= 0; (1<<K) <= n; ++K ) ;
        anc= new node_type*[n];
        for ( auto i= 0; i < n; ++i ) {
            anc[i] = new node_type[K];
            std::for_each(anc[i],anc[i]+K,[&](auto &x){x= n;});
        }
        std::queue<node_type> q;
        bool *seen= new bool[n];
        for ( node_type x= 0; x < n; seen[x++]= false ) ;
        int cnt= 0;
        for ( seen[0]= true, q.push(0); !q.empty(); ) {
            auto x= q.front(); q.pop(), ++cnt;
            auto kinder= topology->children(x);
            std::for_each(kinder.begin(),kinder.end(),[&]( auto y ){
                if ( not seen[y] ) {
                    q.push(y), seen[y] = true, anc[y][0]= x;
                    for ( auto k= 1; anc[y][k-1] < n; anc[y][k]= anc[anc[y][k-1]][k-1], ++k ) ;
                }
            });
        }
        assert( cnt == n );
        delete[] seen;
        init(0,maxw,compressed_version);
        delete compressed_version;
    }

    ext_ptr( const std::string &s, const std::vector<value_type> &w, const std::string &dsname ):ext_ptr(s,w) {
        dataset_name= dsname;
    }

    ext_ptr( const std::string &s ):ext_ptr( s, *(new std::vector<value_type>(s.size()/2,1)) ) {}

    value_type query(const node_type x, const node_type y) const override {
        auto z= lca(x,y);
		assert( z < +OO );
		auto len= depth(x)+depth(y)+1-2*depth(z);
		return _query(x,y,z,weight_of(z),len>>1,lower,upper);
    }

    value_type selection(const node_type x, const node_type y, const size_type qntl) const override {
        auto z= lca(x,y);
		assert( z < +OO );
		auto len= depth(x)+depth(y)+1-2*depth(z);
		return _query(x,y,z,weight_of(z),qntl2rnk(len,qntl),lower,upper);
    }

    /*
    const ext_ptr &root_element,
			node_type x, node_type y, node_type z,
			const value_type p, const value_type q,
			const value_type wc,
		 	std::vector<std::pair<node_type,value_type>> &vec,
		 	bool rprt,
		 	const value_type a, const value_type b ) const
     */

    size_type count(const node_type x, const node_type y, const value_type a, const value_type b) const override {
        auto z= lca(x,y);
        std::vector<std::pair<value_type,size_type>> vec;
        return search(*this,x,y,z,a,b,weight_of(z),vec,false,lower,upper);
    }

    void report(const node_type x, const node_type y, const value_type a, const value_type b,
                std::vector<std::pair<value_type, size_type>> &res) const override {
        auto z= lca(x,y);
        search(*this,x,y,z,a,b,weight_of(z),res,true,lower,upper);
    }

    ~ext_ptr() override {
        for ( auto t= 0; t <= 1; ++t ) {
            if ( subtree[t] ) delete subtree[t];
            if ( precomputed_image and precomputed_image[t] )
                delete[] precomputed_image[t];
        }
        delete[] original_node, delete[] d, delete[] wgt;
        delete[] prnt;
        if ( anc ) {
            for ( auto x= 0; x < n; delete[] anc[x++] ) ;
            delete[] anc;
        }
    }

    nlohmann::json space_report() const override {
        return nlohmann::json();
    }

    size_type size() const override { return n; }

    double size_in_bytes() const override {
        double res= sizeof n + sizeof K + sizeof precomputed_image + sizeof original_node +\
            sizeof prnt + sizeof anc + sizeof wgt + sizeof lower + sizeof upper + sizeof d +\
            sizeof subtree[0] + sizeof subtree[1];
        std::for_each(subtree,subtree+1,[&]( const auto *pt ) { if ( pt ) res+= pt->size_in_bytes(); });
        if ( precomputed_image ) {
            for ( auto t= 0; t <= 1; ++t ) {
                res += sizeof precomputed_image[t];
                if ( precomputed_image[t] )
                    res += n*sizeof *precomputed_image[t];
            }
        }
        if ( anc ) {
            for ( auto x= 0; x < n; ++x ) {
                res += sizeof anc[x];
                if ( anc[x] )
                    res+= K*sizeof *anc[x];
            }
        }
        if ( original_node )
            res += n*sizeof *original_node;
        if ( prnt )
            res += n*sizeof *prnt;
        if ( wgt )
            res += n*sizeof *wgt;
        if ( d )
            res += n*sizeof *d;
        return res;
    }

    double bits_per_node() const override {
        return 8*size_in_bytes()/(size()+0.00);
    }

    node_type parent(const node_type x) const override {
        return prnt[x];
    }

    node_type ancestor(const node_type x, const size_type i) const override {
        return 0;
    }

    std::vector<node_type> children(const node_type x) const override {
        return std::vector<node_type>();
    }

    /* we call LCA only once, at the uppermost level
     * so this all is valid only at that level, otherwise anc is NULL */
    node_type lca(const node_type x, const node_type y) const override {
        if ( depth(x) > depth(y) )
            return lca(up(x,depth(x)-depth(y)),y);
        if ( depth(x) < depth(y) )
            return lca(y,x);
        if ( x == y )
            return x;
        node_type u= x, v= y;
        for ( int k= K-1; k; --k ) {
            assert( anc[u][k] == anc[v][k] );
            if ( anc[u][k-1] != anc[v][k-1] )
                u= anc[u][k-1], v= anc[v][k-1];
        }
        return anc[u][0];
    }

    size_type depth(const node_type x) const override { return d[x]; }

    bool is_ancestor(const node_type p, const node_type x) const override {
        return lca(p,x) == p;
    }

    bool is_leaf(const node_type x) const override {
        return false;
    }
};

#endif //SPQ_EXT_PTR_CPP
