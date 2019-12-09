//
// Created by kazi on 2019-11-29.
//
#ifndef TREE_PATH_QUERIES_QUERY_STREAM_BUILDER_HPP
#define TREE_PATH_QUERIES_QUERY_STREAM_BUILDER_HPP
#include <map>
#include <vector>
#include "pq_request.hpp"
#include "random1d_interval_generator.hpp"

/**
 * @details a factory class that creates a vector
 * consisting of "p" median, "q" counting, and "r" reporting
 * requests; the distribution parameters are adjustable;
 * in particular, "a" and "b" are weight bounds,
 * and "n" is the number of nodes
 * Furthermore, "K" is the scaling parameter for the intervals
 * of weights
 * @tparam node_type
 * @tparam size_type
 * @tparam value_type
 */
template<
        typename node_type= pq_types::node_type,
        typename size_type= pq_types::size_type,
        typename value_type= pq_types::value_type>
class query_stream_builder {
private:
    size_type K_{1}; // for counting and reporting, the parameter
    value_type a_{0},b_{0};
    node_type n_{0};
    std::map<path_queries::QUERY_TYPE,uint64_t> how_many_of;

    std::vector<path_queries::pq_request<node_type,size_type,value_type>> requests;

    std::unique_ptr<random1d_interval_generator<value_type>> weights_gen= nullptr;
    std::unique_ptr<random1d_interval_generator<node_type>> nodes_gen= nullptr;

    std::unique_ptr<std::mt19937> generator= std::make_unique<std::mt19937>();
    std::unique_ptr<std::uniform_int_distribution<size_type>> distribution=
            std::make_unique<std::uniform_int_distribution<size_type>>(1,100);

    using counting_query= path_queries::counting_query<node_type,size_type,value_type>;
    using reporting_query= path_queries::reporting_query<node_type,size_type,value_type>;
    using selection_query= path_queries::selection_query<node_type,size_type,value_type>;
    using median_query= path_queries::median_query<node_type,size_type,value_type>;

public:

    void clear() { requests.clear(); }

    query_stream_builder() {
        K_= 1, requests.clear();
        how_many_of[path_queries::QUERY_TYPE::COUNTING]= 0;
        how_many_of[path_queries::QUERY_TYPE::REPORTING]= 0;
        how_many_of[path_queries::QUERY_TYPE::MEDIAN]= 0;
        how_many_of[path_queries::QUERY_TYPE::SELECTION]= 0;
    }
    query_stream_builder &set_node_range( size_type n ) {
        this->n_= n;
        return *this;
    }
    query_stream_builder &set_weight_range( value_type a, value_type b ) {
        this->a_= a, this->b_= b;
        return *this;
    }
    query_stream_builder &set( path_queries::QUERY_TYPE type, uint64_t cnt, bool overwrite= true ) {
        if ( overwrite )
            how_many_of[type]= cnt;
        else {
            if ( how_many_of.count(type) )
                how_many_of[type]+= cnt;
            else how_many_of[type]= cnt;
        }
        return *this;
    }

    query_stream_builder &set_scaling_param( size_type K ) {
        this->K_= K;
        return *this;
    }

    query_stream_builder &build() {

        nodes_gen= std::make_unique<random1d_interval_generator<node_type>>(0,n_-1);
        weights_gen= std::make_unique<random1d_interval_generator<value_type>>(a_,b_);

        auto vec= (*nodes_gen)(how_many_of[path_queries::QUERY_TYPE::MEDIAN]);
        std::transform(
                std::make_move_iterator(vec.begin()),
                std::make_move_iterator(vec.end()),
                std::back_inserter(requests),
                [&]( auto pr ) {
                    return median_query(pr.first,pr.second);
                });

        vec= (*nodes_gen)(how_many_of[path_queries::QUERY_TYPE::COUNTING]);
        auto weg= (*weights_gen)(how_many_of[path_queries::QUERY_TYPE::COUNTING],K_);
        for ( auto i= 0; i < vec.size(); ++i ) {
            requests.emplace_back(counting_query(vec[i].first,vec[i].second,
                    weg[i].first,weg[i].second));
        }

        vec= (*nodes_gen)(how_many_of[path_queries::QUERY_TYPE::REPORTING]);
        weg= (*weights_gen)(how_many_of[path_queries::QUERY_TYPE::REPORTING],K_);
        for ( auto i= 0; i < vec.size(); ++i ) {
            requests.emplace_back(reporting_query(vec[i].first,vec[i].second,
                                                               weg[i].first,weg[i].second));
        }

        vec= (*nodes_gen)(how_many_of[path_queries::QUERY_TYPE::SELECTION]);
        std::vector<size_type> sel(how_many_of[path_queries::QUERY_TYPE::SELECTION]);
        std::transform(sel.begin(),sel.end(),sel.begin(),[&]( auto x ) {
            return (*distribution)(*generator);
        });
        for ( auto i= 0; i < vec.size(); ++i )
            requests.emplace_back(selection_query(vec[i].first,vec[i].second,sel[i]));

        std::random_shuffle(requests.begin(),requests.end());

        return *this;
    }

    auto begin() const {
        return requests.cbegin();
    }

    auto end() const {
        return requests.cend();
    }

};
#endif //TREE_PATH_QUERIES_QUERY_STREAM_BUILDER_HPP
