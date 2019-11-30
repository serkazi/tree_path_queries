// integration tests for all the path_query_processor interfaces
// checks path selection, path counting, and path reporting
#include "gtest/gtest.h"
#include "gtest/gtest-typed-test.h"
#include "gtest/internal/gtest-internal.h"
#include "path_query_processor.hpp"
#include "path_decomposer.hpp"
#include "hybrid_processor.hpp"
#include "raw_processor.hpp"
#include "tree_ext_sct.hpp"
/*#include "extractions.h"
#include "naive_processor.hpp"
#include "naive_processor_lca.hpp"
#include "nsrs.hpp"
 */
#include <iostream>
#include <fstream>
#include <random>
#include <memory>
#include <string>
#include <algorithm>
#include <set>
/*
#include "defs.h"
#include "wt_hpd.hpp"
#include "wt_hpd_rrr.hpp"
#include "ext_ptr_v2.hpp"
#include "ext_ptr_v3.hpp"
#include <sdsl/bit_vector_il.hpp>
 */

using node_type= pq_types::node_type;
using size_type= pq_types::size_type;
using value_type= pq_types::value_type;

//using constants::ITERS;
const int ITERS= 0x400;

const std::string paths[]= {
	std::string("degenerate_tree_equal_weights.txt"),
	std::string("log_weights.txt"),
	std::string("sqrt_weights.txt"),
	std::string("linear_small_weights.txt"),
	std::string("linear_weights.txt"),
	std::string("us.rd.d.dfs.dimacs.puu")
};

bool multiset_equality( std::vector<std::pair<value_type,size_type>> &a,
						std::vector<std::pair<value_type,size_type>> &b ) {
	std::multiset<value_type, std::greater<value_type> > ma;
	ma.clear();
	if ( a.size() != b.size() ) {
		printf("%d vs %d\n",(int)a.size(),(int)b.size());
		return false ;
	}
	std::for_each(a.begin(),a.end(),[&](const std::pair<value_type,size_type> &pr) {
		ma.insert((value_type) pr.second);
	});
	for ( auto y: b ) {
		auto itr= ma.find((value_type)y.second);
		if ( itr == ma.end() )
			return false ;
		ma.erase(itr);
	}
	return ma.empty();
}

template <class T>
struct my_type_traits {
    static const char *fname;
};

template <class T> const char *degenerate_tree= paths[0].c_str();
template <class T> const char *log_tree= paths[1].c_str();
template <class T> const char *sqrt_tree= paths[2].c_str();
template <class T> const char *lin_small_wgt_tree= paths[3].c_str();

template <class T>
struct QueriesTestDegenerateTreeEqualWeights: testing::Test {

	std::unique_ptr<path_query_processor<node_type,size_type,value_type>> raw;
	std::unique_ptr<T> processor;
	size_type n;
	std::vector<value_type> w;
	std::default_random_engine gen_nodes, gen_weights, gen_qntl;
	std::unique_ptr<std::uniform_int_distribution<node_type>> nodes_distr;
	std::unique_ptr<std::uniform_int_distribution<size_type>> qntl_distr;
	std::unique_ptr<std::uniform_int_distribution<value_type>> weights_distr;
	std::string filename= paths[0];

	QueriesTestDegenerateTreeEqualWeights() {
		std::string s;
		std::ifstream in(this->filename);
		std::cerr << "Reading file: " << this->filename << "\n";
		in >> s; n= s.size()/2;
		//w.clear(); w.reserve(n);
		for ( auto l = 0; l < n; ++l ) {
			pq_types::value_type entry;
			in >> entry;
			w.push_back(entry);
		}
		in.close();
		assert( w.size() == n );
		raw= std::make_unique<raw_processor<node_type,size_type,value_type>>(s,w);
		processor= std::make_unique<T>(s,w);
		nodes_distr= std::make_unique<std::uniform_int_distribution<node_type>>(0,n-1);
		qntl_distr= std::make_unique<std::uniform_int_distribution<size_type>>(1,100);
		value_type lower, upper;
		lower= upper= (w)[0];
		for ( auto l= 1; l < n; ++l )
			lower= std::min(lower,(value_type)(w)[l]), upper= std::max(upper,(value_type)(w)[l]);
		weights_distr= std::make_unique<std::uniform_int_distribution<value_type>>(lower,upper);
	}
};

template <class T>
struct QueriesTestDIMACStree: testing::Test {

	std::unique_ptr<path_query_processor<node_type,size_type,value_type>> raw;
	std::unique_ptr<T> processor;
	size_type n;
	std::vector<value_type> w;
	std::default_random_engine gen_nodes, gen_weights, gen_qntl;
	std::unique_ptr<std::uniform_int_distribution<node_type>> nodes_distr;
	std::unique_ptr<std::uniform_int_distribution<size_type>> qntl_distr;
	std::unique_ptr<std::uniform_int_distribution<value_type>> weights_distr;
	std::string filename= paths[0];

	QueriesTestDIMACStree() {
		std::string s;
		std::ifstream in(this->filename);
		std::cout << "Reading file: " << this->filename << "\n";
		in >> s; n= s.size()/2;
		//w.clear(); w.reserve(n);
		for ( auto l = 0; l < n; ++l ) {
			pq_types::value_type entry;
			in >> entry;
			w.push_back(entry);
		}
		in.close();
		assert( w.size() == n );
		raw= std::make_unique<raw_processor<node_type,size_type,value_type>>(s,w);
		processor= std::make_unique<T>(s,w);
		nodes_distr= std::make_unique<std::uniform_int_distribution<node_type>>(0,n-1);
		qntl_distr= std::make_unique<std::uniform_int_distribution<size_type>>(1,100);
		value_type lower, upper;
		lower= upper= (w)[0];
		for ( auto l= 1; l < n; ++l )
			lower= std::min(lower,(value_type)(w)[l]), upper= std::max(upper,(value_type)(w)[l]);
		weights_distr= std::make_unique<std::uniform_int_distribution<value_type>>(lower,upper);
	}
};

template <class T>
struct QueriesTestLogWeights : testing::Test {

	std::unique_ptr<path_query_processor<node_type,size_type,value_type>> raw;
	std::unique_ptr<T> processor;
	size_type n;
	std::vector<value_type> w;
	std::default_random_engine gen_nodes, gen_weights, gen_qntl;
	std::unique_ptr<std::uniform_int_distribution<node_type>> nodes_distr;
	std::unique_ptr<std::uniform_int_distribution<size_type>> qntl_distr;
	std::unique_ptr<std::uniform_int_distribution<value_type>> weights_distr;
	std::string filename= paths[1];

	QueriesTestLogWeights() {
		std::string s;
		std::ifstream in(this->filename);
		std::cout << "Reading file: " << this->filename << "\n";
		in >> s; n= s.size()/2; 
		//w.clear(); w.reserve(n); 
		for ( auto l = 0; l < n; ++l ) {
			pq_types::value_type entry;
			in >> entry;
			w.push_back(entry);
		}
		in.close();
		assert( w.size() == n );
		raw= std::make_unique<raw_processor<node_type,size_type,value_type>>(s,w);
		processor= std::make_unique<T>(s,w);
		nodes_distr= std::make_unique<std::uniform_int_distribution<node_type>>(0,n-1);
		qntl_distr= std::make_unique<std::uniform_int_distribution<size_type>>(1,100);
		value_type lower, upper;
		lower= upper= (w)[0];
		for ( auto l= 1; l < n; ++l )
			lower= std::min(lower,(value_type)(w)[l]), upper= std::max(upper,(value_type)(w)[l]);
		weights_distr= std::make_unique<std::uniform_int_distribution<value_type>>(lower,upper);
	}
};

template <class T>
struct QueriesTestSqrtWeights : testing::Test {
	std::string filename= paths[2];
	std::unique_ptr<path_query_processor<node_type,size_type,value_type>> raw;
	std::unique_ptr<T> processor;
	size_type n;
	std::vector<value_type> w;
	std::default_random_engine gen_nodes, gen_weights, gen_qntl;
	std::unique_ptr<std::uniform_int_distribution<node_type>> nodes_distr;
	std::unique_ptr<std::uniform_int_distribution<size_type>> qntl_distr;
	std::unique_ptr<std::uniform_int_distribution<value_type>> weights_distr;

	QueriesTestSqrtWeights() {
		std::string s;
		std::ifstream in(this->filename);
		std::cout << "Reading file: " << this->filename << "\n";
		in >> s; n= s.size()/2; 
		//w.clear(); w.reserve(n); 
		for ( auto l = 0; l < n; ++l ) {
			pq_types::value_type entry;
			in >> entry;
			w.push_back(entry);
		}
		in.close();
		assert( w.size() == n );
		raw= std::make_unique<raw_processor<node_type,size_type,value_type>>(s,w);
		processor= std::make_unique<T>(s,w);
		nodes_distr= std::make_unique<std::uniform_int_distribution<node_type>>(0,n-1);
		qntl_distr= std::make_unique<std::uniform_int_distribution<size_type>>(1,100);
		value_type lower, upper;
		lower= upper= (w)[0];
		for ( auto l= 1; l < n; ++l )
			lower= std::min(lower,(value_type)(w)[l]), upper= std::max(upper,(value_type)(w)[l]);
		weights_distr= std::make_unique<std::uniform_int_distribution<value_type>>(lower,upper);
	}
};

template <class T>
struct QueriesTestLinearWeights : testing::Test {

	std::unique_ptr<path_query_processor<node_type,size_type,value_type>> raw;
	std::unique_ptr<T> processor;
	size_type n;
	std::vector<value_type> w;
	std::default_random_engine gen_nodes, gen_weights, gen_qntl;
	std::unique_ptr<std::uniform_int_distribution<node_type>> nodes_distr;
	std::unique_ptr<std::uniform_int_distribution<size_type>> qntl_distr;
	std::unique_ptr<std::uniform_int_distribution<value_type>> weights_distr;
	std::string filename= paths[3];

	QueriesTestLinearWeights() {
		std::string s;
		std::ifstream in(this->filename);
		std::cout << "Reading file: " << this->filename << "\n";
		in >> s; n= s.size()/2; 
		//w.clear(); w.reserve(n); 
		for ( auto l = 0; l < n; ++l ) {
			pq_types::value_type entry;
			in >> entry;
			w.push_back(entry);
		}
		in.close();
		assert( w.size() == n );
		raw= std::make_unique<raw_processor<node_type,size_type,value_type>>(s,w);
		processor= std::make_unique<T>(s,w);
		nodes_distr= std::make_unique<std::uniform_int_distribution<node_type>>(0,n-1);
		qntl_distr= std::make_unique<std::uniform_int_distribution<size_type>>(1,100);
		value_type lower, upper;
		lower= upper= (w)[0];
		for ( auto l= 1; l < n; ++l )
			lower= std::min(lower,(value_type)(w)[l]), upper= std::max(upper,(value_type)(w)[l]);
		weights_distr= std::make_unique<std::uniform_int_distribution<value_type>>(lower,upper);
	}
};

#if GTEST_HAS_TYPED_TEST_P
using testing::Types;

TYPED_TEST_CASE_P(QueriesTestLogWeights);
TYPED_TEST_CASE_P(QueriesTestDIMACStree);
TYPED_TEST_CASE_P(QueriesTestLinearWeights);
TYPED_TEST_CASE_P(QueriesTestSqrtWeights);
TYPED_TEST_CASE_P(QueriesTestDegenerateTreeEqualWeights);

TYPED_TEST_P( QueriesTestDIMACStree, MedianIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	for ( auto it= 0; it < ITERS; ++it ) {
		auto x= nodes_dice(), y= nodes_dice();
		assert( this->w.size() >= this->n );
		auto ans1= this->processor->query(x,y);
		auto ans2= this->raw->query((node_type)x,(node_type)y);
		ASSERT_EQ(ans1,ans2);
	}
};

TYPED_TEST_P( QueriesTestDIMACStree, SelectionIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto qntl_dice= std::bind(*(this->qntl_distr),this->gen_qntl);
	for ( auto it= 0; it < ITERS; ++it ) {
		auto x= nodes_dice(), y= nodes_dice();
		assert( this->w.size() >= this->n );
		auto qntl= qntl_dice();
		auto ans1= this->processor->selection(x,y,qntl);
		auto ans2= this->raw->selection((node_type)x,(node_type)y,qntl);
		ASSERT_EQ(ans1,ans2);
	}
};

TYPED_TEST_P( QueriesTestDIMACStree, CountingIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto weights_dice= std::bind(*(this->weights_distr),this->gen_weights);
	for ( auto it= 0; it < ITERS; ++it ) {
		assert( this->w.size() >= this->n );
		auto x= nodes_dice(), y= nodes_dice();
		auto vl= weights_dice(), vr= weights_dice();
		if ( vl > vr ) std::swap(vl,vr);
		auto ans1= this->processor->count(x,y,static_cast<value_type>(vl),static_cast<value_type>(vr));
		auto ans2= this->raw->count((node_type)x,(node_type)y,vl,vr);
		ASSERT_EQ(ans1,ans2);
	}
};

TYPED_TEST_P( QueriesTestDIMACStree, ReportingIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto weights_dice= std::bind(*(this->weights_distr),this->gen_weights);
	for ( auto it= 0; it < ITERS; ++it ) {
		assert( this->w.size() >= this->n );
		auto x= nodes_dice(), y= nodes_dice();
		auto vl= weights_dice(), vr= weights_dice();
		if ( vl > vr ) std::swap(vl,vr);
		std::vector<std::pair<value_type,size_type >> ans1, ans2;
		this->processor->report(x,y,static_cast<value_type>(vl),static_cast<value_type>(vr),ans1);
		this->raw->report((node_type)x,(node_type)y,vl,vr,ans2);
		ASSERT_TRUE(multiset_equality(ans1,ans2));
	}
};

TYPED_TEST_P( QueriesTestSqrtWeights, MedianIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	for ( auto it= 0; it < ITERS; ++it ) {
		auto x= nodes_dice(), y= nodes_dice();
		assert( this->w.size() >= this->n );
		auto ans1= this->processor->query(x,y);
		if ( path_decomposer *decomp= dynamic_cast<path_decomposer *>((this->processor).get()) ) {
		    fprintf(stderr,"the number of segments: %lld\n",(long long)decomp->get_decomposition_length(x,y));
		    fflush(stderr);
		}
		auto ans2= this->raw->query((node_type)x,(node_type)y);
		ASSERT_EQ(ans1,ans2);
	}
};

TYPED_TEST_P( QueriesTestSqrtWeights, SelectionIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto qntl_dice= std::bind(*(this->qntl_distr),this->gen_qntl);
	for ( auto it= 0; it < ITERS; ++it ) {
		auto x= nodes_dice(), y= nodes_dice();
		assert( this->w.size() >= this->n );
		auto qntl= qntl_dice();
		auto ans1= this->processor->selection(x,y,qntl);
		auto ans2= this->raw->selection((node_type)x,(node_type)y,qntl);
		ASSERT_EQ(ans1,ans2);
	}
};

TYPED_TEST_P( QueriesTestSqrtWeights, CountingIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto weights_dice= std::bind(*(this->weights_distr),this->gen_weights);
	for ( auto it= 0; it < ITERS; ++it ) {
		assert( this->w.size() >= this->n );
		auto x= nodes_dice(), y= nodes_dice();
		auto vl= weights_dice(), vr= weights_dice();
		if ( vl > vr ) std::swap(vl,vr);
		auto ans1= this->processor->count(x,y,static_cast<value_type>(vl),static_cast<value_type>(vr));
		auto ans2= this->raw->count((node_type)x,(node_type)y,vl,vr);
		ASSERT_EQ(ans1,ans2);
	}
};

TYPED_TEST_P( QueriesTestSqrtWeights, ReportingIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto weights_dice= std::bind(*(this->weights_distr),this->gen_weights);
	for ( auto it= 0; it < ITERS; ++it ) {
		assert( this->w.size() >= this->n );
		auto x= nodes_dice(), y= nodes_dice();
		auto vl= weights_dice(), vr= weights_dice();
		if ( vl > vr ) std::swap(vl,vr);
		std::vector<std::pair<value_type,size_type >> ans1, ans2;
		this->processor->report(x,y,static_cast<value_type>(vl),static_cast<value_type>(vr),ans1);
		this->raw->report((node_type)x,(node_type)y,vl,vr,ans2);
		ASSERT_TRUE(multiset_equality(ans1,ans2));
	}
};

TYPED_TEST_P( QueriesTestLinearWeights, MedianIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	for ( auto it= 0; it < ITERS; ++it ) {
		auto x= nodes_dice(), y= nodes_dice();
		assert( this->w.size() >= this->n );
		auto ans1= this->processor->query(x,y);
		if ( path_decomposer *decomp= dynamic_cast<path_decomposer *>((this->processor).get()) ) {
		    fprintf(stderr,"the number of segments: %lld\n",(long long)decomp->get_decomposition_length(x,y));
		    fflush(stderr);
		}
		auto ans2= this->raw->query((node_type)x,(node_type)y);
		ASSERT_EQ(ans1,ans2);
	}
};

TYPED_TEST_P( QueriesTestLinearWeights, CountingIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto weights_dice= std::bind(*(this->weights_distr),this->gen_weights);
	for ( auto it= 0; it < ITERS; ++it ) {
		assert( this->w.size() >= this->n );
		auto x= nodes_dice(), y= nodes_dice();
		auto vl= weights_dice(), vr= weights_dice();
		if ( vl > vr ) std::swap(vl,vr);
		auto ans1= this->processor->count(x,y,static_cast<value_type>(vl),static_cast<value_type>(vr));
		auto ans2= this->raw->count((node_type)x,(node_type)y,vl,vr);
		ASSERT_EQ(ans1,ans2);
	}
};
TYPED_TEST_P( QueriesTestLinearWeights, SelectionIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto qntl_dice= std::bind(*(this->nodes_distr),this->gen_qntl);
	for ( auto it= 0; it < ITERS; ++it ) {
		auto x= nodes_dice(), y= nodes_dice();
		assert( this->w.size() >= this->n );
		auto qntl= qntl_dice();
		auto ans1= this->processor->selection(x,y,qntl);
		auto ans2= this->raw->selection((node_type)x,(node_type)y,qntl);
		ASSERT_EQ(ans1,ans2);
	}
};
TYPED_TEST_P( QueriesTestLinearWeights, ReportingIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto weights_dice= std::bind(*(this->weights_distr),this->gen_weights);
	for ( auto it= 0; it < ITERS; ++it ) {
		assert( this->w.size() >= this->n );
		auto x= nodes_dice(), y= nodes_dice();
		auto vl= weights_dice(), vr= weights_dice();
		if ( vl > vr ) std::swap(vl,vr);
		std::vector<std::pair<value_type,size_type >> ans1, ans2;
		this->processor->report(x,y,static_cast<value_type>(vl),static_cast<value_type>(vr),ans1);
		this->raw->report((node_type)x,(node_type)y,vl,vr,ans2);
		ASSERT_TRUE(multiset_equality(ans1,ans2));
	}
};

TYPED_TEST_P( QueriesTestLogWeights, MedianIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	for ( auto it= 0; it < ITERS; ++it ) {
		auto x= nodes_dice(), y= nodes_dice();
		assert( this->w.size() >= this->n );
		auto ans1= this->processor->query(x,y);
		if ( path_decomposer *decomp= dynamic_cast<path_decomposer *>((this->processor).get()) ) {
		    fprintf(stderr,"the number of segments: %lld\n",(long long)decomp->get_decomposition_length(x,y));
		    fflush(stderr);
		}
		auto ans2= this->raw->query((node_type)x,(node_type)y);
		ASSERT_EQ(ans1,ans2);
	}
};

TYPED_TEST_P( QueriesTestLogWeights, SelectionIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto qntl_dice= std::bind(*(this->nodes_distr),this->gen_qntl);
	for ( auto it= 0; it < ITERS; ++it ) {
		auto x= nodes_dice(), y= nodes_dice();
		assert( this->w.size() >= this->n );
		auto qntl= qntl_dice();
		auto ans1= this->processor->selection(x,y,qntl);
		auto ans2= this->raw->selection((node_type)x,(node_type)y,qntl);
		ASSERT_EQ(ans1,ans2);
	}
};

TYPED_TEST_P( QueriesTestLogWeights, CountingIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto weights_dice= std::bind(*(this->weights_distr),this->gen_weights);
	for ( auto it= 0; it < ITERS; ++it ) {
		assert( this->w.size() >= this->n );
		auto x= nodes_dice(), y= nodes_dice();
		auto vl= weights_dice(), vr= weights_dice();
		if ( vl > vr ) std::swap(vl,vr);
		auto ans1= this->processor->count(x,y,static_cast<value_type>(vl),static_cast<value_type>(vr));
		auto ans2= this->raw->count((node_type)x,(node_type)y,vl,vr);
		ASSERT_EQ(ans1,ans2);
	}
};

TYPED_TEST_P( QueriesTestLogWeights, ReportingIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto weights_dice= std::bind(*(this->weights_distr),this->gen_weights);
	for ( auto it= 0; it < ITERS; ++it ) {
		assert( this->w.size() >= this->n );
		auto x= nodes_dice(), y= nodes_dice();
		auto vl= weights_dice(), vr= weights_dice();
		if ( vl > vr ) std::swap(vl,vr);
		std::vector<std::pair<value_type,size_type >> ans1, ans2;
		this->processor->report(x,y,static_cast<value_type>(vl),static_cast<value_type>(vr),ans1);
		this->raw->report((node_type)x,(node_type)y,vl,vr,ans2);
		ASSERT_TRUE(multiset_equality(ans1,ans2));
	}
};

TYPED_TEST_P( QueriesTestDegenerateTreeEqualWeights, MedianIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	for ( auto it= 0; it < ITERS; ++it ) {
		auto x= nodes_dice(), y= nodes_dice();
		assert( this->w.size() >= this->n );
		auto ans1= this->processor->query(x,y);
		auto ans2= this->raw->query((node_type)x,(node_type)y);
		ASSERT_EQ(ans1,ans2);
	}
};

TYPED_TEST_P( QueriesTestDegenerateTreeEqualWeights, SelectionIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto qntl_dice= std::bind(*(this->nodes_distr),this->gen_qntl);
	for ( auto it= 0; it < ITERS; ++it ) {
		auto x= nodes_dice(), y= nodes_dice();
		assert( this->w.size() >= this->n );
		auto qntl= qntl_dice();
		auto ans1= this->processor->selection(x,y,qntl);
		auto ans2= this->raw->selection((node_type)x,(node_type)y,qntl);
		ASSERT_EQ(ans1,ans2);
	}
};

TYPED_TEST_P( QueriesTestDegenerateTreeEqualWeights, CountingIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto weights_dice= std::bind(*(this->weights_distr),this->gen_weights);
	for ( auto it= 0; it < ITERS; ++it ) {
		assert( this->w.size() >= this->n );
		auto x= nodes_dice(), y= nodes_dice();
		auto vl= weights_dice(), vr= weights_dice();
		if ( vl > vr ) std::swap(vl,vr);
		auto ans1= this->processor->count(x,y,static_cast<value_type>(vl),static_cast<value_type>(vr));
		auto ans2= this->raw->count((node_type)x,(node_type)y,vl,vr);
		ASSERT_EQ(ans1,ans2);
	}
};

TYPED_TEST_P( QueriesTestDegenerateTreeEqualWeights, ReportingIsCorrect ) {
	auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
	auto weights_dice= std::bind(*(this->weights_distr),this->gen_weights);
	for ( auto it= 0; it < ITERS; ++it ) {
		assert( this->w.size() >= this->n );
		auto x= nodes_dice(), y= nodes_dice();
		auto vl= weights_dice(), vr= weights_dice();
		if ( vl > vr ) std::swap(vl,vr);
		std::vector<std::pair<value_type,size_type >> ans1, ans2;
		this->processor->report(x,y,static_cast<value_type>(vl),static_cast<value_type>(vr),ans1);
		this->raw->report((node_type)x,(node_type)y,vl,vr,ans2);
		ASSERT_TRUE(multiset_equality(ans1,ans2));
	}
};

REGISTER_TYPED_TEST_CASE_P(QueriesTestLogWeights,CountingIsCorrect,MedianIsCorrect,SelectionIsCorrect,ReportingIsCorrect);
REGISTER_TYPED_TEST_CASE_P(QueriesTestLinearWeights,CountingIsCorrect,MedianIsCorrect,SelectionIsCorrect,ReportingIsCorrect);
REGISTER_TYPED_TEST_CASE_P(QueriesTestDegenerateTreeEqualWeights,MedianIsCorrect,SelectionIsCorrect,CountingIsCorrect,ReportingIsCorrect);
REGISTER_TYPED_TEST_CASE_P(QueriesTestSqrtWeights,MedianIsCorrect,SelectionIsCorrect,CountingIsCorrect,ReportingIsCorrect);

//REGISTER_TYPED_TEST_CASE_P(QueriesTestDIMACStree,MedianIsCorrect,SelectionIsCorrect,CountingIsCorrect,ReportingIsCorrect);

// REGISTER_TYPED_TEST_CASE_P(QueriesTestLinearWeights,MedianIsCorrect,CountingIsCorrect,ReportingIsCorrect);

//typedef Types<trext_compr_mh<>,tree_extraction_mh_flat<>,tree_extraction_flat<>,wt_hpd,naive_processor,tree_extraction<>,tree_extraction_mh<>,naive_processor_lca,nsrs<>> impls;
//typedef Types<naive_processor_lca,tree_extraction_flat_v2<>,tree_extraction_flat<>,wt_hpd,naive_processor,nsrs<>> impls;
//typedef Types<wt_hpd,ext_ptr,tree_extraction_flat_v2<>> impls;
//typedef Types<wt_hpd<>,ext_ptr_v2,naive_processor,naive_processor_lca,nsrs<>> impls;

//typedef wt_hpd <bp_tree_sada<>,
//                    sdsl::bit_vector_il<>
//                    > wt_hpd_uncompressed;
/*typedef wt_hpd <
                    bp_tree_sada<>,
                    sdsl::bit_vector,
                    sdsl::rank_support_v5<>,
                    sdsl::select_support_mcl<1,1>,
                    sdsl::select_support_mcl<0,1>> wt_hpd_uncompressed;
typedef tree_extraction_flat_v2 <
                    sdsl::bp_support_sada<>,2,
                    sdsl::bit_vector,
                    sdsl::rank_support_v5<>,
                    sdsl::select_support_mcl<1,1>,
                    sdsl::select_support_mcl<0,1>
                    > ext_flat_uncompressed;
typedef wt_hpd <bp_tree_sada<>,sdsl::rrr_vector<>> wt_hpd_compressed;
typedef tree_extraction_flat_v2<sdsl::bp_support_sada<>,2,sdsl::rrr_vector<>> ext_flat__compressed;
typedef hybrid_processor hybrid;
 */
typedef tree_ext_sct<
        pq_types::node_type,
        pq_types::size_type,
        pq_types::value_type,
        sdsl::bp_support_sada<>,2,
        sdsl::bit_vector,
        sdsl::rank_support_v5<>,
        sdsl::select_support_mcl<1,1>,
        sdsl::select_support_mcl<0,1>
> ext_flat_uncompressed;

typedef tree_ext_sct<
        pq_types::node_type,
        pq_types::size_type,
        pq_types::value_type,
        sdsl::bp_support_sada<>,2,sdsl::rrr_vector<>
> ext_flat_compressed;
/*
typedef Types<
        wt_hpd_uncompressed,
        wt_hpd_compressed,
        ext_flat_uncompressed,
        ext_flat__compressed,
        ext_ptr_v2,
        nsrs<>,
        naive_processor,
        naive_processor_lca
        > impls;*/
// typedef Types<ext_ptr_v3,hybrid> impls;
typedef Types<hybrid_processor<node_type,size_type,value_type>,ext_flat_uncompressed,ext_flat_compressed> impls;
//typedef Types<tree_extraction_flat<>,wt_hpd,naive_processor,naive_processor_lca,nsrs<>> impls;
//INSTANTIATE_TYPED_TEST_CASE_P(all_impls,    // Instance name
  //                            QueriesTestDIMACStree, // Test case name
    //                          impls);  // Type list
INSTANTIATE_TYPED_TEST_CASE_P(all_impls,    // Instance name
                              QueriesTestLinearWeights, // Test case name
                              impls);  // Type list
INSTANTIATE_TYPED_TEST_CASE_P(all_impls,    // Instance name
                              QueriesTestLogWeights, // Test case name
                              impls);  // Type list
INSTANTIATE_TYPED_TEST_CASE_P(all_impls,    // Instance name
                              QueriesTestSqrtWeights, // Test case name
                              impls);  // Type list
INSTANTIATE_TYPED_TEST_CASE_P(all_impls,    // Instance name
                              QueriesTestDegenerateTreeEqualWeights, // Test case name
                              impls);  // Type list


#endif  // GTEST_HAS_TYPED_TEST_P

int main( int argc, char **argv ) {
    /*
    const __rlim_t kStackSize= 2*M+8;
    struct rlimit r1;
    int result= getrlimit(RLIMIT_STACK,&r1);
    if ( result == 0 ) {
        if ( r1.rlim_cur < kStackSize ) {
            r1.rlim_cur= kStackSize;
            result= setrlimit(RLIMIT_STACK,&r1);
            if ( result != 0 ) {
                assert( false );
            }
        }
    }
     */
	testing::InitGoogleTest(&argc,argv);
	return RUN_ALL_TESTS();
}

