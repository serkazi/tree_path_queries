//
// Created by sj on 11/01/19.
//
#ifndef SPQ_TREE_EXTRACTION_SUCCINCT
#define SPQ_TREE_EXTRACTION_SUCCINCT
#include <optional>
#include "sdsl/rank_support.hpp"
#include "sdsl/int_vector.hpp"
#include "sdsl/bp_support_sada.hpp"
#include "sdsl/rrr_vector.hpp"
#include "path_query_processor.hpp"
#include "succinct_tree.hpp"
#include "ff_bitvector.hpp"
#include "pq_types.hpp"

template <
           typename node_type= pq_types::node_type,
           typename size_type= pq_types::size_type,
           typename value_type= pq_types::value_type,
           typename t_bp_support= sdsl::bp_support_sada<>,
	       uint8_t r= 2,
		   typename t_bitvector= sdsl::rrr_vector<>,
		   typename t_rank 	   = typename t_bitvector::rank_1_type,
		   typename t_select_one  = typename t_bitvector::select_1_type,
		   typename t_select_zero = typename t_bitvector::select_0_type>
class tree_ext_sct :
    public succinct_tree<node_type,size_type>,
    public path_query_processor<node_type,size_type,value_type> {

private:
	static size_type left_child( size_type x ) { return child(x,0); }
	static size_type right_child( size_type x ) { return child(x,1);	}
	static std::optional<size_type> parent_tree( size_type tree_id ) {
	    return tree_id>=1?std::optional<size_type>((tree_id-1)>>1):std::nullopt;
	}
   	static inline size_type child( size_type tree_id, value_type i ) { return ((tree_id)<<1)+1+i; }
protected:

    uint8_t t_width;
    size_type max_tree_id;
	std::unique_ptr<sdsl::bit_vector> backbone= nullptr;
	std::unique_ptr<t_bp_support> m_tree= nullptr; //the concatenated layers
	std::unique_ptr<
	        ff_bitvector<
	                size_type,value_type,
	                t_bitvector,t_rank,t_select_zero,t_select_one
	                >
	        >B= nullptr;
	mutable std::unique_ptr<sdsl::int_vector<0>> m_path_off= nullptr; // array keeps track of path offsets in select-like methods
	mutable std::unique_ptr<sdsl::int_vector<0>> mb_path_rank_off[2]; // array keeps track of rank values for the offsets

private:

    // returns the absolute position
	size_type beginning_of( size_type tree_id ) const {
		assert( 0 <= tree_id and tree_id <= max_tree_id+1 );
    	return (*m_path_off)[tree_id];
	}
	// returns the absolute position
	size_type end_of( size_type tree_id ) const {
        assert( 0 <= tree_id and tree_id <= max_tree_id );
		return beginning_of(tree_id+1)-1;
	}
	size_type size_of_tree( size_type tree_id ) const {
        if ( tree_id > max_tree_id )
        	return 0;
		return (end_of(tree_id)+1-beginning_of(tree_id)) >> 1;
	}

	size_type node2absposition( size_type tree_id, node_type x ) const {
    	assert( tree_id <= max_tree_id );
		return m_tree->select((*m_path_off)[tree_id]/2+x+1);
	}

	size_type node2position( size_type tree_id, node_type x ) const {
	    return node2absposition(tree_id,x)-(*m_path_off)[tree_id];
	}

	node_type absposition2node( size_type tree_id, size_type i ) const {
		return m_tree->rank(i)-(*m_path_off)[tree_id]/2-1;
	}

	node_type position2node( size_type tree_id, size_type i ) const {
        return absposition2node(tree_id,i+(*m_path_off)[tree_id]);
	}

	size_type depth( size_type tree_id, node_type x ) const {
        auto pr= parent(tree_id,x);
        if ( not pr.has_value() )
    		return 0;
		auto pos= node2absposition(tree_id,x);
		assert( pos != m_tree->size() );
	    return static_cast<size_type>(m_tree->excess(pos)-1);
	}

	virtual inline std::pair<size_type,size_type>
	interval( size_type tree_id, node_type x ) const {
		size_type i= node2absposition(tree_id,x);
		return {i-(*m_path_off)[tree_id],m_tree->find_close(i)-(*m_path_off)[tree_id]};
	}

	virtual inline std::pair<size_type,size_type>
	abs_interval( size_type tree_id, node_type x ) const {
		size_type i= node2absposition(tree_id,x);
		return {i,m_tree->find_close(i)};
	}

	value_type which_son( size_type tree_id ) const {
    	return static_cast<value_type>(1^(tree_id&1));
	}

	virtual inline bool is_homogeneous( size_type tree_id ) const
	{ return empty_tree(left_child(tree_id)) and empty_tree(right_child(tree_id)); }

	std::optional<node_type> parent( size_type tree_id, node_type x ) const {
		size_type pos= node2absposition(tree_id,x);
		auto res= m_tree->enclose(pos);
		if ( res == m_tree->size() )
			return std::nullopt;
		return std::optional<node_type>(absposition2node(tree_id,res));
	}

	bool _is_ancestor( size_type tree_id, const node_type x, const node_type y ) const {
		auto ix= abs_interval(tree_id,x),
			 iy= abs_interval(tree_id,y);
		return ix.first <= iy.first and iy.second <= ix.second;
	}

	virtual std::optional<node_type> _lca( size_type tree_id, node_type x, node_type y ) const {
		/*
		if ( is_ancestor(x,y) )
			return x;
		if ( is_ancestor(y,x) )
			return y;
		*/
		if ( x == y )
			return x;
		auto ix= abs_interval(tree_id,x),
			 iy= abs_interval(tree_id,y);
		if ( ix.first <= iy.first and iy.second <= ix.second )
			return x;
		if ( iy.first <= ix.first and ix.second <= iy.second )
			return y;
		assert( ix.second < iy.first or iy.second < ix.first );
		if ( ix.second < iy.first ) {
	label01:
			auto res= m_tree->double_enclose(ix.first,iy.first);
			if ( res == m_tree->size() )
				return std::nullopt;
			return absposition2node(tree_id,res);
		}
		swap(ix,iy);
		goto label01;
	}

    node_type reverse_convert( size_type tree_id, node_type x, value_type i ) const {
        return B->select((*(mb_path_rank_off[i]))[tree_id]+x+1,i)-(*m_path_off)[tree_id]/2;
    }

	virtual node_type
	pre_image( size_type tree_id, node_type ix ) const {
		if ( 0 == tree_id )
			return ix;
		auto i= which_son(tree_id);
		auto pid= parent_tree(tree_id);
		assert( pid.has_value() );
		return pre_image(pid.value(),reverse_convert(pid.value(),ix,i));
	}

	virtual std::optional<node_type>
	image_of( size_type tree_id, node_type x, value_type i ) const {
		if ( (*B)[(*m_path_off)[tree_id]/2+x] == i ) {
			return B->rank((*m_path_off)[tree_id]/2+x,i)-(*(mb_path_rank_off[i]))[tree_id];
		}
		auto howManyPrecede= B->rank((*m_path_off)[tree_id]/2+x,i)-(*(mb_path_rank_off[i]))[tree_id];
		if ( howManyPrecede == 0 )
			return std::nullopt;
		auto u= B->select((*(mb_path_rank_off[i]))[tree_id]+howManyPrecede,i)-(*m_path_off)[tree_id]/2;
		if ( _is_ancestor(tree_id,u,x) )
			return B->rank((*m_path_off)[tree_id]/2+u,i)-(*(mb_path_rank_off[i]))[tree_id];
		auto v= _lca(tree_id,u,x);
		if ( (v.has_value() and (*B)[(*m_path_off)[tree_id]/2+v.value()] == i) or not v.has_value() )
			return v.has_value()?B->rank((*m_path_off)[tree_id]/2+v.value(),i)-(*(mb_path_rank_off[i]))[tree_id]:v;
		howManyPrecede= B->rank((*m_path_off)[tree_id]/2+v.value(),i)-(*(mb_path_rank_off[i]))[tree_id];
		auto z= B->select((*(mb_path_rank_off[i]))[tree_id]+howManyPrecede+1,i)-(*m_path_off)[tree_id]/2;
		auto image_of_z= B->rank((*m_path_off)[tree_id]/2+z,i)-(*(mb_path_rank_off[i]))[tree_id];
		return image_of_z==0?std::nullopt:parent(child(tree_id,i),image_of_z);
	}

	size_type getlen( const std::string &s, const std::vector<value_type> &wgt,
			value_type a, value_type b ) {

    	std::queue<size_type> que;
    	std::queue<std::pair<value_type,value_type>> qw;
		std::queue<std::pair<long long,long long>> qstr, qwgt;

		char *buff= new char[s.size()];
		auto *weights= new value_type[s.size()/2];
		char *str[]= {new char[s.size()],new char[s.size()]};
		value_type *vec[]= {new value_type[s.size()/2],new value_type[s.size()/2]};
		size_type slen[r], vlen[r], curpos= 0;

		for ( auto tt= 0; tt < s.size(); buff[tt]= s[tt], ++tt ) ;
		for ( auto tt= 0; tt < s.size()/2; weights[tt]= wgt[tt], ++tt ) ;

		for ( que.push(0),qstr.push({0,s.size()-1}),qwgt.push({0,s.size()/2-1}),\
		qw.push({a,b}); not que.empty(); ) {
			auto tree_id= que.front();
			max_tree_id= std::max(max_tree_id,tree_id);
			auto pr= qw.front();
			auto si= qstr.front().first, sj= qstr.front().second;
			auto wi= qwgt.front().first, wj= qwgt.front().second;
			assert( (sj-si+1) == 2*(wj-wi+1) );
			qstr.pop(), qwgt.pop(), qw.pop(), que.pop();
			auto aa= pr.first, bb= pr.second, mid= (aa+bb)>>1;

			for ( auto tt= 0; tt < r; vlen[tt]= slen[tt]= 0, ++tt ) ;

			for ( auto i= wi; i <= wj; ++i ) {
                size_type which_part= (weights[i]<=mid?0:1);
                vec[which_part][vlen[which_part]++]= weights[i];
            }
			assert( vlen[0]+vlen[1] == (wj-wi+1) );

            std::stack<size_type> st;
            size_type V= 0;
            for ( auto i= si, ii= wi; i <= sj; ++i ) {
                if ( buff[i] =='(' ) {
                    st.push(V++);
                    auto which_part= (weights[wi+st.top()]<=mid?0:1);
                    str[which_part][slen[which_part]++]= '(';
                    continue ;
                }
                assert( !st.empty() );
                auto which_part= (weights[wi+st.top()]<=mid?0:1);
				st.pop(), str[which_part][slen[which_part]++]= ')';
            }
            curpos+= (sj-si+1);

   			if ( aa == bb ) continue ;
			assert( aa < bb );

			assert( !(slen[0] & 1) );
			assert( !(slen[1] & 1) );
			assert( 2*vlen[0] == slen[0] );
			assert( 2*vlen[1] == slen[1] );
			assert( vlen[0]+vlen[1] == (wj-wi+1) );
            for ( size_type i= si, j= 0; j < slen[0]; buff[i++]= str[0][j++] ) ;
			for ( size_type i= si+slen[0], j= 0; j < slen[1]; buff[i++]= str[1][j++] ) ;
			for ( size_type i= wi, j= 0; j < vlen[0]; weights[i++]= vec[0][j++] ) ;
			for ( size_type i= wi+vlen[0], j= 0; j < vlen[1]; weights[i++]= vec[1][j++] ) ;

			que.push(left_child(tree_id)),qw.push({aa,mid}),qstr.push({si,si+slen[0]-1}),qwgt.push({wi,wi+vlen[0]-1});
			que.push(right_child(tree_id)),qw.push({mid+1,bb}),qstr.push({si+slen[0],sj}),qwgt.push({wi+vlen[0],wj});

		}

		delete []buff, delete []weights;
		for ( auto ii= 0; ii < 2; ++ii )
			delete[] str[ii], delete[] vec[ii];

		return curpos;

    }

	void bfs( sdsl::bit_vector *bv, sdsl::bit_vector *tbv, size_type &bpos, size_type &tpos,
			const std::string &s, const std::vector<value_type> &wgt,
			value_type a, value_type b ) {

		tpos= bpos= 0;
		assert( not s.empty() );
    	std::queue<size_type> que;
    	std::queue<std::pair<value_type,value_type>> qw;
		std::queue<std::pair<long long,long long>> qstr, qwgt;

		char *buff= new char[s.size()];
		auto *weights= new value_type[s.size()/2];
		char *str[]= {new char[s.size()],new char[s.size()]};
		value_type *vec[]= {new value_type[s.size()/2],new value_type[s.size()/2]};
		size_type slen[r], vlen[r];

		for ( auto tt= 0; tt < s.size(); buff[tt]= s[tt], ++tt ) ;
		for ( auto tt= 0; tt < s.size()/2; weights[tt]= wgt[tt], ++tt ) ;

		for ( que.push(0),qstr.push({0,s.size()-1}),qwgt.push({0,s.size()/2-1}),\
		qw.push({a,b}); !que.empty(); ) {
			auto tree_id= que.front();
			max_tree_id= std::max(max_tree_id,tree_id);
			auto pr= qw.front();
			auto si= qstr.front().first, sj= qstr.front().second;
			assert( sj >= si );
			auto wi= qwgt.front().first, wj= qwgt.front().second;
			assert( (sj-si+1) == 2*(wj-wi+1) );
			qstr.pop(), qwgt.pop(), qw.pop(), que.pop();
			auto aa= pr.first, bb= pr.second, mid= (aa+bb)>>1;

			for ( auto tt= 0; tt < r; vlen[tt]= slen[tt]= 0, ++tt ) ;

			for ( auto i= wi; i <= wj; ++i ) {
				if ( !(aa <= weights[i] && weights[i] <= bb) ) {
				    std::cout << aa << " " << weights[i] << " " << bb << " " << tree_id << "\n";
				}
                assert( aa <= weights[i] && weights[i] <= bb );
                (*bv)[(i-wi)+bpos]= (weights[i]<=mid?0:1);
            }
            bpos+= (wj-wi+1);

			for ( auto i= wi; i <= wj; ++i ) {
                size_type which_part= (weights[i]<=mid?0:1);
                vec[which_part][vlen[which_part]++]= weights[i];
            }
			assert( vlen[0]+vlen[1] == (wj-wi+1) );

            std::stack<size_type> st;
            size_type V= 0;
            for ( auto i= si, ii= wi; i <= sj; ++i ) {
                if ( buff[i] =='(' ) {
                    (*tbv)[tpos+(i-si)]= true, st.push(V++);
                    auto which_part= (weights[wi+st.top()]<=mid?0:1);
                    str[which_part][slen[which_part]++]= '(';
                    continue ;
                }
                assert( !st.empty() );
                auto which_part= (weights[wi+st.top()]<=mid?0:1);
                st.pop(), str[which_part][slen[which_part]++]= ')';
                (*tbv)[tpos+(i-si)]= false;
            }
            tpos+= (sj-si+1), (*m_path_off)[tree_id]= (sj-si+1);

   			if ( aa == bb ) continue ;
			assert( aa < bb );

			assert( !(slen[0] & 1) );
			assert( !(slen[1] & 1) );
			assert( 2*vlen[0] == slen[0] );
			assert( 2*vlen[1] == slen[1] );
			assert( vlen[0]+vlen[1] == (wj-wi+1) );
            for ( size_type i= si, j= 0; j < slen[0]; buff[i++]= str[0][j++] ) ;
			for ( size_type i= si+slen[0], j= 0; j < slen[1]; buff[i++]= str[1][j++] ) ;
			for ( size_type i= wi, j= 0; j < vlen[0]; weights[i++]= vec[0][j++] ) ;
			for ( size_type i= wi+vlen[0], j= 0; j < vlen[1]; weights[i++]= vec[1][j++] ) ;

			if ( vlen[0] )
				que.push(left_child(tree_id)),qw.push({aa,mid}),qstr.push({si,si+slen[0]-1}),qwgt.push({wi,wi+vlen[0]-1});
			que.push(right_child(tree_id)),qw.push({mid+1,bb}),qstr.push({si+slen[0],sj}),qwgt.push({wi+vlen[0],wj});

		}

		delete []buff, delete []weights;
		for ( auto ii= 0; ii < 2; ++ii )
			delete[] str[ii], delete[] vec[ii];
    }

	virtual void init(
			sdsl::bit_vector *bv, sdsl::bit_vector *tbv,
			size_type tree_id, size_type &bpos, size_type &tpos,
			const std::string &s, const std::vector<value_type> &wgt,
			value_type a, value_type b ) {

		max_tree_id= std::max(max_tree_id,tree_id);
		assert( max_tree_id < 2*this->m_sigma+7 );

		if ( a == b ) {
    		std::stack<size_type> st;
			size_type V= 0;
		    for ( auto i= 0; i < s.size(); ++i ) {
                if ( s[i] =='(' ) {
                    (*tbv)[tpos+i]= true, st.push(V++);
                    continue ;
                }
                assert( !st.empty() );
                st.pop(), (*tbv)[tpos+i]= false;
			}
		    //TODO: possible place for optimization: B-vector does not need to grow
		    for ( auto i= 0; i < s.size()/2; ++i )
				(*bv)[i+bpos]= false;
			tpos+= s.size(), bpos+= s.size()/2, (*m_path_off)[tree_id]= s.size();
		    return ;
		}

		auto mid= (a+b)>>1;

		for ( auto l= 0; l < s.size()/2; ++l ) {
			assert( a <= wgt[l] && wgt[l] <= b );
			(*bv)[l+bpos]= (wgt[l]<=mid?0:1);
		}
		bpos+= s.size()/2;

		std::stringstream str[r];
		std::vector<value_type> vec[r];
		for ( auto i= 0; i < r; vec[i++].clear() ) ;
		for ( auto i= 0; i < s.size()/2; ++i ) {
			size_type which_part= wgt[i]<=mid?0:1;
			vec[which_part].push_back(wgt[i]);
		}
		std::stack<size_type> st;
		size_type V= 0;
		for ( auto i= 0; i < s.size(); ++i ) {
			if ( s[i] =='(' ) {
				(*tbv)[tpos+i]= true, st.push(V++);
				auto which_part= wgt[st.top()]<=mid?0:1;
				str[which_part] << "(";
				continue ;
			}
			assert( !st.empty() );
			auto which_part= wgt[st.top()]<=mid?0:1;
			st.pop(), str[which_part] << ")";
			(*tbv)[tpos+i]= false;
		}
		tpos+= s.size(), (*m_path_off)[tree_id]= s.size();
		assert( st.empty() );
		if ( a <= mid )
		    init(bv,tbv,left_child(tree_id),bpos,tpos,str[0].str(),vec[0],a,mid);
		if ( mid+1 <= b )
			init(bv,tbv,right_child(tree_id),bpos,tpos,str[1].str(),vec[1],mid+1,b);
		//assert( str[0].str().size()+str[1].str().size() == s.size() );
	}

	void init_finalize( const sdsl::bit_vector *bv, const sdsl::bit_vector *tbv ) {
		(*m_path_off)[max_tree_id+1]= 0;
    	for ( auto tt= 0; tt <= 1; (*(mb_path_rank_off[tt++]))[max_tree_id+1]= 0 ) ;
    	for ( auto tt= 0; tt < (*m_path_off)[0]/2; ++(*(mb_path_rank_off[(*bv)[tt++]]))[0] ) ;
    	for ( auto tree_id= 1; tree_id <= max_tree_id+1; ++tree_id ) {
			(*m_path_off)[tree_id]+= (*m_path_off)[tree_id-1];
			for ( auto tt= 0; tt <= 1; ++tt )
				(*(mb_path_rank_off[tt]))[tree_id]+= (*(mb_path_rank_off[tt]))[tree_id-1];
			for ( size_type tt= (*m_path_off)[tree_id-1]/2; tt < (*m_path_off)[tree_id]/2; ++tt )
				++(*(mb_path_rank_off[(*bv)[tt]]))[tree_id];
		}
    	for ( auto tree_id= max_tree_id+1; tree_id >= 1; --tree_id ) {
			(*m_path_off)[tree_id]= (*m_path_off)[tree_id-1];
    		for ( auto tt= 0; tt <= 1; ++tt )
				(*(mb_path_rank_off[tt]))[tree_id]= (*(mb_path_rank_off[tt]))[tree_id-1];
    	}
    	(*m_path_off)[0]= 0;
    	for ( auto tt= 0; tt <= 1; (*(mb_path_rank_off[tt++]))[0]= 0 ) ;
    }

    virtual value_type original_weight(
    		size_type tree_id,
			std::optional<node_type> x,
			value_type a, value_type b ) const {
		if ( a == b )
			return a;
		assert( x.has_value() );
		auto i= static_cast<value_type>((*B)[(*m_path_off)[tree_id]/2+x.value()]);
		auto ix= image_of(tree_id,x.value(),i);
		assert( ix.has_value() );
		auto mid= (a+b)>>1;
		return original_weight(child(tree_id,i),ix,i==0?a:mid+1,i==0?mid:b);
	}

	bool empty_tree( size_type tree_id ) const {
    	return 0 == size_of_tree(tree_id);
	}

	virtual value_type
	_query( size_type tree_id,
			std::optional<node_type> x, std::optional<node_type> y, std::optional<node_type> z,
			value_type wc, size_type k,
			value_type a, value_type b ) const {

    	if ( a == b )
			return a;

		auto mid= (a+b)>>1;

		size_type accum= 0;
		for ( value_type i= 0; i < r; ++i ) {
			if ( empty_tree(child(tree_id,i)) ) continue ;
			std::optional<node_type> ix,iy,iz;
			size_type dw,dx,dy,dz;
			ix= (x.has_value()?image_of(tree_id,x.value(),i):x);
			iy= (y.has_value()?image_of(tree_id,y.value(),i):y);
			iz= (z.has_value()?image_of(tree_id,z.value(),i):z);
			dx= (ix.has_value()?depth(child(tree_id,i),ix.value())+1:0);
			dy= (iy.has_value()?depth(child(tree_id,i),iy.value())+1:0);
			dz= (iz.has_value()?depth(child(tree_id,i),iz.value())+1:0);
			//dw= dx+dy+(lies_inside(wc,this->t[i]->a,this->t[i]->b)?1:0)-2*dz;
			auto na= (i==0?a:mid+1), nb= (i==0?mid:b);
			dw= dx+dy+(this->lies_inside(wc,na,nb)?1:0)-2*dz;
			if ( accum+dw > k )
				return _query(child(tree_id,i),ix,iy,iz,wc,k-accum,na,nb);
			accum+= dw;
		}
		assert( false );
		return 0;
	}

	size_type
	search(
	        size_type tree_id,
			std::optional<node_type> x, std::optional<node_type> y, std::optional<node_type> z,
			const value_type p, const value_type q,
			const value_type wc,
		 	std::vector<std::pair<node_type,value_type>> &vec,
		 	bool rprt,
		 	const value_type a, const value_type b ) const {

		std::optional<node_type> ix,iy,iz;
		size_type dw,dx,dy,dz;

		if ( p <= a and b <= q ) {
			if ( rprt ) {
				// FIXME: this doesn't make sense, since the IDs are w.r.t. current tree
				// we need to add a traceback thing => asymptotically it is still the same,
				// since we have already paid the log-sigma cost
				// while descending to here
				for ( auto cx= x; cx.has_value() and cx.value() != z; cx= parent(tree_id,cx.value()) ) {
					auto original_cx= pre_image(tree_id,cx.value());
					vec.emplace_back(original_cx,weight_of(original_cx));
				}
				for ( auto cy= y; cy.has_value() and cy.value() != z; cy= parent(tree_id,cy.value()) ) {
					auto original_cy= pre_image(tree_id,cy.value());
					vec.emplace_back(original_cy,weight_of(original_cy));
				}
				//dw= weight_of(t);
				if ( this->lies_inside(wc,a,b) ) {
					auto original_z= pre_image(tree_id,z.value());
					vec.emplace_back(original_z,weight_of(original_z));
				}
			}
			dx= (x.has_value()?depth(tree_id,x.value())+1:0);
			dy= (y.has_value()?depth(tree_id,y.value())+1:0);
			dz= (z.has_value()?depth(tree_id,z.value())+1:0);
			return dx+dy+(a<=wc and wc<=b?1:0)-2*dz;
		}

		size_type res= 0;

		if ( a > q or b < p ) return res;

		auto mid= (a+b)>>1;

		for ( auto i= 0; i < r; ++i )
			if ( not empty_tree(child(tree_id,i)) ) {
				ix= (x.has_value()?image_of(tree_id,x.value(),i):x);
				iy= (y.has_value()?image_of(tree_id,y.value(),i):y);
				iz= (z.has_value()?image_of(tree_id,z.value(),i):z);
				res+= search(child(tree_id,i),ix,iy,iz,p,q,wc,vec,rprt,(i==0?a:mid+1),(i==0?mid:b));
			}

		return res;

	}

public:

	explicit tree_ext_sct( const std::string &s, const std::vector<value_type> &w ) {
	    this->m_sigma= *(std::max_element(w.begin(),w.end())) + 1;
	    max_tree_id= 0;
	    auto reqlen= getlen(s,w,0,this->m_sigma-1);
	    auto *tbv= new sdsl::bit_vector(reqlen);
	    auto *bv= new sdsl::bit_vector(reqlen/2+8);
		size_type bpos= 0, tpos= 0;
		t_width= static_cast<uint8_t>(std::ceil(log2(reqlen)+1e-3));

		m_path_off= std::make_unique<sdsl::int_vector<>>(max_tree_id+3,0,t_width);
		for ( auto tt= 0; tt <= 1; ++tt )
			mb_path_rank_off[tt]= std::make_unique<sdsl::int_vector<>>(max_tree_id+3,0,t_width);

		max_tree_id= 0;//init(bv,tbv,0,bpos,tpos,s,w,0,m_sigma-1);
		bfs(bv,tbv,bpos,tpos,s,w,0,this->m_sigma-1);
		assert( tpos == 2*bpos );
		init_finalize(bv,tbv);
		B= std::make_unique<ff_bitvector<
		        size_type,value_type,
		        t_bitvector,t_rank,t_select_zero,t_select_one>>(bv);
		backbone= std::make_unique<sdsl::bit_vector>(tpos);
		// std::cerr << tpos << std::endl;
		for ( auto tt= 0; tt < tpos; ++tt )
			(*backbone)[tt]= (*tbv)[tt];
		delete tbv, delete bv;
		m_tree= std::make_unique<t_bp_support>(backbone.get());
	}

	tree_ext_sct( const std::string &s ) :
	tree_ext_sct( s, *(new std::vector<value_type>(s.size()/2,0)) ) {
    }

	value_type weight(const node_type x) const override {
		return original_weight(0,x,0,this->m_sigma-1);
	}
	value_type weight_of(const node_type x) const override {
        return weight(x);
	}

	value_type query(node_type x, node_type y) const override {
    	auto z= lca(x,y);
		auto len= depth(x)+depth(y)+1-2*depth(z);
		return _query(0,x,y,z,weight_of(z),len>>1,0,this->m_sigma-1);
	}

	value_type selection(const node_type x, const node_type y, const size_type qntl) const override {
   		auto z= lca(x,y);
		auto len= depth(x)+depth(y)+1-2*depth(z);
		return _query(0,x,y,z,weight_of(z),this->qntl2rnk(len,qntl),0,this->m_sigma-1);
	}

	size_type count(const node_type x, const node_type y, const value_type p, const value_type q) const override {
        auto z= lca(x,y);
		std::vector<std::pair<node_type,value_type>> vec;
		return search(0,x,y,z,p,q,weight_of(z),vec,false,0,this->m_sigma-1);
	}

	void report(const node_type x, const node_type y, const value_type a, const value_type b,
				std::vector<std::pair<value_type, size_type>> &res) const override {
    	std::vector<std::pair<node_type,value_type>> vec;
		auto z= lca(x,y);
		search(0,x,y,z,a,b,weight_of(z),vec,true,0,this->m_sigma-1);
		std::for_each( vec.begin(), vec.end(),[&]( const std::pair<node_type,value_type> &p ) {
			res.emplace_back(std::make_pair(p.first,p.second));
		});
	}

	/*
	double size_in_bytes() const override {
        auto a= B->size_in_bytes()+sdsl::size_in_bytes(*m_path_off);
        auto b= sdsl::size_in_bytes((*mb_path_rank_off[0]))+sdsl::size_in_bytes((*mb_path_rank_off[1]));
        auto c= sdsl::size_in_bytes(*m_tree);
        auto d= sdsl::size_in_bytes(*backbone);
        auto e= sizeof backbone + sizeof m_path_off + sizeof B + sizeof mb_path_rank_off;
        return a+b+c+d+e;
	}
	double bits_per_node() const override {
		return 8.00*size_in_bytes()/size();
	}
    */

	size_type size() const override {
		return (*m_path_off)[1] >> 1;
	}

	std::optional<node_type> parent(node_type x) const override {
		auto p= parent(0,x);
		return p;
	}

	std::vector<node_type> children(const node_type x) const override {
    	std::vector<node_type> res;
		auto ix= abs_interval(0,x);
		assert( res.empty() );
		for ( auto i= ix.first; i+1 < m_tree->size() and m_tree->is_opening(i+1); i= m_tree->find_close(i+1) )
			res.push_back(absposition2node(0,i+1));
		assert( is_leaf(x) || !res.empty() );
		return res;
	}

	node_type lca(node_type x, node_type y) const override {
		auto z= _lca(0,x,y);
		assert( z.has_value() );
		return z.value();
	}

	size_type depth(const node_type x) const override {
		return depth(0,x);
	}

	bool is_leaf(const node_type x) const override {
        auto i= node2absposition(0,x);
		return m_tree->find_close(i) == i+1;
	}

	bool is_ancestor( const node_type p, const node_type x ) const override {
    	return _is_ancestor(0,p,x);
    }

    std::optional<node_type> ancestor( const node_type x, const size_type i ) const override {
		return 0;
	}

	friend std::ostream &operator << ( std::ostream &ofs, const tree_ext_sct &t ) {
	    std::stack<std::pair<node_type,bool>> st;
	    for ( st.push({0,true}); !st.empty(); ) {
	    	auto pr= st.top(); st.pop();
	    	if ( pr.second ) {
	    		ofs << "(";
	    		auto children= t.children(pr.first);
	    		st.push({pr.first,false});
	    		for ( int l= ((int)children.size()); --l >= 0; st.push({children[l],true}) ) ;
	    	}
	    	else {
	    		ofs << ")";
	    	}
	    }
	    return ofs;
	}
};

#endif //SPQ_TREE_EXTRACTION_SUCCINCT
