//
// Created by sj on 21/11/19.
//

#ifndef PQ_REQUESTS_MEDIAN_QUERY_HPP
#define PQ_REQUESTS_MEDIAN_QUERY_HPP
#include "selection_query.hpp"

namespace path_queries {
    template<typename node_type, typename size_type, typename value_type>
    struct median_query: public selection_query<node_type,size_type,value_type> {
        median_query( node_type x, node_type y ) ;
        explicit median_query( nlohmann::json obj ) ;
    };
    template<typename node_type, typename size_type, typename value_type>
    median_query<node_type, size_type, value_type>::median_query(node_type x, node_type y):
        selection_query<node_type,size_type,value_type>(x, y, 50) {}

    template<typename node_type, typename size_type, typename value_type>
    median_query<node_type, size_type, value_type>::median_query(nlohmann::json obj)
    :abstract_query<node_type,size_type,value_type>(obj) {}
}

#endif //PQ_REQUESTS_MEDIAN_QUERY_HPP
