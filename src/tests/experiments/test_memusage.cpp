#include "experiments_container.hpp"
#include "fixed_dataset_manager.hpp"

#define run_some_random_queries {for( auto it= 0; it < ITERATIONS; ++it ) { arr[it]= processor->query(distribution(engine),distribution(engine));}};

int main( int argc, char **argv ) {

    using node_type= pq_types::node_type;
    using size_type= pq_types::size_type;
    using value_type= pq_types::value_type;

    assert( argc >= 2 );
    std::cerr << "Reading from file " << argv[1] << std::endl;

    // we give the file name as command-line argument
    std::ifstream is(argv[1]);
    std::string topology;
    is >> topology;
    std::vector<value_type> w(topology.size()/2);
    for ( auto &x: w ) is >> x;
    auto n= topology.size()/2;

    std::default_random_engine engine;
    std::uniform_int_distribution<node_type> distribution(0,n-1);

    is.close();
    const int ITERATIONS= 0x400;
    std::array<value_type,ITERATIONS> arr{};
    // here we need memory usage as base level, as that needed for topology and weights

    // at the following points, we need to measure memory usage
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint16_t>(experiments::IMPLS::NV));
        run_some_random_queries;
        processor.reset();
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint16_t>(experiments::IMPLS::NV_LCA));
        run_some_random_queries;
        processor.reset();
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint16_t>(experiments::IMPLS::NV_SUCC));
        run_some_random_queries;
        processor.reset();
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint16_t>(experiments::IMPLS::HYBRID));
        run_some_random_queries;
        processor.reset();
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint16_t>(experiments::IMPLS::TREE_EXT_PTR));
        run_some_random_queries;
        processor.reset();
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint16_t>(experiments::IMPLS::WT_HPD_UN));
        run_some_random_queries;
        processor.reset();
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint16_t>(experiments::IMPLS::WT_HPD_RRR));
        run_some_random_queries;
        processor.reset();
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint16_t>(experiments::IMPLS::TREE_EXT_SCT_UN));
        run_some_random_queries;
        processor.reset();
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint16_t>(experiments::IMPLS::TREE_EXT_SCT_RRR));
        run_some_random_queries;
        processor.reset();
        processor= nullptr;
    }
    return 0;
}
