#include "experiments_container.hpp"
#include "fixed_dataset_manager.hpp"

int main( int argc, char **argv ) {

    using node_type= pq_types::node_type;
    using size_type= pq_types::size_type;
    using value_type= pq_types::value_type;

    // we give the file name as command-line argument
    std::ifstream is(argv[1]);
    std::string topology;
    is >> topology;
    std::vector<value_type> w(topology.size()/2);
    for ( auto &x: w ) is >> x;
    is.close();
    // here we need memory usage as base level, as that needed for topology and weights

    // at the following points, we need to measure memory usage
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint8_t>(experiments::IMPLS::NV));
        std::cerr << processor->query(0,0) << ' ';
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint8_t>(experiments::IMPLS::NV_LCA));
        std::cerr << processor->query(0,0) << ' ';
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint8_t>(experiments::IMPLS::NV_SUCC));
        std::cerr << processor->query(0,0) << ' ';
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint8_t>(experiments::IMPLS::HYBRID));
        std::cerr << processor->query(0,0) << ' ';
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint8_t>(experiments::IMPLS::TREE_EXT_PTR));
        std::cerr << processor->query(0,0) << ' ';
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint8_t>(experiments::IMPLS::WT_HPD_UN));
        std::cerr << processor->query(0,0) << ' ';
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint8_t>(experiments::IMPLS::WT_HPD_RRR));
        std::cerr << processor->query(0,0) << ' ';
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint8_t>(experiments::IMPLS::TREE_EXT_SCT_UN));
        std::cerr << processor->query(0,0) << ' ';
        processor= nullptr;
    }
    {
        auto processor= experiments::instantiate<node_type,size_type,value_type>(topology,w, static_cast<uint8_t>(experiments::IMPLS::TREE_EXT_SCT_RRR));
        std::cerr << processor->query(0,0) << ' ';
        processor= nullptr;
    }
    return 0;
}
