//
// the goal is a benchmarking test of utmost granularity
// in order to use it in the GUI
// the supplied parameters would be the data structure
// and the types of queries
// and if applicable large/medium/small configuration
//
#include <gflags/gflags.h>
#include <sys/resource.h>
#include "benchmark/benchmark.h"
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include "path_query_processor.hpp"
#include "naive_processor_lca.hpp"
#include "pq_request.hpp"
#include "naive_processor.hpp"
#include "wt_hpd.hpp"
#include "wt_hpd_ptr.hpp"
#include <random>
#include <functional>
#include <algorithm>
#include "sdsl/rrr_vector.hpp"
#include <bp_tree_sada.hpp>
#include "nsrs.hpp"
#include "ext_ptr.hpp"
#include "tree_ext_sct.hpp"
#include "query_stream_builder.hpp"
#include "bp_trees.hpp"

using node_type= pq_types::node_type;
using size_type= pq_types::size_type;
using value_type= pq_types::value_type;

using wt_hpd_un= wt_hpd<node_type,size_type,value_type,
	  	bp_trees::bp_gg_fast<node_type,size_type>,
        sdsl::bit_vector,
		sdsl::rank_support_v5<>,
        sdsl::select_support_mcl<1,1>,
        sdsl::select_support_mcl<0,1>
>;
using tree_ext_sct_un= tree_ext_sct<
        node_type,size_type,value_type,
        sdsl::bp_support_gg<>,
		2,
        sdsl::bit_vector ,
        sdsl::rank_support_v5<>,
        sdsl::select_support_mcl<1,1>,
        sdsl::select_support_mcl<0,1>
>;
using wt_hpd_rrr= wt_hpd<
        node_type,size_type,value_type,
	  	bp_trees::bp_gg_fast<node_type,size_type>,
        sdsl::rrr_vector<>
>;
using tree_ext_sct_rrr= tree_ext_sct<
        node_type,size_type,
		value_type,
        sdsl::bp_support_gg<>,
		2,
        sdsl::rrr_vector<>
>;

using nv_simple          = naive_processor<node_type,size_type,value_type>;
using nv_lca             = naive_processor_lca<node_type,size_type,value_type>;
using nv_sct             = nsrs<node_type,size_type,value_type>;
using wt_hpd_ptr         = wt_hpd_ptr<node_type,size_type,value_type>;
using tree_ext_ptr       = ext_ptr<node_type,size_type,value_type>;
/*
using wt_hpd_un= wt_hpd<node_type,size_type,value_type,
        bp_tree_sada<node_type,size_type>,
        sdsl::bit_vector,sdsl::rank_support_v5<>,
        sdsl::select_support_mcl<1,1>,
        sdsl::select_support_mcl<0,1>
>;
using wt_hpd_rrr         = wt_hpd<
        node_type,size_type,value_type,
        bp_tree_sada<node_type,size_type>,
        sdsl::rrr_vector<>
>;
using tree_ext_sct_un    = tree_ext_sct<
        node_type,size_type,value_type,
        sdsl::bp_support_sada<>,2,
        sdsl::bit_vector ,
        sdsl::rank_support_v5<>,
        sdsl::select_support_mcl<1,1>,
        sdsl::select_support_mcl<0,1>
>;
using tree_ext_sct_rrr  = tree_ext_sct<
        node_type,size_type,value_type,
        sdsl::bp_support_sada<>,2,
        sdsl::rrr_vector<>
>;
*/

namespace experiment_settings {
    std::string dataset_path;
    size_t K;
}

/**
 * @detail: the constructor accepts (a,b,K)
 * a call to operator() generates a pair [x,y] such that
 * x has been generated u.a.r., and y is generated from [x,x+(b-x)/K]
 */
template<typename T>
class range_generator {

    std::default_random_engine engine{};
    std::unique_ptr<std::mt19937> generator= std::make_unique<std::mt19937>();
    std::unique_ptr<std::uniform_real_distribution<double>> distribution;
	const T a, b;
	const size_type K;

    T scale( double tau, T a, T b ) {
        return std::min(b,static_cast<T>(floor(a + tau*(b-a)+1e-7)));
    }

    double next_real() { return (*distribution)(*generator); }

public:
	range_generator( T a, T b, size_type K): a(a), b(b), K(k) {
        distribution= std::make_unique<std::uniform_real_distribution<double>>(0,1);
	}
	std::pair<T,T> operator() () {
		auto x= scale(next_real(),a,b);
		assert( a <= x and x <= b );
		auto len= std::max(static_cast<T>(1),static_cast<T>((b-x)/K));
		assert( len >= 1 );
		auto y= x+scale(next_real(),1,len)-1;
		assert( y <= b );
		assert( y >= x and (y-x+1) <= len );
		std::pair<value_type,value_type> res= {x,std::min(y,b)};
		assert( res.first <= res.second );
		assert( a <= res.first and res.first <= b );
		assert( a <= res.second and res.second <= b );
		return res;
	}
};

template<typename T>
class path_queries_benchmark: public benchmark::Fixture {
protected:
    size_type n;
    std::unique_ptr<T> processor;
	std::unqiue_ptr<weight_range_generator<size_type>> nrg; //node-range generator
	std::unique_ptr<weight_range_generator<value_type>> wrg; //weight-range generator
	std::vector<value_type> weights;
public:
    void SetUp(const ::benchmark::State& state) {
        std::ifstream is(experiment_settings::dataset_path);
        std::string topology;
        is >> topology;
        std::vector<value_type> w(n= topology.size()/2);
		weights= w, std::sort(weights.begin(),weights.end());
        for ( auto &x: w ) is >> x;
        auto a= *(std::min_element(w.begin(),w.end()));
        auto b= *(std::max_element(w.begin(),w.end()));
        processor= std::make_unique<T>(topology,w);
		nrg= std::make_unique<range_generator<size_type>>(0,n-1,1);
		wrg= std::make_unique<range_generator<value_type>>(0,n-1,experiment_settings::K);
    }
    void TearDown(const ::benchmark::State& state) {
		processor.reset(), processor= nullptr;
		nrg.reset(), nrg= nullptr;
		wrg.reset(), wrg= nullptr;
    }
    auto median( node_type x, node_type y ) {
        return processor->query(x,y);
    }
    auto counting( node_type x, node_type y, value_type a, value_type b ) {
        return processor->count(x,y,a,b);
    }
    void reporting( node_type x, node_type y, value_type a, value_type b, std::vector<std::pair<value_type,size_type>> &res ) {
        processor->report(x,y,a,b,res);
    }
	std::int64_t num_segments( node_type x, node_type y ) {
		if ( dynamic_cast<path_decomposer<node_type,size_type> *>(processor.get()) ) 
			return dynamic_cast<path_decomposer<node_type,size_type> *>(processor.get())->get_decomposition_length(x,y);
		return 0;
	}
};

//=============================== Median ==============================================/
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_simple_median,nv_simple)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        // we are ready to go now
        state.ResumeTiming();
        auto res= median(x,y);
        benchmark::DoNotOptimize(res); 
        benchmark::ClobberMemory(); 
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_lca_median,nv_lca)(benchmark::State &state) {
    for (auto _ : state) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        // we are ready to go now
        state.ResumeTiming();
        auto res = median(x, y);
        benchmark::DoNotOptimize(res); 
        benchmark::ClobberMemory();
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,nsrs_median,nv_sct)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        // we are ready to go now
        state.ResumeTiming();
        auto res= median(x,y);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_ptr_median,tree_ext_ptr)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        // we are ready to go now
        state.ResumeTiming();
        auto res= median(x,y);
        benchmark::DoNotOptimize(res); 
        benchmark::ClobberMemory(); 
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_ptr_median,wt_hpd_ptr)(benchmark::State &state) {
    for (auto _ : state) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        // we are ready to go now
        state.ResumeTiming();
        auto res = median(x, y);
        benchmark::DoNotOptimize(res); 
        benchmark::ClobberMemory();
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_rrr_median,tree_ext_sct_rrr)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        // we are ready to go now
        state.ResumeTiming();
        auto res= median(x,y);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_un_median,tree_ext_sct_un)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        // we are ready to go now
        state.ResumeTiming();
        auto res= median(x,y);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_un_median,wt_hpd_un)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        // we are ready to go now
        state.ResumeTiming();
        auto res= median(x,y);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_rrr_median,wt_hpd_rrr)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        // we are ready to go now
        state.ResumeTiming();
        auto res= median(x,y);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

//=============================== Counting ==============================================/
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_simple_counting,nv_simple)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        auto res= counting(x,y,a,b);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_lca_counting,nv_lca)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        auto res= counting(x,y,a,b);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_sct_counting,nv_sct)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        auto res= counting(x,y,a,b);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_ptr_counting,tree_ext_ptr)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        auto res= counting(x,y,a,b);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_ptr_counting,wt_hpd_ptr)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        auto res= counting(x,y,a,b);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_rrr_counting,tree_ext_sct_rrr)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        auto res= counting(x,y,a,b);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_un_counting,tree_ext_sct_un)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        auto res= counting(x,y,a,b);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_rrr_counting,wt_hpd_rrr)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        auto res= counting(x,y,a,b);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_un_counting,wt_hpd_un)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        auto res= counting(x,y,a,b);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

//=============================== Reporting ==============================================/
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_simple_reporting,nv_simple)(benchmark::State &state) {
	std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
		res.clear();
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        reporting(x,y,a,b,res);
        benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_lca_reporting,nv_lca)(benchmark::State &state) {
	std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
		res.clear();
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        reporting(x,y,a,b,res);
        benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_sct_reporting,nv_sct)(benchmark::State &state) {
	std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
		res.clear();
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        reporting(x,y,a,b,res);
        benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_ptr_reporting,tree_ext_ptr)(benchmark::State &state) {
	std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
		res.clear();
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        reporting(x,y,a,b,res);
        benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_ptr_reporting,wt_hpd_ptr)(benchmark::State &state) {
	std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
		res.clear();
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        reporting(x,y,a,b,res);
        benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_rrr_reporting,tree_ext_sct_rrr)(benchmark::State &state) {
	std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
		res.clear();
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        reporting(x,y,a,b,res);
        benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_un_reporting,tree_ext_sct_un)(benchmark::State &state) {
	std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
		res.clear();
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        reporting(x,y,a,b,res);
        benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_rrr_reporting,wt_hpd_rrr)(benchmark::State &state) {
	std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
		res.clear();
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        reporting(x,y,a,b,res);
        benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_un_reporting,wt_hpd_un)(benchmark::State &state) {
	std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
		res.clear();
        auto [x,y]= (*nrg)();
        auto [a,b]= (*wdice)();
		a= weights[a], b= weights[b];
        // we are ready to go now
        state.ResumeTiming();
        reporting(x,y,a,b,res);
        benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
    }
}

//"--benchmark_counters_tabular=true --benchmark_format=json --benchmark_out=${res} --benchmark_filter=$queries_to_filter"
DEFINE_uint64(K,1ull,"K= 1 for large, K= 10 for medium, K= 100 for small configuration in counting and reporting");
DEFINE_string(dataset_path,"","full path to the dataset");
DEFINE_string(query_type,"median","type of query: median, counting, reporting");
DEFINE_string(data_structure,"nv","data structure: one of 'nv', 'nv_lca', 'nv_sct', 'ext_ptr', 'whp_ptr', 'ext_sct_un', 'ext_sct_rrr', 'whp_un', 'whp_rrr'");
DEFINE_string(output_format,"json","format of output: csv, json");
DEFINE_string(output_file,"","output file path");
int main( int argc, char **argv ) {
    const rlim_t kStackSize = 20 * 1024ll * 1024ll * 1024ll;
    struct rlimit rl;
    int result;

	const std::map<std::string,std::string> ds2internal= {
		{"nv","nv_simple"},
		{"nv_lca","nv_lca"},
		{"nv_sct","nv_sct"},
		{"ext_ptr","tree_ext_ptr"},
		{"whp_ptr","wt_hpd_ptr"},
		{"ext_sct_un","tree_ext_sct_un"},
		{"ext_sct_rrr","tree_ext_sct_rrr"},
		{"whp_un","wt_hpd_un"},
		{"whp_rrr","wt_hpd_rrr"}
	};

    result= getrlimit(RLIMIT_STACK, &rl);
    if (result == 0) {
        if (rl.rlim_cur < kStackSize) {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if ( result != 0 ) {
                std::cerr << "setrlimit returned result = " << result << std::endl;
            }
			else {
				std::cerr << "Stack limit successfully set" << std::endl;
			}
        }
    }

	std::vector<std::string> flags_for_gbench{};

	gflags::ParseCommandLineFlags(&argc,&argv,true);
    // ".*nv_lca_counting\|.*tree_ext_ptr_counting"
	std::string flag= ".*"+ds2internal->find(FLAGS_data_structure).second+"_"+FLAGS_query_type;
	flags_for_gbench.push_back("--benchmark_filter="+flag);
	flags_for_gbench.push_back("--benchmark_format="+FLAGS_output_format);
	flags_for_gbench.push_back("--benchmark_out="+FLAGS_output_file);
    experiment_settings::dataset_path= FLAGS_dataset_path;
    experiment_settings::K           = FLAGS_K;

	char *margv[argc= 3]= {flags_for_bench[0].c_str(),flags_for_bench[1].c_str(),flags_for_bench[2].c_str()};

    ::benchmark::Initialize (&argc, margv);
    ::benchmark::RunSpecifiedBenchmarks ();
}
