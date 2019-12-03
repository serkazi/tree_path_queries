/**
 * small driver program to test the sanity of the experiments manager
 * and possibly fin-tune it
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

int main() {
    std::ifstream is(root+paths[3]);
    auto fmd= std::make_unique<fixed_dataset_manager<pq_types::node_type,pq_types::size_type,pq_types::value_type>>(is,511,paths[3]);
    nlohmann::json config;
    config["counting"]= 3, config["reporting"]= 5, config["median"]= 7, config["selection"]= 11, config["K"]= 2;
    auto res= fmd->run_config(config);
    std::cout << res.dump(2) << std::endl;
    return 0;
}
