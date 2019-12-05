//
// Created by kazi on 2019-11-25.
//
#include <benchmark/benchmark.h>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include "path_query_processor.hpp"
#include "naive_processor_lca.hpp"
#include "pq_request.hpp"
#include "naive_processor.hpp"
#include "wt_hpd.hpp"
#include "hybrid_processor.hpp"
#include <random>
#include <functional>
#include "sdsl/rrr_vector.hpp"
#include <bp_tree_sada.hpp>
#include "nsrs.hpp"
#include "ext_ptr.hpp"
#include "tree_ext_sct.hpp"

using node_type= pq_types::node_type;
using size_type= pq_types::size_type;
using value_type= pq_types::value_type;

const std::string paths[]= {
        std::string("degenerate_tree_equal_weights.txt"), //0
        std::string("log_weights.txt"), //1
        std::string("sqrt_weights.txt"), //2
        std::string("linear_small_weights.txt"), //3
        std::string("linear_weights.txt"), //4
        std::string("us.rd.d.dfs.dimacs.puu") //5
};

using nv                 = naive_processor<node_type,size_type,value_type>;
using nv_lca             = naive_processor_lca<node_type,size_type,value_type>;
using nv_succ            = nsrs<node_type,size_type,value_type>;
using hybrid             = hybrid_processor<node_type,size_type,value_type>;
using tree_ext_ptr       = ext_ptr<node_type,size_type,value_type>;
using wt_hpd_uncompressed= wt_hpd<node_type,size_type,value_type,
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

const std::string root= "/users/grad/kazi/CLionProjects/tree_path_queries/data/testdata/";

template<typename T>
class path_queries_benchmark: public benchmark::Fixture {
protected:
    std::unique_ptr<T> processor;
    std::default_random_engine engine{};
    std::unique_ptr<std::uniform_int_distribution<node_type>> distribution;
    size_type n;
public:
    void SetUp(const ::benchmark::State& state) {
        std::ifstream is(root+paths[3]);
        std::string topology;
        is >> topology;
        std::vector<value_type> w(n= topology.size()/2);
        for ( auto &x: w ) is >> x;
        processor= std::make_unique<T>(topology,w);
        distribution= std::make_unique<std::uniform_int_distribution<node_type>>(0,n-1);
    }
    void TearDown(const ::benchmark::State& state) {
        //some clean-up
    }
    auto median( node_type x, node_type y ) {
        return processor->query(x,y);
    }
};

BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_median,nv)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto dice= std::bind(*distribution,engine);
        auto x= dice(), y= dice();
        // we are ready to go now
        state.ResumeTiming();
        auto res= median(x,y);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,
                     ext_ptr_median,tree_ext_ptr)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto dice= std::bind(*distribution,engine);
        auto x= dice(), y= dice();
        // we are ready to go now
        state.ResumeTiming();
        auto res= median(x,y);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,
                     ext_ptr_sct_median,tree_ext_sct_un)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto dice= std::bind(*distribution,engine);
        auto x= dice(), y= dice();
        // we are ready to go now
        state.ResumeTiming();
        auto res= median(x,y);
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}


static void CustomArguments(benchmark::internal::Benchmark* b) {
    for (int i = 0; i <= 10; ++i)
        for (int j = 32; j <= 1024*1024; j *= 8)
            b->Args({i, j});
}

// BENCHMARK(BM_SetInsert)->Apply(CustomArguments);

BENCHMARK_MAIN();
