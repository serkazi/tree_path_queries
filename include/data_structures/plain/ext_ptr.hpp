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

/**
 * @note since lca_processor needs a reference to a tree,
 * we formally do implement tree interface -- just at the stage
 * of constructing the lca_processor. Then we call shed_redundancy()
 * and get rid of all the structures that enable
 * usual tree operations such as "list of children" etc.,
 * as we don't need them for answering path queries
 */
template<
        typename node_type= pq_types::node_type,
        typename size_type= pq_types::size_type,
        typename value_type= pq_types::value_type
        >
class ext_ptr:
        public path_query_processor<node_type,size_type,value_type>,
        public succinct_tree<node_type,size_type>
{
public:
    // inner class
    // we are making this inner class, together with the constructor
    // that makes use of it, public so that unique_ptr can be created
    /*
    class weighted_tree {
    private:
        size_type n,E;
        node_type V;
        std::vector<size_type> d;
        std::vector<value_type> weights;
        std::vector<std::optional<node_type>> prnt;
        std::vector<node_type> oid;
        std::vector<std::vector<node_type>> adj;
    public:
        weighted_tree( size_type n ) {
            this->n= n, E= V= 0;
            oid.resize(n), weights.resize(n), prnt.resize(n), d.resize(n), adj.resize(n);
            std::fill(prnt.begin(),prnt.end(),std::nullopt);
            for ( auto &v: adj ) v.clear();
        }
        size_type nvertices() const { return V; }
        node_type original_id( node_type x ) const {
            assert( x < V );
            return oid[x];
        }
        virtual ~weighted_tree() = default;
        // FIXME: this place gives bad_alloc most of the time
        void add_arc( node_type p, node_type x ) { ++E, adj[(prnt[x]= p).value()].push_back(x); }
        value_type weight_of( node_type x ) const { return weights[x]; }
        const std::vector<node_type> &children( node_type x ) const {
            return adj[x];
        }
        size_type size() const { return n; }
        std::optional<node_type> parent( node_type x ) const {
            return prnt[x];
        }
        node_type depth( node_type x ) const { return d[x]; }
        void assign_weight( node_type x, value_type w ) { weights[x]= w; }
        void assign_depth( node_type x, value_type w ) { d[x]= w; }
        void assign_original_id( node_type x, node_type ox ) {
            oid[x]= ox;
        }
        node_type new_node( node_type _oid ) {
            oid[V++]= _oid;
            return V-1;
        }
        size_type nedges() const { return E; }
        weighted_tree( const std::string &s, const std::vector<value_type> &w ) : weighted_tree(s.size()/2) {
            for ( node_type x= 0; x < n; oid[x]= x, ++x ) ;
            std::stack<node_type> stck;
            for ( int i= 0; i < (int)w.size(); weights[i]= w[i], ++i ) ;
            for ( char ch: s )
                if ( ch == '(' ) {
                    if ( not stck.empty() )
                        add_arc(stck.top(),V);
                    d[V]= stck.size(), stck.push(V++);
                }
                else stck.pop();
            assert( V == n );
            assert( E+1 == n );
        }
    };
    */
    struct weighted_tree {
        size_type n,E{};
        node_type V;
        std::vector<size_type> d;
        std::vector<value_type> weights;
        std::vector<std::optional<node_type>> prnt;
        std::vector<node_type> oid;
        std::vector<node_type> to;
        std::vector<std::optional<size_type>> next_,last;
        weighted_tree( size_type n ) {
            this->n= n, E= V= 0;
            oid.resize(n), weights.resize(n), prnt.resize(n), d.resize(n), last.resize(n), next_.resize(n==0?0:n-1);
            std::fill(prnt.begin(),prnt.end(),std::nullopt), to.resize(n==0?0:n-1);
            std::fill(next_.begin(),next_.end(),std::nullopt);
            std::fill(last.begin(),last.end(),std::nullopt);
        }
        size_type nvertices() const { return V; }
        node_type original_id( node_type x ) const {
            assert( x < V );
            return oid[x];
        }
        virtual ~weighted_tree() = default;
        // FIXME: this place gives bad_alloc most of the time
        void add_arc( node_type p, node_type x ) {
            size_type i= E++;
            to[i]= x, next_[i]= last[p], last[p]= i, prnt[x]= p;
        }
        value_type weight_of( node_type x ) const { return weights[x]; }
        std::vector<node_type> children( node_type x ) const {
            std::vector<node_type> res;
            for ( auto i= last[x]; i; i= next_[*i] )
                res.push_back(to[*i]);
            return res;
        }
        size_type size() const { return n; }
        std::optional<node_type> parent( node_type x ) const {
            return prnt[x];
        }
        node_type depth( node_type x ) const { return d[x]; }
        void assign_weight( node_type x, value_type w ) { weights[x]= w; }
        void assign_depth( node_type x, value_type w ) { d[x]= w; }
        void assign_original_id( node_type x, node_type ox ) {
            oid[x]= ox;
        }
        node_type new_node( node_type _oid ) {
            oid[V++]= _oid;
            return V-1;
        }
        size_type nedges() const { return E; }
        weighted_tree( const std::string &s, const std::vector<value_type> &w ) : weighted_tree(s.size()/2) {
            for ( node_type x= 0; x < n; oid[x]= x, ++x ) ;
            std::stack<node_type> stck;
            for ( int i= 0; i < (int)w.size(); weights[i]= w[i], ++i ) ;
            for ( char ch: s )
                if ( ch == '(' ) {
                    if ( not stck.empty() )
                        add_arc(stck.top(),V);
                    d[V]= stck.size(), stck.push(V++);
                }
                else stck.pop();
            assert( V == n );
            assert( E+1 == n );
        }
    };

private:

    size_type n,K,E= 0;
    value_type lower, upper;
    std::vector<std::vector<std::optional<node_type>>> precomputed_image;

	std::vector<std::optional<size_type>> last,next_;
	std::vector<node_type> to;

    std::vector<node_type> original_node;
    std::vector<std::optional<node_type>> prnt;
    std::vector<value_type> wgt;
    std::vector<size_type> d;
    std::vector<std::unique_ptr<ext_ptr<node_type,size_type,value_type>>> subtree;

    std::vector<bool> seen;
    std::stack<node_type> st[2];

	void shed_redundancy() {
		to.clear(), next_.clear(), last.clear();
	}

    void precalc( const weighted_tree *tr, node_type x, std::vector<std::unique_ptr<weighted_tree>> &str, value_type mid ) {
        assert( not seen[x] );
        seen[x]= true;
        value_type color= tr->weight_of(x)<=mid?0:1;
        node_type ix= str[color]->new_node(tr->original_id(x));
        std::optional<node_type> px= (st[color].empty()?std::nullopt:std::optional<node_type>(st[color].top()));
        // str[color]->assign_original_id(ix,x);
        str[color]->assign_weight(ix,tr->weight_of(x));
        str[color]->assign_depth(ix,st[color].size());
        if ( px.has_value() )
            str[color]->add_arc(px.value(),ix);
        st[color].push(ix);
        for ( value_type tt= 0; tt <= 1; ++tt )
            if ( not precomputed_image[tt].empty() )
                precomputed_image[tt][x]= ((not st[tt].empty())?std::optional<node_type>(st[tt].top()):std::nullopt);
        auto kinder= tr->children(x);
        for ( auto y: kinder ) {
            assert( not seen[y] );
            precalc(tr,y,str,mid);
        }
        assert( st[color].top() == ix );
        st[color].pop();
    }

    void init( value_type a, value_type b, const weighted_tree *un_compressed_tree ) {
        subtree.resize(2), std::fill(subtree.begin(),subtree.end(),nullptr);
        if ( (this->lower= a) == (this->upper= b) or not un_compressed_tree )
            return ;
        auto mid= (a+b)>>1;
        size_type nsz[]= {0,0};
        for ( node_type x= 0; x < n; ++x ) {
            assert( a <= un_compressed_tree->weight_of(x) and un_compressed_tree->weight_of(x) <= b );
            ++nsz[un_compressed_tree->weight_of(x) <= mid ? 0 : 1];
        }

        std::vector<std::unique_ptr<weighted_tree>> str;
        str.push_back(std::move(std::make_unique<weighted_tree>(nsz[0])));
        str.push_back(std::move(std::make_unique<weighted_tree>(nsz[1])));

        precomputed_image.resize(2);
        for ( auto tt= 0; tt <= 1; ++tt )
            if ( str[tt] and str[tt]->size() >= 1 )
                precomputed_image[tt].resize(n);
        seen.resize(n), std::fill(seen.begin(),seen.end(),false);
        for ( node_type x= 0; x < n; ++x )
            if ( not seen[x] )
                precalc(un_compressed_tree,x,str,mid);
        seen.clear();
        for ( value_type t= 0; t <= 1; str[t++].reset() )
            if ( nsz[t] ) {
                auto na = (t == 0 ? a : mid + 1), nb = (t == 0 ? mid : b);
                subtree[t] = std::make_unique<ext_ptr<node_type,size_type,value_type>>(na,nb,str[t].get());
            }
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
				const auto &W= root_element.wgt;
                for ( auto cx= x; cx.has_value() and cx.value() != z; cx= prnt[cx.value()] ) {
                    auto original_cx= original_node[cx.value()];
                    vec.emplace_back(original_cx,W[original_cx]);
                }
                for ( auto cy= y; cy.has_value() and cy.value() != z; cy= prnt[cy.value()] ) {
                    auto original_cy= original_node[cy.value()];
                    vec.emplace_back(original_cy,W[original_cy]);
                }
                if ( this->lies_inside(wc,a,b) and z.has_value() ) {
                    auto original_z= original_node[z.value()];
                    //vec.emplace_back(original_z,root_element.weight_of(original_z));
                    vec.emplace_back(original_z,W[original_z]);
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

	void add_arc( node_type p, node_type x ) {
		auto i= E++;
		to[i]= x, next_[i]= last[p], last[p]= i;
	}

public:
    // we had to expose this constructor, as it is needed for make_unique
    // in an ideal world, this constructor should be private
    ext_ptr( value_type a, value_type b,
             const weighted_tree *un_compressed_tree )
    {
        d.resize(n= un_compressed_tree->size());
        original_node.resize(n);
        prnt.resize(n);
        for ( node_type x= 0; x < n; original_node[x]= un_compressed_tree->original_id(x),\
        d[x]= un_compressed_tree->depth(x), prnt[x]= un_compressed_tree->parent(x), ++x ) ;
        init(a,b,un_compressed_tree);
    }

    ext_ptr() {};

    value_type weight_of(node_type x) const override { return wgt[x]; }
    value_type weight(node_type x) const override { return wgt[x]; }

    ext_ptr( const std::string &s, const std::vector<value_type> &w ) {
        auto un_compressed_version= std::make_unique<weighted_tree>(s,w);
        assert( un_compressed_version->nvertices() == s.size()/2 );
        assert( un_compressed_version->nedges()+1 == s.size()/2 );
        wgt.resize(n= un_compressed_version->size());
        E= 0, original_node.resize(n), to.resize(n==0?0:n-1), next_.resize(n==0?0:n-1), last.resize(n);
		std::fill(next_.begin(),next_.end(),std::nullopt);
		std::fill(last.begin(),last.end(),std::nullopt);
        d.resize(n), prnt.resize(n), std::fill(prnt.begin(),prnt.end(),std::nullopt);
        for ( node_type x= 0; x < n;\
        wgt[x]= w[x], original_node[x]= x, d[x]= un_compressed_version->depth(x), ++x ) ;
        for ( node_type x= 0; x < n; ++x )
            if ( x != 0 ) {
				prnt[x]= un_compressed_version->parent(x);
				add_arc(prnt[x].value(),x);
			}
		assert( E+1 == n );
        auto minw= *(std::min_element(wgt.begin(),wgt.end())),
             maxw= *(std::max_element(wgt.begin(),wgt.end()));
        for ( K= 0; (1<<K) <= n; ++K ) ;
        init(0,maxw,un_compressed_version.get());
        prc= std::make_unique<lca_processor<node_type,size_type>>(this);
		un_compressed_version.reset();
		shed_redundancy();
    }

    node_type lca( node_type cx, node_type cy ) const override {
		return (*prc)(cx,cy);
	}

	std::vector<node_type> children(node_type x) const override {
        assert( 0 <= x and x < n );
        std::vector<node_type> res{};
		for ( auto i= last[x]; i; i= next_[*i] )
			res.push_back(to[*i]);
		return res;
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
        res.clear();
        search(*this,x,y,z,a,b,weight_of(z),res,true,lower,upper);
    }

    virtual ~ext_ptr() override = default;

    size_type size() const override { return n; }

    std::optional<node_type> parent(node_type x) const override {
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
		return false ;
    }
};
#endif //SPQ_EXT_PTR_V3_HPP
