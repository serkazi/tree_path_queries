//#include "experiments_container.hpp"
#include "fixed_dataset_manager.hpp"

// Note: since one global randomization mechanism is used, these queries
// may obviously return different results, just because the queries are different --
// every time dice() is thrown, a different number shows up
#define run_some_random_queries {for( auto it= 0; it < ITERATIONS; ++it ) {  \
    arr[it]= processor->query(distribution(engine),distribution(engine));}   \
    std::cerr << std::accumulate(arr.begin(),arr.end(),0) << std::endl; }

using node_type= pq_types::node_type;
using size_type= pq_types::size_type;
using value_type= pq_types::value_type;

std::default_random_engine engine;
std::unique_ptr<std::uniform_int_distribution<node_type>> distribution;
std::unique_ptr<path_query_processor<node_type,size_type,value_type>> processor;
const int ITERATIONS= (1ul << 17);
std::array<value_type,16> arr{};
std::string topology;
std::vector<value_type> w;

void instantiate_exp( uint16_t mask, experiments::IMPLS impl ) {
    double tm= 0;
    if ( mask & static_cast<uint16_t>(impl) ) {
        try {
            {
                duration_timer<std::chrono::minutes> timer(tm);
                processor = experiments::instantiate<node_type, size_type, value_type>(topology, w,
                                                                                       static_cast<uint16_t>(impl));
            }
            std::cerr << "Construction time: " << std::fixed << std::setprecision(2) << tm << "m" << std::endl;
            // we are writing the results so that the compiler does not optimize it out
            for (auto it = 0; it < ITERATIONS; ++it) {
                arr[it & 0xf] = processor->query((*distribution)(engine), (*distribution)(engine));
            }
            std::cerr << std::accumulate(arr.begin(), arr.end(), 0ul) << std::endl;
            // delete the object owned by the processor pointer
            processor.reset();
            processor = nullptr;
        } catch ( std::exception &e ) {
            std::cerr << e.what() << std::endl;
            throw e;
        }
    }
}

// receives two command-line arguments: the full path of the input dataset
// and a bitmask indicating which data structures to instantiate
int main( int argc, char **argv ) {

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

    assert( argc >= 3 );
    std::cerr << "Reading from file " << argv[1] << std::endl;

    // we give the file name as command-line argument
    std::ifstream is(argv[1]);
    is >> topology;
    w.resize(topology.size()/2);
    for ( auto &x: w ) is >> x;
    auto n= topology.size()/2;
    is.close();
    uint16_t mask= std::strtol(argv[2],nullptr,10);
    std::cerr << "Read the mask: " << mask << std::endl;
    distribution= std::make_unique<std::uniform_int_distribution<node_type>>(0,n-1);

    instantiate_exp(mask,experiments::IMPLS::NV);
    instantiate_exp(mask,experiments::IMPLS::NV_LCA);
    instantiate_exp(mask,experiments::IMPLS::TREE_EXT_PTR);
    instantiate_exp(mask,experiments::IMPLS::WT_HPD_PTR);
    instantiate_exp(mask,experiments::IMPLS::NV_SUCC);
    instantiate_exp(mask,experiments::IMPLS::TREE_EXT_SCT_UN);
    instantiate_exp(mask,experiments::IMPLS::TREE_EXT_SCT_RRR);
    instantiate_exp(mask,experiments::IMPLS::WT_HPD_UN);
    instantiate_exp(mask,experiments::IMPLS::WT_HPD_RRR);
    return 0;
}
