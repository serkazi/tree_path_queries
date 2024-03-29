//
// Created by Owner on 11/22/2019.
//
#ifndef PROJECT_EXPERIMENTS_CONTAINER_HPP
#define PROJECT_EXPERIMENTS_CONTAINER_HPP
#include <map>
#include "pq_types.hpp"
#include "experimental_run.hpp"
#include "pq_request.hpp"
#include "counting_query.hpp"
#include "reporting_query.hpp"
#include "selection_query.hpp"
#include "median_query.hpp"
#include "path_query_processor.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <chrono>
#include <fixed_dataset_manager.hpp>
#include "sha512.hh"

// we expose this since it is useful in general
/**
 * a simple struct that records the time of its creation,
 * and writes back the time of its destruction
 */
template<typename time_units= std::chrono::microseconds>
struct duration_timer{
    double &acc;
    long double total_elapsed_time= 0.00;
    std::chrono::time_point<std::chrono::high_resolution_clock,std::chrono::nanoseconds> start;
    explicit duration_timer( double &a ) ; //initializing "start" in the constructor
    ~duration_timer() ; // populating "acc" in the destructor
};
template<typename time_units>
duration_timer<time_units>::duration_timer(double &a) : acc(a) {
    start= std::chrono::high_resolution_clock::now();
}
template<typename time_units>
duration_timer<time_units>::~duration_timer() {
    auto finish = std::chrono::high_resolution_clock::now();
    acc= std::chrono::duration_cast<time_units>(finish-start).count();
};

template<
    typename node_type= pq_types::node_type,
    typename size_type= pq_types::size_type,
    typename value_type= pq_types::value_type
       >
class experiments_container {
private:

    using counting_query= path_queries::counting_query<node_type,size_type,value_type>;
    using reporting_query= path_queries::reporting_query<node_type,size_type,value_type>;
    using selection_query= path_queries::selection_query<node_type,size_type,value_type>;
    using median_query= path_queries::median_query<node_type ,size_type ,value_type >;
    using pq_request= path_queries::pq_request<node_type ,size_type ,value_type >;
    using request_stream= path_queries::request_stream<node_type,size_type,value_type>;

    //std::unique_ptr<path_query_processor<node_type,size_type,value_type>> p;
    const path_query_processor<node_type,size_type,value_type> *p;
    /**
     * @see https://stackoverflow.com/questions/22387586/measuring-execution-time-of-a-function-in-c
     * @see https://github.com/google/benchmark
     */

    struct visitor_ {
        const experiments_container &enclosing;
        std::map<path_queries::QUERY_TYPE,double> aggregates;
        std::map<path_queries::QUERY_TYPE,int64_t> counts;
        std::map<path_queries::QUERY_TYPE,std::string> to_be_hashed;
        void operator () ( const counting_query &q ) ;
        void operator () ( const reporting_query &q ) ;
        void operator () ( const selection_query &q ) ;
        void operator () ( const median_query &q ) ;
        explicit visitor_( const experiments_container &e ) ;
    };
    std::unique_ptr<visitor_> vis;
    //C++17 is needed for std::variant<>
    request_stream requests;
    request_stream read_requests( std::istream &is ) ;

    [[nodiscard]] double get_avg( path_queries::QUERY_TYPE type ) const {
        if ( not vis->counts.count(type) )
            return 0;
        auto total_count_for_type= vis->counts.find(type)->second;
        return vis->aggregates.find(type)->second/total_count_for_type;
    }
    [[nodiscard]] std::string get_sha512( path_queries::QUERY_TYPE type ) const {
        if ( not vis->to_be_hashed.count(type) )
            return "";
        return sw::sha512::calculate(vis->to_be_hashed.find(type)->second);
    }
public:

    /**
     * @brief: constructor; the container is associated with a path query processor
     * @details it should not even know about the underlying weighted tree for which the processor is built
     * @param p
     */
    explicit experiments_container(
            const path_query_processor<node_type,size_type,value_type> *p ) ;
    nlohmann::json submit_jobs( std::istream &is ) ;
};

template<typename node_type, typename size_type, typename value_type>
void experiments_container<node_type,size_type,value_type>
::visitor_::operator()(const experiments_container::counting_query &q) {
    double acc= 0.00;
    size_type cnt;
    {
        // we create a duration_timer here, and it captures "acc" by reference
        duration_timer<std::chrono::microseconds> timer(acc);
        cnt= enclosing.p->count(q.x_,q.y_,q.a_,q.b_);
        // at this point, the duration_timer is destroyed, and "acc" field is populated
        // with the time of "count"'s execution
    }
    auto &v= to_be_hashed[path_queries::QUERY_TYPE::COUNTING];
    v+= " ", v+= std::to_string(cnt);
    aggregates[path_queries::QUERY_TYPE::COUNTING]+= acc;
    ++counts[path_queries::QUERY_TYPE::COUNTING];
}

template<typename node_type, typename size_type, typename value_type>
void experiments_container<node_type,size_type,value_type>
::visitor_::operator()(const experiments_container::reporting_query &q) {
    std::vector<std::pair<value_type,size_type>> res;
    double acc= 0.00;
    {
        // we create a duration_timer here, and it captures "acc" by reference
        duration_timer<std::chrono::microseconds> timer(acc);
        enclosing.p->report(q.x_, q.y_, q.a_, q.b_,res);
        // at this point, the duration_timer is destroyed, and "acc" field is populated
        // with the time of "count"'s execution
    }
    auto &v= to_be_hashed[path_queries::QUERY_TYPE::REPORTING];
    // sort by the second, since the first stands for identifiers, which wt_hpd scrambles anyway
    std::sort(res.begin(),res.end(),[]( const auto &a, const auto &b ) {
        return a.second < b.second;
    });
    v+= std::accumulate(std::make_move_iterator(res.begin()),std::make_move_iterator(res.end()),
            std::string(""),[&]( std::string acc, auto pr ) {
        acc+= " ", acc+= std::to_string(pr.second);
        return acc;
    });
    aggregates[path_queries::QUERY_TYPE::REPORTING]+= acc;
    ++counts[path_queries::QUERY_TYPE::REPORTING];
}

template<typename node_type, typename size_type, typename value_type>
void experiments_container<node_type,size_type,value_type>
::visitor_::operator()(const experiments_container::selection_query &q) {
    double acc= 0.00;
    value_type res;
    {
        // we create a duration_timer here, and it captures "acc" by reference
        duration_timer<std::chrono::microseconds> timer(acc);
        res= enclosing.p->selection(q.x_, q.y_, q.quantile);
        // at this point, the duration_timer is destroyed, and "acc" field is populated
        // with the time of "count"'s execution
    }
    auto &v= to_be_hashed[path_queries::QUERY_TYPE::SELECTION];
    v+= " ", v+= std::to_string(res);
    aggregates[path_queries::QUERY_TYPE::SELECTION]+= acc;
    ++counts[path_queries::QUERY_TYPE::SELECTION];
}

template<typename node_type, typename size_type, typename value_type>
void experiments_container<node_type,size_type,value_type>
::visitor_::operator()(const experiments_container::median_query &q) {
    double acc= 0.00;
    value_type res;
    {
        // we create a duration_timer here, and it captures "acc" by reference
        duration_timer<std::chrono::microseconds> timer(acc);
        res= enclosing.p->query(q.x_,q.y_);
        // at this point, the duration_timer is destroyed, and "acc" field is populated
        // with the time of "count"'s execution
    }
    auto &v= to_be_hashed[path_queries::QUERY_TYPE::MEDIAN];
    v+= " ", v+= std::to_string(res);
    aggregates[path_queries::QUERY_TYPE::MEDIAN]+= acc;
    ++counts[path_queries::QUERY_TYPE::MEDIAN];
}

template<typename node_type, typename size_type, typename value_type>
experiments_container<node_type,size_type,value_type>
::visitor_::visitor_(const experiments_container &e) : enclosing(e) {}



template<typename node_type, typename size_type, typename value_type>
experiments_container<node_type,size_type,value_type>
::experiments_container( const path_query_processor<node_type,size_type,value_type> *p ) {
    vis= std::make_unique<visitor_>(*this);
    this->p= p;
}

template<typename node_type, typename size_type, typename value_type>
path_queries::request_stream<node_type,size_type,value_type>
experiments_container<node_type,size_type,value_type>
::read_requests( std::istream &is ) {
    path_queries::request_stream<node_type,size_type,value_type> vec;
    for ( path_queries::pq_request<node_type,size_type,value_type> r; is >> r; vec.push_back(std::move(r)) ) ;
    return std::move(vec);
}

/**
 * @details returns a JSON object, so that a caller can augment it with other info,
 * such as e.g. the dataset for which the processor is built etc.
 * @tparam node_type
 * @tparam size_type
 * @tparam value_type
 * @param is
 * @return
 */
template<typename node_type, typename size_type, typename value_type>
nlohmann::json experiments_container<node_type,size_type,value_type>
::submit_jobs(std::istream &is) {
    // read the requests
    requests= read_requests(is);
    // answer the queries
    std::for_each( begin(requests),end(requests),[&]( const pq_request &r ) {std::visit(*vis,r);} );
    // JSON-ify the summary
    nlohmann::json obj;
    for ( int t= 0; t < 4; ++t ) {
        auto tp= static_cast<path_queries::QUERY_TYPE>(t);
        if ( vis->aggregates.count(tp) ) {
            nlohmann::json d= tp; // converting the type to JSON --
            // we can do it, as we've created a serialization rule for it
            obj[d.get<std::string>()] = {{"avg",get_avg(tp)},{"sha512", get_sha512(tp)}};
        }
    }
    return std::move(obj);
}
#endif //PROJECT_EXPERIMENTS_CONTAINER_HPP
