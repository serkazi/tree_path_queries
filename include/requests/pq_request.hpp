//
// Created by sj on 21/11/19.
//
#ifndef PQ_REQUEST_HPP
#define PQ_REQUEST_HPP

#include <variant>
#include <optional>
#include <nlohmann/json.hpp>
#include "abstract_query.hpp"
#include "counting_query.hpp"
#include "reporting_query.hpp"
#include "selection_query.hpp"
#include "median_query.hpp"
#include "path_query_processor.hpp"

namespace path_queries {

    enum class QUERY_TYPE {
        SELECTION, //0
        MEDIAN, //1
        COUNTING, //2
        REPORTING //3
    };

    /**
     * @brief acts as a wrapper for a query, uses @code std::variant<> @endcode
     * @tparam node_type
     * @tparam size_type
     * @tparam value_type
     */
    template<typename node_type, typename size_type, typename value_type>
    using pq_request= std::variant<
            counting_query<node_type,size_type,value_type>,
            reporting_query<node_type,size_type,value_type>,
            selection_query<node_type,size_type,value_type>,
            median_query<node_type,size_type,value_type>> ;

    /**
     * @brief converts a request object to JSON format, to be more human-readable
     * @details used inside @code std::visit @endcode
     * @tparam node_type
     * @tparam size_type
     * @tparam value_type
     */
    template<typename node_type, typename size_type, typename value_type>
    class converter_visitor {
    private:
        mutable nlohmann::json obj;
    public:
        void operator () ( const counting_query<node_type,size_type,value_type> &q ) const {
            obj["type"]= "counting", obj["x"]= q.x_, obj["y"]= q.y_, obj["a"]= q.a_, obj["b"]= q.b_;
        }
        void operator () ( const reporting_query<node_type,size_type,value_type> &q ) const {
            obj["type"]= "reporting", obj["x"]= q.x_, obj["y"]= q.y_, obj["a"]= q.a_, obj["b"]= q.b_;
        }
        void operator () ( const selection_query<node_type,size_type,value_type> &q ) const {
            obj["type"]= "selection", obj["x"]= q.x_, obj["y"]= q.y_, obj["quantile"]= q.quantile_ ;
        }
        void operator () ( const median_query<node_type,size_type,value_type> &q ) const {
            obj["type"]= "median", obj["x"]= q.x_, obj["y"]= q.y_;
        }
        auto value() const {
            return std::move(obj);
        }
    };

    /**
     * @brief wrapper around the call to @code std::visit @endcode
     * @tparam node_type
     * @tparam size_type
     * @tparam value_type
     * @param r request
     * @return json representation of the request
     */
    template<typename node_type, typename size_type, typename value_type>
    nlohmann::json to_json( const pq_request<node_type,size_type,value_type> &r ) {
        auto visitor= converter_visitor<node_type,size_type,value_type>();
        std::visit(visitor,r);
        return visitor.value();
    }

    template<typename node_type, typename size_type, typename value_type>
    pq_request<node_type,size_type,value_type> from_json( nlohmann::json obj ) {
        if ( obj.count("counting") )
            return counting_query<node_type,size_type,value_type>(obj);
        if ( obj.count("reporting") )
            return reporting_query<node_type,size_type,value_type>(obj);
        if ( obj.count("median") )
            return median_query<node_type,size_type,value_type>(obj);
        if ( obj.count("selection") )
            return selection_query<node_type,size_type,value_type>(obj);
        assert( false );
    }

    /**
     * @brief outputs a query object in JSON format
     * @tparam node_type
     * @tparam size_type
     * @tparam value_type
     * @param os
     * @param r
     * @return the @code std::ostream & @endcode object
     */
    template<typename node_type, typename size_type, typename value_type>
    std::ostream &operator << ( std::ostream &os, const pq_request<node_type,size_type,value_type> &r ) {
        return os << to_json(r);
    }

    /**
     * @brief we assume that a query is given in JSON format
     * @tparam node_type
     * @tparam size_type
     * @tparam value_type
     * @param is
     * @param r
     * @return the @code std::istream & @endcode object
     */
    template<typename node_type, typename size_type, typename value_type>
    std::istream &operator >> ( std::istream &is, pq_request<node_type,size_type,value_type> &r ) {
        nlohmann::json obj;
        is >> obj, r= path_queries::from_json<node_type,size_type,value_type>(obj);
        return is;
    }

    template<typename node_type, typename size_type, typename value_type>
    using request_stream= std::vector<pq_request<node_type,size_type,value_type>>;

}

#endif //PQ_REQUEST_HPP
