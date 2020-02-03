//
// Created by kazi on 2019-12-16.
//
#include <sys/resource.h>
// #include <benchmark/benchmark.h>
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
#define NVENABLED 1

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

/*
template <
          typename node_type= pq_types::node_type,
          typename size_type= pq_types::size_type,
          uint32_t t_sml_blk= 256,
          uint32_t t_med_deg= 32,
          typename t_rank= sdsl::rank_support_v5<>,
          typename t_select= sdsl::select_support_mcl<>
         >
 */

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

using nv                 = naive_processor<node_type,size_type,value_type>;
using nv_lca             = naive_processor_lca<node_type,size_type,value_type>;
using nv_sct             = nsrs<node_type,size_type,value_type>;
using wt_hp_ptr          = wt_hpd_ptr<node_type,size_type,value_type>;
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

const std::string root= "/users/grad/kazi/CLionProjects/tree_path_queries/data/datasets/";

// having the structure "shared_info", we make sure our processors run the exact same query-set
// and save some time into the bargain
namespace experiment_settings {
    std::string dataset_path;
    int K, nq;

    struct shared_info {
        std::default_random_engine engine{};
        std::unique_ptr<std::uniform_int_distribution<node_type>> distribution;
        std::unique_ptr<std::uniform_int_distribution<value_type>> weight_distribution;
        std::vector<path_queries::counting_query<node_type,size_type,value_type>> cnt_queries;
        std::vector<path_queries::reporting_query<node_type,size_type,value_type>> rpt_queries;
        std::vector<path_queries::median_query<node_type,size_type,value_type>> med_queries;
		std::vector<value_type> weights;

        shared_info() {
            size_type n;
            std::cerr << "Reading dataset: " << dataset_path << std::endl;
            std::ifstream is(experiment_settings::dataset_path);
            std::string topology;
            is >> topology;
            std::vector<value_type> w(n= topology.size()/2);
            for ( auto &x: w ) is >> x;
			weights= w, std::sort(weights.begin(),weights.end());

            auto a= *(std::min_element(w.begin(),w.end()));
            auto b= *(std::max_element(w.begin(),w.end()));
            distribution= std::make_unique<std::uniform_int_distribution<node_type>>(0,n-1);
            weight_distribution= std::make_unique<std::uniform_int_distribution<value_type>>(a,b);

            // generate the queries using our builder
            query_stream_builder<node_type,size_type,value_type> builder;
            builder.set(path_queries::QUERY_TYPE::COUNTING,experiment_settings::nq)
                    .set(path_queries::QUERY_TYPE::MEDIAN,experiment_settings::nq)
                    .set(path_queries::QUERY_TYPE::REPORTING,experiment_settings::nq)
                    .set_scaling_param(experiment_settings::K)
                    .set_node_range(n)
                    .set_weight_range(0,n-1)
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
            std::transform(qrs.begin(),counting_end,std::back_inserter(cnt_queries),[&]( auto qr ) {
				auto q= std::get<path_queries::counting_query<node_type,size_type,value_type>>(qr);
				assert( q.a_ < n );
				q.a_= weights[q.a_], q.b_= weights[q.b_];
                //return std::get<path_queries::counting_query<node_type,size_type,value_type>>(qr);
				return q;
            });
            std::transform(counting_end,reporting_end,std::back_inserter(rpt_queries),[&]( auto qr ) {
			    auto q= std::get<path_queries::reporting_query<node_type,size_type,value_type>>(qr);
				assert( q.a_ < n );
				q.a_= weights[q.a_], q.b_= weights[q.b_];
                //return std::get<path_queries::reporting_query<node_type,size_type,value_type>>(qr);
				return q;
            });
            std::transform(reporting_end,qrs.end(),std::back_inserter(med_queries),[&]( auto qr ) {
                return std::get<path_queries::median_query<node_type,size_type,value_type>>(qr);
            });
            qrs.clear();
        }

        virtual ~shared_info() = default;

    };

    std::unique_ptr<shared_info> shared_info_obj= nullptr;

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
public:
    void SetUp(const ::benchmark::State& state) {
        std::ifstream is(experiment_settings::dataset_path);
        std::string topology;
        is >> topology;
        std::vector<value_type> w(n= topology.size()/2);
        for ( auto &x: w ) is >> x;
        processor= std::make_unique<T>(topology,w);
    }
    void TearDown(const ::benchmark::State& state) {
		processor.reset(), processor= nullptr;
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
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_median,nv)(benchmark::State &state) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_lca_median,nv_lca)(benchmark::State &state) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_ptr_median,tree_ext_ptr)(benchmark::State &state) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        // const auto &dict= med_queries;
        const auto &dict= experiment_settings::shared_info_obj->med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_ptr_median,wt_hp_ptr)(benchmark::State &state) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_rrr_median,tree_ext_sct_rrr)(benchmark::State &state) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        // const auto &dict= med_queries;
        const auto &dict= experiment_settings::shared_info_obj->med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_un_median,tree_ext_sct_un)(benchmark::State &state) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        // const auto &dict= med_queries;
        const auto &dict= experiment_settings::shared_info_obj->med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_rrr_median,wt_hpd_rrr)(benchmark::State &state) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        // const auto &dict= med_queries;
        const auto &dict= experiment_settings::shared_info_obj->med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_un_median,wt_hpd_un)(benchmark::State &state) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        // const auto &dict= med_queries;
        const auto &dict= experiment_settings::shared_info_obj->med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_sct_median,nv_sct)(benchmark::State &state) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->med_queries;
        for ( const auto &qr: dict ) {
            auto res = median(qr.x_,qr.y_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

//=============================== Counting ==============================================/
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_counting,nv)(benchmark::State &state) {
	std::vector<size_type> output_sizes;
    // const auto &dict= cnt_queries;
    const auto &dict= experiment_settings::shared_info_obj->cnt_queries;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
		output_sizes.resize(dict.size());
		int k= 0;
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
			output_sizes[k++]= res;
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
		double mean= state.counters["avgOutputSize"]=
				std::accumulate(
						output_sizes.begin(),
						output_sizes.end(),
						0ull)
				/ (0.00 + output_sizes.size());
		std::vector<double> diff(output_sizes.size());
		std::transform(output_sizes.begin(), output_sizes.end(), diff.begin(), [mean]( auto x ) { return x-mean; });
		state.counters["stddevOfOutputSizes"]= std::sqrt(std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0)/output_sizes.size());
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_lca_counting,nv_lca)(benchmark::State &state) {
    // const auto &dict= cnt_queries;
    const auto &dict= experiment_settings::shared_info_obj->cnt_queries;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_ptr_counting,tree_ext_ptr)(benchmark::State &state) {
    // const auto &dict= cnt_queries;
    const auto &dict= experiment_settings::shared_info_obj->cnt_queries;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_ptr_counting,wt_hp_ptr)(benchmark::State &state) {
    // const auto &dict= cnt_queries;
    const auto &dict= experiment_settings::shared_info_obj->cnt_queries;
	std::vector<std::int64_t> decomposition_lengths;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
		decomposition_lengths.resize(dict.size());
		size_t k= 0;
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
			decomposition_lengths[k++]= num_segments(qr.x_,qr.y_);
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
		double mean= state.counters["avgNumSegments"]= std::accumulate(
						decomposition_lengths.begin(),
						decomposition_lengths.end(),
						0ull,
						[]( std::uint64_t acc, auto x ) { return acc+x; }) / (0.00 + decomposition_lengths.size());
		state.counters["stddev"]= sqrt(std::accumulate(
						decomposition_lengths.begin(),
						decomposition_lengths.end(),
						0.00,
						[&mean]( double acc, auto x ) { return acc+(x-mean)*(x-mean); }) / (decomposition_lengths.size()-1.00)
						);
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_rrr_counting,tree_ext_sct_rrr)(benchmark::State &state) {
    // const auto &dict= cnt_queries;
    const auto &dict= experiment_settings::shared_info_obj->cnt_queries;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_un_counting,tree_ext_sct_un)(benchmark::State &state) {
    // const auto &dict= cnt_queries;
    const auto &dict= experiment_settings::shared_info_obj->cnt_queries;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_rrr_counting,wt_hpd_rrr)(benchmark::State &state) {
    // const auto &dict= cnt_queries;
    const auto &dict= experiment_settings::shared_info_obj->cnt_queries;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_un_counting,wt_hpd_un)(benchmark::State &state) {
    // const auto &dict= cnt_queries;
    const auto &dict= experiment_settings::shared_info_obj->cnt_queries;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_sct_counting,nv_sct)(benchmark::State &state) {
    // const auto &dict= cnt_queries;
    const auto &dict= experiment_settings::shared_info_obj->cnt_queries;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            auto res = counting(qr.x_,qr.y_,qr.a_,qr.b_);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

//=============================== Reporting ==============================================/
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_reporting,nv)(benchmark::State &state) {
    // const auto &dict= rpt_queries;
    const auto &dict= experiment_settings::shared_info_obj->rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_lca_reporting,nv_lca)(benchmark::State &state) {
    // const auto &dict= rpt_queries;
    std::vector<std::uint64_t> output_sizes;
    const auto &dict= experiment_settings::shared_info_obj->rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
		output_sizes.resize(dict.size());
		size_t k= 0;
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
			output_sizes[k++]= res.size();
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
		double mean= state.counters["avgOutputSize"]=
				std::accumulate(
						output_sizes.begin(),
						output_sizes.end(),
						0ull)
				/ (0.00 + output_sizes.size());
		std::vector<double> diff(output_sizes.size());
		std::transform(output_sizes.begin(), output_sizes.end(), diff.begin(), [mean]( auto x ) { return x-mean; });
		state.counters["stddevOfOutputSizes"]= std::sqrt(std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0)/output_sizes.size());
		// @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_ptr_reporting,tree_ext_ptr)(benchmark::State &state) {
    // const auto &dict= rpt_queries;
    const auto &dict= experiment_settings::shared_info_obj->rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_ptr_reporting,wt_hp_ptr)(benchmark::State &state) {
    // const auto &dict= rpt_queries;
    const auto &dict= experiment_settings::shared_info_obj->rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_rrr_reporting,tree_ext_sct_rrr)(benchmark::State &state) {
    // const auto &dict= rpt_queries;
    const auto &dict= experiment_settings::shared_info_obj->rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,tree_ext_sct_un_reporting,tree_ext_sct_un)(benchmark::State &state) {
    // const auto &dict= rpt_queries;
    const auto &dict= experiment_settings::shared_info_obj->rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_rrr_reporting,wt_hpd_rrr)(benchmark::State &state) {
    // const auto &dict= rpt_queries;
    const auto &dict= experiment_settings::shared_info_obj->rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

BENCHMARK_TEMPLATE_F(path_queries_benchmark,wt_hpd_un_reporting,wt_hpd_un)(benchmark::State &state) {
    // const auto &dict= rpt_queries;
    const auto &dict= experiment_settings::shared_info_obj->rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        state.counters["num_queries"]= experiment_settings::nq;
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

#if 0
BENCHMARK_TEMPLATE_F(path_queries_benchmark,nv_sct_reporting,nv_sct)(benchmark::State &state) {
    // const auto &dict= rpt_queries;
    const auto &dict= experiment_settings::shared_info_obj->rpt_queries;
    std::vector<std::pair<value_type,size_type>> res;
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        for ( const auto &qr: dict ) {
            res.clear();
            reporting(qr.x_,qr.y_,qr.a_,qr.b_,res);
            benchmark::DoNotOptimize(res.data());  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
#endif

void RunAllGiven( int argc, char **argv ) {
    const rlim_t kStackSize = 20 * 1024ll * 1024ll * 1024ll;
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

    const int FULL_PATH= 0, NUM_QUERIES= 1, K_VAL= 2;

    // ".*nv_lca_counting\|.*tree_ext_ptr_counting"

    experiment_settings::dataset_path= std::string(argv[FULL_PATH]);
    experiment_settings::K           = strtol(argv[K_VAL],nullptr,10);
    experiment_settings::nq          = strtol(argv[NUM_QUERIES],nullptr,10);

    // set up the shared object, such that the common set of queries is generated beforehand
    experiment_settings::shared_info_obj= std::make_unique<experiment_settings::shared_info>();

    std::cerr << "Dataset path: " << experiment_settings::dataset_path << std::endl;
    std::cerr << "K= " << experiment_settings::K << std::endl;
    ::benchmark::Initialize (&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks ();
}
