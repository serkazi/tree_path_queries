/**
 *
 */
#include <sys/resource.h>
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <random>
#include <functional>
#include <algorithm>
#include "pq_types.hpp"

#include "benchmark/benchmark.h"
#include "sdsl/bp_support.hpp"

using node_type= pq_types::node_type;
using size_type= pq_types::size_type;
using value_type= pq_types::value_type;

// with default params
using bp_g= sdsl::bp_support_g<>;
using bp_gg= sdsl::bp_support_gg<>;
using bp_sada= sdsl::bp_support_sada<>;

const std::string root= "/users/grad/kazi/CLionProjects/tree_path_queries/data/datasets/";
const std::string paths[]= {
        std::string("degenerate_tree_equal_weights.txt"), //0
        std::string("log_weights.txt"), //1
        std::string("sqrt_weights.txt"), //2
        //std::string("linear_small_weights.txt"), //3
        std::string("rnd.100mln.sqrt.puu"), //3
        std::string("linear_weights.txt"), //4
        std::string("us.rd.d.dfs.dimacs.puu") //5
};

// having the structure "shared_info", we make sure our processors run the exact same query-set
// and save some time into the bargain
namespace experiment_settings {
    std::string dataset_path;
    int K, nq;

    struct shared_info {
        std::default_random_engine engine{};
        std::unique_ptr<std::uniform_int_distribution<node_type>> distribution, distr;
		std::vector<size_type> opening_positions, closing_positions;

		std::vector<size_type> data_for_find_open;
		std::vector<size_type> data_for_find_close;
		std::vector<size_type> data_for_enclose;
		std::vector<std::pair<size_type,size_type>> data_for_double_enclose;
		std::vector<size_type> data_for_rank;
		std::vector<size_type> data_for_select;

        shared_info() {
            size_type n;
            std::cerr << "Reading dataset: " << dataset_path << std::endl;
            std::ifstream is(experiment_settings::dataset_path);
            std::string topology;
            is >> topology; n= topology.size()/2;
			opening_positions.clear(), closing_positions.clear();
			for ( size_t i= 0; i < topology.size(); ++i )
				if ( topology[i] == '(' )
					opening_positions.push_back(i);
				else {
					assert( topology[i] == ')' );
					closing_positions.push_back(i);
				}
			assert( opening_positions.size() == closing_positions.size() );
			// since there are exactly "n" opening and closing ones, we can use one distribution for both
            distribution= std::make_unique<std::uniform_int_distribution<node_type>>(0,n-1);
            distr= std::make_unique<std::uniform_int_distribution<node_type>>(0,2*n-1);

			data_for_find_open.resize(1'000'000);
			std::generate(data_for_find_open.begin(),data_for_find_open.end(),[&](){return opening_positions[(*distribution)(engine)];});

			data_for_find_close.resize(1'000'000);
			std::generate(data_for_find_close.begin(),data_for_find_close.end(),[&](){return (*distr)(engine);});

			data_for_enclose= data_for_find_open;

			data_for_double_enclose.resize(1'000'000);
			std::generate(data_for_double_enclose.begin(),data_for_double_enclose.end(),[&](){
				auto x= opening_positions[(*distribution)(engine)], y= opening_positions[(*distribution)(engine)];
				while ( x == y ) 
					x= opening_positions[(*distribution)(engine)], y= opening_positions[(*distribution)(engine)];
				return std::make_pair(std::min(x,y),std::max(x,y)); 
			});

			data_for_rank.resize(1'000'000);
			std::generate(data_for_rank.begin(),data_for_rank.end(),[&](){return (*distr)(engine);});

			data_for_select.resize(1'000'000);
			std::generate(data_for_select.begin(),data_for_select.end(),[&](){return 1+(*distribution)(engine);});
        }

        virtual ~shared_info() = default;

    };

    std::unique_ptr<shared_info> shared_info_obj= nullptr;

}

 /*  BP_SADA:
  *  - find_open
 *   - find_close
 *   - enclose
 *   - double_enclose
 *   - rank
 *   - select
 *   - excess
 *   - rr_enclose
 */

 /* BP_G:
  * - find_open
 *   - find_close
 *   - enclose
 *   - double_enclose
 *   - rank
 *   - select
 *   - excess
 *   - rr_enclose
 */

 /* BP_GG:
  *    - find_open
 *   - find_close
 *   - enclose
 *   - double_enclose
 *   - rank
 *   - select
 *   - excess
 *   - rr_enclose
 */

/**
 * @details The idea is to pre-generate a set of queries, and then
 * run the benchmark as if answering the entire set of queries
 * had been our goal; this way, each of the data structures
 * get a chance to answer the same set of queries, hence
 * the experiment is more on the "no-questions-asked" side
 * @tparam T
 */
template<typename T>
class bp_benchmark: public benchmark::Fixture {
protected:
    size_type n;
    std::unique_ptr<T> processor;
	std::unique_ptr<sdsl::bit_vector> bv;
public:
    void SetUp( const ::benchmark::State& state ) {
        std::ifstream is(experiment_settings::dataset_path);
        std::string topology;
        is >> topology; 
		bv= std::make_unique<sdsl::bit_vector>(topology.size());
		for ( size_t i= 0; i < topology.size(); ++i )
			(*bv)[i]= topology[i]=='('?1:0;
        processor= std::make_unique<T>(bv.get());

    }
    void TearDown(const ::benchmark::State& state) {
		processor.reset(), processor= nullptr;
		bv.reset(), bv= nullptr;
    }

	//! Calculate the matching opening parenthesis to the closing parenthesis at position i
	/*! \param i Index of a closing parenthesis.
  	 * \return * i, if the parenthesis at index i is closing,
  	 *         * the position j of the matching opening parenthesis, if a matching parenthesis exists,
     *         * size() if no matching closing parenthesis exists.
     */
	size_type find_open( size_type i ) {
		return processor->find_open(i);
	}

	/*! Calculate the index of the matching closing parenthesis to the parenthesis at index i.
         * \param i Index of an parenthesis. 0 <= i < size().
         * \return * i, if the parenthesis at index i is closing,
         *         * the position j of the matching closing parenthesis, if a matching parenthesis exists,
         *         * size() if no matching closing parenthesis exists.
         */
     size_type find_close(size_type i) {
		 return processor->find_close(i);
	 }

     //! Calculate the index of the opening parenthesis corresponding to the closest matching parenthesis pair enclosing i.
     /*! \param i Index of an opening parenthesis.
      *  \return The index of the opening parenthesis corresponding to the closest matching parenthesis pair enclosing i,
      *          or size() if no such pair exists.
      */
     size_type enclose(size_type i)const
     {
		return processor->enclose(i);
     }

     //! The double enclose operation
     /*! \param i Index of an opening parenthesis.
      *  \param j Index of an opening parenthesis \f$ i<j \wedge findclose(i) < j \f$.
      *  \return The maximal opening parenthesis, say k, such that \f$ k<j \wedge k>findclose(j) \f$.
      *          If such a k does not exists, double_enclose(i,j) returns size().
      */
     size_type double_enclose(size_type i, size_type j) {
		 return processor->double_enclose(i,j);
	 }

	/*! Returns the number of opening parentheses up to and including index i.
     * \pre{ \f$ 0\leq i < size() \f$ }
     */
     size_type rank( size_type i ) {
		 return processor->rank(i);
     }

	/*! Returns the index of the i-th opening parenthesis.
     * \param i Number of the parenthesis to select.
     * \pre{ \f$1\leq i < rank(size())\f$ }
     * \post{ \f$ 0\leq select(i) < size() \f$ }
     */
     size_type select(size_type i) {
		return processor->select(i);
	 }
};

/*  - find_open
 *  - find_close
 *  - enclose
 *  - double_enclose
 *  - rank
 *  - select
 *  - excess
 *  - rr_enclose
 */
/*==========================find_open()=====================================*/
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_g_find_open,bp_g)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_find_open;
        for ( const auto &qr: dict ) {
            auto res = find_open(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_gg_find_open,bp_gg)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_find_open;
        for ( const auto &qr: dict ) {
            auto res = find_open(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_sada_find_open,bp_sada)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_find_open;
        for ( const auto &qr: dict ) {
            auto res = find_open(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
/*==========================find_close()=====================================*/
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_g_find_close,bp_g)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_find_close;
        for ( const auto &qr: dict ) {
            auto res = find_close(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_gg_close,bp_gg)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_find_close;
        for ( const auto &qr: dict ) {
            auto res = find_close(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_sada_find_close,bp_sada)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_find_close;
        for ( const auto &qr: dict ) {
            auto res = find_close(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
/*==========================enclose()=====================================*/
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_g_enclose,bp_g)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_enclose;
        for ( const auto &qr: dict ) {
            auto res = enclose(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_gg_enclose,bp_gg)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_enclose;
        for ( const auto &qr: dict ) {
            auto res = enclose(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_sada_enclose,bp_sada)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_enclose;
        for ( const auto &qr: dict ) {
            auto res = enclose(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
/*==========================double_enclose()=====================================*/
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_g_double_enclose,bp_g)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_double_enclose;
        for ( const auto &qr: dict ) {
            auto res = double_enclose(qr.first,qr.second);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_gg_double_enclose,bp_gg)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_double_enclose;
        for ( const auto &qr: dict ) {
            auto res = double_enclose(qr.first,qr.second);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_sada_double_enclose,bp_sada)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_double_enclose;
        for ( const auto &qr: dict ) {
            auto res = double_enclose(qr.first,qr.second);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
/*==========================rank()=====================================*/
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_g_rank,bp_g)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_rank;
        for ( const auto &qr: dict ) {
            auto res = rank(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_gg_rank,bp_gg)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_rank;
        for ( const auto &qr: dict ) {
            auto res = rank(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_sada_rank,bp_sada)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_rank;
        for ( const auto &qr: dict ) {
            auto res = rank(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
/*==========================select()=====================================*/
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_g_select,bp_g)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_select;
        for ( const auto &qr: dict ) {
            auto res = select(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_gg_select,bp_gg)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_select;
        for ( const auto &qr: dict ) {
            auto res = select(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}
BENCHMARK_TEMPLATE_F(bp_benchmark,bp_sada_select,bp_sada)( benchmark::State &state ) {
    for ( auto _ : state ) {
        auto start = std::chrono::high_resolution_clock::now();
        // the code that gets measured
        const auto &dict= experiment_settings::shared_info_obj->data_for_select;
        for ( const auto &qr: dict ) {
            auto res = select(qr);
            benchmark::DoNotOptimize(res);  // <-- since we are doing nothing with "res"
            benchmark::ClobberMemory(); // <-- took these lines from the documentation,
        }
        auto end = std::chrono::high_resolution_clock::now();
        state.counters["seconds"]+= std::chrono::duration_cast<std::chrono::seconds>(end-start).count();
        // @see https://github.com/google/benchmark#user-guide
        // end of the code that gets measured
    }
}

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

    const int FULL_PATH= 0;

    // ".*nv_lca_counting\|.*tree_ext_ptr_counting"

    experiment_settings::dataset_path= std::string(argv[FULL_PATH]);
    //experiment_settings::K           = strtol(argv[K_VAL],nullptr,10);
    //experiment_settings::nq          = strtol(argv[NUM_QUERIES],nullptr,10);

    // set up the shared object, such that the common set of queries is generated beforehand
    experiment_settings::shared_info_obj= std::make_unique<experiment_settings::shared_info>();

    std::cerr << "Dataset path: " << experiment_settings::dataset_path << std::endl;
    //std::cerr << "K= " << experiment_settings::K << std::endl;
    ::benchmark::Initialize (&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks ();
}
