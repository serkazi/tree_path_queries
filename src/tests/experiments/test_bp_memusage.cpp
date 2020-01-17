#include "malloc_count.h"
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
#include "sdsl/bp_support.hpp"
#include "sdsl/select_support_scan.hpp"
#include "sdsl/rank_support_scan.hpp"

using node_type= pq_types::node_type;
using size_type= pq_types::size_type;
using value_type= pq_types::value_type;

// with default params
using bp_g= sdsl::bp_support_g<>;
using bp_gg= sdsl::bp_support_gg<>;
using bp_sada= sdsl::bp_support_sada<256,32,sdsl::rank_support_scan<>,sdsl::select_support_scan<>>;

// Note: since one global randomization mechanism is used, these queries
// may obviously return different results, just because the queries are different --
// every time dice() is thrown, a different number shows up
#define run_some_random_queries {for( auto it= 0; it < ITERATIONS; ++it ) {  \
    arr[it]= processor->query(distribution(engine),distribution(engine));}   \
    std::cerr << std::accumulate(arr.begin(),arr.end(),0) << std::endl; }

using node_type= pq_types::node_type;
using size_type= pq_types::size_type;
using value_type= pq_types::value_type;

template<typename T>
void instantiate_exp( std::string filename ) {
    size_type n;
    std::unique_ptr<T> processor;
	std::unique_ptr<sdsl::bit_vector> bv;
try {
	std::ifstream is(filename);
	std::string topology;
	is >> topology; n= topology.size();
	bv= std::make_unique<sdsl::bit_vector>(topology.size());
	for ( size_t i= 0; i < topology.size(); ++i )
		(*bv)[i]= topology[i]=='('?1:0;
	processor= std::make_unique<T>(bv.get());
	malloc_count_print_status();
	unsigned long long ans= 0;
	for ( auto i= 0; i < 1'000'000; ++i )
		ans+= processor->rank(rand()%n);
	// delete the object owned by the processor pointer
	std::cout << ans << std::endl;
	processor.reset(), processor= nullptr;
} catch ( std::exception &e ) {
	std::cerr << e.what() << std::endl;
	throw e;
}
}


// receives two command-line arguments: the full path of the input dataset
// and a bitmask indicating which data structures to instantiate
int main( int argc, char **argv ) {

    const rlim_t kStackSize = 20 * 1024ll * 1024ll * 1024ll;   // min stack size = 42 GiB
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

    std::cerr << "Reading from file " << argv[1] << std::endl;

    // we give the file name as command-line argument
    uint16_t mask= std::strtol(argv[2],nullptr,10);
    std::cerr << "Read the mask: " << mask << std::endl;
	if ( mask == 0 )
	    instantiate_exp<bp_g>(std::string(argv[1]));
	else if ( mask == 1 )
		instantiate_exp<bp_gg>(std::string(argv[1]));
	else 
		instantiate_exp<bp_sada>(std::string(argv[1]));
    return 0;
}
