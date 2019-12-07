/**
 * small driver program to test the sanity of the experiments manager
 * and possibly fine-tune it
 */
#include <iostream>
#include <string>
#include "fixed_dataset_manager.hpp"

const std::string paths[]= {
        std::string("degenerate_tree_equal_weights.txt"), //0
        std::string("log_weights.txt"), //1
        std::string("sqrt_weights.txt"), //2
        std::string("linear_small_weights.txt"), //3
        std::string("linear_weights.txt"), //4
        std::string("us.rd.d.dfs.dimacs.puu") //5
};

const std::string root= "/users/grad/kazi/CLionProjects/tree_path_queries/data/";

// receive the name of the dataset in the command-line arguments
int main( int argc, char **argv ) {

    if ( argc < 4 ) {
        std::cerr << "usage: ./<this_binary> <input_file> <bitmask> <num_of_queries>" << std::endl;
        exit(1);
    }

    std::string filename= std::string(argv[1]); //the file should be full path
    std::ifstream is(filename);
    uint16_t mask= strtol(argv[2],nullptr,10);
    auto nq=  std::strtol(argv[3],nullptr,10);

    std::unique_ptr<fixed_dataset_manager<pq_types::node_type,pq_types::size_type,pq_types::value_type>> fmd;
    try {
        fmd = std::make_unique<
                fixed_dataset_manager<pq_types::node_type, pq_types::size_type, pq_types::value_type>
        >(is, mask, filename);
        nlohmann::json configs[] = {
                {{"counting", nq}, {"reporting", nq}, {"median", nq}, {"selection", 0}, {"K", 1}},
                {{"counting", nq}, {"reporting", nq}, {"median", 0},  {"selection", 0}, {"K", 10}},
                {{"counting", nq}, {"reporting", nq}, {"median", 0},  {"selection", 0}, {"K", 100}},
        };
        for ( const auto& config: configs ) {
            auto res = fmd->run_config(config);
            std::cout << std::fixed << std::setprecision(2) << res.dump(2) << std::endl;
        }
    } catch ( std::exception &e ) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
