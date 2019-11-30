//
// Created by sj on 18/11/19.
//
#ifndef SPQ_EXPERIMENTAL_RUN_HPP
#define SPQ_EXPERIMENTAL_RUN_HPP
#include <string>
#include <cstring>
#include <istream>
#include <ostream>
#include <nlohmann/json.hpp>
#include "pq_request.hpp"

struct experimental_run {

    std::string ds_name; //the name of the data structure
    path_queries::QUERY_TYPE type;
    long double total_space{};   // total space in bytes of the data structure
    long double total_time{};    // total time for running all the experiments
    std::string sha512hash;    //SHA-512 hash as proxy for correctness (between outputs)
    experimental_run() ;
    explicit experimental_run( nlohmann::json obj ) ;
    experimental_run &operator += ( double dt ) ;
};

struct experimental_run_builder {
    unsigned int mask= 0;
    enum class FIELDS {
        NAME, TYPE, SPACE, TIME, HASH
    };
    experimental_run e;
    experimental_run_builder &set_name( const std::string &s ) ;
    experimental_run_builder &set_total_space( long double space ) ;
    experimental_run_builder &set_total_time( long double time ) ;
    experimental_run_builder &set_hash( const std::string &h ) ;
    experimental_run_builder &set_type( path_queries::QUERY_TYPE type ) ;
    bool object_is_sane() ;
    experimental_run build() ;
};

/**
 * Serializes/Deserializes into/from JSON
 * @param os `ostream`
 * @param e, `experimental_run` object
 * @return
 */
std::ostream &operator << ( std::ostream &os, const experimental_run &e ) ;
/**
 * Serializes/Deserializes into/from JSON
 * @param is , `istream`
 * @param e , `experimental_run` object
 * @return
 */
std::istream &operator >> ( std::istream &is, experimental_run &e ) ;

#endif //SPQ_EXPERIMENTAL_RUN_HPP
