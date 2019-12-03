//
// Created by sj on 01/06/19.
//
#ifndef SPQ_EXT_PTR_V3_HPP
#define SPQ_EXT_PTR_V3_HPP
#include "pq_types.hpp"
#include "path_query_processor.hpp"
#include "bender_farach_colton.hpp"
#include <memory>
#include <cstdio>
#include <optional>
#include <stack>

template<
        typename node_type= pq_types::node_type,
        typename size_type= pq_types::size_type,
        typename value_type= pq_types::value_type
        >
class ext_ptr:
        public path_query_processor<node_type,size_type,value_type>,
        public succinct_tree<node_type,size_type>
{
private:
    //inner classes
    class weighted_tree {
    private:
        std::unique_ptr<std::vector<node_type>[]> adj;
        size_type n,E;
        std::unique_ptr<size_type[]> d;
        std::unique_ptr<value_type[]> weights;
        node_type V;
        std::unique_ptr<node_type[]> prnt, oid;
    public:
        weighted_tree( size_type n ) {
            this->n= n, E= V= 0;
            oid= std::make_unique<node_type[]>(n);
            weights= std::make_unique<value_type[]>(n);
            prnt= std::make_unique<node_type[]>(n);
            d= std::make_unique<size_type[]>(n);
            adj= std::make_unique<std::vector<node_type>[]>(n);
            for ( node_type x= 0; x < n; adj[x++].clear() ) ;
        }
        size_type nvertices() const { return V; }
        node_type original_id( node_type x ) const { return oid[x]; }
        ~weighted_tree() = default;
        void add_arc( node_type p, node_type x ) { ++E, adj[prnt[x]= p].push_back(x); }
        value_type weight_of( node_type x ) const { return weights[x]; }
        const std::vector<node_type> &children( node_type x ) const {
            return adj[x];
        }
        size_type size() const { return n; }
        std::optional<node_type> parent( node_type x ) const {
            if ( x == 0 )
                return std::nullopt;
            return prnt[x];
        }
        node_type depth( node_type x ) const { return d[x]; }
        void assign_weight( node_type x, value_type w ) { weights[x]= w; }
        void assign_depth( node_type x, value_type w ) { d[x]= w; }
        node_type new_node( node_type _oid ) {
            oid[V++]= _oid;
            return V-1;
        }
        size_type nedges() const { return E; }
        weighted_tree( const std::string &s, const std::vector<value_type> &w ) {
            this->n= s.size()/2, E= V= 0;
            std::stack<node_type> stck;
            oid= std::make_unique<node_type[]>(n);
            for ( node_type x= 0; x < n; oid[x]= x, ++x ) ;
            weights= std::make_unique<value_type[]>(n);
            prnt= std::make_unique<node_type[]>(n);
            d= std::make_unique<size_type[]>(n);
            adj= std::make_unique<std::vector<node_type>[]>(n);
            for ( node_type x= 0; x < n; adj[x++].clear() ) ;
            for ( int i= 0; i < (int)w.size(); weights[i]= w[i], ++i ) ;
            for ( char ch: s )
                if ( ch == '(' ) {
                    if ( not stck.empty() )
                        add_arc(prnt[V]= stck.top(),V);
                    d[V]= stck.size(), stck.push(V++);
                }
                else stck.pop();
            assert( V == n );
        }
    };

    std::unique_ptr<std::unique_ptr<std::optional<node_type>[]>[]> precomputed_image= nullptr;
    std::unique_ptr<node_type[]> original_node= nullptr, prnt= nullptr;
    std::unique_ptr<value_type[]> wgt= nullptr;
    std::unique_ptr<std::vector<node_type>[]> adj= nullptr;
    std::unique_ptr<size_type[]> d= nullptr;
    size_type n,K;
    value_type lower, upper;
    ext_ptr<node_type,size_type,value_type> *subtree[2];

    ext_ptr( value_type a, value_type b,
            const weighted_tree *un_compressed_tree )
    {
        d= std::make_unique<size_type[]>(n= un_compressed_tree->size());
        original_node= std::make_unique<node_type[]>(n);
        prnt= std::make_unique<node_type[]>(n);
        for ( node_type x= 0; x < n; original_node[x]= un_compressed_tree->original_id(x),\
        d[x]= un_compressed_tree->depth(x), prnt[x]= x==0?0:un_compressed_tree->parent(x).value(), ++x ) ;
        init(a,b,un_compressed_tree);
    }

    node_type *st[2],*top[2];
    std::unique_ptr<bool[]> seen= nullptr;

    void precalc( const weighted_tree *tr, node_type x, weighted_tree *str[], value_type mid ) {
        assert( not seen[x] );
        seen[x]= true;
        value_type color= tr->weight_of(x)<=mid?0:1;
        node_type ix= str[color]->new_node(tr->original_id(x));
        std::optional<node_type> px= (top[color]==st[color]?std::nullopt:std::optional<node_type>(*top[color]));
        str[color]->assign_weight(ix,tr->weight_of(x));
        str[color]->assign_depth(ix,top[color]-st[color]);
        if ( px.has_value() )
            str[color]->add_arc(px.value(),ix);
        *++top[color]= ix;
        for ( value_type tt= 0; tt <= 1; ++tt )
            if ( precomputed_image[tt] != nullptr )
                precomputed_image[tt][x]= (top[tt]>st[tt]?std::optional<node_type>(*top[tt]):std::nullopt);
        auto kinder= tr->children(x);
        for ( auto y: kinder ) {
            assert( not seen[y] );
            precalc(tr,y,str,mid);
        }
        assert( *top[color] == ix );
        --top[color];
    }

    void init( value_type a, value_type b, const weighted_tree *un_compressed_tree ) {
        for ( auto t= 0; t <= 1; subtree[t++] = nullptr ) ;
        if ( (this->lower= a) == (this->upper= b) or not un_compressed_tree )
            return ;
        auto mid= (a+b)>>1;
        size_type nsz[]= {0,0};
        for ( node_type x= 0; x < n; ++x ) {
            assert( a <= un_compressed_tree->weight_of(x) and un_compressed_tree->weight_of(x) <= b );
            ++nsz[un_compressed_tree->weight_of(x) <= mid ? 0 : 1];
        }

        weighted_tree *str[]= { new weighted_tree(nsz[0]), new weighted_tree(nsz[1]) };

        precomputed_image= std::make_unique<std::unique_ptr<std::optional<node_type>[]>[]>(2);
        for ( auto tt= 0; tt <= 1; ++tt )
            if ( str[tt] and str[tt]->size() >= 1 )
                precomputed_image[tt]= std::make_unique<std::optional<node_type>[]>(n);
            else precomputed_image[tt] = nullptr;
        //for ( auto t= 0; t <= 1; precomputed_image[t++]= new node_type[n] ) ;
        for ( auto t= 0; t <= 1; ++t )
            top[t]= st[t]= new node_type[nsz[t]+1];
        seen= std::make_unique<bool[]>(n);
        for ( node_type x= 0; x < n; seen[x++]= false ) ;
        for ( node_type x= 0; x < n; ++x )
            if ( not seen[x] )
                precalc(un_compressed_tree,x,str,mid);
        seen= nullptr;
        for ( value_type t= 0; t <= 1; delete str[t++] )
            if ( nsz[t] ) {
                auto na = (t == 0 ? a : mid + 1), nb = (t == 0 ? mid : b);
                assert(top[t] == st[t]);
                subtree[t] = new ext_ptr<node_type,size_type,value_type>(na,nb,str[t]);
            }
        for ( auto t= 0; t <= 1; delete[] st[t++] ) ;
    }

    std::unique_ptr<lca_processor<node_type,size_type>> prc= nullptr;

    value_type
    _query( std::optional<node_type> x, std::optional<node_type> y, std::optional<node_type> z,
            value_type wc, size_type k,
            value_type a, value_type b ) const {
        if ( a == b ) {
            assert( k < this->size() );
            return a;
        }
        //assert( this->a < this->b );
        auto mid= (a+b)>>1;

        size_type accum= 0;
        for ( value_type i= 0; i < 2; ++i ) {
            if ( not subtree[i] ) continue ;
            std::optional<node_type> ix,iy,iz;
            size_type dw,dx,dy,dz;
            assert( subtree[i]->size() >= 1 );
            ix= (x.has_value()?precomputed_image[i][x.value()]:x);
            iy= (y.has_value()?precomputed_image[i][y.value()]:y);
            iz= (z.has_value()?precomputed_image[i][z.value()]:z);
            dx= (ix.has_value()?subtree[i]->depth(ix.value())+1:0);
            dy= (iy.has_value()?subtree[i]->depth(iy.value())+1:0);
            dz= (iz.has_value()?subtree[i]->depth(iz.value())+1:0);
            //dw= dx+dy+(lies_inside(wc,this->t[i]->a,this->t[i]->b)?1:0)-2*dz;
            auto na= (i==0?a:mid+1), nb= (i==0?mid:b);
            dw= dx+dy+(this->lies_inside(wc,na,nb)?1:0)-2*dz;
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
            std::optional<node_type> x, std::optional<node_type> y, std::optional<node_type> z,
            value_type p, value_type q,
            const value_type wc,
            std::vector<std::pair<value_type,size_type>> &vec,
            bool rprt,
            value_type a, value_type b ) const {

        std::optional<node_type> ix,iy,iz;
        size_type dw,dx,dy,dz;

        if ( p <= a and b <= q ) {
            if ( rprt ) {
                for ( auto cx= x; cx.has_value() and cx.value() != z; cx= this->parent(cx.value()) ) {
                    auto original_cx= original_node[cx.value()];
                    vec.emplace_back(original_cx,root_element.weight_of(original_cx));
                }
                for ( auto cy= y; cy.has_value() and cy.value() != z; cy= this->parent(cy.value()) ) {
                    auto original_cy= original_node[cy.value()];
                    vec.emplace_back(original_cy,root_element.weight_of(original_cy));
                }
                //FIXME: what to do? z is now std::optional<>
                if ( this->lies_inside(wc,a,b) ) {
                    auto original_z= original_node[z.value()];
                    vec.emplace_back(original_z,root_element.weight_of(original_z));
                }
            }
            dx= (x.has_value()?depth(x.value())+1:0);
            dy= (y.has_value()?depth(y.value())+1:0);
            dz= (z.has_value()?depth(z.value())+1:0);
            dw= dx+dy+(this->lies_inside(wc,a,b)?1:0)-2*dz;
            return dw;
        }

        size_type res= 0;
        if ( a > q or b < p ) return res;
        auto mid= (a+b)>>1;
        for ( auto i= 0; i < 2; ++i )
            if ( subtree[i] and subtree[i]->size() >= 1 ) {
                ix= (x.has_value()?precomputed_image[i][x.value()]:x);
                iy= (y.has_value()?precomputed_image[i][y.value()]:y);
                iz= (z.has_value()?precomputed_image[i][z.value()]:z);
                res+= subtree[i]->search(root_element,ix,iy,iz,p,q,wc,vec,rprt,(i==0?a:mid+1),(i==0?mid:b));
            }
        return res;
    }

public:

    ext_ptr() {};

    value_type weight_of(node_type x) const override { return wgt[x]; }
    value_type weight(node_type x) const override { return wgt[x]; }

    /*
    [[nodiscard]] double size_in_bytes() const override {
        return 0;
    }
    */

    ext_ptr( const std::string &s, const std::vector<value_type> &w ) {
        //TODO: add a constructor for weighted tree
        auto un_compressed_version= std::make_unique<weighted_tree>(s,w);
        assert( un_compressed_version->nvertices() == s.size()/2 );
        assert( un_compressed_version->nedges() == s.size()/2-1 );
        wgt= std::make_unique<value_type[]>(n= un_compressed_version->size());
        original_node= std::make_unique<node_type[]>(n);
        adj= std::make_unique<std::vector<node_type>[]>(n);
        for ( auto ii= 0; ii < n; adj[ii++].clear() ) ;
        d= std::make_unique<size_type[]>(n), prnt= std::make_unique<node_type[]>(n);
        for ( node_type xx= 0; xx < n; prnt[xx++]= std::numeric_limits<node_type>::max() ) ;
        for ( node_type x= 0; x < n;\
        wgt[x]= w[x], original_node[x]= x, d[x]= un_compressed_version->depth(x), ++x ) ;
        for ( node_type x= 0; x < n; ++x )
            if ( x != 0 )
                adj[prnt[x]= un_compressed_version->parent(x).value()].push_back(x);
        int sm= 0;
        for ( node_type x= 0; x < n; sm += adj[x++].size() ) ;
        assert( sm == n-1 );
        auto minw= *(std::min_element(wgt.get(),wgt.get()+n)),
             maxw= *(std::max_element(wgt.get(),wgt.get()+n));
        for ( K= 0; (1<<K) <= n; ++K ) ;
        init(0,maxw,un_compressed_version.get());
        prc= std::make_unique<lca_processor<node_type,size_type>>(this);
    }

    /*
    double size_in_bytes() const override {
        auto res= ext_ptr::size_in_bytes();
        if ( prc ) {
            res+= prc->size_in_bytes();
        }
        if ( adj ) {
            for (node_type x = 0; x < n; ++x)
                res += (sizeof(node_type)) * adj[x].size();
        }
        return res+sizeof adj+sizeof prc;
    }
    */


    node_type lca( node_type cx, node_type cy ) const override {
		return (*prc)(cx,cy);
	}

	std::vector<node_type> children(node_type x) const override {
        assert( 0 <= x and x < n );
        return adj[x];
    }

    //====================================================================================
    value_type query(node_type x, node_type y) const override {
        auto z= lca(x,y);
        auto len= depth(x)+depth(y)+1-2*depth(z);
        return _query(x,y,z,weight_of(z),len>>1,lower,upper);
    }

    value_type selection(node_type x, node_type y, size_type qntl) const override {
        auto z= lca(x,y);
        auto len= depth(x)+depth(y)+1-2*depth(z);
        return _query(x,y,z,weight_of(z),this->qntl2rnk(len,qntl),lower,upper);
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

    size_type count(node_type x, node_type y, value_type a, value_type b) const override {
        auto z= lca(x,y);
        std::vector<std::pair<value_type,size_type>> vec;
        return search(*this,x,y,z,a,b,weight_of(z),vec,false,lower,upper);
    }

    void report(node_type x, node_type y, value_type a, value_type b,
                std::vector<std::pair<value_type, size_type>> &res) const override {
        auto z= lca(x,y);
        search(*this,x,y,z,a,b,weight_of(z),res,true,lower,upper);
    }

    virtual ~ext_ptr() override = default;

    size_type size() const override { return n; }

    /*
     * //TODO
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
        if ( original_node )
            res += n*sizeof *original_node;
        if ( prnt )
            res += n*sizeof *prnt;
        if ( wgt )
            res += n*sizeof *wgt;
        if ( d )
            res += n*sizeof *d;
        if ( prc ) {
            res+= prc->size_in_bytes();
        }
        if ( adj ) {
            for (node_type x = 0; x < n; ++x)
                res += (sizeof(node_type)) * adj[x].size();
        }
        return res+sizeof adj+sizeof prc;
    }
    */

    /*double bits_per_node() const override {
        return 8*size_in_bytes()/(size()+0.00);
    }*/

    std::optional<node_type> parent(node_type x) const override {
        if ( x == 0 ) return std::nullopt;
        return prnt[x];
    }

    std::optional<node_type> ancestor(node_type x, size_type i) const override {
        return std::nullopt;
    }

    size_type depth(node_type x) const override { return d[x]; }

    bool is_ancestor(node_type p, node_type x) const override {
        return lca(p,x) == p;
    }

    bool is_leaf(node_type x) const override {
        return adj[x].empty();
    }
};
#endif //SPQ_EXT_PTR_V3_HPP
