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
        void to_json(nlohmann::json& j, const median_query& p) {
            j = nlohmann::json{{"x", p.x_}, {"y",p.y_}};
        }
        void from_json(const nlohmann::json& j, median_query& p) {
            j.at("x").get_to(p.x_);
            j.at("y").get_to(p.y_);
        }
    };
    template<typename node_type, typename size_type, typename value_type>
    median_query<node_type, size_type, value_type>::median_query(node_type x, node_type y):
        selection_query<node_type,size_type,value_type>(x, y, 50) {}

    template<typename node_type, typename size_type, typename value_type>
    median_query<node_type, size_type, value_type>::median_query(nlohmann::json obj)
    :selection_query<node_type,size_type,value_type>(obj) {}
}

#endif //PQ_REQUESTS_MEDIAN_QUERY_HPP
