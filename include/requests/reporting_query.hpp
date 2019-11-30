//
// Created by sj on 21/11/19.
//

#ifndef PQ_REQUESTS_REPORTING_QUERY_HPP
#define PQ_REQUESTS_REPORTING_QUERY_HPP
#include "abstract_query.hpp"

namespace path_queries {
    template<typename node_type, typename size_type, typename value_type>
    struct reporting_query : public abstract_query<node_type, size_type, value_type> {
        value_type a_,b_;
        reporting_query(node_type x, node_type y, value_type a, value_type b);
        explicit reporting_query( nlohmann::json obj ) ;
    };
    template<typename node_type, typename size_type, typename value_type>
    reporting_query<node_type, size_type, value_type>::reporting_query(node_type x, node_type y, value_type a,
                                                                       value_type b) :
            abstract_query<node_type,size_type,value_type>(x,y)
    { a_= a, b_= b; }

    template<typename node_type, typename size_type, typename value_type>
    reporting_query<node_type, size_type, value_type>::reporting_query(nlohmann::json obj)
    :abstract_query<node_type,size_type,value_type>(obj) {
        a_= obj["a"], b_= obj["b"];
    }
}

#endif //PQ_REQUESTS_REPORTING_QUERY_HPP
