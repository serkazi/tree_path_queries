//
// Created by kazi on 2019-12-01.
//

#ifndef TREE_PATH_QUERIES_FIXED_DATASET_MANAGER_HPP
#define TREE_PATH_QUERIES_FIXED_DATASET_MANAGER_HPP
#include "fixed_processor_manager.hpp"
#include "query_stream_builder.hpp"
#include <algorithm>
#include "sdsl/util.hpp"

template<typename node_type,typename size_type,typename value_type>
class fixed_dataset_manager {

private:

    std::unique_ptr<fixed_processor_manager<node_type,size_type,value_type,P>> processor_manager;
    std::string description;
    value_type a,b;
    size_type n;
    std::string topology;
    std::vector<value_type> w;

public:

    explicit fixed_dataset_manager( std::istream &is, const std::string &descr= "N/A" ) {
        std::string s; is >> topology;
        w.resize(topology.size()/2);
        for ( auto &x: w )
            is >> x;
        n= s.size()/2, a= *(std::min_element(begin(w),end(w))), b= *(std::max_element(begin(w),end(w)));
        description= descr;
    }

    /**
     * @details the same set of queries should be run on all the processors
     * @param configs, a JSON object saying how many of each sort of queries to perform
     * for counting and reporting, it also contains the "K" parameter
     * @return
     */
    nlohmann::json run_config( nlohmann::json configs ) {
        query_stream_builder<node_type,size_type,value_type> builder;
        builder.set_node_range(n).set(path_queries::QUERY_TYPE::MEDIAN,configs["median"]);
        builder.set_weight_range(a,b).set(path_queries::QUERY_TYPE::COUNTING,configs["counting"]);
        builder.set(path_queries::QUERY_TYPE::REPORTING,configs["reporting"]);
        builder.set(path_queries::QUERY_TYPE::SELECTION,configs["selection"]);
        builder.set_scaling_param(configs["K"]);
        /**
         * create some temporary file, feed the queries into it
         * TODO: look how Gog uses this id() in his own code
         */
         std::string filename= "query_file_"+std::to_string(sdsl::util::id())+
                 "_on_"+std::to_string(sdsl::util::pid())+".json";
         std::ofstream os(filename);
         builder.build();
         for ( const auto &q: builder )
             os << q << '\n';
         os.close();
         nlohmann::json res;
         for ( int i= 0; i < 7; ++i ) {
             if ( not (mask & (1u<<i)) ) continue ;
             std::ifstream is(filename);
             processor_manager= std::make_unique<fixed_processor_manager<node_type,size_type,value_type,P>>(topology,w);
             auto obj= processor_manager->invoke_with(is);
             res["data struc"]= obj; //FIXME
         }
         res["dataset"]= description;
         return res;
    }
};

#endif //TREE_PATH_QUERIES_FIXED_DATASET_MANAGER_HPP
