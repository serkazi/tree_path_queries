//
// Created by sj on 18/11/19.
//

#include "experimental_run.hpp"
#include <iomanip>

std::ostream &operator<<(std::ostream &os, const experimental_run &e) {
    nlohmann::json obj;
    obj["name"]= e.ds_name, obj["type"]= static_cast<int>(e.type);
    obj["space"]= e.total_space, obj["time"]= e.total_time, obj["hash"]= e.sha512hash;
    return os << std::setprecision(7) << obj;
}

std::istream &operator>>(std::istream &is, experimental_run &e) {
    // we assume that the object is represented as json in the stream
    nlohmann::json obj; is >> obj;
    e= experimental_run(move(obj));
    return is;
}

experimental_run_builder &experimental_run_builder::set_name(const std::string &s) {
    mask|= (1ull << static_cast<int>(FIELDS::NAME));
    e.ds_name= s;
    return *this;
}

experimental_run_builder &experimental_run_builder::set_total_space(long double space) {
    mask|= (1ull << static_cast<int>(FIELDS::SPACE));
    e.total_space= space;
    return *this;
}

experimental_run_builder &experimental_run_builder::set_total_time(long double time) {
    mask|= (1ull << static_cast<int>(FIELDS::TIME));
    e.total_time= time;
    return *this;
}

experimental_run_builder &experimental_run_builder::set_hash(const std::string &h) {
    mask|= (1ull << static_cast<int>(FIELDS::HASH));
    e.sha512hash= h;
    return *this;
}

bool experimental_run_builder::object_is_sane() {
    return mask == 0x1f;
}

experimental_run experimental_run_builder::build() {
    assert( object_is_sane() );
    return std::move(e);
}

experimental_run_builder &experimental_run_builder::set_type( path_queries::QUERY_TYPE type ) {
    mask|= (1ull<< static_cast<int>(FIELDS::TYPE));
    e.type= type;
    return *this;
}

experimental_run::experimental_run() { type= path_queries::QUERY_TYPE::SELECTION; }

experimental_run::experimental_run(nlohmann::json obj) {
    ds_name= obj["name"], total_space= obj["space"], total_time= obj["time"], sha512hash= obj["hash"];
    type= static_cast<path_queries::QUERY_TYPE>(obj["type"]);
}

experimental_run &experimental_run::operator+=( double dt ) {
    total_time+= dt;
    return *this;
}
