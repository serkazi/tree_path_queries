//
// Created by sj on 21/11/19.
//
#ifndef PQ_REQUESTS_ABSTRACT_QUERY_HPP
#define PQ_REQUESTS_ABSTRACT_QUERY_HPP

namespace path_queries {
/**
     * @brief each path query has a path-part, hence the refactoring
     * @tparam node_type
     * @tparam size_type
     * @tparam value_type
     */
    template<
            typename node_type,
            typename size_type,
            typename value_type
    >
    struct abstract_query {
        node_type x_, y_;
        abstract_query(node_type x, node_type y);
        virtual ~abstract_query() = default;
        explicit abstract_query( nlohmann::json obj ) ;
    };

    template<typename node_type, typename size_type, typename value_type>
    abstract_query<node_type, size_type, value_type>::abstract_query(node_type x, node_type y) : x_(x), y_(y) {}

    template<typename node_type, typename size_type, typename value_type>
    abstract_query<node_type, size_type, value_type>::abstract_query(nlohmann::json obj) {
        assert( obj.count("x") );
        assert( obj.count("y") );
        x_= obj["x"], y_= obj["y"];
    }
}

#endif //PQ_REQUESTS_ABSTRACT_QUERY_HPP
