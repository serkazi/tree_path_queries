//
// Created by sj on 21/11/19.
//

#ifndef PQ_REQUESTS_COUNTING_QUERY_HPP
#define PQ_REQUESTS_COUNTING_QUERY_HPP
#include "abstract_query.hpp"

namespace path_queries {
    template<typename node_type, typename size_type, typename value_type>
    struct counting_query: public abstract_query<node_type,size_type,value_type> {
        value_type a_,b_;
        counting_query( node_type x, node_type y, value_type a, node_type b ) ;
        // an empty constructor is needed here just to appease the std::variant,
        // which is initialized by its first variant
        counting_query() {};
        ~counting_query() override;
        explicit counting_query( nlohmann::json obj );
    };
    template<typename node_type, typename size_type, typename value_type>
    counting_query<node_type, size_type, value_type>::counting_query(node_type x, node_type y, value_type a,
                                                                     node_type b)
            :
            abstract_query<node_type,size_type,value_type>(x,y)
    { a_= a, b_= b; }

    template<typename node_type, typename size_type, typename value_type>
    counting_query<node_type, size_type, value_type>::counting_query(nlohmann::json obj) : 
    abstract_query<node_type,size_type,value_type>(obj) {
        assert( obj.count("a") );
        assert( obj.count("b") );
        a_= obj["a"], b_= obj["b"];
    }

    template<typename node_type, typename size_type, typename value_type>
    counting_query<node_type, size_type, value_type>::~counting_query() = default;
}

#endif //PQ_REQUESTS_COUNTING_QUERY_HPP
