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
#include <nlohmann/json.hpp>
#include <experiments_container.hpp>

/**
 * @details the path query processor is fixed, but we can now invoke it
 * with arbitrary stream of queries
 * @tparam node_type
 * @tparam size_type
 * @tparam value_type
 * @tparam P
 */
template <
        typename node_type,
        typename size_type,
        typename value_type,
        typename P
        >
class fixed_processor_manager {
private:
    std::unique_ptr<P> processor;
    std::string descr;
public:
    explicit fixed_processor_manager( std::istream &is, const std::string& description= "" ) {
        std::string s; is >> s;
        std::vector<value_type> w(s.size()/2);
        for ( auto &x: w ) is >> x;
        processor= std::make_unique<P>(s,w);
        descr= description;
    }
    nlohmann::json invoke_with( std::istream &queries ) {
        experiments_container<node_type,size_type,value_type> container(processor.get());
        auto obj= container.submit_jobs(queries);
        if ( not descr.empty() )
            obj["dataset"]= descr;
        return std::move(obj);
    }
};
#endif //TREE_PATH_QUERIES_FIXED_PROCESSOR_MANAGER_HPP
