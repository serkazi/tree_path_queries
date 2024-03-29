//
// Created by sj on 21/11/19.
//

#ifndef PQ_REQUESTS_SELECTION_QUERY_HPP
#define PQ_REQUESTS_SELECTION_QUERY_HPP
#include "abstract_query.hpp"

namespace path_queries {
    template<typename node_type, typename size_type, typename value_type>
    struct selection_query: public abstract_query<node_type,size_type,value_type> {
        size_type quantile;
        selection_query( node_type x, node_type y, size_type qntl ) ;
        explicit selection_query( nlohmann::json obj ) ;
        void to_json(nlohmann::json& j, const selection_query& p) {
            j = nlohmann::json{{"x", p.x_}, {"y",p.y_},{"quantile",p.quantile}};
        }
        void from_json(const nlohmann::json& j, selection_query& p) {
            j.at("x").get_to(p.x_);
            j.at("y").get_to(p.y_);
        }
    };
    template<typename node_type, typename size_type, typename value_type>
    selection_query<node_type, size_type, value_type>
    ::selection_query(node_type x, node_type y, size_type qntl) :
            abstract_query<node_type,size_type,value_type>(x,y) { quantile = qntl; }

    template<typename node_type, typename size_type, typename value_type>
    selection_query<node_type, size_type, value_type>::selection_query(nlohmann::json obj)
    :abstract_query<node_type,size_type,value_type>(obj) {
        if ( obj.count("quantile") ) {
            quantile = obj["quantile"];
        }
        else quantile= 50;
    }
}


#endif //PQ_REQUESTS_SELECTION_QUERY_HPP
