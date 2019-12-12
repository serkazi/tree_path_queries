/*
 * we need to check that the number of segments in heavy path decomposition
 * is indeed O(logn)
 */
#include "benchmark/benchmark.h"
#include <sys/resource.h>
#include <iostream>
#include "pq_types.hpp"
#include <memory>
#include <random>
#include <fstream>
#include <algorithm>
#include <functional>
#include "hybrid_processor.hpp"
#include "wt_hpd.hpp"
#include <bp_tree_sada.hpp>
#include <sdsl/rrr_vector.hpp>

using node_type= pq_types::node_type;
using size_type= pq_types::size_type;
using value_type= pq_types::value_type;

using hybrid             = hybrid_processor<node_type,size_type,value_type>;
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

namespace experiment_settings {
    std::string dataset_path;
    int K, nq;
}

template<typename T>
class hpd_benchmark: public benchmark::Fixture {
protected:
    size_type n;
    std::unique_ptr<T> processor;
    std::default_random_engine engine{};
    std::unique_ptr<std::uniform_int_distribution<node_type>> distribution;
    std::unique_ptr<std::uniform_int_distribution<value_type>> weight_distribution;
public:
    void SetUp(const ::benchmark::State& state) {
        std::ifstream is(experiment_settings::dataset_path);
        std::string topology;
        is >> topology;
        std::vector<value_type> w(n= topology.size()/2);
        for ( auto &x: w ) is >> x;
        auto a= *(std::min_element(w.begin(),w.end()));
        auto b= *(std::max_element(w.begin(),w.end()));
        try {
            processor = std::make_unique<T>(topology, w);
        } catch ( std::exception &e ) {
            std::cerr << e.what() << std::endl;
        }
        distribution= std::make_unique<std::uniform_int_distribution<node_type>>(0,n-1);
        weight_distribution= std::make_unique<std::uniform_int_distribution<value_type>>(a,b);
    }
    void TearDown(const ::benchmark::State& state) {
        //some clean-up
    }
    auto num_semgnets( node_type x, node_type y ) {
        return processor->num_segments(x,y);
    }
};

BENCHMARK_TEMPLATE_F(hpd_benchmark,hybrid,hybrid)(benchmark::State &state) {
    auto dice= std::bind(*distribution,engine);
    for ( auto _ : state ) {
        // the code that gets measured
        auto x= dice(), y= dice();
        auto res= num_semgnets(x,y) ;
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        // TODO: present this as a number of iterations
        state.counters["numsegments"]+= res;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(hpd_benchmark,wt_hpd,wt_hpd_uncompressed)(benchmark::State &state) {
    auto dice= std::bind(*distribution,engine);
    for ( auto _ : state ) {
        // the code that gets measured
        auto x= dice(), y= dice();
        auto res= num_semgnets(x,y) ;
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        // TODO: present this as a number of iterations
        state.counters["numsegments"]+= res;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(hpd_benchmark,wt_hpd_rrr,wt_hpd_rrr)(benchmark::State &state) {
    auto dice= std::bind(*distribution,engine);
    for ( auto _ : state ) {
        // the code that gets measured
        state.PauseTiming(); // <-- need to pause for a set up though
        auto x= dice(), y= dice();
        auto res= num_semgnets(x,y) ;
        benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
        benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        // TODO: present this as a number of iterations
        state.counters["numsegments"]+= res;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

int main (int argc, char** argv)
{
    const rlim_t kStackSize = 20 * 1024ll * 1024ll * 1024ll;   // min stack size = 20 GiB
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0) {
        if (rl.rlim_cur < kStackSize) {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if ( result != 0 ) {
                std::cerr << "setrlimit returned result = " << result << std::endl;
            }
        }
    }

    const int METHOD= 1,
            FULL_PATH= 2,
            NUM_QUERIES= 3,
            K_VAL= 4;

    experiment_settings::dataset_path= std::string(argv[FULL_PATH]);
    /*
    experiment_settings::K           = strtol(argv[K_VAL],nullptr,10);
    experiment_settings::nq          = strtol(argv[NUM_QUERIES],nullptr,10);
    */
    std::cerr << "Dataset path: " << experiment_settings::dataset_path << std::endl;
    ::benchmark::Initialize (&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks ();
}

