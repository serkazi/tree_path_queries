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
// TODO: add the possibility of adding abitmask to restrict the experiment to the specified data structures
int main( int argc, char **argv ) {
    std::ifstream is(root+paths[3]);
    auto fmd= std::make_unique<fixed_dataset_manager<pq_types::node_type,pq_types::size_type,pq_types::value_type>>
    (is,511,argc>1?std::string(argv[1]):paths[3]);
    const int MLN= 1'000'000;
    nlohmann::json configs[]= {
                                {{"counting",MLN},{"reporting",MLN},{"median",MLN},{"selection",0},{"K",1}},
                                {{"counting",MLN},{"reporting",MLN},{"median",0},  {"selection",0},{"K",10}},
                                {{"counting",MLN},{"reporting",MLN},{"median",0},  {"selection",0},{"K",100}},
                              };
    for ( auto config: configs ) {
        auto res = fmd->run_config(config);
        std::cout << std::fixed << std::setprecision(2) << res.dump(2) << std::endl;
    }
    return 0;
}
