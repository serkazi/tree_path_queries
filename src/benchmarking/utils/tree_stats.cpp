#include "gflags/gflags.h"
#include <sys/resource.h>
// #include <benchmark/benchmark.h>
// #include "benchmark/benchmark.h"
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

std::unique_ptr<wt_hpd_un> processor;

DEFINE_string(dataset_path,"","full path to the dataset");

int main( int argc, char **argv ) {
    const rlim_t kStackSize = 20 * 1024ll * 1024ll * 1024ll;
    struct rlimit rl;
    int result;

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

	gflags::ParseCommandLineFlags(&argc,&argv,true);

	std::ifstream is(FLAGS_dataset_path);

	std::string s;
	is >> s;
	std::vector<value_type> w(s.size()/2);
	for ( auto &x: w ) is >> x;
	processor= std::make_unique<wt_hpd_un>(s,w);
	std::cerr << "Processor built" << std::endl;

	std::map<size_type,size_t> counts{};
	double ss= 0;
	for ( node_type x= 0; x < s.size()/2; ++x ) {
		if ( processor->is_leaf(x) ) {
			auto k= processor->get_decomposition_length(x,0);
			++counts[k];
		}
		auto vec= processor->children(x);
		if ( vec.size() == 1 ) {
			ss+= 1;
		}
	}
	for ( auto [key,val]: counts )
		std::cout << key << " " << val << std::endl;
	std::cout << "===================\n" << (ss/processor->size())*100.00 << std::endl;
	return 0;
}

