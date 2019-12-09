//
// Created by kazi on 2019-12-09.
//
#include <sys/resource.h>
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
#include <algorithm>
#include "sdsl/rrr_vector.hpp"
#include <bp_tree_sada.hpp>
#include "nsrs.hpp"
#include "ext_ptr.hpp"
#include "tree_ext_sct.hpp"
#include "query_stream_builder.hpp"

using node_type= pq_types::node_type;
using size_type= pq_types::size_type;
using value_type= pq_types::value_type;

const std::string paths[]= {
        std::string("degenerate_tree_equal_weights.txt"), //0
        std::string("log_weights.txt"), //1
        std::string("sqrt_weights.txt"), //2
        //std::string("linear_small_weights.txt"), //3
        std::string("rnd.100mln.sqrt.puu"),
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

const std::string root= "/users/grad/kazi/CLionProjects/tree_path_queries/data/datasets/";

namespace experiment_settings {
    std::string dataset_path;
    int K, nq;
}

/**
 * @details The idea is to pre-generate a set of queries, and then
 * run the benchmark as if answering the entire set of queries
 * had been our goal; this way, each of the data structures
 * get a chance to answer the same set of queries, hence
 * the experiment is more on the "no-questions-asked" side
 * @tparam T
 */
template<typename T>
class path_queries_benchmark: public benchmark::Fixture {
protected:
    size_type n;
    std::unique_ptr<T> processor;
    std::default_random_engine engine{};
    std::unique_ptr<std::uniform_int_distribution<node_type>> distribution;
    std::unique_ptr<std::uniform_int_distribution<value_type>> weight_distribution;
    std::vector<path_queries::counting_query<node_type,size_type,value_type>> cnt_queries;
    std::vector<path_queries::reporting_query<node_type,size_type,value_type>> rpt_queries;
    std::vector<path_queries::median_query<node_type,size_type,value_type>> med_queries;
public:
    void SetUp(const ::benchmark::State& state) {
        std::ifstream is(experiment_settings::dataset_path);
        std::string topology;
        is >> topology;
        std::vector<value_type> w(n= topology.size()/2);
        for ( auto &x: w ) is >> x;
        auto a= *(std::min_element(w.begin(),w.end()));
        auto b= *(std::max_element(w.begin(),w.end()));
        processor= std::make_unique<T>(topology,w);
        distribution= std::make_unique<std::uniform_int_distribution<node_type>>(0,n-1);
        weight_distribution= std::make_unique<std::uniform_int_distribution<value_type>>(a,b);

        // generate the queries using our builder
        query_stream_builder<node_type,size_type,value_type> builder;
        builder.set(path_queries::QUERY_TYPE::COUNTING,experiment_settings::nq)
               .set(path_queries::QUERY_TYPE::MEDIAN,experiment_settings::nq)
               .set(path_queries::QUERY_TYPE::REPORTING,experiment_settings::nq)
               .set_scaling_param(experiment_settings::K)
               .set_node_range(n)
               .set_weight_range(a,b)
               .build();

        // insert all the queries into our internal storage, so that we can partition it
        std::vector<path_queries::pq_request<node_type,size_type,value_type>> qrs(builder.begin(),builder.end());

        // clear the builder to save space
        builder.clear();

        // partition the queries such that our filtered benchmarks can access only the relevant kind
        auto counting_end= std::partition(qrs.begin(),qrs.end(),[&]( const auto &q ) {
            return std::holds_alternative<
                    path_queries::counting_query<node_type,size_type,value_type>>(q);
        });
        auto reporting_end= std::partition(counting_end,qrs.end(),[&]( const auto &q ) {
            return std::holds_alternative<
                    path_queries::reporting_query<node_type,size_type,value_type>>(q);
        });
        auto median_end= std::partition(reporting_end,qrs.end(),[&]( const auto &q ) {
            return std::holds_alternative<
                    path_queries::median_query<node_type,size_type,value_type>>(q);
        });
        // unpack the values, so that we don't have to unpack them during the benchmark runs
        std::transform(qrs.begin(),counting_end,std::back_inserter(cnt_queries),[&]( const auto &qr ) {
            return std::get<path_queries::counting_query<node_type,size_type,value_type>>(qr);
        });
        std::transform(counting_end,reporting_end,std::back_inserter(rpt_queries),[&]( const auto &qr ) {
            return std::get<path_queries::reporting_query<node_type,size_type,value_type>>(qr);
        });
        std::transform(reporting_end,qrs.end(),std::back_inserter(med_queries),[&]( const auto &qr ) {
            return std::get<path_queries::median_query<node_type,size_type,value_type>>(qr);
        });
        qrs.clear();
    }
    void TearDown(const ::benchmark::State& state) {
        //some clean-up
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
};

BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_median,nv)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        const auto &dict= med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_lca_median,nv_lca)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        const auto &dict= med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nsrs_median,nv_succ)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        const auto &dict= med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,hybrid_median,hybrid)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        const auto &dict= med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_ptr_median,tree_ext_ptr)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        const auto &dict= med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_un_median,tree_ext_sct_un)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        const auto &dict= med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_rrr_median,tree_ext_sct_rrr)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        const auto &dict= med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_un_median,wt_hpd_uncompressed)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        const auto &dict= med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_rrr_median,wt_hpd_rrr)(benchmark::State &state) {
    for ( auto _ : state ) {
        // the code that gets measured
        const auto &dict= med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

//=============================== Counting ==============================================/
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_counting,nv)(benchmark::State &state) {
    const auto &dict= cnt_queries;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_lca_counting,nv_lca)(benchmark::State &state) {
    const auto &dict= cnt_queries;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nsrs_counting,nv_succ)(benchmark::State &state) {
    const auto &dict= cnt_queries;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,hybrid_counting,hybrid)(benchmark::State &state) {
    const auto &dict= cnt_queries;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_ptr_counting,tree_ext_ptr)(benchmark::State &state) {
    const auto &dict= cnt_queries;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_un_counting,tree_ext_sct_un)(benchmark::State &state) {
    const auto &dict= cnt_queries;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_rrr_counting,tree_ext_sct_rrr)(benchmark::State &state) {
    const auto &dict= cnt_queries;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_uncompressed_counting,wt_hpd_uncompressed)(benchmark::State &state) {
    const auto &dict= cnt_queries;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_rrr_counting,wt_hpd_rrr)(benchmark::State &state) {
    const auto &dict= cnt_queries;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

//=============================== Reporting ==============================================/
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_reporting,nv)(benchmark::State &state) {
    const auto &dict= rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_lca_reporting,nv_lca)(benchmark::State &state) {
    const auto &dict= rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nsrs_reporting,nv_succ)(benchmark::State &state) {
    const auto &dict= rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,hybrid_reporting,hybrid)(benchmark::State &state) {
    const auto &dict= rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_ptr_reporting,tree_ext_ptr)(benchmark::State &state) {
    const auto &dict= rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_un_reporting,tree_ext_sct_un)(benchmark::State &state) {
    const auto &dict= rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_rrr_reporting,tree_ext_sct_rrr)(benchmark::State &state) {
    const auto &dict= rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_uncompressed_reporting,wt_hpd_uncompressed)(benchmark::State &state) {
    const auto &dict= rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_rrr_reporting,wt_hpd_rrr)(benchmark::State &state) {
    const auto &dict= rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
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
// @see https://stackoverflow.com/questions/51519376/how-to-pass-arguments-to-a-google-benchmark-program

// BENCHMARK_MAIN();
// BENCHMARK(BM_test)->Unit(benchmark::kMillisecond);
// BENCHMARK_MAIN() lives in benchmark_api.h
// should be run like this:
// perl benchmark_runner.pl count /users/grad/kazi/CLionProjects/tree_path_queries/data/testdata/linear_small_weights.txt
// so essentially:
// perl benchmark_aggregate_runner.pl <median|count|report|select> <full_path_to_dataset> <num_queries> <K>
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
    experiment_settings::K           = strtol(argv[K_VAL],nullptr,10);
    experiment_settings::nq          = strtol(argv[NUM_QUERIES],nullptr,10);

    std::cerr << "Dataset path: " << experiment_settings::dataset_path << std::endl;
    ::benchmark::Initialize (&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks ();
}
