//
// Created by kazi on 2019-12-01.
//
#ifndef TREE_PATH_QUERIES_FIXED_PROCESSOR_MANAGER_HPP
#define TREE_PATH_QUERIES_FIXED_PROCESSOR_MANAGER_HPP
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <istream>
#include "experiments_container.hpp"
#include "nlohmann/json.hpp"

/**
 * @details the path query processor is fixed, but we can now invoke it
 * with arbitrary stream of queries
 * @tparam node_type
 * @tparam size_type
 * @tparam value_type
 * @tparam P
 */
template <typename node_type,typename size_type,typename value_type>
class fixed_processor_manager {
private:
    using processor_type = path_query_processor<node_type,size_type,value_type>;
    std::unique_ptr<processor_type> processor;
public:
    explicit fixed_processor_manager( std::unique_ptr<processor_type> proc ) {
        processor= std::move(proc);
    }
    nlohmann::json invoke_with( std::istream &queries ) {
        experiments_container<node_type,size_type,value_type> container(processor.get());
        return container.submit_jobs(queries);
    }
};
#endif //TREE_PATH_QUERIES_FIXED_PROCESSOR_MANAGER_HPP
